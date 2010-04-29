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

#ifndef _DtaPatternScript_h_
#define _DtaPatternScript_h_

#include <map>
#include <memory>

#include "DtaScript.h"

namespace CodeWorker {
	class ScpStream;
	class DtaProtectedAreasBag;
	class DtaPatternScript;
	class DtaProject;
	class DtaAspectAdvices;
	class GrfJointPoint;
	class GrfAspectAdvice;

	class GrfText;
	class GrfSetFloatingLocation;

	class DtaOutputFile {
	private:
		static int _iNumberOfTemporaryFiles;
		DtaPatternScript* _pPatternScript;
		DtaOutputFile* _pOldOutputFile;
		mutable DtaAspectAdvices* _pAspectAdvices;
		ScpStream* _pOutputStream;
		ScpStream* _pInputStream;
		std::string _sTemporaryDirectory;
		DtaProtectedAreasBag* _pProtectedAreasBag;
		DtaScriptVariable* _pOutputCoverage;

	public:
		DtaOutputFile(DtaPatternScript* pPatternScript);
		~DtaOutputFile();

		inline ScpStream* getOutputStream() const { return _pOutputStream; }
		inline ScpStream* getInputStream() const { return _pInputStream; }
		inline void setInputStream(ScpStream* pInputStream) { _pInputStream = pInputStream; }
		inline DtaProtectedAreasBag& getProtectedAreasBag() const { return *_pProtectedAreasBag; }
		inline DtaScriptVariable* const* getOutputCoveragePtr() const { return &_pOutputCoverage; }
		void insertAspectAdvice(GrfAspectAdvice* pAdvice);

		void prepareCoverage(DtaScriptVariable& thisContext, int iTargetLocation);

		SEQUENCE_INTERRUPTION_LIST generate(const char* tcFile, DtaScriptVariable& thisContext);
		SEQUENCE_INTERRUPTION_LIST generateString(std::string& sOutput, DtaScriptVariable& thisContext);
		SEQUENCE_INTERRUPTION_LIST expand(const char* tcFile, DtaScriptVariable& thisContext);

		void captureOutputFile(const char* tcFile);
		ScpStream* openGenerate(bool bAutomatic, const char* sFile, ScpStream*& pOldOutputStream);
		ScpStream* openAppend(bool bAutomatic, const char* sFile, ScpStream*& pOldOutputStream);
		std::string catchGenerateExecution(bool bAutomatic, ScpStream* pOldOutputStream, UtlException* pCatchedException);
		std::string closeGenerate(bool bAutomatic, const char* sFile, ScpStream* pOldOutputStream);

		std::list<std::string> getProtectionKeys() const;
		std::list<std::string> remainingProtectionKeys() const;
		void populateProtectedArea(const std::string& sProtectedAreaName, const std::string& sContent);
		void setProtectedArea(const std::string& sProtectedAreaName);
		bool removeProtectedArea(const std::string& sProtectedAreaName);

		bool weaveBeforeIteration(GrfJointPoint& jointPoint, DtaScriptVariable& context);
		bool weaveAroundIteration(GrfJointPoint& jointPoint, DtaScriptVariable& context);
		bool weaveAfterIteration(GrfJointPoint& jointPoint, DtaScriptVariable& context);
		bool weaveBefore(GrfJointPoint& jointPoint, DtaScriptVariable& context);
		bool weaveAround(GrfJointPoint& jointPoint, DtaScriptVariable& context);
		bool weaveAfter(GrfJointPoint& jointPoint, DtaScriptVariable& context);

		GrfText* createText(ScpStream& stream, const char* sText) const;

	private:
		DtaOutputFile();
		DtaOutputFile(const DtaOutputFile&);

		void openAndloadProtectedCode(const char* sFile);
		void saveProtectedCode();
		void saveProtectedCodeAndClose();
		void copyFromTemporaryFile(const char* sFile);

		bool createCoverageTreeForFinalInfo(const std::string& sAbsolutePath);
		void restoreCoverageTree(DtaScriptVariable& thisContext, bool bCreateCoverageTreeForFinalInfo, const std::string& sAbsolutePath);

		std::string getProtectedAreaIndentation() const;
		void writeStandardArea(const std::string& sAreaCode);
		void writeTargetedArea(const std::string& sAreaCode, const std::string& sAreaTarget);
	};

	
	class DtaPatternScript : public DtaScript {
	private:
		bool _bExecMode;
		int _iExecStartLocation;
		DtaOutputFile* _pOutputFile;
		EXECUTE_FUNCTION* _executeFunction;

	protected:
		ExprScriptVariable* _pCoverageTree;

	public:
		DtaPatternScript(bool bExecMode = false);
		DtaPatternScript(/*DtaScriptVariable* pVisibility, */GrfBlock* pParentBlock, bool bExecMode = false);
		DtaPatternScript(EXECUTE_FUNCTION* executeFunction, bool bExecMode = false);
		virtual ~DtaPatternScript();

		inline void setExecMode(bool bExecMode) { _bExecMode = bExecMode; }

		virtual DtaScriptFactory::SCRIPT_TYPE getType() const;
		virtual bool isAGenerateScript() const;
		inline DtaProtectedAreasBag& getProtectedAreasBag() const { return _pOutputFile->getProtectedAreasBag(); }

		inline ScpStream* getOutputStream() const { return _pOutputFile->getOutputStream(); }
		inline ScpStream* getInputStream() const { return _pOutputFile->getInputStream(); }

		virtual void traceEngine() const;

		virtual SEQUENCE_INTERRUPTION_LIST execute(DtaScriptVariable& thisContext);
		SEQUENCE_INTERRUPTION_LIST executeExpansion(const std::string& sScriptContent, DtaScriptVariable& thisContext, int iTargetLocation);

		virtual SEQUENCE_INTERRUPTION_LIST generate(const char* sFile, DtaScriptVariable& thisContext) { return _pOutputFile->generate(sFile, thisContext); }
		virtual SEQUENCE_INTERRUPTION_LIST generateString(std::string& sOutput, DtaScriptVariable& thisContext) { return _pOutputFile->generateString(sOutput, thisContext); }
		virtual SEQUENCE_INTERRUPTION_LIST expand(const char* sFile, DtaScriptVariable& thisContext) { return _pOutputFile->expand(sFile, thisContext); }
		virtual SEQUENCE_INTERRUPTION_LIST autoexpand(const char* sFile, DtaScriptVariable& thisContext) { return _pOutputFile->expand(sFile, thisContext); }

		inline ScpStream* openGenerate(bool bAutomatic, const char* sFile, ScpStream*& pOldOutputStream) { return _pOutputFile->openGenerate(bAutomatic, sFile, pOldOutputStream); }
		inline void catchGenerateExecution(bool bAutomatic, ScpStream* pOldOutputStream, UtlException* pCatchedException) { _pOutputFile->catchGenerateExecution(bAutomatic, pOldOutputStream, pCatchedException); }
		inline std::string closeGenerate(bool bAutomatic, const char* sFile, ScpStream* pOldOutputStream) { return _pOutputFile->closeGenerate(bAutomatic, sFile, pOldOutputStream); }

		inline std::list<std::string> getProtectionKeys() const { return _pOutputFile->getProtectionKeys(); }
		inline std::list<std::string> remainingProtectionKeys() const { return _pOutputFile->remainingProtectionKeys(); }
		inline void populateProtectedArea(const std::string& sProtectedAreaName, const std::string& sContent) { _pOutputFile->populateProtectedArea(sProtectedAreaName, sContent); }
		inline void setProtectedArea(const std::string& sProtectedAreaName) { _pOutputFile->setProtectedArea(sProtectedAreaName); }
		inline bool removeProtectedArea(const std::string& sProtectedAreaName) { return _pOutputFile->removeProtectedArea(sProtectedAreaName); }

		inline bool weaveBeforeIteration(GrfJointPoint& jointPoint, DtaScriptVariable& context) { return _pOutputFile->weaveBeforeIteration(jointPoint, context); }
		inline bool weaveAroundIteration(GrfJointPoint& jointPoint, DtaScriptVariable& context) { return _pOutputFile->weaveAroundIteration(jointPoint, context); }
		inline bool weaveAfterIteration(GrfJointPoint& jointPoint, DtaScriptVariable& context) { return _pOutputFile->weaveAfterIteration(jointPoint, context); }
		inline bool weaveBefore(GrfJointPoint& jointPoint, DtaScriptVariable& context) { return _pOutputFile->weaveBefore(jointPoint, context); }
		inline bool weaveAround(GrfJointPoint& jointPoint, DtaScriptVariable& context) { return _pOutputFile->weaveAround(jointPoint, context); }
		inline bool weaveAfter(GrfJointPoint& jointPoint, DtaScriptVariable& context) { return _pOutputFile->weaveAfter(jointPoint, context); }

	protected:
		virtual bool betweenCommands(ScpStream& script, GrfBlock& block);
		virtual void handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block);
		virtual void handleNotAWordCommand(ScpStream& script, GrfBlock& block);
		virtual void parsePreprocessorDirective(const std::string& sDirective, ScpStream& script, GrfBlock& block);
		virtual void parseJointPoint(GrfBlock& block, ScpStream& script);
		virtual void parseAdvice(GrfBlock& block, ScpStream& script);
		virtual void parseGeneratedFile(GrfBlock& block, ScpStream& script);
		virtual void parseGeneratedString(GrfBlock& block, ScpStream& script);
		virtual void parseAppendedFile(GrfBlock& block, ScpStream& script);
//##markup##"parsing"
//##begin##"parsing"
		virtual void parseAllFloatingLocations(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseAttachOutputToSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseDetachOutputFromSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseIncrementIndentLevel(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseInsertText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseInsertTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseInsertTextToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseInsertTextOnceToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseOverwritePortion(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parsePopulateProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseResizeOutputStream(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseSetFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseSetOutputLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseSetProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseWriteBytes(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseWriteText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseWriteTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
//##end##"parsing"

		virtual DtaBNFScript& getAlienParser() const;

	private:
		bool handleEmbeddedExpression(ScpStream& script, GrfBlock& block, std::auto_ptr<ExprScriptExpression>& pExpr);
		SEQUENCE_INTERRUPTION_LIST executeLight(DtaScriptVariable& thisContext);

		friend class DtaOutputFile;
	};
}

#endif
