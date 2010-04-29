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

#ifndef _DtaBNFScript_h_
#define _DtaBNFScript_h_

#include <list>
#include <map>

#include "ExprScriptFunction.h"
#include "DtaPatternScript.h"
#include "BNFIgnore.h" // for the enum 'IGNORE_MODE'


#define BNF_SYMBOL_HAS_FAILED \
	if (_bContinue) CGRuntime::throwBNFExecutionError(toString());\
	CGRuntime::setInputLocation(iLocation);\
	if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);\
	return BREAK_INTERRUPTION;


#define CPP_COMPILER_BNF_SYMBOL_BEGIN \
	int iCursor = theCompilerEnvironment.newCursor();\
	char tcLocation[64];\
	sprintf(tcLocation, "_compilerClauseLocation_%d", iCursor);\
	char tcImplicitCopy[64];\
	sprintf(tcImplicitCopy, "_compilerClauseImplicitCopy_%d", iCursor);\
	theCompilerEnvironment.getBody() << theCompilerEnvironment.getIndentation() << "// " << toString();\
	theCompilerEnvironment.getBody().endl();\
	theCompilerEnvironment.getBody() << theCompilerEnvironment.getIndentation() << "int " << tcLocation << " = CGRuntime::getInputLocation();";\
	theCompilerEnvironment.getBody().endl();\
	theCompilerEnvironment.getBody() << theCompilerEnvironment.getIndentation() << "int " << tcImplicitCopy << " = theEnvironment.skipEmptyChars();";\
	theCompilerEnvironment.getBody().endl()

#define CPP_COMPILER_BNF_SYMBOL_HAS_FAILED \
	if (_bContinue) {\
		theCompilerEnvironment.getBody() << theCompilerEnvironment.getIndentation() << "\tCGRuntime::throwBNFExecutionError(";\
		theCompilerEnvironment.getBody().writeString(toString());\
		theCompilerEnvironment.getBody() << ");";\
	} else {\
		theCompilerEnvironment.getBody() << theCompilerEnvironment.getIndentation() << "\tCGRuntime::setInputLocation(" << tcLocation << ");";\
		theCompilerEnvironment.getBody().endl();\
		theCompilerEnvironment.getBody() << theCompilerEnvironment.getIndentation() << "\tif (" << tcImplicitCopy << " >= 0) CGRuntime::resizeOutputStream(" << tcImplicitCopy << ");";\
		theCompilerEnvironment.getBody().endl();\
		theCompilerEnvironment.getBody() << theCompilerEnvironment.getIndentation() << "\t_compilerClauseSuccess = false;";\
	}\
	theCompilerEnvironment.getBody().endl()



namespace CodeWorker {
	class ExprScriptVariable;
	class DtaScriptVariable;
	class BNFClause;
	class BNFClauseCall;
	class BNFStepper;
	class BNFStepintoHook;
	class BNFStepoutHook;

	class BNFTransformRules; // defined in "DtaBNFScript.cpp"
	struct TemporaryMatchingStorage; // defined in "DtaBNFScript.cpp"

	struct BNFClauseMatchingArea;
	struct BNFMultiplicityBoundaries;


	class DtaBNFScript : public DtaPatternScript {
	private:
		bool _bBNFMode;
		std::string _sParsedFileOrContent; // just for passing the parameters to 'execute()'
		bool _bParsedFile;
		ScpStream* _pStream;
		std::map<std::string, std::list<BNFClause*> > _listOfClauses;
		std::list<BNFTransformRules*> _listOfTransformRules;
		IGNORE_MODE _eIgnoreMode;
		BNFClause* _pIgnoreClause;
		std::map<std::string, BNFClause*> _mapOfIgnoreClauses;
		bool _bNoCase;
		int _iRatchetPosition;

		BNFStepintoHook* _pStepintoHook;
		BNFStepoutHook* _pStepoutHook;
		bool _bTrace;
		std::string _sTraceIndentation;

		ExprScriptVariable* _pMatchingAreasContainer;
		ExprScriptVariable* _pASTContainer;
		BNFClauseMatchingArea* _pParentClauseMatching;

	protected:
		bool _bImplicitCopy;
		ExprScriptFunction* _pImplicitCopyFunction;

	public:
		DtaBNFScript();
		DtaBNFScript(/*DtaScriptVariable* pVisibility, */GrfBlock* pParentBlock);
		DtaBNFScript(EXECUTE_FUNCTION* executeFunction);
		virtual ~DtaBNFScript();

		inline const std::map<std::string, std::list<BNFClause*> >& getClauses() const { return _listOfClauses; }

		inline void setBNFMode(bool bBNFMode) { _bBNFMode = bBNFMode; }
		inline IGNORE_MODE getIgnoreMode() const { return _eIgnoreMode; }
		inline void setIgnoreMode(IGNORE_MODE eMode) { _eIgnoreMode = eMode; }
		inline BNFClause* getIgnoreClause() const { return _pIgnoreClause; }
		inline void setIgnoreClause(BNFClause* pIgnoreClause) {
			_pIgnoreClause = pIgnoreClause;
		}
		inline bool implicitCopy() const { return _bImplicitCopy; }
		inline void implicitCopy(bool bImplicitCopy) { _bImplicitCopy = bImplicitCopy; }
		inline ExprScriptFunction* getImplicitCopyFunction() const { return _pImplicitCopyFunction; }
		inline void setImplicitCopyFunction(ExprScriptFunction* pFunction) { _pImplicitCopyFunction = pFunction; }
		inline int getRatchetPosition() const { return _iRatchetPosition; }
		inline void setRatchetPosition(int iPosition) { _iRatchetPosition = iPosition; }
		inline bool trace() const { return _bTrace; }
		inline void incrementTraceIndentation() { _sTraceIndentation += " "; }
		inline void decrementTraceIndentation() { _sTraceIndentation = _sTraceIndentation.substr(1); }
		inline const std::string& traceIndentation() const { return _sTraceIndentation; }
		inline BNFStepintoHook* getStepintoHook() const { return _pStepintoHook; }
		inline void setStepintoHook(BNFStepintoHook* pHook) { _pStepintoHook = pHook; }
		inline BNFStepoutHook* getStepoutHook() const { return _pStepoutHook; }
		inline void setStepoutHook(BNFStepoutHook* pHook) { _pStepoutHook = pHook; }

		inline ExprScriptVariable* getMatchingAreasContainer() const { return _pMatchingAreasContainer; }
		inline ExprScriptVariable* getASTContainer() const { return _pASTContainer; }
		inline bool hasCoverageRecording() const { return (_pMatchingAreasContainer != NULL) || (_pASTContainer != NULL); }
		inline BNFClauseMatchingArea* getParentClauseMatching() const { return _pParentClauseMatching; }
		inline void setParentClauseMatching(BNFClauseMatchingArea* pParentClauseMatching) { _pParentClauseMatching = pParentClauseMatching; }
		void storeMatchingAreas(DtaScriptVariable& thisContext, DtaScriptVariable* pStorage = NULL);
		void storeAST(DtaScriptVariable& thisContext, ScpStream& inputStream);

		virtual DtaScriptFactory::SCRIPT_TYPE getType() const;
		virtual bool isAParseScript() const;
		virtual bool isAGenerateScript() const;

		virtual void traceEngine() const;

		int skipEmptyChars(DtaScriptVariable& visibility);
		void writeBinaryData(const char* tcText, int iLength);

		virtual SEQUENCE_INTERRUPTION_LIST generate(const char* sFile, DtaScriptVariable& thisContext);
		virtual SEQUENCE_INTERRUPTION_LIST generate(ScpStream& stream, DtaScriptVariable& thisContext);
		virtual SEQUENCE_INTERRUPTION_LIST generateString(std::string& sContent, DtaScriptVariable& thisContext);
		virtual SEQUENCE_INTERRUPTION_LIST execute(DtaScriptVariable& thisContext);

		virtual void compileCppHeaderIncludes(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual void compileCppFunctions(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual void compileCppBNFAssignment(CppCompilerEnvironment& theCompilerEnvironment, int iClauseReturnType, ExprScriptVariable& variableToAssign, bool bConcatVariable, const char* tcText);

		static std::string assignmentToString(ExprScriptVariable* pVariableToAssign, bool bConcatenateVariable);
		static std::string constantsToString(const std::vector<std::string>& listOfConstants);

		static std::string IGNORE_MODEtoString(IGNORE_MODE eMode);
	protected:
		virtual bool betweenCommands(ScpStream& script, GrfBlock& block);
		virtual void handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block);
		virtual void handleNotAWordCommand(ScpStream& script, GrfBlock& block);
		virtual void parseBNFPreprocessorDirective(int iStatementBeginning, const std::string& sDirective, ScpStream& script, GrfBlock& block);
		virtual void handleUnknownTokenDirective(const std::string& sDirective, ScpStream& script, BNFClause& rule, GrfBlock& block, bool& bContinue, bool& bNoCase, bool bLiteralOnly, BNFStepper* pStepper);

		virtual void traceInternalEngine() const;

		virtual void parsePreprocessorDirective(const std::string& sDirective, ScpStream& script, GrfBlock& block);
		BNFClause& parseBNFClause(ScpStream& script, GrfBlock& block, int iStatementBeginning, const std::string& sClauseName, bool bOverload);
		virtual void parseInstruction(ScpStream& script, GrfBlock& block);
		void parseStepintoHook(GrfBlock& block, ScpStream& script);
		void parseStepoutHook(GrfBlock& block, ScpStream& script);

		bool parseBNFLitteral(ScpStream& script, BNFClause& rule, GrfBlock& block, bool& bContinue, bool& bNoCase, bool bLiteralOnly, BNFStepper* pStepperRE);
//##markup##"parsing"
//##begin##"parsing"
		virtual void parseAttachInputToSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseDetachInputFromSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseGoBack(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseSetInputLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
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
		void storeClauseMatching(DtaScriptVariable& ruleNames, std::map<int, std::map<int, std::list<TemporaryMatchingStorage*> > >& mapOfAreas, BNFClauseMatchingArea* pClauseMatching);

		BNFClause& buildClause(ScpStream& script, GrfBlock& parent, const std::string& sName, const std::string& sTemplateKey, bool bGenericKey, int iReturnType, const std::vector<std::string>& listOfParameters, const std::vector<EXPRESSION_TYPE>& listOfParameterTypes, bool bOverload);
		BNFClause& buildClause(ScpStream& script, GrfBlock& parent, const std::string& sName, unsigned int iArity);

		void parseBNFDisjunction(ScpStream& script, BNFClause& rule, GrfBlock& block, bool bNoCase, BNFStepper* pStepperRE);
		void parseBNFConjunction(ScpStream& script, BNFClause& rule, GrfBlock& block, bool bContinue, bool bNoCase, BNFStepper* pStepperRE);
		void parseBNFSequence(ScpStream& script, BNFClause& rule, GrfBlock& block, bool bContinue, bool bNoCase, BNFStepper* pStepperRE);
		ExprScriptVariable* parseBNFVariableExpression(GrfBlock& block, ScpStream& script);
		ExprScriptVariable* parseLiteralAssignment(ScpStream& script, GrfBlock& block, std::vector<std::string>& listOfConstants, bool& bConcatVariable, bool bVariableAllowed);
		BNFClauseCall* parseBNFClauseCall(ScpStream& script, GrfBlock& block, const std::string& sClauseCallName, bool bContinue, bool bNoCase);
		BNFMultiplicityBoundaries* parseMultiplicity(ScpStream& script, GrfBlock& block, BNFMultiplicityBoundaries* boundaries = NULL);
		void parseAndPropagateParameters(ScpStream& script, GrfBlock& block);
		void parseTransformRules(ScpStream& script, GrfBlock& block);

		IGNORE_MODE parseIgnoreMode(ScpStream& script, BNFClause*& pPreprocessingIgnoreClause);
	};
}

#endif
