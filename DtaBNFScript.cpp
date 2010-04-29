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

#include <stdlib.h>

#include "UtlException.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"

#include "ExprScriptVariable.h"
#include "DtaScriptVariable.h"
#include "DtaProject.h"
#include "DtaClass.h"
#include "BNFDisjunction.h"
#include "BNFConjunction.h"
#include "BNFString.h"
#include "BNFEndOfFile.h"
#include "BNFEndOfLine.h"
#include "BNFNextStep.h"
#include "BNFScanWindow.h"
#include "BNFAndOrJunction.h"
#include "BNFBreak.h"
#include "BNFComplementary.h"
#include "BNFFindToken.h"
#include "BNFNot.h"
#include "BNFCharLitteral.h"
#include "BNFCharBoundaries.h"
#include "BNFPushItem.h"
#include "BNFInsert.h"
#include "BNFReadChar.h"
#include "BNFReadByte.h"
#include "BNFReadAdaString.h"
#include "BNFReadCString.h"
#include "BNFReadCChar.h"
#include "BNFReadIdentifier.h"
#include "BNFReadCompleteIdentifier.h"
#include "BNFReadInteger.h"
#include "BNFReadNumeric.h"
#include "BNFReadPythonString.h"
#include "BNFReadText.h"
#include "BNFReadBytes.h"
#include "BNFReadChars.h"
#include "BNFCheck.h"
#include "BNFForeach.h"
#include "BNFClauseCall.h"
#include "BNFMultiplicity.h"
#include "BNFClause.h"
#include "BNFRatchet.h"
#include "BNFTryCatch.h"
#include "BNFStepintoHook.h"
#include "BNFStepoutHook.h"
//##markup##"includes"
//##begin##"includes"
#include "GrfAttachInputToSocket.h"
#include "GrfDetachInputFromSocket.h"
#include "GrfGoBack.h"
#include "GrfSetInputLocation.h"
//##end##"includes"
#include "DtaASTNavigator.h"
#include "ASTCommand.h"
#include "GrfParsedFile.h"
#include "GrfParsedString.h"
#include "GrfGeneratedFile.h"
#include "GrfGeneratedString.h"
#include "GrfAppendedFile.h"
#include "GrfDebugExecution.h"
#include "DtaBNFScript.h"

#include "DtaTranslateScript.h"
#include "BNFIgnore.h"
#include "BNFSkipIgnore.h"
#include "BNFMoveAhead.h"
#include "BNFReadUptoIgnore.h"


namespace CodeWorker {
	class BNFDisengageIgnore {
	private:
		DtaBNFScript* _pBNFScript;

	public:
		BNFDisengageIgnore(DtaBNFScript* pBNFScript) : _pBNFScript(pBNFScript) {
			pBNFScript->setIgnoreMode(NOT_IGNORE);
		}

		~BNFDisengageIgnore() {
			_pBNFScript->setIgnoreMode(IGNORE_CLAUSE);
		}
	};


	class BNFTransformRules {
		private:
			ExprScriptExpression* _pFilter;
			DtaTranslateScript* _pPrototypeScript;
			DtaTranslateScript* _pProductionRuleScript;

		public:
			inline BNFTransformRules() : _pFilter(NULL), _pPrototypeScript(NULL), _pProductionRuleScript(NULL) {}
			~BNFTransformRules() {
				delete _pFilter;
				delete _pPrototypeScript;
				delete _pProductionRuleScript;
			}

			inline ExprScriptExpression* getFilter() const { return _pFilter; }
			inline void setFilter(ExprScriptExpression* pFilter) { _pFilter = pFilter; }
			inline DtaTranslateScript* getPrototypeScript() const { return _pPrototypeScript; }
			inline void setPrototypeScript(DtaTranslateScript* pPrototypeScript) { _pPrototypeScript = pPrototypeScript; }
			inline DtaTranslateScript* getProductionRuleScript() const { return _pProductionRuleScript; }
			inline void setProductionRuleScript(DtaTranslateScript* pProductionRuleScript) { _pProductionRuleScript = pProductionRuleScript; }

			virtual bool applyRuleTransformer(ScpStream& script, GrfBlock& block, int iStatementBeginning, int iProductionRuleBeginning, const std::string& sClauseName, const std::string& sPrototype, const std::string& sProductionRule) {
				DtaScriptVariable theContext;
				theContext.setValueAtVariable("x", sClauseName.c_str());
				std::string sResult = _pFilter->getValue(theContext);
				if (sResult.empty()) return false;
				if (_pPrototypeScript != NULL) {
					std::string sNewPrototype;
					_pPrototypeScript->translateString(sPrototype, sNewPrototype, DtaProject::getInstance());
					if (sNewPrototype != sPrototype) script.insertText(sNewPrototype, iStatementBeginning, sPrototype.size());
				}
				if (_pProductionRuleScript != NULL) {
					std::string sNewProductionRule;
					_pProductionRuleScript->translateString(sProductionRule, sNewProductionRule, DtaProject::getInstance());
					if (sNewProductionRule != sProductionRule) {
						script.insertText(sNewProductionRule, iProductionRuleBeginning, sProductionRule.size());
					}
				}
				return true;
			}
	};


	struct TemporaryMatchingStorage {
		bool bBegin;
		DtaScriptVariable* pClauseNode;
		TemporaryMatchingStorage(bool b, DtaScriptVariable* p) : bBegin(b), pClauseNode(p) {}
	};


	DtaBNFScript::DtaBNFScript() : DtaPatternScript(true), _bBNFMode(true), _pIgnoreClause(NULL), _bNoCase(false), _bImplicitCopy(false), _bParsedFile(false), _pStream(NULL), _pImplicitCopyFunction(NULL), _eIgnoreMode(NOT_IGNORE), _bTrace(false), _pStepintoHook(NULL), _pStepoutHook(NULL), _iRatchetPosition(-1), _pMatchingAreasContainer(NULL), _pASTContainer(NULL), _pParentClauseMatching(NULL) {
	}

	DtaBNFScript::DtaBNFScript(GrfBlock* pParentBlock) : DtaPatternScript(pParentBlock, true), _bBNFMode(true), _pIgnoreClause(NULL), _bNoCase(false), _bImplicitCopy(false), _bParsedFile(false), _pStream(NULL), _pImplicitCopyFunction(NULL), _eIgnoreMode(NOT_IGNORE), _bTrace(false), _pStepintoHook(NULL), _pStepoutHook(NULL), _iRatchetPosition(-1), _pMatchingAreasContainer(NULL), _pASTContainer(NULL), _pParentClauseMatching(NULL) {
	}

	DtaBNFScript::DtaBNFScript(EXECUTE_FUNCTION* executeFunction) : DtaPatternScript(executeFunction, true), _bBNFMode(true), _pIgnoreClause(NULL), _bNoCase(false), _bImplicitCopy(false), _bParsedFile(false), _pStream(NULL), _pImplicitCopyFunction(NULL), _eIgnoreMode(NOT_IGNORE), _bTrace(false), _pStepintoHook(NULL), _pStepoutHook(NULL), _iRatchetPosition(-1), _pMatchingAreasContainer(NULL), _pASTContainer(NULL), _pParentClauseMatching(NULL) {
	}

	DtaBNFScript::~DtaBNFScript() {
		delete _pImplicitCopyFunction;
		for (std::map<std::string, std::list<BNFClause*> >::const_iterator i = _listOfClauses.begin(); i != _listOfClauses.end(); i++) {
	//		for (std::list<BNFClause*>::const_iterator j = i->second.begin(); j != i->second.end(); j++) delete *j;
		}
		for (std::list<BNFTransformRules*>::const_iterator j = _listOfTransformRules.begin(); j != _listOfTransformRules.end(); ++j) {
			delete *j;
		}
		delete _pStepintoHook;
		delete _pStepoutHook;
		delete _pMatchingAreasContainer;
		delete _pASTContainer;
		delete _pParentClauseMatching;
	}

	DtaScriptFactory::SCRIPT_TYPE DtaBNFScript::getType() const { return DtaScriptFactory::BNF_SCRIPT; }
	bool DtaBNFScript::isAParseScript() const { return true; }
	bool DtaBNFScript::isAGenerateScript() const { return false; }

	void DtaBNFScript::traceEngine() const {
		if (getFilenamePtr() == NULL) CGRuntime::traceLine("Extended-BNF script (no filename):");
		else CGRuntime::traceLine("Extended-BNF script \"" + std::string(getFilenamePtr()) + "\":");
		traceInternalEngine();
	}

	void DtaBNFScript::traceInternalEngine() const {
		char tcMessage[80];
		sprintf(tcMessage, "\tNumber of production rules = %d", _listOfClauses.size());
		CGRuntime::traceLine(tcMessage);
		if (_eIgnoreMode == NOT_IGNORE) CGRuntime::traceLine("\tignore insignificant chars = OFF");
		else if (_pIgnoreClause == NULL) CGRuntime::traceLine("\tignore insignificant chars = " + IGNORE_MODEtoString(_eIgnoreMode));
		else CGRuntime::traceLine("\tignore insignificant chars = clause '#ignore'");
		if (_bNoCase) CGRuntime::traceLine("\tignore case = ON");

		if (!_mapOfIgnoreClauses.empty()) {
			CGRuntime::traceText("\tignore clauses: ");
			for (std::map<std::string, BNFClause*>::const_iterator i = _mapOfIgnoreClauses.begin(); i != _mapOfIgnoreClauses.end(); ++i) {
				if (i != _mapOfIgnoreClauses.begin()) CGRuntime::traceText(", ");
				CGRuntime::traceText(i->first);
			}
			CGRuntime::traceLine("");
		}

		if (_bTrace) CGRuntime::traceLine("\ttrace = ON");
		if (_bImplicitCopy) CGRuntime::traceLine("\timplicitCopy = ON");
		if (_pImplicitCopyFunction != NULL) {
			CGRuntime::traceLine("\timplicitCopy function = " + _pImplicitCopyFunction->toString());
		}
		DtaScript::traceInternalEngine();
	}

	DtaBNFScript& DtaBNFScript::getAlienParser() const {
		DtaBNFScript* pScript = DtaProject::getInstance().getBNFAlienParser();
		if (pScript == NULL) throw UtlException("compiling a BNF script in " + _sTargetLanguage + " requires \"($CODEWORKER_HOME)/" + _sTargetLanguage + "/default-scripts/" + _sTargetLanguage + "BNFScript.cwp\"");
		return *pScript;
	}

	int DtaBNFScript::skipEmptyChars(DtaScriptVariable& visibility) {
		int iImplicitCopyPosition;
		int iLocation;
		if (_bImplicitCopy) {
			iImplicitCopyPosition = CGRuntime::getOutputLocation();
			iLocation = CGRuntime::getInputLocation();
		} else {
			iImplicitCopyPosition = -1;
		}
		if ((_eIgnoreMode != NOT_IGNORE) && (_eIgnoreMode != UNDEFINED_IGNORE)) {
			bool bCopyImplicitly = _bImplicitCopy;
			switch(_eIgnoreMode) {
				case IGNORE_CPP:
				case IGNORE_JAVA:
					CGRuntime::_pInputStream->skipEmpty();
					break;
				case IGNORE_HTML:
					CGRuntime::_pInputStream->skipEmptyHTML();
					break;
				case IGNORE_BLANKS:
					CGRuntime::_pInputStream->skipBlanks();
					break;
				case IGNORE_SPACES:
					CGRuntime::_pInputStream->skipSpaces();
					break;
				case IGNORE_ADA:
					CGRuntime::_pInputStream->skipEmptyAda();
					break;
				case IGNORE_LATEX:
					CGRuntime::_pInputStream->skipEmptyLaTeX();
					break;
				case IGNORE_CPP_EXCEPT_DOXYGEN:
					CGRuntime::_pInputStream->skipEmptyCppExceptDoxygen();
					break;
				case IGNORE_CLAUSE:
					bCopyImplicitly = false;
					if (_pIgnoreClause != NULL) {
						BNFDisengageIgnore disengage(this);
						_pIgnoreClause->executeClause(visibility, 0);
					}
					break;
				default:
					throw UtlException("internal error in DtaBNFScript::skipEmptyChars(): unrecognized ignore mode encountered");
			}
			if (bCopyImplicitly) {
				int iLastLocation = CGRuntime::getInputLocation();
				if (iLastLocation > iLocation) {
					std::string sText = CGRuntime::getLastReadChars(iLastLocation - iLocation);
					writeBinaryData(sText.c_str(), sText.size());
				}
			}
		}
		return iImplicitCopyPosition;
	}

	void DtaBNFScript::writeBinaryData(const char* tcText, int iLength) {
		if (_pImplicitCopyFunction != NULL) {
			ExprScriptConstant* pConstant = dynamic_cast<ExprScriptConstant*>(_pImplicitCopyFunction->getParameters()[0]);
			pConstant->setConstant(tcText, iLength);
			std::string sNewText = _pImplicitCopyFunction->getValue(CGRuntime::getThisInternalNode());
			CGRuntime::_pOutputStream->writeBinaryData(sNewText.c_str(), sNewText.size());
		} else {
			CGRuntime::_pOutputStream->writeBinaryData(tcText, iLength);
		}
	}

	void DtaBNFScript::storeClauseMatching(DtaScriptVariable& ruleNames, std::map<int, std::map<int, std::list<TemporaryMatchingStorage*> > >& mapOfAreas, BNFClauseMatchingArea* pClauseMatching) {
		if (pClauseMatching == NULL) return;
		int iBegin = pClauseMatching->beginPosition;
		int iEnd = pClauseMatching->endPosition;
		if (iBegin != iEnd) {
			DtaScriptVariable* pClauseNode = ruleNames.getArrayElement(pClauseMatching->clause->getSignature());
			mapOfAreas[iBegin][iEnd].push_back(new TemporaryMatchingStorage(true, pClauseNode));
			for (std::list<BNFClauseMatchingArea*>::iterator i = pClauseMatching->childs.begin(); i != pClauseMatching->childs.end(); ++i) {
				storeClauseMatching(ruleNames, mapOfAreas, *i);
			}
			mapOfAreas[iEnd][iBegin].push_back(new TemporaryMatchingStorage(false, pClauseNode));
		}
	}

	void DtaBNFScript::storeMatchingAreas(DtaScriptVariable& thisContext, DtaScriptVariable* pStorage) {
		// storage of matching areas
		if (pStorage == NULL) {
			if (_pMatchingAreasContainer == NULL) return;
			pStorage = thisContext.getVariable(*_pMatchingAreasContainer);
		}
		DtaScriptVariable* pAreas = pStorage->insertNode("areas");
		DtaScriptVariable* pRules = pStorage->insertNode("rules");
		for (std::map<std::string, std::list<BNFClause*> >::iterator i = _listOfClauses.begin(); i != _listOfClauses.end(); ++i) {
			for (std::list<BNFClause*>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
				std::string sSignature = (*j)->getSignature();
				DtaScriptVariable* pClauseNode = pRules->addElement(sSignature);
				pClauseNode->setValue(sSignature.c_str());
			}
		}
		std::map<int, std::map<int, std::list<TemporaryMatchingStorage*> > > mapOfAreas;
		storeClauseMatching(*pRules, mapOfAreas, _pParentClauseMatching);
		{
			for (std::map<int, std::map<int, std::list<TemporaryMatchingStorage*> > >::iterator i = mapOfAreas.begin(); i != mapOfAreas.end(); ++i) {
				DtaScriptVariable* pPosition = pAreas->addElement(i->first);
				for (std::map<int, std::list<TemporaryMatchingStorage*> >::reverse_iterator j = i->second.rbegin(); j != i->second.rend(); ++j) {
					{
						for (std::list<TemporaryMatchingStorage*>::iterator k = j->second.begin(); k != j->second.end(); ++k) {
							if ((*k)->bBegin) {
								pPosition->insertNode("begin")->addElement(j->first)->pushItem("")->setValue((*k)->pClauseNode);
							}
						}
					}
					{
						for (std::list<TemporaryMatchingStorage*>::iterator k = j->second.begin(); k != j->second.end(); ++k) {
							if (!(*k)->bBegin) {
								pPosition->insertNode("end")->addElement(j->first)->pushItem("")->setValue((*k)->pClauseNode);
							}
						}
					}
					{
						// delete the temporary storage
						for (std::list<TemporaryMatchingStorage*>::iterator k = j->second.begin(); k != j->second.end(); ++k) {
							delete *k;
						}
					}
				}
			}
		}
	}

	void DtaBNFScript::storeAST(DtaScriptVariable& thisContext, ScpStream& inputStream) {
		DtaASTNavigator navigator;
		std::auto_ptr<DtaASTStructure> structure(navigator.detectASTStructure(*this));
		DtaScriptVariable* pStorage = thisContext.getVariable(*_pASTContainer);
		structure->build(*pStorage);
		structure->populateAST(*pStorage, *_pParentClauseMatching, inputStream);
	}

	SEQUENCE_INTERRUPTION_LIST DtaBNFScript::generate(ScpStream& stream, DtaScriptVariable& thisContext) {
		CGRuntimeOutputStream noOutput(NULL);
		_bBNFMode = true;
		_sParsedFileOrContent = "";
		_bParsedFile = false;
		_pStream = &stream;
		// TO DO PROPERLY
		return execute(thisContext);
	}

	SEQUENCE_INTERRUPTION_LIST DtaBNFScript::generate(const char* tcFile, DtaScriptVariable& thisContext) {
		CGRuntimeOutputStream noOutput(NULL);
		_bBNFMode = true;
		// absolute path of the parsed file:
		// avoid ambiguities
		std::string sAbsolutePath;
		if (ScpStream::existInputFileFromIncludePath(tcFile, sAbsolutePath)) {
			if (!ScpStream::existVirtualFile(tcFile)) {
				sAbsolutePath = CGRuntime::canonizePath(sAbsolutePath);
			}
		} else {
			// should have found! Do not touch the file name, for further error message
			sAbsolutePath = tcFile;
		}
		_sParsedFileOrContent = sAbsolutePath;
		_bParsedFile = true;
		// TO DO PROPERLY
		SEQUENCE_INTERRUPTION_LIST result = execute(thisContext);
		if (getFilenamePtr() != NULL) {
			DtaProject::getInstance().captureInputFile(sAbsolutePath.c_str(), getFilenamePtr());
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST DtaBNFScript::generateString(std::string& sContent, DtaScriptVariable& thisContext) {
		CGRuntimeOutputStream noOutput(NULL);
		_bBNFMode = true;
		_sParsedFileOrContent = sContent;
		_bParsedFile = false;
		_pStream = NULL;
		// TO DO PROPERLY
		return execute(thisContext);
	}

	SEQUENCE_INTERRUPTION_LIST DtaBNFScript::execute(DtaScriptVariable& thisContext) {
		SEQUENCE_INTERRUPTION_LIST result;
		std::string sParsedFileOrContent = _sParsedFileOrContent; // because '_sParsedFileOrContent' is shared
		ScpStream* pStream;
		bool bStreamOwner = (_pStream == NULL);
		if (bStreamOwner) {
			if (_bParsedFile) {
				pStream = new ScpStream(sParsedFileOrContent, ScpStream::IN | ScpStream::PATH);
				if (pStream == NULL) throw UtlException("unable to open file \"" + sParsedFileOrContent + "\"");
			} else {
				pStream = new ScpStream;
				(*pStream) << sParsedFileOrContent;
			}
		} else {
			pStream = _pStream;
		}
		// note if 'final info' was requiring to store coverage,
		// while this BNF script hasn't required coverage recording
		bool bCreateMatchingAreasContainerForFinalInfo = false;
		if ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_PARSE_COVERAGE_FOR_EVERYBODY) != 0) {
			if (_bParsedFile && !hasCoverageRecording()) {
				// create a global variable expression for coverage recording
				// when final info requires coverage on parsed files;
				// the variable expression points to an item whose key is the name
				// of the parsed file.
				if (DtaProject::getInstance().getGlobalVariable("_FINAL_INFO_PARSE_COVERAGE") == NULL) {
					DtaProject::getInstance().setGlobalVariable("_FINAL_INFO_PARSE_COVERAGE");
				}
				_pMatchingAreasContainer = new ExprScriptVariable("_FINAL_INFO_PARSE_COVERAGE");
				_pMatchingAreasContainer->setArrayKey(new ExprScriptConstant(sParsedFileOrContent.c_str()));
				bCreateMatchingAreasContainerForFinalInfo = true;
			}
		}
		if (hasCoverageRecording()) {
			// prepare for recording matching areas
			delete _pParentClauseMatching;
			_pParentClauseMatching = NULL;
		}

		ScpStream* pOldInputStream = CGRuntime::_pInputStream;
		CGRuntime::_pInputStream = pStream;
		try {
			result = DtaPatternScript::execute(thisContext);
			if (hasCoverageRecording()) {
				// storage of matching areas / AST in the container
				CGThisModifier thisModifier(&thisContext);
				if (_pMatchingAreasContainer != NULL) storeMatchingAreas(thisContext);
				if (_pASTContainer != NULL) storeAST(thisContext, *CGRuntime::_pInputStream);
				if (!bCreateMatchingAreasContainerForFinalInfo) {
					// the variable expression wasn't built for 'final info', but
					// the latter requires perhaps the coverage
					if ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_PARSE_COVERAGE) != 0) {
						//yes, the coverage tree must be copied for 'final info'
						DtaScriptVariable* pGlobal = DtaProject::getInstance().getGlobalVariable("_FINAL_INFO_PARSE_COVERAGE");
						if (pGlobal == NULL) {
							pGlobal = DtaProject::getInstance().setGlobalVariable("_FINAL_INFO_PARSE_COVERAGE");
						}
						DtaScriptVariable* pStorage = pGlobal->addElement(sParsedFileOrContent);
						storeMatchingAreas(thisContext, pStorage);
					}
				}
				// temporary low-level structure used to build the
				// coverage tree; won't serve anymore
				delete _pParentClauseMatching;
				_pParentClauseMatching = NULL;
			}
		} catch(UtlException& exception) {
			int iLine = pStream->getLineCount();
			int iCol = pStream->getColCount();
			if (bStreamOwner) {
				pStream->close();
				delete pStream;
			}
			CGRuntime::_pInputStream = pOldInputStream;
			std::string sException = exception.getMessage();
			std::string sMessage;
			char tcNumber[64];
			if (_bParsedFile) {
				sMessage = sParsedFileOrContent;
				sprintf(tcNumber, "(%d,%d):", iLine, iCol);
			} else if (!sParsedFileOrContent.empty()) {
				sMessage = CGRuntime::endl() + "----------------- content -----------------" + CGRuntime::endl() + sParsedFileOrContent + CGRuntime::endl() + "-------------------------------------------" + CGRuntime::endl();
				sprintf(tcNumber, "line %d, col %d:", iLine, iCol);
			}
			sMessage += tcNumber + CGRuntime::endl() + sException;
			throw UtlException(exception.getTraceStack(), sMessage);
		} catch(std::exception&) {
			if (bStreamOwner) {
				pStream->close();
				delete pStream;
			}
			CGRuntime::_pInputStream = pOldInputStream;
			throw;
		} catch(...) {
			int iLine = pStream->getLineCount();
			int iCol = pStream->getColCount();
			if (bStreamOwner) {
				pStream->close();
				delete pStream;
			}
			CGRuntime::_pInputStream = pOldInputStream;
			std::string sMessage;
			char tcNumber[64];
			if (_bParsedFile) {
				sMessage = sParsedFileOrContent;
				sprintf(tcNumber, "(%d,%d):", iLine, iCol);
			} else if (!sParsedFileOrContent.empty()) {
				sMessage = CGRuntime::endl() + "----------------- content -----------------" + CGRuntime::endl() + sParsedFileOrContent + CGRuntime::endl() + "-------------------------------------------" + CGRuntime::endl();
				sprintf(tcNumber, "line %d, col %d:", iLine, iCol);
			}
			sMessage += tcNumber + CGRuntime::endl() + "Fatal error: ellipsis exception";
			throw UtlException(sMessage);
		}
		if (bStreamOwner) {
			pStream->close();
			delete pStream;
		}
		CGRuntime::_pInputStream = pOldInputStream;
		return result;
	}

	std::string DtaBNFScript::IGNORE_MODEtoString(IGNORE_MODE eMode) {
		std::string sResult;
		switch(eMode) {
			case NOT_IGNORE: break;
			case IGNORE_CPP: sResult = "C++";break;
			case IGNORE_JAVA: sResult = "JAVA";break;
			case IGNORE_HTML: sResult = "HTML";break;
			case IGNORE_BLANKS: sResult = "blanks";break;
			case IGNORE_SPACES: sResult = "spaces";break;
			case IGNORE_ADA: sResult = "Ada";break;
			case IGNORE_LATEX: sResult = "LaTeX";break;
			case IGNORE_CPP_EXCEPT_DOXYGEN: sResult = "C++/Doxygen";break;
			case IGNORE_CLAUSE:
				throw UtlException("IGNORE_MODEtoString('IGNORE_CLAUSE') requires a specific handling!");
			default:
				throw UtlException("internal error in DtaBNFScript::IGNORE_MODEtoString(): unrecognized ignore mode encountered");
		}
		return sResult;
	}

	BNFClause& DtaBNFScript::buildClause(ScpStream& script, GrfBlock& parent, const std::string& sName, const std::string& sTemplateKey, bool bGenericKey, int iReturnType, const std::vector<std::string>& listOfParameters, const std::vector<EXPRESSION_TYPE>& listOfParameterTypes, bool bOverload) {
		std::list<BNFClause*>& listOfHomonyms = _listOfClauses[sName];
		BNFClause* pClause = NULL;
		std::list<BNFClause*>::iterator i;
		for (i = listOfHomonyms.begin(); i != listOfHomonyms.end(); i++) {
			if ((*i)->getArity() >= (int) listOfParameters.size()) {
				for (std::string::size_type j = 0; j < listOfParameters.size(); j++) {
					const std::string& sClauseParameter = (*i)->getParameter(j);
					if (listOfParameters[j] != sClauseParameter) {
						if (sClauseParameter.empty()) {
							(*i)->setParameter(j, listOfParameters[j], listOfParameterTypes[j]);
						} else throw UtlException(script, "parameter '" + listOfParameters[j] + "' must be called '" + sClauseParameter + "' into BNF clause declaration");
					}
				}
				if (((*i)->getArity() > (int) listOfParameters.size()) && !(*i)->isPropagatedParameter()) {
					throw UtlException(script, "not enough parameters passed to the non-terminal '" + sName + "'");
				}
				if (((*i)->getTemplateKey() == sTemplateKey) && ((*i)->isGenericKey() == bGenericKey)) {
					pClause = (*i);
					pClause->setParent(&parent);
					break;
				}
			} else {
				throw UtlException(script, "too many parameters passed to the non-terminal '" + sName + "'");
			}
		}
		if (pClause == NULL) {
			if (bOverload) throw UtlException(script, "cannot overload a clause that doesn't exist yet!");
			BNFClause* pTemplateClause = NULL;
			if (!sTemplateKey.empty()) pTemplateClause = &buildClause(script, parent, sName, "", false, iReturnType, listOfParameters, listOfParameterTypes, false);
			pClause = new BNFClause(this, &parent, sName, sTemplateKey, bGenericKey, listOfParameters, listOfParameterTypes);
			if ((sName[0] != '#') && (_graph.getNbCommands() == 0)) {
				pClause->setPreprocessingIgnoreMode(NOT_IGNORE, NULL);
				BNFClauseCall* pClauseCall = new BNFRootClauseCall(this, &_graph, _bNoCase);
				if (requiresParsingInformation()) pClauseCall->setParsingInformation(getFilenamePtr(), script);
				_graph.add(pClauseCall);
				pClauseCall->setClause(pClause);
			}
			if (!sTemplateKey.empty()) pClause->setTemplateClause(pTemplateClause);
			listOfHomonyms.push_front(pClause);
		} else if (bOverload) {
			BNFClause* pOverloadedClause = pClause;
			pClause = new BNFClause(this, &parent, sName, sTemplateKey, bGenericKey, listOfParameters, listOfParameterTypes);
			pOverloadedClause->setOverloadClause(pClause);
		}
		pClause->setReturnType(iReturnType);
		return *pClause;
	}

	BNFClause& DtaBNFScript::buildClause(ScpStream& script, GrfBlock& parent, const std::string& sName, unsigned int iArity) {
		std::list<BNFClause*>& listOfHomonyms = _listOfClauses[sName];
		BNFClause* pClause = NULL;
		for (std::list<BNFClause*>::iterator i = listOfHomonyms.begin(); i != listOfHomonyms.end(); i++) {
			if ((*i)->getArity() < iArity) {
				throw UtlException(script, "too many parameters passed to the non-terminal '" + sName + "'");
			} else if (((*i)->getArity() > iArity) && !(*i)->isPropagatedParameter()) {
				throw UtlException(script, "not enough parameters passed to the non-terminal '" + sName + "'");
			} else if ((*i)->getTemplateKey().empty()) {
				pClause = (*i);
				break;
			}
		}
		if (pClause == NULL) {
			pClause = new BNFClause(this, &parent, sName, iArity);
			listOfHomonyms.push_front(pClause);
		}
		return *pClause;
	}

	void DtaBNFScript::parsePreprocessorDirective(const std::string& sDirective, ScpStream& script, GrfBlock& block) {
		if (sDirective == "#applyBNFRule") {
			script.skipEmpty();
			std::string sIdentifier;
			if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "the directive #applyBNFRule should be followed by a non-terminal");
			BNFClauseCall* pClauseCall = parseBNFClauseCall(script, block, sIdentifier, false, _bNoCase);
			if (requiresParsingInformation()) pClauseCall->setParsingInformation(getFilenamePtr(), script);
			std::vector<std::string> listOfConstants;
			bool bConcat;
			ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, true);
			pClauseCall->setVariableToAssign(pVarToAssign, bConcat);
			if (!listOfConstants.empty()) pClauseCall->setConstantsToMatch(listOfConstants);
			script.skipEmpty();
			if (!script.isEqualTo(';')) throw UtlException(script, "syntax error, ';' expected to close the directive #applyBNFRule");
		} else {
			DtaPatternScript::parsePreprocessorDirective(sDirective, script, block);
		}
	}

	void DtaBNFScript::parseInstruction(ScpStream& script, GrfBlock& block) {
		if (_bBNFMode) {
			int iStatementBeginning = script.getInputLocation();
			if (script.isEqualTo('#')) {
				std::string sDirective;
				script.skipEmpty();
				if (!script.readIdentifier(sDirective)) throw UtlException(script, "directive name expected");
				sDirective = "#" + sDirective;
				parseBNFPreprocessorDirective(iStatementBeginning, sDirective, script, block);
			} else {
				std::string sClauseName;
				if (!script.readIdentifier(sClauseName)) {
					throw UtlException(script, "identifier expected: name of a BNF clause or 'function' or 'declare'");
				}
				if ((sClauseName == "function") || (sClauseName == "external") || (sClauseName == "declare")) {
					script.skipEmpty();
					std::string sIdentifier;
					int iCursor = script.getInputLocation();
					if (script.readIdentifier(sIdentifier)) {
						script.setInputLocation(iCursor);
						_bBNFMode = false;
						if (sClauseName == "function") parseFunction(block, script);
						else if (sClauseName == "external") parseExternal(block, script);
						else parseDeclare(block, script);
						_bBNFMode = true;
						return;
					}
					throw UtlException("syntax error after keyword '" + sClauseName + "'");
				} else if ((sClauseName == "readonlyHook") || (sClauseName == "writefileHook")) {
					_bBNFMode = false;
					if (sClauseName == "readonlyHook") parseReadonlyHook(block, script);
					else if (sClauseName == "writefileHook") parseWritefileHook(block, script);
					_bBNFMode = true;
					return;
				} else if ((sClauseName == "stepintoHook") || (sClauseName == "stepoutHook")) {
					_bBNFMode = false;
					if (sClauseName == "stepintoHook") parseStepintoHook(block, script);
					else if (sClauseName == "stepoutHook") parseStepoutHook(block, script);
					_bBNFMode = true;
					return;
				}
				parseBNFClause(script, block, iStatementBeginning, sClauseName, false);
			}
		} else {
			DtaPatternScript::parseInstruction(script, block);
		}
	}

	void DtaBNFScript::parseStepintoHook(GrfBlock& block, ScpStream& script) {
		if (getStepintoHook() != NULL) throw UtlException(script, "function 'stepintoHook(<clause_signature>, <parameters>)' has already been defined and can't be implemented twice");
		BNFStepintoHook* pStepintoHook = new BNFStepintoHook(this, &block);
		if (requiresParsingInformation()) pStepintoHook->setParsingInformation(getFilenamePtr(), script);
		block.addFunction(pStepintoHook);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
		script.skipEmpty();
		std::string sIdentifier;
		if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "syntax error: argument name expected for <clause_signature>");
		pStepintoHook->setClauseSignature(sIdentifier.c_str());
		script.skipEmpty();
		if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
		script.skipEmpty();
		if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "syntax error: argument name expected for <parameters>");
		pStepintoHook->setParameters(sIdentifier.c_str());
		script.skipEmpty();
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		std::string _sOldFunctionBody = _sCurrentFunctionBody;
		std::string _sOldFunctionTemplateBody = _sCurrentFunctionTemplateBody;
		bool bOldCurrentFunctionGenericTemplateKey = _bCurrentFunctionGenericTemplateKey;
		_sCurrentFunctionBody = pStepintoHook->getFunctionName();
		_sCurrentFunctionTemplateBody = "";
		_bCurrentFunctionGenericTemplateKey = false;
		pStepintoHook->isBodyDefined(true);
		parseBlock(script, *pStepintoHook);
		_sCurrentFunctionBody = _sOldFunctionBody;
		_sCurrentFunctionTemplateBody = _sOldFunctionTemplateBody;
		_bCurrentFunctionGenericTemplateKey = bOldCurrentFunctionGenericTemplateKey;
	}

	void DtaBNFScript::parseStepoutHook(GrfBlock& block, ScpStream& script) {
		if (getStepoutHook() != NULL) throw UtlException(script, "function 'StepoutHook(<clause_signature>, <parameters>, <success>)' has already been defined and can't be implemented twice");
		BNFStepoutHook* pStepoutHook = new BNFStepoutHook(this, &block);
		if (requiresParsingInformation()) pStepoutHook->setParsingInformation(getFilenamePtr(), script);
		block.addFunction(pStepoutHook);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
		script.skipEmpty();
		std::string sIdentifier;
		if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "syntax error: argument name expected for <clause_signature>");
		pStepoutHook->setClauseSignature(sIdentifier.c_str());
		script.skipEmpty();
		if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
		script.skipEmpty();
		if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "syntax error: argument name expected for <parameters>");
		pStepoutHook->setParameters(sIdentifier.c_str());
		script.skipEmpty();
		if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
		script.skipEmpty();
		if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "syntax error: argument name expected for <success>");
		pStepoutHook->setSuccess(sIdentifier.c_str());
		script.skipEmpty();
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		std::string _sOldFunctionBody = _sCurrentFunctionBody;
		std::string _sOldFunctionTemplateBody = _sCurrentFunctionTemplateBody;
		bool bOldCurrentFunctionGenericTemplateKey = _bCurrentFunctionGenericTemplateKey;
		_sCurrentFunctionBody = pStepoutHook->getFunctionName();
		_sCurrentFunctionTemplateBody = "";
		_bCurrentFunctionGenericTemplateKey = false;
		pStepoutHook->isBodyDefined(true);
		parseBlock(script, *pStepoutHook);
		_sCurrentFunctionBody = _sOldFunctionBody;
		_sCurrentFunctionTemplateBody = _sOldFunctionTemplateBody;
		_bCurrentFunctionGenericTemplateKey = bOldCurrentFunctionGenericTemplateKey;
	}

	void DtaBNFScript::parseBNFPreprocessorDirective(int iStatementBeginning, const std::string& sDirective, ScpStream& script, GrfBlock& block) {
		if (sDirective == "#ignore") {
			std::string sClauseName = sDirective;
			std::string sKey;
			script.skipEmpty();
			if (script.isEqualTo('[')) {
				script.skipEmpty();
				if (!script.readPythonString(sKey)) throw UtlException(script, "syntax error, constant string expected");
				script.skipEmpty();
				if (!script.isEqualTo(']')) throw UtlException(script, "syntax error, ']' expected");
				if (!sKey.empty()) sClauseName += "[" + sKey + "]";
			}
			BNFClause& theIgnoreClause = parseBNFClause(script, block, iStatementBeginning, sClauseName, false);
			if (theIgnoreClause.getArity() != 0) throw UtlException(script, "clause '#ignore' doesn't accept parameters");
			_mapOfIgnoreClauses[sClauseName] = &theIgnoreClause;
		} else if (sDirective == "#noCase") {
			_bNoCase = true;
		} else if (sDirective == "#trace") {
			_bTrace = true;
		} else if (sDirective == "#matching") {
			script.skipEmpty();
			if (!script.isEqualTo('(')) throw UtlException(script, "syntax error, '(' expected");
			script.skipEmpty();
			if (_pMatchingAreasContainer != NULL) {
				delete _pMatchingAreasContainer;
				_pMatchingAreasContainer = NULL;
			}
			_pMatchingAreasContainer = parseBNFVariableExpression(block, script);
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error, ')' expected");
		} else if (sDirective == "#parameters") {
			parseAndPropagateParameters(script, block);
		} else if (sDirective == "#transformRules") {
			parseTransformRules(script, block);
		} else if (sDirective == "#overload") {
			std::string sClauseName;
			script.skipEmpty();
			if (script.isEqualTo('#')) {
				script.skipEmpty();
				if (script.isEqualToIdentifier("ignore")) sClauseName = "#ignore";
			} else if (!script.readIdentifier(sClauseName)) throw UtlException(script, "clause name to overload expected");
			parseBNFClause(script, block, iStatementBeginning, sClauseName, true);
		} else if (sDirective == "#buildAST") {
			script.skipEmpty();
			if (!script.isEqualTo('(')) throw UtlException(script, "syntax error, '(' expected");
			script.skipEmpty();
			if (_pASTContainer != NULL) {
				delete _pASTContainer;
				_pASTContainer = NULL;
			}
			_pASTContainer = parseBNFVariableExpression(block, script);
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error, ')' expected");
		} else DtaPatternScript::parsePreprocessorDirective(sDirective, script, block);
	}

	void DtaBNFScript::parseAndPropagateParameters(ScpStream& script, GrfBlock& block) {
		std::string sFunctionQuantity;
		script.skipEmpty();
		if (!script.readIdentifier(sFunctionQuantity)) throw UtlException(script, "function quantity expected");
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected");
		std::vector<std::string> listOfParameters;
		std::vector<EXPRESSION_TYPE> listOfParameterTypes;
		script.skipEmpty();
		if (!script.isEqualTo(')')) {
			int iIndex = 0;
			EXPRESSION_TYPE paramType;
			do {
				std::string sParameter;
				script.skipEmpty();
				if (!script.readIdentifier(sParameter)) throw UtlException(script, "parameter name expected");
				script.skipEmpty();
				if (!script.isEqualTo(':')) throw UtlException(script, "':' expected after parameter '" + sParameter + "'");
				paramType = parseVariableType(block, script);
				listOfParameters.push_back(sParameter);
				listOfParameterTypes.push_back(paramType);
				iIndex++;
			} while (script.isEqualTo(','));
			if (!script.isEqualTo(')')) throw UtlException(script, "')' expected");
		}
		std::auto_ptr<ExprScriptExpression> pFilter(parseExpression(block, script));
		for (std::map<std::string, std::list<BNFClause*> >::iterator i = _listOfClauses.begin(); i != _listOfClauses.end(); ++i) {
			for (std::list<BNFClause*>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
				(*j)->propagateParameters(*pFilter, sFunctionQuantity, listOfParameters, listOfParameterTypes);
			}
		}
	}

	void DtaBNFScript::parseTransformRules(ScpStream& script, GrfBlock& block) {
		BNFTransformRules* pTransformer = new BNFTransformRules;
		_listOfTransformRules.push_back(pTransformer);
		pTransformer->setFilter(parseExpression(block, script));
		script.skipEmpty();
		int iBeforeBrace = script.getInputLocation();
		if (!script.isEqualTo('{')) throw UtlException(script, "'{' expected");
		script.skipEmpty();
		if (!script.isEqualTo('}')) {
			script.setInputLocation(iBeforeBrace);
			pTransformer->setPrototypeScript(new DtaTranslateScript(&block));
			pTransformer->getPrototypeScript()->parseEmbeddedScript(script);
		}
		script.skipEmpty();
		iBeforeBrace = script.getInputLocation();
		if (!script.isEqualTo('{')) throw UtlException(script, "'{' expected to transform the production rule");
		script.skipEmpty();
		if (!script.isEqualTo('}')) {
			script.setInputLocation(iBeforeBrace);
			pTransformer->setProductionRuleScript(new DtaTranslateScript(&block));
			pTransformer->getProductionRuleScript()->parseEmbeddedScript(script);
		}
	}

	BNFClause& DtaBNFScript::parseBNFClause(ScpStream& script, GrfBlock& block, int iStatementBeginning, const std::string& sClauseName, bool bOverload) {
		if (!_listOfTransformRules.empty()) {
			int iLocation = script.getInputLocation();
			std::string sCompleteClauseName = sClauseName;
			script.skipEmpty();
			if (script.isEqualTo('<')) {
				script.skipEmpty();
				std::string sText;
				bool bString = false;
				if (script.readIdentifier(sText) || script.readPythonString(sText) && (bString = true)) {
					script.skipEmpty();
					if (script.isEqualTo('>')) {
						if (bString) sText = "\"" + CGRuntime::composeCLikeString(sText) + "\"";
						sCompleteClauseName += "<" + sText + ">";
					} else {
						// syntax error, to resolve further
						script.setInputLocation(iLocation);
					}
				} else {
					// syntax error, to resolve further
					script.setInputLocation(iLocation);
				}
			}
			if (script.findString("::=")) {
				int iStartProductionRule = script.getInputLocation();
				char cChar = '\0';
				script.setOutputLocation(iStartProductionRule - 3);
				script.writeBinaryData(&cChar, 1);
				std::string sPrototype = script.readBuffer() + iStatementBeginning;
				cChar = ':';
				script.setOutputLocation(iStartProductionRule - 3);
				script.writeBinaryData(&cChar, 1);
				int iBrackets = 0;
				bool bInstruction = false;
				bool bFail = true;
				std::string sText;
				while (bFail && script.skipEmpty()) {
					switch(script.readChar()) {
						case '\'':
						case '\"':
							script.goBack();
							script.readStringOrCharLiteral(sText);
							break;
						case '%':
							if (!script.isEqualTo('>')) break;
						case '@':
							do {
								if (script.isEqualTo('<') && script.isEqualTo('%')) break;
								script.isEqualTo('\\');
							} while (!script.isEqualTo('@'));
							break;
						case '{':
							bInstruction = false;
							iBrackets++;
							break;
						case '}':
							iBrackets--;
							break;
						case '=':
							if (!bInstruction) bInstruction = script.isEqualTo('>');
							break;
						case ';':
							if (iBrackets == 0) {
								if (!bInstruction) bFail = false;
								else bInstruction = false;
							}
					}
				}
				if (!bFail) {
					int iEndProductionRule = script.getInputLocation() - 1;
					cChar = '\0';
					script.setOutputLocation(iEndProductionRule);
					script.writeBinaryData(&cChar, 1);
					std::string sProductionRule = script.readBuffer() + iStartProductionRule;
					cChar = ';';
					script.setOutputLocation(iEndProductionRule);
					script.writeBinaryData(&cChar, 1);
					for (std::list<BNFTransformRules*>::const_iterator i = _listOfTransformRules.begin(); i != _listOfTransformRules.end(); ++i) {
						// if the current rule transformer applies with success,
						// the new rule replaces the old one in the input stream
						if ((*i)->applyRuleTransformer(script, block, iStatementBeginning, iStartProductionRule, sCompleteClauseName, sPrototype, sProductionRule)) break;
					}
				}
				script.setInputLocation(iLocation);
			}
		}
		std::vector<std::string> listOfParameters;
		std::vector<EXPRESSION_TYPE> listOfParameterTypes;
		std::string sTemplateKey;
		bool bGenericKey = false;
		script.skipEmpty();
		if (script.isEqualTo('<')) {
			script.skipEmpty();
			if (!script.readPythonString(sTemplateKey)) {
				if (script.readIdentifier(sTemplateKey)) {
					if (sTemplateKey != "true") {
						if (sTemplateKey == "false") sTemplateKey = "";
						else bGenericKey = true;
					}
				} else throw UtlException(script, "template key expected for clause '" + sClauseName + "'");
			}
			script.skipEmpty();
			if (!script.isEqualTo('>')) throw UtlException(script, "'>' to end the template key '" + sTemplateKey + "' for clause '" + sClauseName + "'");
			script.skipEmpty();
		}
		if (script.isEqualTo('(')) {
			do {
				std::string sParameter;
				script.skipEmpty();
				if (!script.readIdentifier(sParameter)) throw UtlException(script, "parameter expected while parsing a BNF clause for declaration");
				if (sParameter == sClauseName) throw UtlException(script, "the parameter '" + sParameter + "' cannot hold the same name as the BNF rule");
				listOfParameters.push_back(sParameter);
				script.skipEmpty();
				if (!script.isEqualTo(':')) throw UtlException(script, "':' expected after parameter '" + sParameter + "' while parsing a BNF clause for declaration");
				script.skipEmpty();
				std::string sType;
				if (!script.readIdentifier(sType)) throw UtlException(script, "parameter type expected for '" + sParameter + "' while parsing a BNF clause for declaration");
				EXPRESSION_TYPE exprType;
				if (sType == "node") exprType = NODE_EXPRTYPE;
				else if (sType == "reference") exprType = REFERENCE_EXPRTYPE;
				else if (sType == "value") exprType = VALUE_EXPRTYPE;
				else if (sType == "variable") {
					std::string sErrorMessage = script.getMessagePrefix() + "warning! parameter type 'variable' is obsolete since version 3.8.7 -> replace it by 'node'";
					if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
					exprType = NODE_EXPRTYPE;
				} else throw UtlException(script, "unknown type '" + sType + "' for parameter '" + sParameter + "' while parsing a BNF clause for declaration");
				exprType = parseClassType(block, script, exprType);
				listOfParameterTypes.push_back(exprType);
				script.skipEmpty();
			} while (script.isEqualTo(','));
			if (!script.isEqualTo(')')) throw UtlException(script, "')' expected while parsing a BNF clause declaration");
			script.skipEmpty();
		}
		if (!script.isEqualTo(':')) throw UtlException(script, "'::=' or ':' expected after parsing a BNF clause declaration");
		int iReturnType = BNFClause::NO_RETURN_TYPE;
		IGNORE_MODE ePreprocessingIgnoreMode = UNDEFINED_IGNORE;
		BNFClause* pPreprocessingIgnoreClause = NULL;
		if (!script.isEqualTo(':')) {
			std::string sReturnType;
			bool bDefinitionExpected = false;
			script.skipEmpty();
			if (script.readIdentifier(sReturnType)) {
				if (sReturnType == "list") iReturnType = BNFClause::LIST_RETURN_TYPE;
				else if (sReturnType == "node") iReturnType = BNFClause::NODE_RETURN_TYPE;
				else if (sReturnType == "value") iReturnType = BNFClause::VALUE_RETURN_TYPE;
				else throw UtlException(script, "'list' or 'node' expected instead of '" + sReturnType + "', while parsing return type of a BNF clause for declaration");
				script.skipEmpty();
				if (script.isEqualTo("::=")) bDefinitionExpected = true;
				else if (!script.isEqualTo(':')) throw UtlException(script, "'::=' expected after parsing a BNF clause declaration");
			}
			if (!bDefinitionExpected) {
				if (script.isEqualTo('#')) {
					if (script.isEqualTo('!')) {
						if (!script.isEqualToIdentifier("ignore")) throw UtlException(script, "'#!ignore' expected as preprocessing of a BNF clause");
						ePreprocessingIgnoreMode = NOT_IGNORE;
					} else if (script.isEqualToIdentifier("ignore")) {
						ePreprocessingIgnoreMode = parseIgnoreMode(script, pPreprocessingIgnoreClause);
					} else {
						throw UtlException(script, "'::=' or ':' or '#ignore' expected after parsing a BNF clause declaration");
					}
					script.skipEmpty();
				}
				script.skipEmpty();
				if (!script.isEqualTo("::=")) throw UtlException(script, "'::=' expected after parsing a BNF clause declaration");
			}
		} else if (!script.isEqualTo('=')) throw UtlException(script, "'::=' expected after parsing a BNF clause declaration");
		BNFClause& myClause = buildClause(script, block, sClauseName, sTemplateKey, bGenericKey, iReturnType, listOfParameters, listOfParameterTypes, bOverload);
		if (myClause.getNbCommands() > 0) throw UtlException(script, "clause '" + myClause.getSignature() + "' has already been described");
		// register the clause name as a local variable, if returned value/node expected
		if ((iReturnType == BNFClause::LIST_RETURN_TYPE) || (iReturnType == BNFClause::NODE_RETURN_TYPE)) {
			myClause.addLocalVariable(sClauseName, NODE_EXPRTYPE);
		} else if (iReturnType == BNFClause::VALUE_RETURN_TYPE) {
			myClause.addLocalVariable(sClauseName, VALUE_EXPRTYPE);
		}
		// register the generic template key, if any
		if (bGenericKey && !sTemplateKey.empty()) {
			myClause.addLocalVariable(sTemplateKey, VALUE_EXPRTYPE);
		}
		GrfBlock* pParent;
		if (ePreprocessingIgnoreMode != UNDEFINED_IGNORE) {
			myClause.setPreprocessingIgnoreMode(ePreprocessingIgnoreMode, pPreprocessingIgnoreClause);
			BNFIgnore* pIgnore = new BNFIgnore(this, &myClause, ePreprocessingIgnoreMode, pPreprocessingIgnoreClause);
			myClause.add(pIgnore);
			pParent = pIgnore;
		} else {
			pParent = &myClause;
		}
		script.skipEmpty();
		parseBNFDisjunction(script, myClause, *pParent, _bNoCase, NULL);
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "';' expected at the end of a BNF clause definition");
		return myClause;
	}

	void DtaBNFScript::parseBNFDisjunction(ScpStream& script, BNFClause& rule, GrfBlock& block, bool bNoCase, BNFStepper* pStepperRE) {
		BNFDisjunction* pDisjunction = new BNFDisjunction(this, &block);
		if (requiresParsingInformation()) pDisjunction->setParsingInformation(getFilenamePtr(), script);
		block.add(pDisjunction);
		parseBNFConjunction(script, rule, *pDisjunction, false, bNoCase, pStepperRE);
		if (script.isEqualTo('|')) {
			do {
				parseBNFConjunction(script, rule, *pDisjunction, false, bNoCase, pStepperRE);
			} while (script.isEqualTo('|'));
		} else {
			pDisjunction->moveCommands(block);
			block.removeCommand(pDisjunction);
		}
	}

	void DtaBNFScript::parseBNFConjunction(ScpStream& script, BNFClause& rule, GrfBlock& block, bool bContinue, bool bNoCase, BNFStepper* pStepperRE) {
		BNFConjunction* pConjunction = new BNFConjunction(this, &block);
		if (requiresParsingInformation()) pConjunction->setParsingInformation(getFilenamePtr(), script);
		block.add(pConjunction);
		parseBNFSequence(script, rule, *pConjunction, bContinue, bNoCase, pStepperRE);
		if (pConjunction->getNbCommands() == 1) {
			pConjunction->moveCommands(block);
			block.removeCommand(pConjunction);
		}
	}

	void DtaBNFScript::parseBNFSequence(ScpStream& script, BNFClause& rule, GrfBlock& block, bool bContinue, bool bNoCase, BNFStepper* pStepperRE) {
		bool bSuccess = parseBNFLitteral(script, rule, block, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE);
		if (!bSuccess) {
			int iChar =  script.readChar();
			std::string sChar;
			if (iChar < 0) sChar = "end of file";
			else sChar = std::string("'") + (char) iChar + "'";
			throw UtlException(script, "literal expected, instead of " + sChar);
		}
		while (parseBNFLitteral(script, rule, block, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
	}

	ExprScriptVariable* DtaBNFScript::parseBNFVariableExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptVariable* pVariableExpr = NULL;
		script.skipEmpty();
		std::string sVariableName;
		if (script.readIdentifier(sVariableName)) {
			block.addBNFLocalVariable(sVariableName, NODE_EXPRTYPE);
			pVariableExpr = new ExprScriptVariable(sVariableName.c_str());
		}
		return parseVariableExpression(block, script, pVariableExpr);
	}

	ExprScriptVariable* DtaBNFScript::parseLiteralAssignment(ScpStream& script, GrfBlock& block, std::vector<std::string>& listOfConstants, bool& bConcatVariable, bool bVariableAllowed) {
		ExprScriptVariable* pVariableExpr = NULL;
		script.skipEmpty();
		int iLocation = script.getInputLocation();
		if (script.isEqualTo(':')) {
			bool bTryVariable = false;
			script.skipEmpty();
			if (script.isEqualTo('\"')) {
				script.goBack();
				std::string sConstant;
				if (!script.readPythonString(sConstant)) throw UtlException(script, "constant string expected");
				listOfConstants.push_back(sConstant);
				bTryVariable = bVariableAllowed;
			} else if (script.isEqualTo('\'')) {
				script.goBack();
				int iChar;
				if (!script.readCharLiteral(iChar)) throw UtlException(script, "char literal expected");
				listOfConstants.push_back(std::string(1, (unsigned char) iChar));
				bTryVariable = bVariableAllowed;
			} else if (script.isEqualTo('{')) {
				do {
					script.skipEmpty();
					std::string sConstant;
					if (!script.readStringOrCharLiteral(sConstant)) throw UtlException(script, "constant string expected");
					listOfConstants.push_back(sConstant);
					script.skipEmpty();
				} while (script.isEqualTo(','));
				if (!script.isEqualTo('}')) throw UtlException(script, "'}' expected to close a set of constant strings");
				bTryVariable = bVariableAllowed;
			} else if (bVariableAllowed) {
				bConcatVariable = script.isEqualTo('+');
				if (hasTargetLanguage()) {
					pVariableExpr = parseAlienVariableExpression(block, script);
				} else {
					pVariableExpr = parseBNFVariableExpression(block, script);
				}
			} else {
				script.setInputLocation(iLocation);
			}
			if (bTryVariable) {
				script.skipEmpty();
				if (script.isEqualTo(':')) {
					bConcatVariable = script.isEqualTo('+');
					if (hasTargetLanguage()) {
						pVariableExpr = parseAlienVariableExpression(block, script);
					} else {
						pVariableExpr = parseBNFVariableExpression(block, script);
					}
				}
			}
		}
		return pVariableExpr;
	}

	bool DtaBNFScript::parseBNFLitteral(ScpStream& script, BNFClause& rule, GrfBlock& block, bool& bContinue, bool& bNoCase, bool bLiteralOnly, BNFStepper* pStepperRE) {
		bool bSuccess = true;
		script.skipEmpty();
		if (script.isEqualTo('\"')) {
			script.goBack();
			std::string sText;
			if (!script.readPythonString(sText)) throw UtlException(script, "string literal expected into BNF clause definition");
			BNFString* pString = new BNFString(this, &block, sText, bContinue, bNoCase);
			if (requiresParsingInformation()) pString->setParsingInformation(getFilenamePtr(), script);
			block.add(pString);
			std::vector<std::string> listOfConstants;
			bool bConcat;
			ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
			pString->setVariableToAssign(pVarToAssign, bConcat, rule);
			if (!listOfConstants.empty()) throw UtlException(script, "it has no sense to match a string literal with a constant string");
		} else if (script.isEqualTo('\'')) {
			script.goBack();
			int iChar;
			if (!script.readCharLiteral(iChar)) throw UtlException(script, "char literal expected (between quotes)");
			script.skipEmpty();
			if (script.isEqualTo("..")) {
				script.skipEmpty();
				int iEndChar;
				if (!script.readCharLiteral(iEndChar)) throw UtlException(script, "char literal expected (between quotes) after '..'");
				if (iChar > iEndChar) throw UtlException(script, "range expression '..' must be ordered correctly");
				BNFCharBoundaries* pCharBoundaries = new BNFCharBoundaries(this, &block, iChar, iEndChar, bContinue, bNoCase);
				if (requiresParsingInformation()) pCharBoundaries->setParsingInformation(getFilenamePtr(), script);
				block.add(pCharBoundaries);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pCharBoundaries->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pCharBoundaries->setConstantsToMatch(listOfConstants);
			} else {
				BNFCharLitteral* pCharLitteral = new BNFCharLitteral(this, &block, iChar, bContinue, bNoCase);
				if (requiresParsingInformation()) pCharLitteral->setParsingInformation(getFilenamePtr(), script);
				block.add(pCharLitteral);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pCharLitteral->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) throw UtlException(script, "a char literal shouldn't expect a set of constant values");
			}
		} else if (script.isEqualTo('~') || script.isEqualTo('^')) {
			BNFComplementary* pComplementary = new BNFComplementary(this, &block, bContinue);
			if (requiresParsingInformation()) pComplementary->setParsingInformation(getFilenamePtr(), script);
			block.add(pComplementary);
			bool bNextContinue = false;
			bool bNextNoCase = bNoCase;
			parseBNFLitteral(script, rule, *pComplementary, bNextContinue, bNextNoCase, true /* if '=>' or ':<variable>' after reading this literal, it doesn't belong to it */, pComplementary);
			std::vector<std::string> listOfConstants;
			bool bConcat;
			ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
			pComplementary->setVariableToAssign(pVarToAssign, bConcat, rule);
			if (!listOfConstants.empty()) pComplementary->setConstantsToMatch(listOfConstants);
		} else if (script.isEqualTo("->")) {
			BNFFindToken* pFindToken = new BNFFindToken(this, &block, bContinue);
			if (requiresParsingInformation()) pFindToken->setParsingInformation(getFilenamePtr(), script);
			block.add(pFindToken);
			bool bConcat;
			ExprScriptVariable* pVarToAssign;
			BNFMultiplicityBoundaries* pBoundaries = parseMultiplicity(script, block);
			if (pBoundaries != NULL) {
				pFindToken->setBoundaries(pBoundaries);
				script.skipEmpty();
			}
			if (script.isEqualTo('(')) {
				std::vector<std::string> listOfIntermediateConstants;
				pVarToAssign = parseLiteralAssignment(script, block, listOfIntermediateConstants, bConcat, true);
				pFindToken->setIntermediateVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfIntermediateConstants.empty()) pFindToken->setIntermediateConstantsToMatch(listOfIntermediateConstants);
				script.skipEmpty();
				if (script.isEqualTo('-')) {
					std::vector<std::string> listOfFinalConstants;
					pVarToAssign = parseLiteralAssignment(script, block, listOfFinalConstants, bConcat, true);
					pFindToken->setFinalVariableToAssign(pVarToAssign, bConcat, rule);
					if (!listOfFinalConstants.empty()) pFindToken->setFinalConstantsToMatch(listOfFinalConstants);
					script.skipEmpty();
				}
				if (!script.isEqualTo(')')) {
					parseBNFDisjunction(script, rule, pFindToken->createIntermediateSequence(), bNoCase, pStepperRE);
					if (!script.isEqualTo(')')) throw UtlException(script, "')' expected");
				}
			}
			bool bNextContinue = false;
			bool bNextNoCase = bNoCase;
			parseBNFLitteral(script, rule, *pFindToken, bNextContinue, bNextNoCase, true /* if '=>' or ':<variable>' after reading this literal, it doesn't belong to it */, pFindToken);
			std::vector<std::string> listOfConstants;
			pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
			pFindToken->setVariableToAssign(pVarToAssign, bConcat, rule);
			if (!listOfConstants.empty()) pFindToken->setConstantsToMatch(listOfConstants);
		} else if (script.isEqualTo('!')) {
			BNFNot* pNot = new BNFNot(this, &block, bContinue);
			if (requiresParsingInformation()) pNot->setParsingInformation(getFilenamePtr(), script);
			block.add(pNot);
			bool bNextContinue = false;
			bool bNextNoCase = bNoCase;
			parseBNFLitteral(script, rule, *pNot, bNextContinue, bNextNoCase, true /* if '=>' or ':<variable>' after reading this literal, it doesn't belong to it */, pStepperRE);
		} else if (script.isEqualTo('[')) {
			BNFMultiplicity* pMultiplicity = new BNFMultiplicity(this, &block, bContinue);
			if (requiresParsingInformation()) pMultiplicity->setParsingInformation(getFilenamePtr(), script);
			block.add(pMultiplicity);
			parseBNFDisjunction(script, rule, *pMultiplicity, bNoCase, pStepperRE);
			if (!script.isEqualTo(']')) throw UtlException(script, "']' expected into BNF clause definition");
			parseMultiplicity(script, block, &pMultiplicity->getBoundaries());
			std::vector<std::string> listOfConstants;
			bool bConcat;
			ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
			pMultiplicity->setVariableToAssign(pVarToAssign, bConcat, rule);
			if (!listOfConstants.empty()) pMultiplicity->setConstantsToMatch(listOfConstants);
		} else if (script.isEqualTo('#')) {
			int iSharpLocation = script.getInputLocation() - 1;
			std::string sIdentifier;
			if (!script.readIdentifier(sIdentifier)) {
				if (script.isEqualTo('!') && script.readIdentifier(sIdentifier)) {
					if (sIdentifier == "ignore") {
						BNFIgnore* pIgnore = new BNFIgnore(this, &block, NOT_IGNORE, NULL);
						if (requiresParsingInformation()) pIgnore->setParsingInformation(getFilenamePtr(), script);
						block.add(pIgnore);
						while (parseBNFLitteral(script, rule, *pIgnore, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
					} else {
						throw UtlException(script, "macro instruction '#!" + sIdentifier + "' doesn't exist");
					}
				} else {
					throw UtlException(script, "macro instruction expected after '#'");
				}
			} else if (sIdentifier == "catch") {
				script.setInputLocation(iSharpLocation);
				return false;
			} else if (sIdentifier == "continue") {
				bool bNextContinue = true;
				script.skipEmpty();
				if (script.isEqualTo('(')) {
					script.skipEmpty();
					std::string sVariableName;
					if (!script.readIdentifier(sVariableName)) throw UtlException(script, "variable name expected to catch the original syntax error");
					if (block.getVariable(sVariableName) == UNKNOWN_EXPRTYPE) block.addBNFLocalVariable(sVariableName, VALUE_EXPRTYPE);
					script.skipEmpty();
					if (!script.isEqualTo(',')) throw UtlException(script, "',' expected");
					std::auto_ptr<ExprScriptExpression> pErrorMessageExpr(parseExpression(block, script));
					script.skipEmpty();
					if (!script.isEqualTo(')')) throw UtlException(script, "')' expected to close the expression of '#continue'");
					BNFContinue* pBNFContinue = new BNFContinue(this, &block, new ExprScriptVariable(sVariableName.c_str()), pErrorMessageExpr.release());
					if (requiresParsingInformation()) pBNFContinue->setParsingInformation(getFilenamePtr(), script);
					block.add(pBNFContinue);
					while (parseBNFLitteral(script, rule, *pBNFContinue->getTryBlock(), bNextContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
				} else {
					while (parseBNFLitteral(script, rule, block, bNextContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
				}
			} else if (sIdentifier == "noCase") {
				bool bNextNoCase = true;
				while (parseBNFLitteral(script, rule, block, bContinue, bNextNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "ratchet") {
				BNFRatchet* pBNFRatchet = new BNFRatchet(this, &block);
				if (requiresParsingInformation()) pBNFRatchet->setParsingInformation(getFilenamePtr(), script);
				block.add(pBNFRatchet);
				while (parseBNFLitteral(script, rule, block, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "try") {
				BNFTryCatch* pBNFTryCatch = new BNFTryCatch(this, &block);
				if (requiresParsingInformation()) pBNFTryCatch->setParsingInformation(getFilenamePtr(), script);
				block.add(pBNFTryCatch);
				do {
					script.skipEmpty();
					if (script.isEqualTo('#')) {
						int iLocation = script.getInputLocation();
						script.skipEmpty();
						if (script.isEqualTo("catch")) {
							script.skipEmpty();
							if (!script.isEqualTo('(')) throw UtlException(script, "syntax error, directive '#catch' expects a '('");
							script.skipEmpty();
							std::string sVariableName;
							if (!script.readIdentifier(sVariableName)) throw UtlException(script, "variable name expected to catch the error message");
							if (block.getVariable(sVariableName) == UNKNOWN_EXPRTYPE) block.addBNFLocalVariable(sVariableName, VALUE_EXPRTYPE);
							pBNFTryCatch->setErrorVariable(parseVariableExpression(block, script, new ExprScriptVariable(sVariableName.c_str())));
							script.skipEmpty();
							if (!script.isEqualTo(')')) throw UtlException(script, "syntax error, directive '#catch' expects a ')'");
							break;
						} else {
							script.setInputLocation(iLocation - 1);
						}
					} else {
						script.goBack();
					}
				} while (parseBNFLitteral(script, rule, *pBNFTryCatch->getTryBlock(), bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE));
				if (pBNFTryCatch->getErrorVariable() == NULL) throw UtlException(script, "directive '#catch' expected before the end of the sequence");
			} else if (sIdentifier == "parsedFile") {
				script.skipEmpty();
				if (!isAParseScript()) throw UtlException(script, "directive '#parsedFile' is available on parsing scripts only");
				GrfParsedFile* pParsedFile = new GrfParsedFile(&block);
				if (requiresParsingInformation()) pParsedFile->setParsingInformation(getFilenamePtr(), script);
				block.add(pParsedFile);
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by clause '#parsedFile'");
				script.skipEmpty();
				pParsedFile->setInputFile(parseExpression(block, script));
				script.skipEmpty();
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by clause '#parsedFile'");
				while (parseBNFLitteral(script, rule, *pParsedFile, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "parsedString") {
				script.skipEmpty();
				if (!isAParseScript()) throw UtlException(script, "directive '#parsedString' is available on parsing scripts only");
				GrfParsedString* pParsedString = new GrfParsedString(&block);
				if (requiresParsingInformation()) pParsedString->setParsingInformation(getFilenamePtr(), script);
				block.add(pParsedString);
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by clause '#parsedString'");
				script.skipEmpty();
				pParsedString->setInputString(parseExpression(block, script));
				script.skipEmpty();
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by clause '#parsedString'");
				while (parseBNFLitteral(script, rule, *pParsedString, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "generatedFile") {
				script.skipEmpty();
				if (!isAGenerateScript()) throw UtlException(script, "directive '#generatedFile' is available on pattern scripts only");
				GrfGeneratedFile* pGeneratedFile = new GrfGeneratedFile(this, &block);
				if (requiresParsingInformation()) pGeneratedFile->setParsingInformation(getFilenamePtr(), script);
				block.add(pGeneratedFile);
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by clause '#generatedFile'");
				script.skipEmpty();
				pGeneratedFile->setOutputFile(parseExpression(block, script));
				script.skipEmpty();
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by clause '#generatedFile'");
				while (parseBNFLitteral(script, rule, *pGeneratedFile, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "generatedString") {
				script.skipEmpty();
				if (!isAGenerateScript()) throw UtlException(script, "directive '#generatedString' is available on pattern scripts only");
				GrfGeneratedString* pGeneratedString = new GrfGeneratedString(this, &block);
				if (requiresParsingInformation()) pGeneratedString->setParsingInformation(getFilenamePtr(), script);
				block.add(pGeneratedString);
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by clause '#generatedString'");
				script.skipEmpty();
				pGeneratedString->setOutputString(parseVariableExpression(block, script));
				script.skipEmpty();
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by clause '#generatedString'");
				while (parseBNFLitteral(script, rule, *pGeneratedString, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "appendedFile") {
				script.skipEmpty();
				if (!isAGenerateScript()) throw UtlException(script, "statement modifier 'append_file' is available on pattern scripts only");
				GrfAppendedFile* pAppendedFile = new GrfAppendedFile(this, &block);
				if (requiresParsingInformation()) pAppendedFile->setParsingInformation(getFilenamePtr(), script);
				block.add(pAppendedFile);
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by clause '#appendedFile'");
				script.skipEmpty();
				pAppendedFile->setOutputFile(parseExpression(block, script));
				script.skipEmpty();
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by clause '#appendedFile'");
				while (parseBNFLitteral(script, rule, *pAppendedFile, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "debug") {
				GrfDebugExecution* pDebug = new GrfDebugExecution(&block);
				if (requiresParsingInformation()) pDebug->setParsingInformation(getFilenamePtr(), script);
				block.add(pDebug);
				while (parseBNFLitteral(script, rule, *pDebug, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "ignore") {
				BNFClause* pIgnoreClause = NULL;
				IGNORE_MODE eMode = parseIgnoreMode(script, pIgnoreClause);
				BNFIgnore* pIgnore = new BNFIgnore(this, &block, eMode, pIgnoreClause);
				if (requiresParsingInformation()) pIgnore->setParsingInformation(getFilenamePtr(), script);
				block.add(pIgnore);
				while (parseBNFLitteral(script, rule, *pIgnore, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "nextStep") {
				if (pStepperRE == NULL) throw UtlException(script, "'#nextStep' must be declared into a jump ('->') or repeat ('[...]*') statement");
				BNFNextStep* pNextStep = new BNFNextStep(this, &block, pStepperRE);
				if (requiresParsingInformation()) pNextStep->setParsingInformation(getFilenamePtr(), script);
				block.add(pNextStep);
			} else if (sIdentifier == "break") {
				BNFBreak* pBreak = new BNFBreak(&block);
				if (requiresParsingInformation()) pBreak->setParsingInformation(getFilenamePtr(), script);
				block.add(pBreak);
				script.skipEmpty();
				if (script.isEqualTo('(')) {
					pBreak->setCondition(parseExpression(block, script));
					if (!script.isEqualTo(')')) throw UtlException(script, "')' expected to close the expression of '#break'");
				}
			} else if (sIdentifier == "empty") {
				BNFEndOfFile* pEndOfFile = new BNFEndOfFile(this, &block, bContinue);
				if (requiresParsingInformation()) pEndOfFile->setParsingInformation(getFilenamePtr(), script);
				block.add(pEndOfFile);
			} else if (sIdentifier == "pushItem") {
				BNFPushItem* pPushItem = new BNFPushItem(this, &block);
				if (requiresParsingInformation()) pPushItem->setParsingInformation(getFilenamePtr(), script);
				block.add(pPushItem);
				script.skipEmpty();
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected after '#pushItem' to pass the array");
				pPushItem->setVariable(parseVariableExpression(block, script));
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected after passing the array to '#pushItem'");
				parseBNFSequence(script, rule, *pPushItem, bContinue, bNoCase, pStepperRE);
			} else if (sIdentifier == "insert") {
				BNFInsert* pInsert = new BNFInsert(this, &block);
				if (requiresParsingInformation()) pInsert->setParsingInformation(getFilenamePtr(), script);
				block.add(pInsert);
				script.skipEmpty();
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected after '#insert' to pass the array");
				pInsert->setVariable(parseVariableExpression(block, script));
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected after passing the array to '#insert'");
				parseBNFSequence(script, rule, *pInsert, bContinue, bNoCase, pStepperRE);
			} else if (sIdentifier == "EOL") {
				BNFEndOfLine* pEndOfLine = new BNFEndOfLine(this, &block, bContinue);
				if (requiresParsingInformation()) pEndOfLine->setParsingInformation(getFilenamePtr(), script);
				block.add(pEndOfLine);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pEndOfLine->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pEndOfLine->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readChar") {
				BNFReadChar* pReadChar = new BNFReadChar(this, &block, bContinue);
				if (requiresParsingInformation()) pReadChar->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadChar);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadChar->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadChar->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readByte") {
				BNFReadByte* pReadByte = new BNFReadByte(this, &block, bContinue);
				if (requiresParsingInformation()) pReadByte->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadByte);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadByte->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadByte->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readAdaString") {
				BNFReadAdaString* pReadAdaString = new BNFReadAdaString(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadAdaString->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadAdaString);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadAdaString->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadAdaString->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readCString") {
				BNFReadCString* pReadCString = new BNFReadCString(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadCString->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadCString);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadCString->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadCString->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readCChar") {
				BNFReadCChar* pReadCChar = new BNFReadCChar(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadCChar->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadCChar);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadCChar->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadCChar->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readPythonString") {
				BNFReadPythonString* pReadPythonString = new BNFReadPythonString(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadPythonString->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadPythonString);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadPythonString->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadPythonString->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readIdentifier") {
				BNFReadIdentifier* pReadIdentifier = new BNFReadIdentifier(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadIdentifier->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadIdentifier);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadIdentifier->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadIdentifier->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readCompleteIdentifier") {
				BNFReadCompleteIdentifier* pReadCompleteIdentifier = new BNFReadCompleteIdentifier(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadCompleteIdentifier->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadCompleteIdentifier);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadCompleteIdentifier->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadCompleteIdentifier->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readInteger") {
				BNFReadInteger* pReadInteger = new BNFReadInteger(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadInteger->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadInteger);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadInteger->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadInteger->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readNumeric") {
				BNFReadNumeric* pReadNumeric = new BNFReadNumeric(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadNumeric->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadNumeric);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadNumeric->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadNumeric->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readText") {
				BNFReadText* pReadText = new BNFReadText(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadText->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadText);
				script.skipEmpty();
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by clause '#readText'");
				pReadText->setText(parseExpression(block, script));
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by clause '#readText'");
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadText->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadText->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readBytes") {
				BNFReadBytes* pReadBytes = new BNFReadBytes(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadBytes->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadBytes);
				script.skipEmpty();
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by clause '#readBytes'");
				pReadBytes->setLength(parseExpression(block, script));
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by clause '#readBytes'");
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadBytes->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadBytes->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readChars") {
				BNFReadChars* pReadChars = new BNFReadChars(this, &block, bContinue, bNoCase);
				if (requiresParsingInformation()) pReadChars->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadChars);
				script.skipEmpty();
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by clause '#readChars'");
				pReadChars->setLength(parseExpression(block, script));
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by clause '#readChars'");
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadChars->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadChars->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "readUptoIgnore") {
				BNFClause* pIgnoreClause = NULL;
				IGNORE_MODE eMode;
				script.skipEmpty();
				if (script.peekChar() == '(') {
					eMode = parseIgnoreMode(script, pIgnoreClause);
				} else {
					eMode = UNDEFINED_IGNORE;
				}
				BNFReadUptoIgnore* pReadUptoIgnore = new BNFReadUptoIgnore(this, &block, eMode, pIgnoreClause, bContinue);
				if (requiresParsingInformation()) pReadUptoIgnore->setParsingInformation(getFilenamePtr(), script);
				block.add(pReadUptoIgnore);
				script.skipEmpty();
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pReadUptoIgnore->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pReadUptoIgnore->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "skipIgnore") {
				BNFClause* pIgnoreClause = NULL;
				IGNORE_MODE eMode;
				script.skipEmpty();
				if (script.peekChar() == '(') {
					eMode = parseIgnoreMode(script, pIgnoreClause);
				} else {
					eMode = UNDEFINED_IGNORE;
				}
				BNFSkipIgnore* pSkipIgnore = new BNFSkipIgnore(this, &block, eMode, pIgnoreClause, bContinue);
				if (requiresParsingInformation()) pSkipIgnore->setParsingInformation(getFilenamePtr(), script);
				block.add(pSkipIgnore);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pSkipIgnore->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pSkipIgnore->setConstantsToMatch(listOfConstants);
			} else if (sIdentifier == "moveAhead") {
				BNFMoveAhead* pMoveAhead = new BNFMoveAhead(&block, bContinue);
				if (requiresParsingInformation()) pMoveAhead->setParsingInformation(getFilenamePtr(), script);
				block.add(pMoveAhead);
			} else if (sIdentifier == "check") {
				BNFCheck* pCheck = new BNFCheck(this, &block, bContinue);
				if (requiresParsingInformation()) pCheck->setParsingInformation(getFilenamePtr(), script);
				block.add(pCheck);
				script.skipEmpty();
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by directive '#check'");
				pCheck->setCondition(parseExpression(block, script));
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by directive '#check'");
			} else if (sIdentifier == "foreach") {
				BNFForeach* pForeach = new BNFForeach(this, &block, bContinue);
				if (requiresParsingInformation()) pForeach->setParsingInformation(getFilenamePtr(), script);
				block.add(pForeach);
				script.skipEmpty();
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected by directive '#foreach'");
				parseForeachListDeclaration(block, script, pForeach);
				script.skipEmpty();
				if (!script.isEqualTo(')')) throw UtlException(script, "')' expected by directive '#foreach'");
				while (parseBNFLitteral(script, rule, *pForeach, bContinue, bNoCase, false /* if '=>' or ':<variable>' after reading this literal, it belongs to it */, pStepperRE)) ;
			} else if (sIdentifier == "super") {
				int iSuperClause = 0;
				std::string sIdentifier;
				for (;;) {
					iSuperClause++;
					script.skipEmpty();
					if (!script.isEqualTo("::")) throw UtlException(script, "'::' expected by directive '#super'");
					script.skipEmpty();
					if (script.isEqualTo('#')) {
						script.skipEmpty();
						if (!script.isEqualToIdentifier("super")) throw UtlException(script, "'::#super' expected");
					} else {
						if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "non-terminal call expected by directive '#super'");
						break;
					}
				}
				BNFClauseCall* pClauseCall = parseBNFClauseCall(script, block, sIdentifier, bContinue, bNoCase);
				pClauseCall->setSuperCallDepth(iSuperClause);
				if (requiresParsingInformation()) pClauseCall->setParsingInformation(getFilenamePtr(), script);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pClauseCall->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pClauseCall->setConstantsToMatch(listOfConstants);
			} else if ((sIdentifier.size() > 3) && (sIdentifier.substr(sIdentifier.size() - 3) == "AST")) {
				ASTCommand* pCommand = NULL;
				if (sIdentifier == "thisAST") pCommand = new ASTThisCommand(script);
				else if (sIdentifier == "valueAST") pCommand = new ASTValueCommand(script);
				else if (sIdentifier == "refAST") pCommand = new ASTRefCommand(script);
				else if (sIdentifier == "slideAST") pCommand = new ASTSlideCommand(script);
				else {
					handleUnknownTokenDirective(sIdentifier, script, rule, block, bContinue, bNoCase, bLiteralOnly, pStepperRE);
				}
				if (pCommand != NULL) {
					int iPos = block.getNbCommands();
					parseBNFLitteral(script, rule, block, bContinue, bNoCase, bLiteralOnly, pStepperRE);
					if (iPos == block.getNbCommands()) {
						throw UtlException(script, "non-terminal call expected after an AST directive (#" + sIdentifier + " here)");
					}
					BNFClauseCall* pNonTerminal = dynamic_cast<BNFClauseCall*>(block.getCommand(iPos));
					if (pNonTerminal == NULL) {
						throw UtlException(script, "non-terminal call expected after an AST directive (#" + sIdentifier + " here)");
					}
					pNonTerminal->addASTCommand(pCommand);
				}
			} else {
				handleUnknownTokenDirective(sIdentifier, script, rule, block, bContinue, bNoCase, bLiteralOnly, pStepperRE);
			}
		} else if (script.isEqualTo("=>")) {
			_bBNFMode = false;
			script.skipEmpty();
			if (script.isEqualTo('{')) {
				script.goBack();
				GrfBlock* pBlock = new GrfBlock(&block);
				if (requiresParsingInformation()) pBlock->setParsingInformation(getFilenamePtr(), script);
				block.add(pBlock);
				if (hasTargetLanguage()) {
					parseAlienBlock(script, *pBlock);
				} else {
					parseBlock(script, *pBlock);
				}
			} else {
				if (hasTargetLanguage()) {
					parseAlienInstruction(script, block);
				} else {
					parseInstruction(script, block);
				}
			}
			_bBNFMode = true;
		} else {
			std::string sIdentifier;
			if (script.readIdentifier(sIdentifier)) {
				BNFClauseCall* pClauseCall = parseBNFClauseCall(script, block, sIdentifier, bContinue, bNoCase);
				if (requiresParsingInformation()) pClauseCall->setParsingInformation(getFilenamePtr(), script);
				std::vector<std::string> listOfConstants;
				bool bConcat;
				ExprScriptVariable* pVarToAssign = parseLiteralAssignment(script, block, listOfConstants, bConcat, !bLiteralOnly);
				pClauseCall->setVariableToAssign(pVarToAssign, bConcat, rule);
				if (!listOfConstants.empty()) pClauseCall->setConstantsToMatch(listOfConstants);
			} else {
				bSuccess = false;
			}
		}
		if (bSuccess && !bLiteralOnly) {
			// binary BNF operators
			script.skipEmpty();
			if (script.isEqualTo("|>")) {
				BNFScanWindow* pScanWindow = new BNFScanWindow(this, &block, bContinue);
				if (requiresParsingInformation()) pScanWindow->setParsingInformation(getFilenamePtr(), script);
				int iIndex = block.getNbCommands() - 1;
				GrfCommand* pLeftMember = block.getCommand(iIndex);
				block.setCommand(iIndex, pScanWindow);
				pScanWindow->setWindow(pLeftMember);
				bool bNextContinue = false;
				bool bNextNoCase = bNoCase;
				parseBNFLitteral(script, rule, *pScanWindow, bNextContinue, bNextNoCase, true /* if '=>' or ':<variable>' after reading this literal, it doesn't belong to it */, NULL);
			} else if (script.isEqualTo("&|")) {
				BNFAndOrJunction* pAndOrJunction = new BNFAndOrJunction(this, &block, bContinue);
				if (requiresParsingInformation()) pAndOrJunction->setParsingInformation(getFilenamePtr(), script);
				int iIndex = block.getNbCommands() - 1;
				GrfCommand* pLeftMember = block.getCommand(iIndex);
				block.setCommand(iIndex, pAndOrJunction);
				pAndOrJunction->setLeftMember(pLeftMember);
				bool bNextContinue = false;
				bool bNextNoCase = bNoCase;
				parseBNFLitteral(script, rule, *pAndOrJunction, bNextContinue, bNextNoCase, true /* if '=>' or ':<variable>' after reading this literal, it doesn't belong to it */, NULL);
			}
		}
		return bSuccess;
	}

	BNFClauseCall* DtaBNFScript::parseBNFClauseCall(ScpStream& script, GrfBlock& block, const std::string& sClauseCallName, bool bContinue, bool bNoCase) {
		BNFClauseCall* pClauseCall = new BNFClauseCall(this, &block, bContinue, bNoCase);
		if (requiresParsingInformation()) pClauseCall->setParsingInformation(getFilenamePtr(), script);
		block.add(pClauseCall);
		script.skipEmpty();
		if (script.isEqualTo('<')) {
			ExprScriptExpression* pTemplateExpression = DtaScript::parseKeyTemplateExpression(block, script);
			pClauseCall->setTemplateExpression(pTemplateExpression);
			script.skipEmpty();
			if (!script.isEqualTo('>')) throw UtlException(script, "'>' expected to end the template expression of clause call '" + sClauseCallName + "'");
			script.skipEmpty();
		}
		if (script.isEqualTo('(')) {
			do {
				ExprScriptExpression* pParameterExpr = DtaScript::parseExpression(block, script);
				pClauseCall->addParameter(pParameterExpr);
				script.skipEmpty();
			} while (script.isEqualTo(','));
			if (!script.isEqualTo(')')) throw UtlException(script, "')' expected");
		}
		BNFClause& myClause = buildClause(script, block, sClauseCallName, pClauseCall->getParameters().size());
		pClauseCall->setClause(&myClause);
		return pClauseCall;
	}

	BNFMultiplicityBoundaries* DtaBNFScript::parseMultiplicity(ScpStream& script, GrfBlock& block, BNFMultiplicityBoundaries* boundaries) {
		BNFMultiplicityBoundaries* pResult = NULL;
		script.skipEmpty();
		if (script.isEqualTo('*')) {
			if (boundaries == NULL) boundaries = new BNFMultiplicityBoundaries;
			pResult = boundaries;
			boundaries->setMultiplicity(0, -1);
		} else if (script.isEqualTo('+')) {
			if (boundaries == NULL) boundaries = new BNFMultiplicityBoundaries;
			pResult = boundaries;
			boundaries->setMultiplicity(1, -1);
		} else if (script.isEqualTo('?')) {
			if (boundaries == NULL) boundaries = new BNFMultiplicityBoundaries;
			pResult = boundaries;
			boundaries->setMultiplicity(0, 1);
		} else if (script.isEqualTo("#repeat")) {
			script.skipEmpty();
			if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected after '#repeat'");
			script.skipEmpty();
			std::auto_ptr<ExprScriptExpression> pBegin(parseExpression(block, script));
			script.skipEmpty();
			if (script.isEqualTo(',')) {
				script.skipEmpty();
				std::auto_ptr<ExprScriptExpression> pEnd(parseExpression(block, script));
				script.skipEmpty();
				if (boundaries == NULL) boundaries = new BNFMultiplicityBoundaries;
				boundaries->setMultiplicity(pBegin.release(), pEnd.release());
			} else {
				if (boundaries == NULL) boundaries = new BNFMultiplicityBoundaries;
				boundaries->setMultiplicity(pBegin.release(), NULL);
			}
			if (!script.isEqualTo(')')) {
				if (pResult == NULL) delete boundaries;
				throw UtlException(script, "')' expected to close '#repeat'");
			}
			pResult = boundaries;
		} else {
			int iBegin;
			int iEnd;
			if (script.readInt(iBegin)) {
				if (iBegin < 0) throw UtlException(script, "multiplicity 'm..n' or 'm' doesn't allow negative boundaries");
				script.skipEmpty();
				if (script.isEqualTo("..")) {
					script.skipEmpty();
					if (script.isEqualTo('*') || script.isEqualTo('n')) iEnd = -1;
					else {
						if (!script.readInt(iEnd)) throw UtlException(script, "multiplicity expected as 'm..n' format");
						if (iEnd < iBegin) throw UtlException(script, "multiplicity 'm..n' doesn't allow 'm' being greater than 'n'");
					}
				} else {
					iEnd = iBegin;
				}
				if (boundaries == NULL) boundaries = new BNFMultiplicityBoundaries;
				pResult = boundaries;
			} else {
				iBegin = 1;
				iEnd = 1;
				if (boundaries == NULL) boundaries = new BNFMultiplicityBoundaries;
			}
			boundaries->setMultiplicity(iBegin, iEnd);
		}
		return pResult;
	}

	bool DtaBNFScript::betweenCommands(ScpStream& script, GrfBlock& block) {
		return DtaScript::betweenCommands(script, block);
	}

	void DtaBNFScript::handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block) {
		DtaScript::handleUnknownCommand(sCommand, script, block);
	}

	void DtaBNFScript::handleNotAWordCommand(ScpStream& script, GrfBlock& block) {
		DtaScript::handleNotAWordCommand(script, block);
	}

	void DtaBNFScript::handleUnknownTokenDirective(const std::string& sDirective, ScpStream& script, BNFClause& rule, GrfBlock& block, bool& bContinue, bool& bNoCase, bool bLiteralOnly, BNFStepper* pStepper) {
		throw UtlException(script, "unknown macro instruction '#" + sDirective + "' encountered");
	}

	void DtaBNFScript::compileCppHeaderIncludes(CppCompilerEnvironment& theCompilerEnvironment) const {
		theCompilerEnvironment.getHeader() << "namespace CodeWorker {";
		theCompilerEnvironment.getHeader().endl();
		theCompilerEnvironment.getHeader() << "\tclass CGBNFRuntimeEnvironment;";
		theCompilerEnvironment.getHeader().endl();
		theCompilerEnvironment.getHeader() << "}";
		theCompilerEnvironment.getHeader().endl();
		DtaScript::compileCppHeaderIncludes(theCompilerEnvironment);
	}

	void DtaBNFScript::compileCppFunctions(CppCompilerEnvironment& theCompilerEnvironment) const {
		DtaScript::compileCppFunctions(theCompilerEnvironment);
		for (std::map<std::string, std::list<BNFClause*> >::const_iterator i = _listOfClauses.begin(); i != _listOfClauses.end(); i++) {
			for (std::list<BNFClause*>::const_iterator j = i->second.begin(); j != i->second.end(); j++) {
				(*j)->compileCppFunction(theCompilerEnvironment);
			}
		}
	}

	void DtaBNFScript::compileCppBNFAssignment(CppCompilerEnvironment& theCompilerEnvironment, int iClauseReturnType, ExprScriptVariable& variableToAssign, bool bConcatVariable, const char* tcText) {
		CW_BODY_INDENT;
		variableToAssign.compileCppForBNFSet(theCompilerEnvironment);
		if (iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
			CW_BODY_STREAM << ".pushItem(" << tcText << ");";
		} else if (hasTargetLanguage()) {
			if (bConcatVariable) {
				CW_BODY_STREAM << " += " << tcText << ";";
			} else {
				CW_BODY_STREAM << " = " << tcText << ";";
			}
		} else {
			if (bConcatVariable) {
				CW_BODY_STREAM << ".concatenateValue(" << tcText << ");";
			} else {
				CW_BODY_STREAM << ".setValue(" << tcText << ");";
			}
		}
		CW_BODY_ENDL;
	}

	std::string DtaBNFScript::assignmentToString(ExprScriptVariable* pVariableToAssign, bool bConcatenateVariable) {
		if (pVariableToAssign != NULL) return ((bConcatenateVariable) ? ":+" : ":") + pVariableToAssign->toString();
		return "";
	}

	std::string DtaBNFScript::constantsToString(const std::vector<std::string>& listOfConstants) {
		std::string sText;
		if (listOfConstants.size() == 1) {
			sText = ":\"" + CGRuntime::composeCLikeString(listOfConstants[0]) + "\"";
		} else if (!listOfConstants.empty()) {
			sText = ":{";
			for (std::vector<std::string>::const_iterator i = listOfConstants.begin(); i != listOfConstants.end(); ++i) {
				if (i != listOfConstants.begin()) sText += ", ";
				sText += "\"" + CGRuntime::composeCLikeString(*i) + "\"";
			}
			sText += "}";
		}
		return sText;
	}

	IGNORE_MODE DtaBNFScript::parseIgnoreMode(ScpStream& script, BNFClause*& pPreprocessingIgnoreClause) {
		IGNORE_MODE eMode;
		pPreprocessingIgnoreClause = NULL;
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			if (script.isEqualTo("C++/Doxygen")) eMode = IGNORE_CPP_EXCEPT_DOXYGEN;
			else if (script.isEqualTo("C++")) eMode = IGNORE_CPP;
			else if (script.isEqualTo("JAVA")) eMode = IGNORE_JAVA;
			else if (script.isEqualTo("HTML") || script.isEqualTo("XML")) eMode = IGNORE_HTML;
			else if (script.isEqualTo("blanks")) eMode = IGNORE_BLANKS;
			else if (script.isEqualTo("spaces")) eMode = IGNORE_SPACES;
			else if (script.isEqualTo("Ada")) eMode = IGNORE_ADA;
			else if (script.isEqualTo("LaTeX")) eMode = IGNORE_LATEX;
			else {
				eMode = IGNORE_CLAUSE;
				std::string sKey;
				script.readPythonString(sKey);
				if (sKey.empty()) sKey = "#empty";
				else sKey = "#ignore[" + sKey + "]";
				if (_mapOfIgnoreClauses.find(sKey) == _mapOfIgnoreClauses.end()) {
					throw UtlException(script, "the ignore clause '" + sKey + "' doesn't exist");
				}
				pPreprocessingIgnoreClause = _mapOfIgnoreClauses[sKey];
			}
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "')' or correct mode expected by clause '#ignore'");
		} else {
			eMode = IGNORE_CLAUSE;
			if (_mapOfIgnoreClauses.find("#ignore") == _mapOfIgnoreClauses.end()) {
				throw UtlException(script, "the ignore clause '#ignore' doesn't exist");
			}
			pPreprocessingIgnoreClause = _mapOfIgnoreClauses["#ignore"];
		}
		return eMode;
	}

//##markup##"parsing"
//##begin##"parsing"
	void DtaBNFScript::parseAttachInputToSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		GrfAttachInputToSocket* pAttachInputToSocket = new GrfAttachInputToSocket;
		if (requiresParsingInformation()) pAttachInputToSocket->setParsingInformation(getFilenamePtr(), script);
		block.add(pAttachInputToSocket);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
		script.skipEmpty();
		if (pMethodCaller == NULL) pAttachInputToSocket->setSocket(parseExpression(block, script));
		else pAttachInputToSocket->setSocket(pMethodCaller);
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaBNFScript::parseDetachInputFromSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		GrfDetachInputFromSocket* pDetachInputFromSocket = new GrfDetachInputFromSocket;
		if (requiresParsingInformation()) pDetachInputFromSocket->setParsingInformation(getFilenamePtr(), script);
		block.add(pDetachInputFromSocket);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
		script.skipEmpty();
		if (pMethodCaller == NULL) pDetachInputFromSocket->setSocket(parseExpression(block, script));
		else pDetachInputFromSocket->setSocket(pMethodCaller);
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaBNFScript::parseGoBack(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		GrfGoBack* pGoBack = new GrfGoBack;
		if (requiresParsingInformation()) pGoBack->setParsingInformation(getFilenamePtr(), script);
		block.add(pGoBack);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
		script.skipEmpty();
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaBNFScript::parseSetInputLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		GrfSetInputLocation* pSetInputLocation = new GrfSetInputLocation;
		if (requiresParsingInformation()) pSetInputLocation->setParsingInformation(getFilenamePtr(), script);
		block.add(pSetInputLocation);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
		script.skipEmpty();
		if (pMethodCaller == NULL) pSetInputLocation->setLocation(parseExpression(block, script));
		else pSetInputLocation->setLocation(pMethodCaller);
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaBNFScript::parseAllFloatingLocations(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseAllFloatingLocations(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseAttachOutputToSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseAttachOutputToSocket(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseDetachOutputFromSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseDetachOutputFromSocket(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseIncrementIndentLevel(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseIncrementIndentLevel(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseInsertText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseInsertText(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseInsertTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseInsertTextOnce(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseInsertTextToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseInsertTextToFloatingLocation(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseInsertTextOnceToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseInsertTextOnceToFloatingLocation(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseOverwritePortion(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseOverwritePortion(block, script, pMethodCaller);
	}

	void DtaBNFScript::parsePopulateProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parsePopulateProtectedArea(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseResizeOutputStream(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseResizeOutputStream(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseSetFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseSetFloatingLocation(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseSetOutputLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseSetOutputLocation(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseSetProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseSetProtectedArea(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseWriteBytes(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseWriteBytes(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseWriteText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseWriteText(block, script, pMethodCaller);
	}

	void DtaBNFScript::parseWriteTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
		DtaScript::parseWriteTextOnce(block, script, pMethodCaller);
	}

//##end##"parsing"
}
