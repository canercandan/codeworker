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

#ifndef _DtaProject_h_
#define _DtaProject_h_

#include <map>

#include "ExprScriptFunction.h"
#include "DtaScriptVariable.h"

namespace CodeWorker {
	class ScpStream;
	class UtlTimer;

	class DtaScript;
	class GrfReadonlyHook;
	class GrfWritefileHook;
	class CppParsingTree_value;

	class DtaBNFScript;
	class DtaPatternScript;
	class DtaCustomTagsHandler; // private class

	class DtaGlobalVariableData;

	class DtaProject : public DtaScriptVariable {
	public:
		enum TEXT_MODE { BINARY_MODE, DOS_MODE, UNIX_MODE };

	private:
		DtaScript*  _pScript;
		std::string _sVersion;
		bool _bVerbose;
		std::string _sTargetLanguage;
		mutable DtaBNFScript* _pCommonAlienParser;
		mutable DtaBNFScript* _pBNFAlienParser;
		mutable DtaBNFScript* _pTemplateAlienParser;
		mutable DtaBNFScript* _pTranslationAlienParser;
		std::string _sCodeWorkerHome;
		std::map<std::string, DtaGlobalVariableData*> _mapOfGlobalVariables;

		std::string _sMarkupKey;
		std::string _sMarkupValue;
		std::string _sWorkingPath;
		std::map<std::string, std::string> _mapOfDefinedTargets;
		std::string _sCommentBegin;
		std::string _sCommentEnd;
		std::string _sGenerationHeader;
		std::map<std::string, int> _mapOfWarnings;
		UtlTimer* _pDelayTimer;
		double _dLastDelay;
		GrfReadonlyHook* _pReadonlyHook;
		GrfWritefileHook* _pWritefileHook;
		bool _bSpeed;
		TEXT_MODE _iTextMode;
		std::map<std::string, DtaCustomTagsHandler*> _mapOfGenerationTagsHandlers;
		std::string _sCurrentGenerationTagsHandler;
		std::map<std::string, std::set<std::string> > _mapOfCapturedOutputFiles;
		std::map<std::string, std::set<std::string> > _mapOfCapturedInputFiles;
		std::string sFinalInfoOutputFilename_;
		int iFinalInfoFlag_;
		std::set<std::string> noWarnings_;
		std::string sRemoteDebugHost_;
		int iRemoteDebugPort_;

		static DtaProject* _pInstance;
		static std::list<DtaProject*> _stackOfProjects;

	public:
		enum FINAL_INFO_TYPE {
			FINAL_INFO_GENERATED_FILES = 1, FINAL_INFO_PARSED_FILES = 2,
			FINAL_INFO_TEMPLATE_COVERAGE = 4, FINAL_INFO_PARSE_COVERAGE = 8,
			FINAL_INFO_TEMPLATE_COVERAGE_FOR_EVERYBODY = 16, FINAL_INFO_PARSE_COVERAGE_FOR_EVERYBODY = 32
		};

	public:
		DtaProject();
		~DtaProject();

		void reset();

		inline static bool existInstance() { return (_pInstance != NULL); }
		inline static DtaProject& getInstance() { return *_pInstance; }
		static DtaProject& createRootInstance();

		inline DtaScript* getScript() const { return _pScript; }
		inline void setScript(DtaScript* pScript) { _pScript = pScript; }
		inline const std::string& getVersion() const { return _sVersion; }
		inline void setVersion(const std::string& sVersion) { _sVersion = sVersion; }
		inline bool getVerbose() const { return _bVerbose; }
		inline void setVerbose(bool bVerbose) { _bVerbose = bVerbose; }
		inline const std::string& getCodeWorkerHome() const { return _sCodeWorkerHome; }
		void setCodeWorkerHome(const std::string& sPath);
		inline const std::string& getMarkupKey() const { return _sMarkupKey; }
		inline void setMarkupKey(const std::string& sMarkupKey) { _sMarkupKey = sMarkupKey; }
		inline const std::string& getMarkupValue() const { return _sMarkupValue; }
		inline void setMarkupValue(const std::string& sMarkupValue) { _sMarkupValue = sMarkupValue; }
		inline const std::string& getWorkingPath() const { return _sWorkingPath; }
		inline void setWorkingPath(const std::string& sWorkingPath) { _sWorkingPath = sWorkingPath; }
		const std::string& getDefineTarget(const std::string& sTarget) const;
		void setDefineTarget(const std::string& sTarget, const std::string& sValue);
		inline void setMapOfDefinedTargets(const std::map<std::string, std::string>& mapOfDefinedTargets) { _mapOfDefinedTargets = mapOfDefinedTargets; }
		inline const std::string& getCommentBegin() const { return _sCommentBegin; }
		inline void setCommentBegin(const std::string& sCommentBegin) { _sCommentBegin = sCommentBegin; }
		inline const std::string& getCommentEnd() const { return _sCommentEnd; }
		inline void setCommentEnd(const std::string& sCommentEnd) { _sCommentEnd = sCommentEnd; }
		inline const std::string& getGenerationHeader() const { return _sGenerationHeader; }
		inline void setGenerationHeader(const std::string& sGenerationHeader) { _sGenerationHeader = sGenerationHeader; }
		inline UtlTimer* getDelayTimer() const { return _pDelayTimer; }
		inline void setDelayTimer(UtlTimer* pTimer) { _pDelayTimer = pTimer; }
		double getLastDelay() const;
		inline void setLastDelay(double dDelay) { _dLastDelay = dDelay; }
		inline GrfReadonlyHook* getReadonlyHook() const { return _pReadonlyHook; }
		inline void setReadonlyHook(GrfReadonlyHook* pReadonlyHook) { _pReadonlyHook = pReadonlyHook; }
		inline GrfWritefileHook* getWritefileHook() const { return _pWritefileHook; }
		inline void setWritefileHook(GrfWritefileHook* pWritefileHook) { _pWritefileHook = pWritefileHook; }
		inline bool getSpeed() const { return _bSpeed; }
		inline void setSpeed(bool bSpeed) { _bSpeed = bSpeed; }
		inline TEXT_MODE getTextMode() const { return _iTextMode; }
		void setTextMode(TEXT_MODE iTextMode);
		inline bool noWarning(const std::string& sWarningType) const { return noWarnings_.find(sWarningType) != noWarnings_.end(); }
		inline void addNoWarning(const std::string& sWarningType) { noWarnings_.insert(sWarningType); }
		inline const std::map<std::string, std::set<std::string> >& getCapturedOutputFiles() const { return _mapOfCapturedOutputFiles; }
		inline const std::map<std::string, std::set<std::string> >& getCapturedInputFiles() const { return _mapOfCapturedInputFiles; }
		void setFinalInfo(const std::string& sOutputFilename, int iFlag);
		inline const std::string& getFinalInfoOutputFilename() const { return sFinalInfoOutputFilename_; }
		inline int getFinalInfoFlag() const { return iFinalInfoFlag_; }
		void setRemoteDebug(const std::string& sHost, int iPort);
		inline const std::string& getRemoteDebugHost() const { return sRemoteDebugHost_; }
		inline int getRemoteDebugPort() const { return iRemoteDebugPort_; }

		inline const std::string& getTargetLanguage() const { return _sTargetLanguage; }
		inline bool hasTargetLanguage() const { return !_sTargetLanguage.empty(); }
		inline void setTargetLanguage(const std::string& sTargetLanguage) { _sTargetLanguage = sTargetLanguage; }
		static std::string getTargetScriptFilename(const std::string& sTargetLanguage, const std::string& sScriptShortFilename);
		DtaBNFScript* getCommonAlienParser() const;
		DtaBNFScript* getBNFAlienParser() const;
		DtaBNFScript* getTemplateAlienParser() const;
		DtaBNFScript* getTranslationAlienParser() const;

		DtaScriptVariable* getGlobalVariable(const std::string& sName) const;
		EXPRESSION_TYPE getGlobalVariableType(const std::string& sName) const;
		DtaScriptVariable* setGlobalVariable(const std::string& sName, EXPRESSION_TYPE variableType = NODE_EXPRTYPE);
		void setGlobalVariableType(const std::string& sName, EXPRESSION_TYPE variableType);

		void copyCommon(const DtaProject& project);
		std::string getTraceStack(ScpStream& script) const;
		void traceEngine() const;

		void prepareExecution();
		int addWarning(const std::string& sWarning);
		bool isScriptVersionOlderThan(int* tiVersion) const;

		bool addGenerationTagsHandler(const std::string& sKey, DtaBNFScript* pReader, DtaPatternScript* pWriter);
		bool removeGenerationTagsHandler(const std::string& sKey);
		bool existGenerationTagsHandler(const std::string& sKey) const;
		bool selectGenerationTagsHandler(const std::string& sKey);
		bool getGenerationTagsHandler(const std::string& sKey, DtaBNFScript*& pReader, DtaPatternScript*& pWriter) const;
		inline const std::string& getCurrentGenerationTagsHandler() const { return _sCurrentGenerationTagsHandler; }

		void captureOutputFile(const char* tcOutputFile, const char* tcTemplateScript);
		void captureInputFile(const char* tcInputFile, const char* tcBNFScript);
	};
}

#endif
