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

#ifndef _DtaScript_h_
#define _DtaScript_h_

#pragma warning(disable : 4786)

#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>

#include "DtaScriptFactory.h"
#include "GrfBlock.h"

namespace CodeWorker {
	class ScpStream;

	class DtaScriptVariable;
	class ExprScriptVariable;
	class ExprScriptFunction;
	class ExprScriptExpression;
	class ExprScriptMotif;
	class ExprScriptMotifStep;
	class ExprScriptMotifPath;
	class ExprScriptScriptFile;

	class EXECUTE_FUNCTION;

	class DtaBNFScript;
	class GrfForeach;


	class GrfScriptBlock : public GrfBlock {
		public:
			GrfScriptBlock(GrfBlock* pParentBlock) : GrfBlock(pParentBlock) {}

		protected:
			virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
	};

	class DtaScript {
	private:
		static bool _bParseWithSymbols;
		static std::vector<const char*> _listOfAllFilesAlreadyParsed;

		DtaScript* _pPreviousScript;
		std::string _sPreviousVersion;
		std::string _sPreviousMarkupKey;
		std::string _sPreviousMarkupValue;
		const char* _tcFilePtr;
		std::list<GrfBlock*> _stackOfParsedBlocks;
		bool _bNumericalExpression;
		std::set<std::string> _setOfIncludedFiles;


	protected:
		std::string _sCurrentFunctionBody;
		std::string _sCurrentFunctionTemplateBody;
		bool _bCurrentFunctionGenericTemplateKey;
		std::string _sTargetLanguage;

	protected:
		static std::map<std::string, EXECUTE_FUNCTION*> _mapOfScriptFunctions;
		static std::map<std::string, std::string> _mapOfSyntaxModes;
		static std::map<std::string, int> _mapOfReferenceMagicNumbers;

		std::map<int, int> _mapOfAttachedAreas;

		GrfScriptBlock _graph;
//		DtaScriptVariable* _pVisibility;

	public:
		DtaScript(/*DtaScriptVariable* pVisibility, */GrfBlock* pParentBlock = NULL, const char* sFilename = NULL);
		virtual ~DtaScript();

		static std::fstream* openOutputFile(const char* sFileName, bool bThrowOnError);
		static void registerScript(const char* sRegistration, EXECUTE_FUNCTION* executeFunction);
		static EXECUTE_FUNCTION* getRegisteredScript(const char* sRegistration);

		static inline bool requiresParsingInformation() { return _bParseWithSymbols; }
		static inline void requiresParsingInformation(bool bParseWithSymbols) { _bParseWithSymbols = bParseWithSymbols; }

		inline GrfBlock& getBlock() { return _graph; }
		void setFilename(const char* sFilename);
		inline void setFilenamePtr(const char* tcFilePtr) { _tcFilePtr = tcFilePtr; }
		inline const char* getFilenamePtr() const { return _tcFilePtr; }
		virtual DtaScriptFactory::SCRIPT_TYPE getType() const;
		virtual bool isAParseScript() const;
		virtual bool isAGenerateScript() const;

		virtual void traceEngine() const;

		void parseFile(const char* sScriptFileName, GrfBlock* pIncluded = NULL, bool bDebugMode = false, const std::string& sQuantifyFile = "");
		void parseFile(const char* sScriptName, const std::string& sCurrentDirectoryAtCompileTime);
		void parseStream(ScpStream& myStream, bool bDebugMode = false, const std::string& sQuantifyFile = "");
		void parseBlock(ScpStream& script, GrfBlock& block);
		virtual SEQUENCE_INTERRUPTION_LIST execute(DtaScriptVariable& thisContext);

		ExprScriptVariable* parseVariableExpression(GrfBlock& block, ScpStream& script, ExprScriptVariable* pParentExpr = NULL);
		ExprScriptExpression* parseExpression(GrfBlock& block, ScpStream& script);
		ExprScriptScriptFile* parseScriptFileExpression(GrfBlock& block, ScpStream& script, DtaScriptFactory::SCRIPT_TYPE eScriptType);

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment, const std::string& sScriptFilename) const;
		virtual void compileCppHeaderIncludes(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual void compileCppFunctions(CppCompilerEnvironment& theCompilerEnvironment) const;

		static std::string convertFilenameAsIdentifier(const std::string& sFilename);
		static bool equalsIgnoringGenerationHeader(ScpStream& theInputStream, ScpStream& theOutputStream, int& iPosition);

	protected:
		void allocateLocalParameters();
		void freeLocalParameters();

		virtual void traceInternalEngine() const;

		virtual std::map<std::string, int>& getParsingFunctionRegister();

		void parseScript(ScpStream& script, GrfBlock& block);
		virtual void parseInstruction(ScpStream& script, GrfBlock& block);

		virtual void beforeParsingABlock(ScpStream& /*script*/, GrfBlock& /*block*/);
		virtual void afterParsingABlock(ScpStream& /*script*/, GrfBlock& /*block*/);
		bool parseKeyword(const std::string& sCommand, ScpStream& script, GrfBlock& block, ExprScriptVariable* pMethodCaller = NULL);

		virtual bool betweenCommands(ScpStream& /*script*/, GrfBlock& /*block*/);
		virtual void handleNotAWordCommand(ScpStream& script, GrfBlock& block);
		virtual void parsePreprocessorDirective(const std::string& sDirective, ScpStream& script, GrfBlock& block);
		virtual void handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block);
//##markup##"special procedures"
//##begin##"special procedures"
		virtual void parseAttachInputToSocket(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseDetachInputFromSocket(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseGoBack(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseSetInputLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseAllFloatingLocations(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseAttachOutputToSocket(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseDetachOutputFromSocket(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseIncrementIndentLevel(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseInsertText(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseInsertTextOnce(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseInsertTextToFloatingLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseInsertTextOnceToFloatingLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseOverwritePortion(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parsePopulateProtectedArea(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseResizeOutputStream(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseSetFloatingLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseSetOutputLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseSetProtectedArea(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseWriteBytes(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseWriteText(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
		virtual void parseWriteTextOnce(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/);
//##end##"special procedures"
		virtual void parseGeneratedFile(GrfBlock& /*block*/, ScpStream& script);
		virtual void parseGeneratedString(GrfBlock& /*block*/, ScpStream& script);
		virtual void parseAppendedFile(GrfBlock& /*block*/, ScpStream& script);

		static int computeReferenceMagicNumber(ScpStream& script, const char* tcFilename, const std::string& sKey);
		static int computeReferenceMagicNumber(ScpStream& theStream, const std::string& sKey);
		static bool extractReferenceMagicNumber(const std::string& sReferenceFile, const std::string& sKey, int& iMagicNumber);

		inline bool hasTargetLanguage() const { return !_sTargetLanguage.empty(); }
		inline const std::string& getTargetLanguage() const { return _sTargetLanguage; }
		virtual DtaBNFScript& getAlienParser() const;
		void parseAlienInstructions(DtaScriptVariable& parseTree, ScpStream& script, GrfBlock& block);
		void parseAlienScript(ScpStream& script, GrfBlock& block);
		void parseAlienBlock(ScpStream& script, GrfBlock& block);
		void parseAlienInstruction(ScpStream& script, GrfBlock& block);
		ExprScriptVariable* parseAlienVariableExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseAlienExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseAlienExpression(DtaScriptVariable& parseTree, ScpStream& script, GrfBlock& block);

	protected:
		void parseEmbeddedScript(ScpStream& script);
		ExprScriptExpression* parseConstantNodeExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseAssignmentExpression(GrfBlock& block, ScpStream& script);

		ExprScriptExpression* parseConcatenationExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseKeyTemplateExpression(GrfBlock& block, ScpStream& script);
		void parseDeclare(GrfBlock& block, ScpStream& script);
		void parseExternal(GrfBlock& block, ScpStream& script);
		EXPRESSION_TYPE parseVariableType(GrfBlock& block, ScpStream& script);
		GrfFunction* parseFunction(GrfBlock& block, ScpStream& script, bool bPrototypeOnly = false);
		void parseReadonlyHook(GrfBlock& block, ScpStream& script);
		void parseWritefileHook(GrfBlock& block, ScpStream& script);
		EXPRESSION_TYPE parseClassType(GrfBlock& block, ScpStream& script, EXPRESSION_TYPE exprType);
		void parseForeachListDeclaration(GrfBlock& block, ScpStream& script, GrfForeach* pForeach);

	private:
		void parseForeach(GrfBlock& block, ScpStream& script);
		void parseForfile(GrfBlock& block, ScpStream& script);
		void parseSelect(GrfBlock& block, ScpStream& script);
		void parseDo(GrfBlock& block, ScpStream& script);
		void parseWhile(GrfBlock& block, ScpStream& script);
		void parseSwitch(GrfBlock& block, ScpStream& script);
		void parseIfThenElse(GrfBlock& block, ScpStream& script);
		void parseTryCatch(GrfBlock& block, ScpStream& script);
		void parseContinue(GrfBlock& block, ScpStream& script);
		void parseBreak(GrfBlock& block, ScpStream& script);
		void parseFinally(GrfBlock& block, ScpStream& script);
		void parseReturn(GrfBlock& block, ScpStream& script);
		void parseExit(GrfBlock& block, ScpStream& script);
		void parseDelay(GrfBlock& block, ScpStream& script);
		void parseNewProject(GrfBlock& block, ScpStream& script);
		void parseDebug(GrfBlock& block, ScpStream& script);
		void parseQuantify(GrfBlock& block, ScpStream& script);
		void parseQuiet(GrfBlock& block, ScpStream& script);
		void parseFileAsStandardInput(GrfBlock& block, ScpStream& script);
		void parseStringAsStandardInput(GrfBlock& block, ScpStream& script);
		void parseNop(GrfBlock& block, ScpStream& script);
		void parseLocalVariable(GrfBlock& block, ScpStream& script);
		void parseLocalReference(GrfBlock& block, ScpStream& script);
		void parseGlobalVariable(GrfBlock& block, ScpStream& script);
		void parseSetAssignment(GrfBlock& block, ScpStream& script);
		void parseInsertAssignment(GrfBlock& block, ScpStream& script);
		void parseSetAll(GrfBlock& block, ScpStream& script);
		void parseMerge(GrfBlock& block, ScpStream& script);
		void parseReference(GrfBlock& block, ScpStream& script);
		void parsePushItem(GrfBlock& block, ScpStream& script);
		void parseSyntax(GrfBlock& block, ScpStream& script);
		DtaClass* parseClass(GrfBlock& block, ScpStream& script);
		void checkIfAllowedFunction(ScpStream& script, ExprScriptFunction& theFunction);
//##markup##"common procedures"
//##begin##"common procedures"
		void parseAppendFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseAutoexpand(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseClearVariable(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseCompileToCpp(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseCopyFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseCopyGenerableFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseCopySmartDirectory(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseCutString(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseEnvironTable(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseError(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseExecuteString(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseExpand(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseExtendExecutedScript(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseGenerate(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseGenerateString(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseInsertElementAt(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseInvertArray(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseListAllGeneratedFiles(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseLoadProject(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseOpenLogFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseParseAsBNF(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseParseStringAsBNF(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseParseFree(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseProduceHTML(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parsePutEnv(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseRandomSeed(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseRemoveAllElements(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseRemoveElement(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseRemoveFirstElement(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseRemoveLastElement(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseRemoveRecursive(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseRemoveVariable(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSaveBinaryToFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSaveProject(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSaveProjectTypes(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSaveToFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetCommentBegin(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetCommentEnd(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetGenerationHeader(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetIncludePath(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetNow(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetProperty(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetTextMode(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetVersion(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetWriteMode(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSetWorkingPath(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSleep(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSlideNodeContent(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseSortArray(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseTraceEngine(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseTraceLine(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseTraceObject(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseTraceStack(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseTraceText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseTranslate(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		void parseCloseSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
//##end##"common procedures"
		void parseParsedFile(GrfBlock& block, ScpStream& script);
		void parseParsedString(GrfBlock& block, ScpStream& script);

		ExprScriptExpression* parsePreprocessorExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseConstantLiteral(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseLiteralUnsignedExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseLiteralExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseFactorExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseShiftExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseArithmeticExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseComparisonExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseTernaryExpression(GrfBlock& block, ScpStream& script);
		ExprScriptExpression* parseBooleanExpression(GrfBlock& block, ScpStream& script);
		ExprScriptVariable* parseIndexExpression(GrfBlock& /*block*/, ScpStream& script);
		ExprScriptVariable* parseReferenceExpression(GrfBlock& /*block*/, ScpStream& script);
		ExprScriptFunction* parseFunctionExpression(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller, ExprScriptFunction* pFunction, ExprScriptExpression* pTemplate, bool bBracket);
		ExprScriptVariable* parseVariableExpression(GrfBlock& block, ScpStream& script, ExprScriptVariable* pParentExpr, ExprScriptExpression*& pMethodExpr, GrfCommand*& pMethodProc);

		ExprScriptMotifStep* parseMotifStepExpression(GrfBlock& block, ScpStream& script, ExprScriptMotifPath*& pPathExpr);
		ExprScriptMotifPath* parseMotifPathExpression(GrfBlock& block, ScpStream& script);
		ExprScriptMotif* parseMotifConcatExpression(GrfBlock& block, ScpStream& script);
		ExprScriptMotif* parseMotifAndExpression(GrfBlock& block, ScpStream& script);
		ExprScriptMotif* parseMotifExpression(GrfBlock& block, ScpStream& script);
	};
}

#endif
