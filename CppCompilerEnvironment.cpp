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
#endif

#include <set>

#include "ScpStream.h"
#include "UtlException.h"

#include "CGRuntime.h"
#include "DtaProject.h"
#include "DtaPatternScript.h"
#include "GrfFunction.h"
#include "BNFClause.h"
#include "DynFunction.h"
#include "DynPackage.h"
#include "CGCompiler.h"
#include "CppCompilerEnvironment.h"

namespace CodeWorker {
	class CppStack {
	private:
		std::set<std::string> _setOfVariables;

	public:
		CppStack() {}
		~CppStack() {}

		bool addVariable(const std::string& sVariable);
		bool existVariable(const std::string& sVariable) const;
	};

	bool CppStack::addVariable(const std::string& sVariable) {
		std::set<std::string>::const_iterator cursor = _setOfVariables.find(sVariable);
		if (cursor != _setOfVariables.end()) return false;
		_setOfVariables.insert(sVariable);
		return true;
	}

	bool CppStack::existVariable(const std::string& sVariable) const {
		std::set<std::string>::const_iterator cursor = _setOfVariables.find(sVariable);
		return (cursor != _setOfVariables.end());
	}


	class CppVariableScope {
	private:
		std::list<CppStack*> _stack;
		std::string _sClauseReturnValue;
		bool _bHasEvaluatedExpressionInScope;

	public:
		CppVariableScope() : _bHasEvaluatedExpressionInScope(false) { addStack(); }
		~CppVariableScope() { removeStack(); }

		bool addVariable(const std::string& sVariable) { return _stack.front()->addVariable(sVariable); }
		bool existVariable(const std::string& sVariable) const;
		void addStack() { _stack.push_front(new CppStack); }
		void removeStack() { delete _stack.front(); _stack.pop_front(); }
		void setClauseReturnValue(const std::string& sClauseName);
		bool isClauseReturnValue(const std::string& sVariableName) const;
		void hasEvaluatedExpressionInScope(bool bEvaluated) { _bHasEvaluatedExpressionInScope = bEvaluated; }
		bool hasEvaluatedExpressionInScope() const { return _bHasEvaluatedExpressionInScope; }

	private:
		CppVariableScope(const CppVariableScope&);
		CppVariableScope& operator =(const CppVariableScope&);
	};

	bool CppVariableScope::existVariable(const std::string& sVariable) const {
		for (std::list<CppStack*>::const_iterator i = _stack.begin(); i != _stack.end(); i++) {
			if ((*i)->existVariable(sVariable)) return true;
		}
		return false;
	}

	void CppVariableScope::setClauseReturnValue(const std::string& sClauseName) {
		_sClauseReturnValue = sClauseName;
	}

	bool CppVariableScope::isClauseReturnValue(const std::string& sVariableName) const {
		return (_sClauseReturnValue == sVariableName);
	}


	CppScriptSession::CppScriptSession(const std::string& sFilename, const std::string& sCppRadical) : _sFilename(sFilename), _sCppRadical(sCppRadical), _header(0), _body(0) {
		_header = new DtaPatternScript;
		std::string sSourceFile = sCppRadical + ".h";
		ScpStream* pOldStream;
		_header->openGenerate(false, sSourceFile.c_str(), pOldStream);
		_body = new DtaPatternScript;
		sSourceFile = sCppRadical + ".cpp";
		_body->openGenerate(false, sSourceFile.c_str(), pOldStream);
		pushVariableScope();
	}
	CppScriptSession::CppScriptSession(const CppScriptSession& copy) : _header(copy._header), _body(copy._body) {
		_sIndentation = copy._sIndentation;
		_setOfProjectFunctions = copy._setOfProjectFunctions;
		_setOfProjectClauses = copy._setOfProjectClauses;
		_sFilename = copy._sFilename;
		_sCppRadical = copy._sCppRadical;
		copy._header = 0;
		copy._body = 0;
		_stackOfScopes = copy._stackOfScopes;
		copy._stackOfScopes = std::list<CppVariableScope*>();
	}
	CppScriptSession& CppScriptSession::operator =(const CppScriptSession& copy) {
		_sIndentation = copy._sIndentation;
		_setOfProjectFunctions = copy._setOfProjectFunctions;
		_setOfProjectClauses = copy._setOfProjectClauses;
		_sFilename = copy._sFilename;
		_sCppRadical = copy._sCppRadical;
		_header = copy._header;
		_body = copy._body;
		copy._header = 0;
		copy._body = 0;
		_stackOfScopes = copy._stackOfScopes;
		copy._stackOfScopes = std::list<CppVariableScope*>();
		return *this;
	}
	CppScriptSession::~CppScriptSession() {
		std::string sExceptionMessage;
		std::string sExceptionStack;
		if (_header != 0) {
			try {
				ScpStream* pOldStream = NULL;
				std::string sSourceFile = _sCppRadical + ".h";
				_header->closeGenerate(false, sSourceFile.c_str(), pOldStream);
			} catch(UtlException& exception) {
				sExceptionMessage = exception.getMessage();
				sExceptionStack = exception.getTraceStack();
			}
			delete _header;
		}
		if (_body != 0) {
			try {
				ScpStream* pOldStream = NULL;
				std::string sSourceFile = _sCppRadical + ".cpp";
				_body->closeGenerate(false, sSourceFile.c_str(), pOldStream);
			} catch(UtlException& exception) {
				if (!sExceptionMessage.empty() || !sExceptionStack.empty()) sExceptionMessage += "\n";
				sExceptionMessage += exception.getMessage();
				sExceptionStack += exception.getTraceStack();
			}
			delete _body;
		}

		for (std::list<CppVariableScope*>::iterator i = _stackOfScopes.begin(); i != _stackOfScopes.end(); ++i) {
			delete *i;
		}

		if (!sExceptionMessage.empty() || !sExceptionStack.empty()) throw UtlException(sExceptionMessage, sExceptionStack);
	}

	void CppScriptSession::incrementIndentation() {
		_sIndentation += "\t";
		_stackOfScopes.front()->addStack();
	}

	void CppScriptSession::decrementIndentation() {
		if (_sIndentation.size() == 1) _sIndentation = "";
		else _sIndentation = _sIndentation.substr(1);
		_stackOfScopes.front()->removeStack();
	}

	void CppScriptSession::pushVariableScope() const {
		_stackOfScopes.push_front(new CppVariableScope);
	}

	void CppScriptSession::popVariableScope() const {
		delete _stackOfScopes.front();
		_stackOfScopes.pop_front();
	}

	void CppScriptSession::pushForeach(GrfForeach* pForeach) {
		_listOfForeachStatements.push_front(pForeach);
	}

	GrfForeach* CppScriptSession::getLastForeach() const {
		return _listOfForeachStatements.front();
	}

	void CppScriptSession::popForeach() {
		_listOfForeachStatements.pop_front();
	}

	void CppScriptSession::catchFilename() {
		ScpStream* pOldStream = NULL;
		_header->catchGenerateExecution(false, pOldStream, NULL);
		delete _header;
		_header = 0;
		_body->catchGenerateExecution(false, pOldStream, NULL);
		delete _body;
		_body = 0;
	}


	CppNewVariableScopeEnvironment::CppNewVariableScopeEnvironment(CppCompilerEnvironment& env) : _session(env.getCurrentScriptSession()) {
		_stackOfScopes = _session._stackOfScopes;
		_session._stackOfScopes = std::list<CppVariableScope*>();
		_session.pushVariableScope();
	}

	CppNewVariableScopeEnvironment::~CppNewVariableScopeEnvironment() {
		_session.popVariableScope();
		_session._stackOfScopes = _stackOfScopes;
	}



	CppCompilerEnvironment::CppCompilerEnvironment(const std::string& sCppProjectDirectory) : _bBracketsToNextBlock(true), _bCarriageReturnAfterBlock(true), _iSwitchNumber(0), _iCursorNumber(0), _bErrorEncountered(false), _iPointerToDeclarations(0), _pCurrentClause(NULL), _iBNFStepperCursor(0), _iInlineScriptCounter(0) {
		_sCodeWorkerDirectory = DtaProject::getInstance().getCodeWorkerHome();
		if ((sCppProjectDirectory[0] == '/') || (!sCppProjectDirectory.empty() && (sCppProjectDirectory[1] == ':'))) _sCppProjectDirectory = CGRuntime::canonizePath(sCppProjectDirectory);
		else _sCppProjectDirectory = CGRuntime::relativePath(sCppProjectDirectory, CGRuntime::getWorkingPath());
		if (_sCppProjectDirectory[_sCppProjectDirectory.size() - 1] != '/') _sCppProjectDirectory += "/";
		CGRuntime::setProperty("c++2target-path", _sCppProjectDirectory);
		addGlobalVariable("_ARGS");
		addGlobalVariable("_REQUEST"); // for CGI program
		try {
			std::string sCWIncludeDir = _sCodeWorkerDirectory;
			if (CGRuntime::existFile(sCWIncludeDir + "include/UtlException.h")) sCWIncludeDir += "include/";
			CGRuntime::copySmartFile(sCWIncludeDir + "UtlException.h", _sCppProjectDirectory + "UtlException.h");
			CGRuntime::copySmartFile(sCWIncludeDir + "CppParsingTree.h", _sCppProjectDirectory + "CppParsingTree.h");
			CGRuntime::copySmartFile(sCWIncludeDir + "CGRuntime.h", _sCppProjectDirectory + "CGRuntime.h");
			CGRuntime::copySmartFile(sCWIncludeDir + "CGExternalHandling.h", _sCppProjectDirectory + "CGExternalHandling.h");
			CGRuntime::copySmartFile(sCWIncludeDir + "DynPackage.h", _sCppProjectDirectory + "DynPackage.h");
		} catch(UtlException&) {
			CGRuntime::traceLine("WARNING: (considering '" + _sCodeWorkerDirectory + "' as the root path of 'CodeWorker') unable to copy files 'UtlException.h' + 'CppParsingTree.h' + 'CGRuntime.h' in the project directory '" + _sCppProjectDirectory + "'");
		}
	}

	CppCompilerEnvironment::~CppCompilerEnvironment() {
		if (!_bErrorEncountered) {
			generateDynamicPackages();
			generateDSP();
			generateMakefile();
		}
	}

	void CppCompilerEnvironment::pushLastRepeatCursor(int iCursor) {
		_listOfLastRepeatCursors.push_back(iCursor);
		_listOfLastRepeatCursorUsed.push_back(false);
	}

	int CppCompilerEnvironment::getLastRepeatCursor(bool bUsed) {
		if (_listOfLastRepeatCursors.empty()) return -1;
		if (bUsed) _listOfLastRepeatCursorUsed.back() = true;
		return _listOfLastRepeatCursors.back();
	}

	bool CppCompilerEnvironment::popLastRepeatCursor() {
		bool bUsed = _listOfLastRepeatCursorUsed.back();
		_listOfLastRepeatCursors.pop_back();
		_listOfLastRepeatCursorUsed.pop_back();
		return bUsed;
	}

	void CppCompilerEnvironment::generateDynamicPackages() {
		const std::map<std::string, DynPackage*>& allPackages = DynPackage::allPackages();
		for (std::map<std::string, DynPackage*>::const_iterator i = allPackages.begin(); i != allPackages.end(); ++i) {
			std::auto_ptr<ScpStream> pModuleHeader(new ScpStream(_sCppProjectDirectory + i->first + ".h", ScpStream::OUT));
			std::auto_ptr<ScpStream> pModuleBody(new ScpStream(_sCppProjectDirectory + i->first + ".cpp", ScpStream::OUT));
			(*pModuleHeader) << "#ifndef _" << i->first << "_h_" << ScpStream::ENDL;
			(*pModuleHeader) << "#define _" << i->first << "_h_" << ScpStream::ENDL << ScpStream::ENDL;
			(*pModuleHeader) << "#include <string>" << ScpStream::ENDL;
			(*pModuleHeader) << "#include \"DynPackage.h\"" << ScpStream::ENDL;
			(*pModuleHeader) << ScpStream::ENDL << "namespace CodeWorker {" << ScpStream::ENDL;
			(*pModuleHeader) << "\tclass CppParsingTree_value;" << ScpStream::ENDL;
			(*pModuleHeader) << "\tclass CppParsingTree_var;" << ScpStream::ENDL;
			(*pModuleHeader) << ScpStream::ENDL << "\tclass " << i->first << " : public CodeWorker::DynPackage {" << ScpStream::ENDL;
			(*pModuleHeader) << "\t\tpublic:" << ScpStream::ENDL;
			(*pModuleHeader) << "\t\t\t" << i->first << "();" << ScpStream::ENDL;
			(*pModuleHeader) << "\t\t\tstatic " << i->first << "& instance();" << ScpStream::ENDL;
			(*pModuleBody) << "#ifdef WIN32" << ScpStream::ENDL;
			(*pModuleBody) << "#pragma warning(disable: 4786)" << ScpStream::ENDL;
			(*pModuleBody) << "#endif" << ScpStream::ENDL << ScpStream::ENDL;
			(*pModuleBody) << "#include \"UtlException.h\"" << ScpStream::ENDL;
			(*pModuleBody) << "#include \"CppParsingTree.h\"" << ScpStream::ENDL;
			(*pModuleBody) << "#include \"" << i->first << ".h\"" << ScpStream::ENDL;
			(*pModuleBody) << ScpStream::ENDL << "namespace CodeWorker {" << ScpStream::ENDL;
			(*pModuleBody) << "\t" << i->first << "* " << i->first << "::pInstance_ = NULL;" << ScpStream::ENDL << ScpStream::ENDL;
			(*pModuleBody) << "\t" << i->first << "::" << i->first << "() : CodeWorker::DynPackage(\"" << i->first << "\") {" << ScpStream::ENDL;
			(*pModuleBody) << "\t\tpInstance_ = this;" << ScpStream::ENDL;
			(*pModuleBody) << "\t}" << ScpStream::ENDL << ScpStream::ENDL;
			(*pModuleBody) << "\t" << i->first << "& " << i->first << "::instance() {" << ScpStream::ENDL;
			(*pModuleBody) << "\t\tif (pInstance_ == NULL) throw UtlException(\"The dynamic package '" << i->first << "' should have been initialized before use!\");" << ScpStream::ENDL;
			(*pModuleBody) << "\t\treturn *pInstance_;" << ScpStream::ENDL;
			(*pModuleBody) << "\t}" << ScpStream::ENDL << ScpStream::ENDL;
			for (std::map<std::string, DynFunction*>::const_iterator j = i->second->allFunctions().begin(); j != i->second->allFunctions().end(); ++j) {
				(*pModuleHeader) << "\t\t\tstd::string _compilerFunction_" << j->first << "(";
				(*pModuleBody) << "\tstd::string " + i->first + "::_compilerFunction_" << j->first << "(";
				int iIndex = 0;
				std::list<std::string>::const_iterator k;
				for (k = j->second->getParameters().begin(); k != j->second->getParameters().end(); ++k) {
					if (iIndex > 0) {
						(*pModuleHeader) << ", ";
						(*pModuleBody) << ", ";
					}
					switch(j->second->getParameterType(iIndex) & 0x00FF) {
						case VALUE_EXPRTYPE:
							(*pModuleHeader) << "const CodeWorker::CppParsingTree_value& ";
							(*pModuleBody) << "const CodeWorker::CppParsingTree_value& ";
							break;
						case NODE_EXPRTYPE:
							(*pModuleHeader) << "const CodeWorker::CppParsingTree_var& ";
							(*pModuleBody) << "const CodeWorker::CppParsingTree_var& ";
							break;
						default:
							throw UtlException("fatal error while generating the C++ layer of the dynamic module '" + i->first + "': unrecognized parameter type of variable '" + *k + "'");
					}
					(*pModuleHeader) << *k;
					(*pModuleBody) << *k;
					iIndex++;
				}
				(*pModuleHeader) << ");" << ScpStream::ENDL;
				(*pModuleBody) << ") {" << ScpStream::ENDL;
				(*pModuleBody) << "\t\tstatic CALL" << iIndex << "_FUNCTION pfFunctionCall = (CALL" << iIndex << "_FUNCTION) findFunction(_hHandle, \"" << j->first << "\");" << ScpStream::ENDL;
				(*pModuleBody) << "\t\tif (pfFunctionCall == NULL) throw UtlException(\"the module '" << i->first << "' doesn't export the function '" << i->first << "::" << j->first << "'\");" << ScpStream::ENDL;
				iIndex = 0;
				if (j->second->getParameters().size() <= 4) {
					(*pModuleBody) << "\t\tconst char* tcResult = pfFunctionCall(_pInterpreter";
					for (k = j->second->getParameters().begin(); k != j->second->getParameters().end(); ++k) {
						(*pModuleBody) << ", ";
						switch(j->second->getParameterType(iIndex) & 0x00FF) {
							case VALUE_EXPRTYPE:
								(*pModuleBody) << "(char*) " << *k << ".getValue()";
								break;
							case NODE_EXPRTYPE:
								(*pModuleBody) << *k << ".getInternalNode()";
								break;
							default:
								throw UtlException("fatal error while generating the C++ layer of the dynamic module '" + i->first + "': unrecognized parameter type of variable '" + *k + "'");
						}
						iIndex++;
					}
					(*pModuleBody) << ");" << ScpStream::ENDL;
				} else {
					(*pModuleBody) << "\t\tCodeWorker::Parameter* tParams = new CodeWorker::Parameter[" << (int) j->second->getParameters().size() << "];";
					for (k = j->second->getParameters().begin(); k != j->second->getParameters().end(); ++k) {
						(*pModuleBody) << ", ";
						switch(j->second->getParameterType(iIndex) & 0x00FF) {
							case VALUE_EXPRTYPE:
								(*pModuleBody) << "\t\ttParams[" << iIndex << "] = (char*) " << *k << ".getValue();" << ScpStream::ENDL;
								break;
							case NODE_EXPRTYPE:
								(*pModuleBody) << "\t\ttParams[" << iIndex << "] = " << *k << ".getInternalNode();" << ScpStream::ENDL;
								break;
							default:
								throw UtlException("fatal error while generating the C++ layer of the dynamic module '" + i->first + "': unrecognized parameter type of variable '" + *k + "'");
						}
						iIndex++;
					}
					(*pModuleBody) << "\t\tconst char* tcResult = pfFunctionCall(_pInterpreter, tParams);" << ScpStream::ENDL;
					(*pModuleBody) << "\t\tdelete [] tParams;" << ScpStream::ENDL;
				}
				(*pModuleBody) << "\t\tif (tcResult == NULL) return \"\";" << ScpStream::ENDL;
				(*pModuleBody) << "\t\treturn tcResult;" << ScpStream::ENDL;
				(*pModuleBody) << "\t}" << ScpStream::ENDL << ScpStream::ENDL;
			}
			(*pModuleBody) << "}" << ScpStream::ENDL;
			(*pModuleHeader) << "\t\tprivate:" << ScpStream::ENDL;
			(*pModuleHeader) << "\t\t\tstatic " << i->first << "* " << "pInstance_;" << ScpStream::ENDL;
			(*pModuleHeader) << "\t};" << ScpStream::ENDL;
			(*pModuleHeader) << "}" << ScpStream::ENDL;
			(*pModuleHeader) << "#endif" << ScpStream::ENDL;
			{
				std::auto_ptr<ScpStream> pOldHeader(new ScpStream(_sCppProjectDirectory + i->first + ".h", ScpStream::IN | ScpStream::OUT));
				int iPosition;
				if (!pModuleHeader->equals(*pOldHeader, iPosition)) pModuleHeader->saveIntoFile(_sCppProjectDirectory + i->first + ".h", true);
			}
			{
				std::auto_ptr<ScpStream> pOldBody(new ScpStream(_sCppProjectDirectory + i->first + ".cpp", ScpStream::IN | ScpStream::OUT));
				int iPosition;
				if (!pModuleBody->equals(*pOldBody, iPosition)) pModuleBody->saveIntoFile(_sCppProjectDirectory + i->first + ".cpp", true);
			}
		}
	}

	void CppCompilerEnvironment::generateDSP() {
		std::string sBinDir = _sCodeWorkerDirectory;
		if (CGRuntime::existFile(sBinDir + "bin/CodeWorker.lib")) sBinDir += "bin/";
		else if (CGRuntime::existFile(sBinDir + "../bin/CodeWorker.lib")) sBinDir += "../bin/";
		else if (CGRuntime::existFile(sBinDir + "lib/CodeWorker.lib")) sBinDir += "lib/";
		else if (CGRuntime::existFile(sBinDir + "../lib/CodeWorker.lib")) sBinDir += "../lib/";
		else sBinDir += "Release/";
		std::string sProjectName = getRadical(_listOfProjectModules.front());
		std::string::size_type iIndex = sProjectName.find_last_of('_');
		if ((iIndex != std::string::npos) && (iIndex != 0)) sProjectName = sProjectName.substr(0, iIndex);
		std::auto_ptr<ScpStream> pMakeFile(new ScpStream(_sCppProjectDirectory + sProjectName + ".dsp", ScpStream::IN | ScpStream::OUT));
		if (pMakeFile->empty()) {
			(*pMakeFile) << "# Microsoft Developer Studio Project File - Name=\"" << sProjectName << "\" - Package Owner=<4>\r\n";
			(*pMakeFile) << "# Microsoft Developer Studio Generated Build File, Format Version 6.00\r\n";
			(*pMakeFile) << "# ** DO NOT EDIT **\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "# TARGTYPE \"Win32 (x86) Console Application\" 0x0103\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "CFG=" << sProjectName << " - Win32 Debug\r\n";
			(*pMakeFile) << "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\r\n";
			(*pMakeFile) << "!MESSAGE use the Export Makefile command and run\r\n";
			(*pMakeFile) << "!MESSAGE \r\n";
			(*pMakeFile) << "!MESSAGE NMAKE /f \"" << sProjectName << ".mak\".\r\n";
			(*pMakeFile) << "!MESSAGE \r\n";
			(*pMakeFile) << "!MESSAGE You can specify a configuration when running NMAKE\r\n";
			(*pMakeFile) << "!MESSAGE by defining the macro CFG on the command line. For example:\r\n";
			(*pMakeFile) << "!MESSAGE \r\n";
			(*pMakeFile) << "!MESSAGE NMAKE /f \"" << sProjectName << ".mak\" CFG=\"" << sProjectName << " - Win32 Debug\"\r\n";
			(*pMakeFile) << "!MESSAGE \r\n";
			(*pMakeFile) << "!MESSAGE Possible choices for configuration are:\r\n";
			(*pMakeFile) << "!MESSAGE \r\n";
			(*pMakeFile) << "!MESSAGE \"" << sProjectName << " - Win32 Release\" (based on \"Win32 (x86) Console Application\")\r\n";
			(*pMakeFile) << "!MESSAGE \"" << sProjectName << " - Win32 Debug\" (based on \"Win32 (x86) Console Application\")\r\n";
			(*pMakeFile) << "!MESSAGE \r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "# Begin Project\r\n";
			(*pMakeFile) << "# PROP AllowPerConfigDependencies 0\r\n";
			(*pMakeFile) << "# PROP Scc_ProjName \"\"\r\n";
			(*pMakeFile) << "# PROP Scc_LocalPath \"\"\r\n";
			(*pMakeFile) << "CPP=cl.exe\r\n";
			(*pMakeFile) << "RSC=rc.exe\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "!IF  \"$(CFG)\" == \"" << sProjectName << " - Win32 Release\"\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "# PROP BASE Use_MFC 0\r\n";
			(*pMakeFile) << "# PROP BASE Use_Debug_Libraries 0\r\n";
			(*pMakeFile) << "# PROP BASE Output_Dir \"Release\"\r\n";
			(*pMakeFile) << "# PROP BASE Intermediate_Dir \"Release\"\r\n";
			(*pMakeFile) << "# PROP BASE Target_Dir \"\"\r\n";
			(*pMakeFile) << "# PROP Use_MFC 0\r\n";
			(*pMakeFile) << "# PROP Use_Debug_Libraries 0\r\n";
			(*pMakeFile) << "# PROP Output_Dir \"Release\"\r\n";
			(*pMakeFile) << "# PROP Intermediate_Dir \"Release\"\r\n";
			(*pMakeFile) << "# PROP Ignore_Export_Lib 0\r\n";
			(*pMakeFile) << "# PROP Target_Dir \"\"\r\n";
			(*pMakeFile) << "# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_CONSOLE\" /D \"_MBCS\" /YX /FD /c\r\n";
			(*pMakeFile) << "# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D \"NDEBUG\" /D \"WIN32\" /D \"_CONSOLE\" /D \"_MBCS\" /FD /c\r\n";
			(*pMakeFile) << "# SUBTRACT CPP /YX\r\n";
			(*pMakeFile) << "# ADD BASE RSC /l 0x40c /d \"NDEBUG\"\r\n";
			(*pMakeFile) << "# ADD RSC /l 0x40c /d \"NDEBUG\"\r\n";
			(*pMakeFile) << "BSC32=bscmake.exe\r\n";
			(*pMakeFile) << "# ADD BASE BSC32 /nologo\r\n";
			(*pMakeFile) << "# ADD BSC32 /nologo\r\n";
			(*pMakeFile) << "LINK32=link.exe\r\n";
			(*pMakeFile) << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386\r\n";
			(*pMakeFile) << "# ADD LINK32 " << sBinDir << "CodeWorker.lib ws2_32.lib rpcrt4.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "!ELSEIF  \"$(CFG)\" == \"" << sProjectName << " - Win32 Debug\"\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "# PROP BASE Use_MFC 0\r\n";
			(*pMakeFile) << "# PROP BASE Use_Debug_Libraries 1\r\n";
			(*pMakeFile) << "# PROP BASE Output_Dir \"Debug\"\r\n";
			(*pMakeFile) << "# PROP BASE Intermediate_Dir \"Debug\"\r\n";
			(*pMakeFile) << "# PROP BASE Target_Dir \"\"\r\n";
			(*pMakeFile) << "# PROP Use_MFC 0\r\n";
			(*pMakeFile) << "# PROP Use_Debug_Libraries 1\r\n";
			(*pMakeFile) << "# PROP Output_Dir \"Debug\"\r\n";
			(*pMakeFile) << "# PROP Intermediate_Dir \"Debug\"\r\n";
			(*pMakeFile) << "# PROP Ignore_Export_Lib 0\r\n";
			(*pMakeFile) << "# PROP Target_Dir \"\"\r\n";
			(*pMakeFile) << "# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_CONSOLE\" /D \"_MBCS\" /YX /FD /GZ /c\r\n";
			(*pMakeFile) << "# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D \"_DEBUG\" /D \"WIN32\" /D \"_CONSOLE\" /D \"_MBCS\" /FD /GZ /c\r\n";
			(*pMakeFile) << "# SUBTRACT CPP /YX\r\n";
			(*pMakeFile) << "# ADD BASE RSC /l 0x40c /d \"_DEBUG\"\r\n";
			(*pMakeFile) << "# ADD RSC /l 0x40c /d \"_DEBUG\"\r\n";
			(*pMakeFile) << "BSC32=bscmake.exe\r\n";
			(*pMakeFile) << "# ADD BASE BSC32 /nologo\r\n";
			(*pMakeFile) << "# ADD BSC32 /nologo\r\n";
			(*pMakeFile) << "LINK32=link.exe\r\n";
			(*pMakeFile) << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept\r\n";
			(*pMakeFile) << "# ADD LINK32 " << _sCodeWorkerDirectory << "Debug/CodeWorkerD.lib ws2_32.lib rpcrt4.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "!ENDIF \r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "# Begin Target\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "# Name \"" << sProjectName << " - Win32 Release\"\r\n";
			(*pMakeFile) << "# Name \"" << sProjectName << " - Win32 Debug\"\r\n";
			(*pMakeFile) << "# Begin Group \"Source Files\"\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "# PROP Default_Filter \"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\"\r\n";
		} else {
			if (!pMakeFile->findString("# Begin Source File")) throw UtlException("bad format encountered into Visual C++ project \"" + sProjectName + ".dsp\": unable to find tag '# Begin Source File'");
			pMakeFile->setOutputLocation(pMakeFile->getInputLocation() - 19);
		}
		std::list<std::string>::const_iterator i;
		for (i = _listOfProjectModules.begin(); i != _listOfProjectModules.end(); i++) {
			(*pMakeFile) << "# Begin Source File\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "SOURCE=\".\\" << getRelativePath(*i) << ".cpp\"\r\n";
			(*pMakeFile) << "# End Source File\r\n";
		}
		{
			const std::map<std::string, DynPackage*>& allPackages = DynPackage::allPackages();
			for (std::map<std::string, DynPackage*>::const_iterator i = allPackages.begin(); i != allPackages.end(); ++i) {
				(*pMakeFile) << "# Begin Source File\r\n";
				(*pMakeFile) << "\r\n";
				(*pMakeFile) << "SOURCE=\".\\" << i->first << ".cpp\"\r\n";
				(*pMakeFile) << "# End Source File\r\n";
			}
		}
		(*pMakeFile) << "# End Group\r\n";
		(*pMakeFile) << "# Begin Group \"Header Files\"\r\n";
		(*pMakeFile) << "\r\n";
		(*pMakeFile) << "# PROP Default_Filter \"h;hpp;hxx;hm;inl\"\r\n";
		for (i = _listOfProjectModules.begin(); i != _listOfProjectModules.end(); i++) {
			std::string::size_type iIndex = i->find_last_of("/\\");
			std::string sFile;
			if (iIndex != std::string::npos) sFile = i->substr(iIndex + 1);
			else sFile = *i;
			(*pMakeFile) << "# Begin Source File\r\n";
			(*pMakeFile) << "\r\n";
			(*pMakeFile) << "SOURCE=\".\\" << getRelativePath(*i) << ".h\"\r\n";
			(*pMakeFile) << "# End Source File\r\n";
		}
		{
			const std::map<std::string, DynPackage*>& allPackages = DynPackage::allPackages();
			for (std::map<std::string, DynPackage*>::const_iterator i = allPackages.begin(); i != allPackages.end(); ++i) {
				(*pMakeFile) << "# Begin Source File\r\n";
				(*pMakeFile) << "\r\n";
				(*pMakeFile) << "SOURCE=\".\\" << i->first << ".h\"\r\n";
				(*pMakeFile) << "# End Source File\r\n";
			}
		}
		(*pMakeFile) << "# Begin Source File\r\n";
		(*pMakeFile) << "\r\n";
		(*pMakeFile) << "SOURCE=.\\CppParsingTree.h\r\n";
		(*pMakeFile) << "# End Source File\r\n";
		(*pMakeFile) << "# Begin Source File\r\n";
		(*pMakeFile) << "\r\n";
		(*pMakeFile) << "SOURCE=.\\UtlException.h\r\n";
		(*pMakeFile) << "# End Source File\r\n";
		(*pMakeFile) << "# End Group\r\n";
		(*pMakeFile) << "# Begin Group \"Resource Files\"\r\n";
		(*pMakeFile) << "\r\n";
		(*pMakeFile) << "# PROP Default_Filter \"ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe\"\r\n";
		(*pMakeFile) << "# End Group\r\n";
		(*pMakeFile) << "# End Target\r\n";
		(*pMakeFile) << "# End Project\r\n";
		std::auto_ptr<ScpStream> pOldMakeFile(new ScpStream(_sCppProjectDirectory + sProjectName + ".dsp", ScpStream::IN | ScpStream::OUT));
		int iPosition;
		if (!pMakeFile->equals(*pOldMakeFile, iPosition)) pMakeFile->saveIntoFile(_sCppProjectDirectory + sProjectName + ".dsp", true);
	}

	void CppCompilerEnvironment::generateMakefile() {
		std::string sBinDir = _sCodeWorkerDirectory;
		if (CGRuntime::existFile(sBinDir + "bin/libCodeWorker.a")) sBinDir += "bin/";
		else if (CGRuntime::existFile(sBinDir + "../bin/libCodeWorker.a")) sBinDir += "../bin/";
		else if (CGRuntime::existFile(sBinDir + "lib/libCodeWorker.a")) sBinDir += "lib/";
		else if (CGRuntime::existFile(sBinDir + "../lib/libCodeWorker.a")) sBinDir += "../lib/";
		std::string sProjectName = getRadical(_listOfProjectModules.front());
		std::string::size_type iIndex = sProjectName.find_last_of('_');
		if ((iIndex != std::string::npos) && (iIndex != 0)) sProjectName = sProjectName.substr(0, iIndex);
		std::auto_ptr<ScpStream> pMakeFile(new ScpStream(_sCppProjectDirectory + "Makefile", ScpStream::IN | ScpStream::OUT));
		std::auto_ptr<ScpStream> pOldMakeFile(new ScpStream(_sCppProjectDirectory + "Makefile", ScpStream::IN | ScpStream::OUT));
		(*pMakeFile) << "INCDIRS	= -I.\n";
		(*pMakeFile) << "CXXFLAGS	= -O2 -g $(INCDIRS)\n";
		(*pMakeFile) << "LFLAGS	= -g -lm -ldl -L" << sBinDir << " -lcodeworker\n";
		(*pMakeFile) << "CC	= g++\n";
		(*pMakeFile) << "\n";
		(*pMakeFile) << "OBJECTS = ";
		std::list<std::string>::const_iterator i;
		for (i = _listOfProjectModules.begin(); i != _listOfProjectModules.end(); i++) {
			std::string sRadical = getRelativePath(*i);
			if (pMakeFile->getColCount() + sRadical.size() > 70) {
				(*pMakeFile) << "\\\n\t\t  ";
			} else {
				(*pMakeFile) << " ";
			}
			(*pMakeFile) << sRadical << ".o";
		}
		{
			const std::map<std::string, DynPackage*>& allPackages = DynPackage::allPackages();
			for (std::map<std::string, DynPackage*>::const_iterator i = allPackages.begin(); i != allPackages.end(); ++i) {
				if (pMakeFile->getColCount() + i->first.size() > 70) {
					(*pMakeFile) << "\\\n\t\t  ";
				} else {
					(*pMakeFile) << " ";
				}
				(*pMakeFile) << i->first << ".o";
			}
		}
		(*pMakeFile) << "\n\n";
		(*pMakeFile) << "all: build\n";
		(*pMakeFile) << "build: $(OBJECTS)\n";
		(*pMakeFile) << "\t$(CC) -o " << sProjectName << " $(OBJECTS) $(LFLAGS)\n";
		(*pMakeFile) << "clean:\n";
		(*pMakeFile) << "\t-rm -f *.o " << sProjectName << " core\n";
		int iPosition;
		if (!pMakeFile->equals(*pOldMakeFile, iPosition)) pMakeFile->saveIntoFile(_sCppProjectDirectory + "Makefile", true);
	}

	void CppCompilerEnvironment::optimizeSources() {
		std::string sOptimizerScript = DtaProject::getTargetScriptFilename(DtaProject::getInstance().getTargetLanguage(), "cppOptimizer.cwp");
		if (sOptimizerScript.empty()) return;
		CGCompiledTranslationScript script(sOptimizerScript);
		DtaScriptVariable parseTree(NULL, "parseTree");
		for (std::list<std::string>::const_iterator i = _listOfProjectModules.begin(); i != _listOfProjectModules.end(); i++) {
			std::string sRadical  = _sCppProjectDirectory + *i;
			std::string sInputFile  = sRadical + ".cpp";
			std::string sOutputFile = sRadical + ".out";
			DtaScriptVariable* pLast = parseTree.pushItem(sRadical);
			script.translate(pLast, sInputFile, sOutputFile);
			if (pLast->getNode("implicit_copy") != NULL) parseTree.insertNode("implicit_copy")->setValue(true);
		}
		if (parseTree.getNode("implicit_copy") == NULL) parseTree.insertNode("explicit_copy")->setValue(true);
		const std::list<DtaScriptVariable*>* pList = parseTree.getArray();
		if (pList != NULL) {
			for (std::list<DtaScriptVariable*>::const_iterator i = pList->begin(); i != pList->end(); ++i) {
				std::string sRadical  = (*i)->getValue();
				std::string sInputFile  = sRadical + ".out";
				std::string sOutputFile = sRadical + ".out";
				DtaScriptVariable* pVariables = (*i)->getNode("variables");
				if ((pVariables != NULL) && (parseTree.getNode("explicit_copy") != NULL)) {
					const std::list<DtaScriptVariable*>* pVariablesList = pVariables->getArray();
					if (pVariablesList != NULL) {
						for (std::list<DtaScriptVariable*>::const_iterator j = pVariablesList->begin(); j != pVariablesList->end(); ++j) {
							if (CGRuntime::startString((*j)->getName(), "_compilerClauseImplicitCopy_")) {
								(*j)->insertNode("used")->setValue(false);
								(*j)->insertNode("active")->setValue(true);
							}
						}
					}
				}
				if (parseTree.getNode("explicit_copy") != NULL) (*i)->insertNode("explicit_copy")->setValue(true);
				if ((*i)->getNode("explicit_copy") != NULL) {
					script.translate(*i, sInputFile, sOutputFile);
				}
			}
		}
		for (std::list<std::string>::const_iterator i = _listOfProjectModules.begin(); i != _listOfProjectModules.end(); i++) {
			std::string sRadical  = _sCppProjectDirectory + *i;
			std::string sInputFile  = sRadical + ".out";
			std::string sOutputFile = sRadical + ".cpp";
			DtaScriptVariable* pLast = parseTree.pushItem(sRadical);
			pLast->insertNode("rules")->setValue(true);
			script.translate(pLast, sInputFile, sOutputFile);
		}
	}

	void CppCompilerEnvironment::translateToTargetLanguage(const std::string& sTargetLanguage) {
		std::string sParserScript = DtaProject::getTargetScriptFilename(sTargetLanguage, "CWcpp-parser.cwp");
		if (sParserScript.empty()) throw UtlException("C++-to-" + sTargetLanguage + " translation error: unable to find the BNF-parse script \"CWcpp-parser.cwp\"");
		std::string sTranslatorScript = DtaProject::getTargetScriptFilename(sTargetLanguage, "CWcpp2" + sTargetLanguage + ".cws");
		if (sTranslatorScript.empty()) {
			if (sTargetLanguage == "cpp") return;
			throw UtlException("C++-to-" + sTargetLanguage + " translation error: unable to find the translation script \"CWcpp2" + sTargetLanguage + ".cws\"");
		}
		CGCompiledBNFScript parser(sParserScript);
		CGCompiledCommonScript translator;
		translator.buildFromFile(sTranslatorScript);
		DtaScriptVariable parseTree(NULL, "parseTree");
		for (std::list<std::string>::const_iterator i = _listOfProjectModules.begin(); i != _listOfProjectModules.end(); i++) {
			std::string sCppFile = _sCppProjectDirectory + *i + ".cpp";
			parser.parse(&parseTree, sCppFile);
		}
		translator.execute(&parseTree);
	}

	ScpStream& CppCompilerEnvironment::getHeader() const {
		DtaPatternScript* pScript = getCurrentScriptSession()._header;
		return *(pScript->getOutputStream());
	}

	ScpStream& CppCompilerEnvironment::getMainHeader() const {
		DtaPatternScript* pScript = _listOfScriptSessions.back()._header;
		return *(pScript->getOutputStream());
	}

	ScpStream& CppCompilerEnvironment::getBody() const {
		DtaPatternScript* pScript = getCurrentScriptSession()._body;
		return *(pScript->getOutputStream());
	}

	std::string CppCompilerEnvironment::filename2Module(const std::string& sFilename) {
		std::string sModule;
		std::string::size_type iIndex = sFilename.find('.');
		if (iIndex != std::string::npos) {
			sModule = sFilename.substr(0, iIndex) + '_' + filename2Module(sFilename.substr(iIndex + 1));
		} else sModule = sFilename;
		return sModule;
	}

	bool CppCompilerEnvironment::pushFilename(const std::string& sFilename) {
		std::string sModule = filename2Module(sFilename);
		for (std::list<std::string>::const_iterator i = _listOfProjectModules.begin(); i != _listOfProjectModules.end(); i++) {
			if ((*i) == sModule) return false;
		}
		CppScriptSession session(sFilename, _sCppProjectDirectory + getRadical(sModule));
		_listOfScriptSessions.push_front(session);
		_listOfProjectModules.push_back(sModule);
		return true;
	}

	std::string CppCompilerEnvironment::getRadical() const {
		return getRadical(getCurrentScriptSession()._sCppRadical);
	}

	std::string CppCompilerEnvironment::getMainRadical() const {
		return getRadical(_listOfScriptSessions.back()._sCppRadical);
	}

	std::string CppCompilerEnvironment::getRadical(const std::string& sFile) {
		std::string sRadical = sFile;
		std::string::size_type iIndex = sRadical.find_last_of("/\\");
		if (iIndex != std::string::npos) sRadical = sRadical.substr(iIndex + 1);
		iIndex = sRadical.find('.');
		if (iIndex != std::string::npos) sRadical = sRadical.substr(0, iIndex);
		return sRadical;
	}

	std::string CppCompilerEnvironment::getRelativePath(const std::string& sFile) {
		std::string sRadical;
		std::string::size_type iIndex = sFile.find_last_of("/\\");
		if (iIndex != std::string::npos) sRadical = sFile.substr(iIndex + 1);
		else sRadical = sFile;
		return sRadical;
	}

	std::string CppCompilerEnvironment::convertTemplateKey(const std::string& sKey) {
		std::string sText;
		for (std::string::size_type i = 0; i < sKey.size(); i++) {
			char a = sKey[i];
			if (((a >= 'A') && (a <= 'Z')) || ((a >= 'a') && (a <= 'z')) || ((a >= '0') && (a <= '9'))) sText += a;
			else switch(a) {
				case '+': sText += "_compilerPlus_";break;
				case '-': sText += "_compilerMinus_";break;
				case '.': sText += "_compilerDot_";break;
				case ',': sText += "_compilerComma_";break;
				default: sText += '_';
			}
		}
		return sText;
	}

	std::string CppCompilerEnvironment::convertToCppVariable(const std::string& sVariable) {
		if ((sVariable == "namespace") || (sVariable == "class")) return "_compilerCppVariable_" + sVariable;
		return sVariable;
	}

	std::string CppCompilerEnvironment::getIncludeParentScript() const {
		std::string sRadical;
		if (_listOfScriptSessions.front()._sFilename != _listOfScriptSessions.back()._sFilename) {
			std::list<CppScriptSession>::const_iterator cursor = _listOfScriptSessions.begin();
			++cursor;
			sRadical = filename2Module(cursor->_sFilename);
		}
		return sRadical;
	}

	void CppCompilerEnvironment::incrementIndentation() {
		_listOfScriptSessions.front().incrementIndentation();
	}

	void CppCompilerEnvironment::pushVariableScope() {
		_listOfScriptSessions.front().pushVariableScope();
	}

	std::string CppCompilerEnvironment::newInlineScriptFilename() {
		_iInlineScriptCounter++;
		char tcFile[80];
		sprintf(tcFile, "InlineScript2File_%d", _iInlineScriptCounter);
		return tcFile;
	}

	void CppCompilerEnvironment::newFunction(const GrfFunction* pFunction) {
		_listOfScriptSessions.front()._setOfProjectFunctions.insert(pFunction->getFunctionName());
		_pCurrentFunction = pFunction;
	}

	void CppCompilerEnvironment::newClause(const BNFClause* pClause) {
		_listOfScriptSessions.front()._setOfProjectFunctions.insert(pClause->getName());
		_pCurrentClause = pClause;
	}

	std::string CppCompilerEnvironment::getFunctionModule(const std::string& sFunction) const {
		for (std::list<CppScriptSession>::const_iterator i = _listOfScriptSessions.begin(); i != _listOfScriptSessions.end(); i++) {
			if (i->_setOfProjectFunctions.find(sFunction) != i->_setOfProjectFunctions.end()) return i->_sCppRadical;
		}
		return _listOfScriptSessions.front()._sCppRadical;
	}

	std::string CppCompilerEnvironment::getClauseModule(const std::string& sClause) const {
		for (std::list<CppScriptSession>::const_iterator i = _listOfScriptSessions.begin(); i != _listOfScriptSessions.end(); i++) {
			if (i->_setOfProjectClauses.find(sClause) != i->_setOfProjectClauses.end()) return i->_sCppRadical;
		}
		return _listOfScriptSessions.front()._sCppRadical;
	}

	void CppCompilerEnvironment::popVariableScope() {
		_listOfScriptSessions.front().popVariableScope();
	}

	bool CppCompilerEnvironment::addVariable(const std::string& sVariable) {
		return _listOfScriptSessions.front()._stackOfScopes.front()->addVariable(sVariable);
	}

	bool CppCompilerEnvironment::existVariable(const std::string& sVariable) const {
		return _listOfScriptSessions.front()._stackOfScopes.front()->existVariable(sVariable) | (_globalVariables.find(sVariable) != _globalVariables.end());
	}

	void CppCompilerEnvironment::addGlobalVariable(const std::string& sVariable) {
		_globalVariables.insert(sVariable);
	}

	void CppCompilerEnvironment::setClauseReturnValue(const std::string& sClauseName) {
		_listOfScriptSessions.front()._stackOfScopes.front()->setClauseReturnValue(sClauseName);
	}

	bool CppCompilerEnvironment::isClauseReturnValue(const std::string& sVariableName) const {
		return _listOfScriptSessions.front()._stackOfScopes.front()->isClauseReturnValue(sVariableName);
	}

	void CppCompilerEnvironment::hasEvaluatedExpressionInScope(bool bEvaluated) {
		_listOfScriptSessions.front()._stackOfScopes.front()->hasEvaluatedExpressionInScope(bEvaluated);
	}

	bool CppCompilerEnvironment::hasEvaluatedExpressionInScope() const {
		return _listOfScriptSessions.front()._stackOfScopes.front()->hasEvaluatedExpressionInScope();
	}

	int CppCompilerEnvironment::newSwitch() {
		_iSwitchNumber++;
		return _iSwitchNumber;
	}

	int CppCompilerEnvironment::newCursor() {
		_iCursorNumber++;
		return _iCursorNumber;
	}

	void CppCompilerEnvironment::decrementIndentation() {
		_listOfScriptSessions.front().decrementIndentation();
	}

	void CppCompilerEnvironment::pushForeach(GrfForeach* pForeach) {
		_listOfScriptSessions.front().pushForeach(pForeach);
	}

	GrfForeach* CppCompilerEnvironment::getLastForeach() const {
		return _listOfScriptSessions.front().getLastForeach();
	}

	void CppCompilerEnvironment::popForeach() {
		_listOfScriptSessions.front().popForeach();
	}

	void CppCompilerEnvironment::catchFilename(UtlException& exception) {
		_listOfScriptSessions.front().catchFilename();
		_bErrorEncountered = true;
		throw UtlException(exception.getTraceStack(), exception.getMessage());
	}

	void CppCompilerEnvironment::popFilename() {
		_listOfScriptSessions.pop_front();
	}
}
//##markup##"build C++ headers"
