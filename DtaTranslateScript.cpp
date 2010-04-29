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

#include "UtlException.h"
#include "ScpStream.h"
#include "CGRuntime.h"
#include "DtaProject.h"

#include "ExprScriptFunction.h"
#include "ExprScriptVariable.h"
#include "BNFClause.h"
#include "BNFTransformationMode.h"
#include "DtaTranslateScript.h"


namespace CodeWorker {
	DtaTranslateScript::~DtaTranslateScript() {}

	DtaScriptFactory::SCRIPT_TYPE DtaTranslateScript::getType() const { return DtaScriptFactory::TRANSLATE_SCRIPT; }
	bool DtaTranslateScript::isAParseScript() const { return true; }
	bool DtaTranslateScript::isAGenerateScript() const { return true; }

	void DtaTranslateScript::traceEngine() const {
		if (getFilenamePtr() == NULL) CGRuntime::traceLine("translation script (no filename):");
		else CGRuntime::traceLine("translation script \"" + std::string(getFilenamePtr()) + "\":");
		traceInternalEngine();
	}

	DtaBNFScript& DtaTranslateScript::getAlienParser() const {
		DtaBNFScript* pScript = DtaProject::getInstance().getTranslationAlienParser();
		if (pScript == NULL) throw UtlException("compiling a translation script in " + _sTargetLanguage + " requires \"($CODEWORKER_HOME)/" + _sTargetLanguage + "/default-scripts/" + _sTargetLanguage + "TranslationScript.cwp\"");
		return *pScript;
	}

	SEQUENCE_INTERRUPTION_LIST DtaTranslateScript::translate(const std::string& sInputFile, const std::string& sOutputFile, DtaScriptVariable& thisContext) {
		// absolute path of the parsed file:
		// avoid ambiguities
		std::string sAbsolutePath;
		if (ScpStream::existInputFileFromIncludePath(sOutputFile.c_str(), sAbsolutePath)) {
			if (!ScpStream::existVirtualFile(sOutputFile)) {
				sAbsolutePath = CGRuntime::canonizePath(sAbsolutePath);
			}
		} else {
			sAbsolutePath = CGRuntime::canonizePath(sOutputFile);
		}
		_sOutputFile = sAbsolutePath;
		_bGeneratedFile = true;
		SEQUENCE_INTERRUPTION_LIST result = generate(sInputFile.c_str(), thisContext);
		if (getFilenamePtr() != NULL) {
			DtaProject::getInstance().captureOutputFile(sAbsolutePath.c_str(), getFilenamePtr());
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST DtaTranslateScript::translateString(const std::string& sInputString, std::string& sOutputString, DtaScriptVariable& thisContext) {
		_sOutputFile = sInputString;
		_bGeneratedFile = false;
		SEQUENCE_INTERRUPTION_LIST result = DtaBNFScript::generateString(_sOutputFile, thisContext);
		if (result == NO_INTERRUPTION) sOutputString = _sOutputFile;
		_sOutputFile = "";
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST DtaTranslateScript::execute(DtaScriptVariable& thisContext) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		ScpStream* pOldOutputStream;
		bool bCreateCoverageTreeForFinalInfo = false;
		if (_bGeneratedFile) {
			if ((_pCoverageTree == NULL) && ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_TEMPLATE_COVERAGE_FOR_EVERYBODY) != 0)) {
				// create a global variable expression for coverage recording
				// when final info requires coverage on generated files;
				// the variable expression points to an item whose key is the name
				// of the generated file.
				if (DtaProject::getInstance().getGlobalVariable("_FINAL_INFO_TEMPLATE_COVERAGE") == NULL) {
					DtaProject::getInstance().setGlobalVariable("_FINAL_INFO_TEMPLATE_COVERAGE");
				}
				_pCoverageTree = new ExprScriptVariable("_FINAL_INFO_TEMPLATE_COVERAGE");
				_pCoverageTree->setArrayKey(new ExprScriptConstant(_sOutputFile.c_str()));
				bCreateCoverageTreeForFinalInfo = true;
			}
			openGenerate(true, _sOutputFile.c_str(), pOldOutputStream);
		} else {
			openGenerate(true, NULL, pOldOutputStream);
		}
		try {
			result = DtaBNFScript::execute(thisContext);
		} catch(UtlException& exception) {
			catchGenerateExecution(true, pOldOutputStream, &exception);
		} catch(std::exception&) {
			catchGenerateExecution(true, pOldOutputStream, 0);
			throw;
		}
		if (_bGeneratedFile) {
			closeGenerate(true, _sOutputFile.c_str(), pOldOutputStream);
			if (_pCoverageTree != NULL) {
				if (bCreateCoverageTreeForFinalInfo) {
					delete _pCoverageTree;
					_pCoverageTree = NULL;
				} else if ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_TEMPLATE_COVERAGE) != 0) {
					//yes, the coverage tree must be copied for 'final info'
					DtaScriptVariable* pOutputCoverage = thisContext.getVariable(*_pCoverageTree);
					if (pOutputCoverage != NULL) {
						DtaScriptVariable* pGlobal = DtaProject::getInstance().getGlobalVariable("_FINAL_INFO_TEMPLATE_COVERAGE");
						if (pGlobal == NULL) {
							pGlobal = DtaProject::getInstance().setGlobalVariable("_FINAL_INFO_TEMPLATE_COVERAGE");
						}
						DtaScriptVariable* pStorage = pGlobal->addElement(_sOutputFile);
						pStorage->copyAll(*pOutputCoverage);
					}
				}
			}
		} else {
			_sOutputFile = closeGenerate(true, NULL, pOldOutputStream);
		}
		return result;
	}

	bool DtaTranslateScript::betweenCommands(ScpStream& script, GrfBlock& block) {
		return DtaPatternScript::betweenCommands(script, block);
	}

	void DtaTranslateScript::handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block) {
		DtaPatternScript::handleUnknownCommand(sCommand, script, block);
	}

	void DtaTranslateScript::handleNotAWordCommand(ScpStream& script, GrfBlock& block) {
		DtaPatternScript::handleNotAWordCommand(script, block);
	}

	ExprScriptFunction* DtaTranslateScript::parseImplicitCopyFunction(GrfBlock& block, ScpStream& script) {
		ExprScriptFunction* pImplicitFunction = NULL;
		if (script.isEqualTo('(')) {
			std::string sFunctionName;
			std::string sTemplateKey;
			bool bGenericKey = false;
			script.skipEmpty();
			if (!script.readIdentifier(sFunctionName)) throw UtlException(script, "function name expected for implicit copy");
			script.skipEmpty();
			if (script.isEqualTo('<')) {
				script.skipEmpty();
				if (!script.readString(sTemplateKey)) {
					if (script.readIdentifier(sTemplateKey)) bGenericKey = true;
					else throw UtlException(script, "template key expected for the function of '#implicitCopy'");
				}
				script.skipEmpty();
				if (!script.isEqualTo('>')) throw UtlException(script, "'>' expected");
				script.skipEmpty();
			}
			if (!script.isEqualTo(')')) throw UtlException(script, "')' expected");
			pImplicitFunction = ExprScriptFunction::create(block, script, sFunctionName, sTemplateKey, bGenericKey);
			if (pImplicitFunction == NULL) throw UtlException(script, "unknown function '" + sFunctionName + "'");
			if (pImplicitFunction->getArity() != 1) throw UtlException(script, "function '" + sFunctionName + "' should have one and only one value parameter");
			pImplicitFunction->addParameter(new ExprScriptConstant);
		}
		return pImplicitFunction;
	}

	void DtaTranslateScript::handleUnknownTokenDirective(const std::string& sDirective, ScpStream& script, BNFClause& rule, GrfBlock& block, bool& bContinue, bool& bNoCase, bool bLiteralOnly, BNFStepper* pStepper) {
		if (sDirective == "implicitCopy") {
			BNFTransformationMode* pMode = new BNFTransformationMode(this, &block, true, parseImplicitCopyFunction(block, script));
			if (requiresParsingInformation()) pMode->setParsingInformation(getFilenamePtr(), script);
			block.add(pMode);
			while (parseBNFLitteral(script, rule, *pMode, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepper)) ;
		} else if (sDirective == "explicitCopy") {
			BNFTransformationMode* pMode = new BNFTransformationMode(this, &block, false, NULL);
			if (requiresParsingInformation()) pMode->setParsingInformation(getFilenamePtr(), script);
			block.add(pMode);
			while (parseBNFLitteral(script, rule, *pMode, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepper)) ;
		} else {
			DtaBNFScript::handleUnknownTokenDirective(sDirective, script, rule, block, bContinue, bNoCase, bLiteralOnly, pStepper);
		}
	}

	void DtaTranslateScript::parseBNFPreprocessorDirective(int iStatementBeginning, const std::string& sDirective, ScpStream& script, GrfBlock& block) {
		if (sDirective == "#implicitCopy") {
			_bImplicitCopy = true;
			script.skipEmpty();
			ExprScriptFunction* pImplicitFunction = parseImplicitCopyFunction(block, script);
			if (pImplicitFunction != NULL) {
				delete _pImplicitCopyFunction;
				_pImplicitCopyFunction = pImplicitFunction;
			}
		} else if (sDirective == "#explicitCopy") {
			_bImplicitCopy = false;
		} else {
			DtaBNFScript::parseBNFPreprocessorDirective(iStatementBeginning, sDirective, script, block);
		}
	}

//##markup##"parsing"
//##begin##"parsing"
	void DtaTranslateScript::parseAllFloatingLocations(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseAllFloatingLocations(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseAttachOutputToSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseAttachOutputToSocket(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseDetachOutputFromSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseDetachOutputFromSocket(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseIncrementIndentLevel(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseIncrementIndentLevel(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseInsertText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseInsertText(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseInsertTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseInsertTextOnce(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseInsertTextToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseInsertTextToFloatingLocation(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseInsertTextOnceToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseInsertTextOnceToFloatingLocation(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseOverwritePortion(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseOverwritePortion(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parsePopulateProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parsePopulateProtectedArea(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseResizeOutputStream(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseResizeOutputStream(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseSetFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseSetFloatingLocation(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseSetOutputLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseSetOutputLocation(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseSetProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseSetProtectedArea(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseWriteBytes(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseWriteBytes(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseWriteText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseWriteText(block, script, pMethodCaller);
	}

	void DtaTranslateScript::parseWriteTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaPatternScript::parseWriteTextOnce(block, script, pMethodCaller);
	}

//##end##"parsing"
}
