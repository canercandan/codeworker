/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2002 Cédric Lemaire

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

To contact the author: codeworker@free.fr
*/

#ifdef WIN32
#pragma warning (disable : 4786)
#pragma warning (disable : 4503)
#endif

#include "ScpStream.h"
#include "UtlException.h"
#include "UtlTrace.h"
#include "UtlTimer.h"

#include "CGRuntime.h"
#include "DtaScript.h"
#include "DtaBNFScript.h"
#include "DtaPatternScript.h"
#include "DtaProject.h"

namespace CodeWorker {

	class DtaCustomTagsHandler {
	public:
		DtaBNFScript* _pReader;
		DtaPatternScript* _pWriter;

		inline DtaCustomTagsHandler(DtaBNFScript* pReader, DtaPatternScript* pWriter) : _pReader(pReader), _pWriter(pWriter) {}
	};


	class DtaGlobalVariableData {
	public:
		DtaScriptVariable* _variableNode;
		EXPRESSION_TYPE _variableType;

		inline DtaGlobalVariableData(DtaScriptVariable* variableNode, EXPRESSION_TYPE variableType) : _variableNode(variableNode), _variableType(variableType) {}
		inline DtaGlobalVariableData(EXPRESSION_TYPE variableType) : _variableNode(NULL), _variableType(variableType) {}
		~DtaGlobalVariableData() { delete _variableNode; }
	};


	DtaProject* DtaProject::_pInstance = NULL;
	std::list<DtaProject*> DtaProject::_stackOfProjects;

	DtaProject::DtaProject() :	DtaScriptVariable(NULL, "project"), _pScript(NULL), _bVerbose(false),
								_pCommonAlienParser(NULL), _pBNFAlienParser(NULL),
								_pTemplateAlienParser(NULL), _pTranslationAlienParser(NULL),
								_sCommentBegin("//"), _sCommentEnd("\n"),
								_pDelayTimer(NULL), _dLastDelay(0.0),
								_pReadonlyHook(NULL), _pWritefileHook(NULL),
								iRemoteDebugPort_(0)
	{
		if (_pInstance != NULL) {
			_stackOfProjects.push_back(_pInstance);
			copyCommon(*_pInstance);
		} else {
#ifdef WIN32
			_iTextMode = DOS_MODE;
#else
			_iTextMode = UNIX_MODE;
#endif
		}
		_pInstance = this;
	}

	DtaProject::~DtaProject() {
		for (std::map<std::string, DtaGlobalVariableData*>::const_iterator i = _mapOfGlobalVariables.begin(); i != _mapOfGlobalVariables.end(); i++) {
			delete i->second;
		}
		for (std::map<std::string, DtaCustomTagsHandler*>::const_iterator j = _mapOfGenerationTagsHandlers.begin(); j != _mapOfGenerationTagsHandlers.end(); ++j) {
			delete j->second;
		}
		if (_stackOfProjects.empty()) {
			_pInstance = NULL;
		} else {
			_pInstance = _stackOfProjects.back();
			_stackOfProjects.pop_back();
			_pInstance->copyCommon(*this);
		}
	}

	void DtaProject::reset() {
		for (std::map<std::string, DtaGlobalVariableData*>::const_iterator i = _mapOfGlobalVariables.begin(); i != _mapOfGlobalVariables.end(); i++) {
			delete i->second;
		}
		_mapOfGlobalVariables = std::map<std::string, DtaGlobalVariableData*>();
		_sCommentBegin = "//";
		_sCommentEnd = "\n";
		_pScript = NULL;
		_pDelayTimer = NULL;
		_dLastDelay = 0.0;
		_pReadonlyHook = NULL;
		_pWritefileHook = NULL;
		_bVerbose = false;
		_sTargetLanguage = "";
		for (std::map<std::string, DtaCustomTagsHandler*>::const_iterator j = _mapOfGenerationTagsHandlers.begin(); j != _mapOfGenerationTagsHandlers.end(); ++j) {
			delete j->second;
		}
		_mapOfGenerationTagsHandlers = std::map<std::string, DtaCustomTagsHandler*>();
		_sCurrentGenerationTagsHandler = "";
		_mapOfCapturedOutputFiles = std::map<std::string, std::set<std::string> >();
		_mapOfCapturedInputFiles = std::map<std::string, std::set<std::string> >();
	}

	DtaProject& DtaProject::createRootInstance() {
		if (!existInstance()) new DtaProject;
		return getInstance();
	}

	void DtaProject::setCodeWorkerHome(const std::string& sPath) {
		_sCodeWorkerHome = CGRuntime::canonizePath(sPath);
		if (!_sCodeWorkerHome.empty()) {
			char c = _sCodeWorkerHome[_sCodeWorkerHome.size() - 1];
			if ((c != '\\') && (c != '/')) _sCodeWorkerHome += "/";
		}
	}

	void DtaProject::setFinalInfo(const std::string& sOutputFilename, int iFlag) {
		sFinalInfoOutputFilename_ = sOutputFilename;
		iFinalInfoFlag_ = iFlag;
	}

	void DtaProject::setRemoteDebug(const std::string& sHost, int iPort) {
		sRemoteDebugHost_ = sHost;
		iRemoteDebugPort_ = iPort;
	}

	std::string DtaProject::getTargetScriptFilename(const std::string& sTargetLanguage, const std::string& sScriptShortFilename) {
		std::string sFile;
		std::string sCodeWorkerHome = getInstance().getCodeWorkerHome();
		if (!sTargetLanguage.empty()) {
			sFile = sCodeWorkerHome + sTargetLanguage + "/user-scripts/" + sScriptShortFilename;
			if (CGRuntime::existFile(sFile)) return sFile;
			sFile = sCodeWorkerHome + sTargetLanguage + "/default-scripts/" + sScriptShortFilename;
			if (CGRuntime::existFile(sFile)) return sFile;
		}
		sFile = sCodeWorkerHome + "cpp/user-scripts/" + sScriptShortFilename;
		if (CGRuntime::existFile(sFile)) return sFile;
		sFile = sCodeWorkerHome + "cpp/default-scripts/" + sScriptShortFilename;
		if (CGRuntime::existFile(sFile)) return sFile;
		return "";
	}

	DtaBNFScript* DtaProject::getCommonAlienParser() const {
		if (_pCommonAlienParser == NULL) {
			if (!_sTargetLanguage.empty()) {
				std::string sScriptFile = getTargetScriptFilename(_sTargetLanguage, _sTargetLanguage + "CommonScript.cwp");
				if (sScriptFile.empty()) {
					throw UtlException("compiling a common script in " + _sTargetLanguage + " requires \"($CODEWORKER_HOME)/" + _sTargetLanguage + "/default-scripts/" + _sTargetLanguage + "CommonScript.cwp\"");
				}
				std::auto_ptr<DtaBNFScript> pScript(new DtaBNFScript);
				pScript->parseFile(sScriptFile.c_str());
				_pCommonAlienParser = pScript.release();
			}
		}
		return _pCommonAlienParser;
	}

	DtaBNFScript* DtaProject::getBNFAlienParser() const {
		if (_pBNFAlienParser == NULL) {
			if (!_sTargetLanguage.empty()) {
				std::string sScriptFile = getTargetScriptFilename(_sTargetLanguage, _sTargetLanguage + "BNFScript.cwp");
				if (sScriptFile.empty()) {
					throw UtlException("compiling a BNF script in " + _sTargetLanguage + " requires \"($CODEWORKER_HOME)/" + _sTargetLanguage + "/default-scripts/" + _sTargetLanguage + "BNFScript.cwp\"");
				}
				std::auto_ptr<DtaBNFScript> pScript(new DtaBNFScript);
				pScript->parseFile(sScriptFile.c_str());
				_pBNFAlienParser = pScript.release();
			}
		}
		return _pBNFAlienParser;
	}

	DtaBNFScript* DtaProject::getTemplateAlienParser() const {
		if (_pTemplateAlienParser == NULL) {
			if (!_sTargetLanguage.empty()) {
				std::string sScriptFile = getTargetScriptFilename(_sTargetLanguage, _sTargetLanguage + "TemplateScript.cwp");
				if (sScriptFile.empty()) {
					throw UtlException("compiling a template script in " + _sTargetLanguage + " requires \"($CODEWORKER_HOME)/" + _sTargetLanguage + "/default-scripts/" + _sTargetLanguage + "TemplateScript.cwp\"");
				}
				std::auto_ptr<DtaBNFScript> pScript(new DtaBNFScript);
				pScript->parseFile(sScriptFile.c_str());
				_pTemplateAlienParser = pScript.release();
			}
		}
		return _pTemplateAlienParser;
	}

	DtaBNFScript* DtaProject::getTranslationAlienParser() const {
		if (_pTranslationAlienParser == NULL) {
			if (!_sTargetLanguage.empty()) {
				std::string sScriptFile = getTargetScriptFilename(_sTargetLanguage, _sTargetLanguage + "TranslationScript.cwp");
				if (sScriptFile.empty()) {
					throw UtlException("compiling a translation script in " + _sTargetLanguage + " requires \"($CODEWORKER_HOME)/" + _sTargetLanguage + "/default-scripts/" + _sTargetLanguage + "TranslationScript.cwp\"");
				}
				std::auto_ptr<DtaBNFScript> pScript(new DtaBNFScript);
				pScript->parseFile(sScriptFile.c_str());
				_pTranslationAlienParser = pScript.release();
			}
		}
		return _pTranslationAlienParser;
	}

	const std::string& DtaProject::getDefineTarget(const std::string& sTarget) const {
		std::map<std::string, std::string>::const_iterator i = _mapOfDefinedTargets.find(sTarget);
		if (i != _mapOfDefinedTargets.end()) return i->second;
		return ((DtaProject*) this)->_mapOfDefinedTargets[""];
	}

	void DtaProject::setDefineTarget(const std::string& sTarget, const std::string& sValue) {
		if (sTarget.empty()) throw UtlException("empty property '=\"" + sValue + "\"' is forbidden");
		_mapOfDefinedTargets[sTarget] = sValue;
	}

	DtaScriptVariable* DtaProject::getGlobalVariable(const std::string& sName) const {
		std::map<std::string, DtaGlobalVariableData*>::const_iterator cursor = _mapOfGlobalVariables.find(sName);
		if (cursor == _mapOfGlobalVariables.end()) return NULL;
		return cursor->second->_variableNode;
	}

	EXPRESSION_TYPE DtaProject::getGlobalVariableType(const std::string& sName) const {
		std::map<std::string, DtaGlobalVariableData*>::const_iterator cursor = _mapOfGlobalVariables.find(sName);
		if (cursor == _mapOfGlobalVariables.end()) {
			if ((sName == "_ARGS") || (sName == "_REQUEST")) return NODE_EXPRTYPE;
			return UNKNOWN_EXPRTYPE;
		}
		return cursor->second->_variableType;
	}

	DtaScriptVariable* DtaProject::setGlobalVariable(const std::string& sName, EXPRESSION_TYPE variableType) {
		DtaScriptVariable* pGlobalVariable;
		std::map<std::string, DtaGlobalVariableData*>::const_iterator cursor = _mapOfGlobalVariables.find(sName);
		if (cursor != _mapOfGlobalVariables.end()) {
			pGlobalVariable = cursor->second->_variableNode;
			if (pGlobalVariable == NULL) {
				pGlobalVariable = new DtaScriptVariable(NULL, sName);
				cursor->second->_variableNode = pGlobalVariable;
			} else {
				pGlobalVariable->clearContent();
			}
		} else {
			pGlobalVariable = new DtaScriptVariable(NULL, sName);
			_mapOfGlobalVariables[sName] = new DtaGlobalVariableData(pGlobalVariable, variableType);
		}
		return pGlobalVariable;
	}

	void DtaProject::setGlobalVariableType(const std::string& sName, EXPRESSION_TYPE variableType) {
		std::map<std::string, DtaGlobalVariableData*>::const_iterator cursor = _mapOfGlobalVariables.find(sName);
		if (cursor != _mapOfGlobalVariables.end()) {
			cursor->second->_variableType = variableType;
		} else {
			_mapOfGlobalVariables[sName] = new DtaGlobalVariableData(NULL, variableType);
		}
	}

	void DtaProject::setTextMode(TEXT_MODE iTextMode) {
		_iTextMode = iTextMode;
		if (iTextMode == DOS_MODE) {
			if (strcmp(ScpStream::ENDL.c_str(), "\r\n") != 0) ScpStream::ENDL = "\r\n";
		} else {
			if (strcmp(ScpStream::ENDL.c_str(), "\n") != 0) ScpStream::ENDL = "\n";
		}
	}

	double DtaProject::getLastDelay() const {
		if (_pDelayTimer != NULL) return _pDelayTimer->getTimeInSec();
		return _dLastDelay;
	}

	void DtaProject::copyCommon(const DtaProject& project) {
		_pScript = project._pScript;
		_bVerbose = project._bVerbose;
		_sVersion = project._sVersion;
		_sMarkupKey = project._sMarkupKey;
		_sMarkupValue = project._sMarkupValue;
		_sWorkingPath = project._sWorkingPath;
		_mapOfDefinedTargets = project._mapOfDefinedTargets;
		_sCommentBegin = project._sCommentBegin;
		_sCommentEnd = project._sCommentEnd;
		_sGenerationHeader = project._sGenerationHeader;
		_mapOfWarnings = project._mapOfWarnings;
		_pReadonlyHook = project._pReadonlyHook;
		_pWritefileHook = project._pWritefileHook;
		_bSpeed = project._bSpeed;
		_iTextMode = project._iTextMode;
		_sCurrentGenerationTagsHandler = project._sCurrentGenerationTagsHandler;
	}

	std::string DtaProject::getTraceStack(ScpStream& script) const {
		std::string sStack = UtlTrace::getTraceStack();
		const char* tcFilename = getScript()->getFilenamePtr();
		if (tcFilename != NULL) {
			sStack += tcFilename;
		}
		char tcNumber[16];
		sprintf(tcNumber, "(%d)", script.getLineCount());
		sStack += tcNumber;
		return sStack;
	}

	void DtaProject::traceEngine() const {
		if (_pScript != NULL) {
			_pScript->traceEngine();
		}
	}

	void DtaProject::prepareExecution() {
		_mapOfWarnings = std::map<std::string, int>();
	}

	int DtaProject::addWarning(const std::string& sWarning) {
		int& iCounter = _mapOfWarnings[sWarning];
		iCounter++;
		return iCounter;
	}

	bool DtaProject::isScriptVersionOlderThan(int* tiVersion) const {
		return true;
	}

	bool DtaProject::addGenerationTagsHandler(const std::string& sKey, DtaBNFScript* pReader, DtaPatternScript* pWriter) {
		if (_mapOfGenerationTagsHandlers.find(sKey) != _mapOfGenerationTagsHandlers.end()) return false;
		_mapOfGenerationTagsHandlers[sKey] = new DtaCustomTagsHandler(pReader, pWriter);
		return true;
	}

	bool DtaProject::removeGenerationTagsHandler(const std::string& sKey) {
		std::map<std::string, DtaCustomTagsHandler*>::iterator cursor = _mapOfGenerationTagsHandlers.find(sKey);
		if (cursor == _mapOfGenerationTagsHandlers.end()) return false;
		_mapOfGenerationTagsHandlers.erase(cursor);
		if (_sCurrentGenerationTagsHandler == sKey) _sCurrentGenerationTagsHandler = "";
		return true;
	}

	bool DtaProject::selectGenerationTagsHandler(const std::string& sKey) {
		std::map<std::string, DtaCustomTagsHandler*>::const_iterator cursor = _mapOfGenerationTagsHandlers.find(sKey);
		if (cursor == _mapOfGenerationTagsHandlers.end()) return false;
		_sCurrentGenerationTagsHandler = sKey;
		return true;
	}

	bool DtaProject::existGenerationTagsHandler(const std::string& sKey) const {
		std::map<std::string, DtaCustomTagsHandler*>::const_iterator cursor = _mapOfGenerationTagsHandlers.find(sKey);
		return (cursor != _mapOfGenerationTagsHandlers.end());
	}

	bool DtaProject::getGenerationTagsHandler(const std::string& sKey, DtaBNFScript*& pReader, DtaPatternScript*& pWriter) const {
		std::map<std::string, DtaCustomTagsHandler*>::const_iterator cursor = _mapOfGenerationTagsHandlers.find(sKey);
		if (cursor == _mapOfGenerationTagsHandlers.end()) return false;
		pReader = cursor->second->_pReader;
		pWriter = cursor->second->_pWriter;
		return true;
	}

	void DtaProject::captureOutputFile(const char* tcOutputFile, const char* tcTemplateScript) {
		_mapOfCapturedOutputFiles[tcOutputFile].insert(tcTemplateScript);
	}

	void DtaProject::captureInputFile(const char* tcInputFile, const char* tcBNFScript) {
		_mapOfCapturedInputFiles[tcInputFile].insert(tcBNFScript);
	}
}
