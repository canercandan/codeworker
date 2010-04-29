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
#include "UtlTrace.h"

#ifndef WIN32
#	include "UtlString.h" // for Debian/gcc 2.95.4
#endif

#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "GrfExecutionContext.h"
#include "DtaBNFScript.h"
#include "GrfFunction.h"
#include "BNFStepintoHook.h"
#include "BNFStepoutHook.h"
#include "BNFClause.h"
#include "DtaVisitor.h"
#include "ASTCommand.h"
#include "BNFClauseCall.h"

namespace CodeWorker {
	class GrfClauseDepth {
	public:
		GrfClauseDepth() {
			GrfFunction::_iCurrentStackDepth++;
			if (GrfFunction::_iCurrentStackDepth > GrfFunction::_iMaxStackDepth) {
				throw UtlException("stack overflow! You may increase the stack reserved for function calls with option '-stack'.");
			}
		}
		inline ~GrfClauseDepth() {
			GrfFunction::_iCurrentStackDepth--;
		}
	};


	BNFClauseCall::BNFClauseCall(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue, bool bNoCase) : _pBNFScript(pBNFScript), GrfCommand(pParent), _pClause(NULL), _pTemplateExpression(NULL), _pVariableToAssign(NULL), _bContinue(bContinue), _iClauseReturnType(BNFClause::NO_RETURN_TYPE), _bNoCase(bNoCase), _iSuperCallDepth(0), _pASTcommands(NULL) {}

	BNFClauseCall::~BNFClauseCall() {
		for (std::vector<ExprScriptExpression*>::iterator i = _listOfParameters.begin(); i != _listOfParameters.end(); i++) {
			delete *i;
		}
		delete _pTemplateExpression;
		delete _pVariableToAssign;
		delete _pASTcommands;
	}

	void BNFClauseCall::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFClauseCall(*this, env);
	}

	bool BNFClauseCall::isABNFCommand() const { return true; }

	void BNFClauseCall::addASTCommand(ASTCommand* pCommand) {
		if (_pASTcommands == NULL) {
			_pASTcommands = new std::list<ASTCommand*>;
		}
		_pASTcommands->push_back(pCommand);
	}

	void BNFClauseCall::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatVariable = bConcat;
		}
	}

	void BNFClauseCall::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			_bConcatVariable = bConcat;
		}
	}

	void BNFClauseCall::callBeforeExecutionCBK(DtaScriptVariable& visibility) {
		callRecursiveBeforeExecutionCBK(getCurrentExecutionContext(), visibility);
	}

	void BNFClauseCall::callRecursiveBeforeExecutionCBK(GrfExecutionContext* pContext, DtaScriptVariable& visibility) {
		if (pContext != NULL) {
			callRecursiveBeforeExecutionCBK(pContext->getLastExecutionContext(), visibility);
			pContext->handleStartingBNFClause(this);
			pContext->handleBeforeExecutionCBK(this, visibility);
			pContext = pContext->getLastExecutionContext();
		}
	}

	void BNFClauseCall::callAfterExecutionCBK(DtaScriptVariable& visibility) {
		GrfExecutionContext* pContext = getCurrentExecutionContext();
		while (pContext != NULL) {
			pContext->handleAfterExecutionCBK(this, visibility);
			pContext->handleEndingBNFClause(this);
			pContext = pContext->getLastExecutionContext();
		}
	}

	BNFClause& BNFClauseCall::getInstantiatedClause(DtaScriptVariable& visibility, std::string& sInstantiationKey) const {
		BNFClause* pClause = NULL;
		if (_pTemplateExpression != NULL) {
			sInstantiationKey = _pTemplateExpression->getValue(visibility);
			pClause = _pClause->getInstantiatedClause(sInstantiationKey);
			if (pClause == NULL) throw UtlException("template clause '" +  _pClause->getName() + "<\"" + sInstantiationKey + "\">' hasn't been implemented");
		} else {
			pClause = _pClause;
		}
		return *pClause;
	}

	SEQUENCE_INTERRUPTION_LIST BNFClauseCall::executeInternal(DtaScriptVariable& visibility) {
		std::string sInstantiationKey;
		BNFClause& myClause = getInstantiatedClause(visibility, sInstantiationKey);
		std::string sClauseName = myClause.getName();
		if (!sInstantiationKey.empty()) {
			if (myClause.isGenericKey()) sClauseName += "<" + myClause.getTemplateKey() + "=\"" + sInstantiationKey + "\">";
			else sClauseName += "<\"" + sInstantiationKey + "\">";
		}
		if (_pBNFScript->trace()) {
			char tcMessage[80];
			sprintf(tcMessage, "' [%d]", CGRuntime::getInputLocation());
			CGRuntime::traceLine(_pBNFScript->traceIndentation() + "IN '" + sClauseName + tcMessage);
			_pBNFScript->incrementTraceIndentation();
			if (!CGRuntime::inputKey(false).empty()) {
				char tcMessage[512];
				std::string sFilename = CGRuntime::getInputFilename();
				sprintf(tcMessage, "%s(%d): trace mode suspended", sFilename.c_str(), CGRuntime::countInputLines());
				CGRuntime::traceLine(tcMessage);
				std::string sCommand = CGRuntime::inputLine(true, "> ");
			}

		}
		UTLTRACE_STACK_FUNCTION(_sParsingFilePtr, sClauseName.c_str(), _iFileLocation);
		GrfClauseDepth newClauseCall;
		if (myClause.getNbCommands() == 0) {
			throw UtlException("clause '" + myClause.getSignature() + "' has never been implemented");
		}
		std::string sName = "##stack## clause '";
		sName += sClauseName + "'";
		DtaScriptVariable stackClause(&visibility, sName.c_str());
		DtaScriptVariable* pReturnValue = stackClause.getNonRecursiveVariable(myClause.getName().c_str());
		if (!sInstantiationKey.empty() && myClause.isGenericKey()) {
			stackClause.setValueAtVariable(myClause.getTemplateKey().c_str(), sInstantiationKey.c_str());
		}
		int iLocation = CGRuntime::getInputLocation();
		int iNotEmptyLocation = 0; // used only if variable to assign or constant to check
		int iImplicitCopyPosition = -1;
		if (myClause.getPreprocessingIgnoreMode() == UNDEFINED_IGNORE) iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		if ((_pVariableToAssign != NULL) || !_listOfConstants.empty()) {
			iNotEmptyLocation = CGRuntime::getInputLocation();
		}
		if (myClause.isPropagatedParameter()) {
			if (myClause.getParameters().size() < _listOfParameters.size()) {
				char tcExpectedNumber[96];
				sprintf(tcExpectedNumber, "' requires less than %d parameters (%d parameters, propagated)", _listOfParameters.size(), myClause.getParameters().size());
				throw UtlException("call to non-terminal '" + myClause.getSignature() + tcExpectedNumber);
			}
		} else {
			if (myClause.getParameters().size() != _listOfParameters.size()) {
				char tcExpectedNumber[64];
				sprintf(tcExpectedNumber, "' doesn't require %d parameters", _listOfParameters.size());
				throw UtlException("call to non-terminal '" + myClause.getSignature() + tcExpectedNumber);
			}
		}
		std::vector<std::string>::const_iterator j = myClause.getParameters().begin();
		std::vector<EXPRESSION_TYPE>::const_iterator k = myClause.getParameterTypes().begin();
		for (std::vector<ExprScriptExpression*>::const_iterator i = _listOfParameters.begin(); i != _listOfParameters.end(); i++) {
			switch(*k & 0x00FF) {
				case VALUE_EXPRTYPE:
					{
						std::string sValue = (*i)->getValue(visibility);
						stackClause.setValueAtVariable(j->c_str(), sValue.c_str());
					}
					break;
				case ITERATOR_EXPRTYPE:
				case REFERENCE_EXPRTYPE:
				case NODE_EXPRTYPE:
					{
						DtaScriptVariable* pVariable;
						register const ExprScriptVariable* pVarExpr = dynamic_cast<const ExprScriptVariable*>(*i);
						if (pVarExpr == NULL) {
							ExprScriptResolvedVariable* pResolvedVariable = dynamic_cast<ExprScriptResolvedVariable*>(*i);
							if (pResolvedVariable == NULL) {
								std::string sValue = (*i)->getValue(visibility);
								sValue = CGRuntime::composeCLikeString(sValue);
								throw UtlException("in clause '" + myClause.getSignature() + "', parameter '" + *j + ":node' expects a tree node, not the expression \"" + sValue + "\"");
							} else {
								pVariable = pResolvedVariable->getVariable();
							}
						} else {
							pVariable = visibility.getVariable(*pVarExpr);
						}
						DtaScriptVariable* pParameter = stackClause.createNodeArgument(j->c_str(), pVariable);
					}
					break;
			}
			j++;
			k++;
		}
		if (myClause.isPropagatedParameter()) {
			while (j != myClause.getParameters().end()) {
				switch(*k & 0x00FF) {
					case VALUE_EXPRTYPE:
						{
							DtaScriptVariable* pVariable = visibility.getVariable(j->c_str());
							stackClause.setValueAtVariable(j->c_str(), pVariable->getValue());
						}
						break;
					case ITERATOR_EXPRTYPE:
					case REFERENCE_EXPRTYPE:
					case NODE_EXPRTYPE:
						{
							DtaScriptVariable* pVariable = visibility.getVariable(j->c_str());
							DtaScriptVariable* pParameter = stackClause.createNodeArgument(j->c_str(), pVariable);
						}
						break;
				}
				++j;
				++k;
			}
		}
		if (_pBNFScript->getStepintoHook() != NULL) {
			_pBNFScript->getStepintoHook()->executeHook(CGRuntime::getThisInternalNode(), sClauseName, stackClause);
		}
		SEQUENCE_INTERRUPTION_LIST result = myClause.executeClause(stackClause, _iSuperCallDepth);
		if (_pBNFScript->getStepoutHook() != NULL) {
			_pBNFScript->getStepoutHook()->executeHook(CGRuntime::getThisInternalNode(), sClauseName, stackClause, (result != BREAK_INTERRUPTION));
		}
		if (result == NO_INTERRUPTION) {
			j = myClause.getParameters().begin();
			k = myClause.getParameterTypes().begin();
			for (std::vector<ExprScriptExpression*>::const_iterator i = _listOfParameters.begin(); i != _listOfParameters.end(); i++) {
				if ((*k & 0x00FF) == REFERENCE_EXPRTYPE) {
					DtaScriptVariable* pVariable = visibility.getVariableForReferenceAssignment((const ExprScriptVariable&) (*(*i)));
					DtaScriptVariable* pReference = stackClause.getNodeArgument(j->c_str())->getReferencedVariable();
					pVariable->setValue(pReference);
				}
				j++;
				k++;
			}
			if (myClause.isPropagatedParameter()) {
				while (j != myClause.getParameters().end()) {
					if ((*k  & 0x00FF) == REFERENCE_EXPRTYPE) {
						DtaScriptVariable* pVariable = visibility.getVariable(j->c_str(), true, true, false);
						DtaScriptVariable* pReference = stackClause.getNodeArgument(j->c_str())->getReferencedVariable();
						pVariable->setValue(pReference);
					}
					++j;
					++k;
				}
			}
			if (!_listOfConstants.empty()) {
				bool bMatch = false;
				std::string sTextToMatch;
				if (myClause.getReturnType() == BNFClause::VALUE_RETURN_TYPE) {
					const char* tcReturnValue = pReturnValue->getValue();
					if (tcReturnValue != NULL) sTextToMatch = tcReturnValue;
				} else sTextToMatch = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - iNotEmptyLocation);
				for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
					if (_bNoCase) bMatch = (stricmp(i->c_str(), sTextToMatch.c_str()) == 0);
					else bMatch = ((*i) == sTextToMatch);
					if (bMatch) break;
				}
				if (!bMatch) {
					if (_bContinue) CGRuntime::throwBNFExecutionError(toString());
					if (_pBNFScript->getRatchetPosition() <= iLocation) CGRuntime::setInputLocation(iLocation);
					else CGRuntime::throwBNFExecutionError(toString(), "#ratchet position reached");
					if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
					result = BREAK_INTERRUPTION;
				}
			}
			if ((result != BREAK_INTERRUPTION) && (_pVariableToAssign != NULL)) {
				DtaScriptVariable* pVariable = visibility.getVariable(*_pVariableToAssign);
				if (myClause.getReturnType() == BNFClause::LIST_RETURN_TYPE) {
					pVariable->addArrayElements(*pReturnValue);
				} else if (myClause.getReturnType() == BNFClause::NODE_RETURN_TYPE) {
					pVariable->copyAll(*pReturnValue);
				} else if (myClause.getReturnType() == BNFClause::VALUE_RETURN_TYPE) {
					const char* tcReturnValue = pReturnValue->getValue();
					std::string sValue;
					if (tcReturnValue != NULL) sValue = tcReturnValue;
					if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
						pVariable->pushItem(sValue);
					} else {
						if (_bConcatVariable) pVariable->concatValue(sValue.c_str());
						else pVariable->setValue(sValue.c_str());
					}
				} else {
					std::string sText = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - iNotEmptyLocation);
					if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
						pVariable->pushItem(sText);
					} else {
						if (_bConcatVariable) pVariable->concatValue(sText.c_str());
						else pVariable->setValue(sText.c_str());
					}
				}
			}
		} else {
			if (_bContinue) CGRuntime::throwBNFExecutionError(toString());
			if (_pBNFScript->getRatchetPosition() <= iLocation) CGRuntime::setInputLocation(iLocation);
			else CGRuntime::throwBNFExecutionError(toString(), "#ratchet position reached");
			if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
		}
		if (_pBNFScript->trace()) {
			_pBNFScript->decrementTraceIndentation();
			std::string sMessage = _pBNFScript->traceIndentation() + "OUT ";
			if (result == BREAK_INTERRUPTION) sMessage += "fail ";
			else sMessage += "OK ";
			sMessage += "'" + sClauseName+ "'";
			if (result != BREAK_INTERRUPTION) {
				char tcMessage[80];
				sprintf(tcMessage, " [%d]", CGRuntime::getInputLocation());
				sMessage += tcMessage;
			}
			CGRuntime::traceLine(sMessage);
		}
		return result;
	}

	void BNFClauseCall::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		int iCursor = theCompilerEnvironment.newCursor();
		BNFClause& myClause = getClause();
		CW_BODY_INDENT << "// " << toString();CW_BODY_ENDL;
		char tcLocation[64];
		sprintf(tcLocation, "_compilerClauseLocation_%d", iCursor);
		char tcImplicitCopy[64];
		sprintf(tcImplicitCopy, "_compilerClauseImplicitCopy_%d", iCursor);
		CW_BODY_INDENT << "int " << tcLocation << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
		if (myClause.getPreprocessingIgnoreMode() == UNDEFINED_IGNORE) {
			CW_BODY_INDENT << "int " << tcImplicitCopy << " = theEnvironment.skipEmptyChars();";CW_BODY_ENDL;
		} else {
			CW_BODY_INDENT << "int " << tcImplicitCopy << " = -1;";CW_BODY_ENDL;
		}
		if ((_pVariableToAssign != NULL) || !_listOfConstants.empty()) {
			CW_BODY_INDENT << "int _compilerClauseNotEmptyLocation_" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
		}
		// cretae the name of the clause
		std::string sClauseName = myClause.getName();
		bool bTemplateDispatching = false;
		if (_pTemplateExpression != NULL) {
			// call to template clause
			sClauseName = "_compilerTemplateClause_" + sClauseName + "_compilerInstantiation_";
			ExprScriptConstant* pConstant = dynamic_cast<ExprScriptConstant*>(_pTemplateExpression);
			if (pConstant != NULL) {
				BNFClause* pTemplateClause = myClause.getInstantiatedClause(pConstant->getConstant());
				if ((pTemplateClause != 0) && (pTemplateClause->getTemplateKey() == pConstant->getConstant())) {
					// it exists a clause instantiation for the given template key
					sClauseName += theCompilerEnvironment.convertTemplateKey(pConstant->getConstant());
				} else {
					bTemplateDispatching = true;
				}
			} else {
				bTemplateDispatching = true;
			}
		} else if (sClauseName[0] == '#') {
			sClauseName = "_compilerDirectiveClause_" + sClauseName.substr(1);
		} else if (myClause.isATemplateInstantiation()) {
			// default template clause (equivalent to an empty template key)
			sClauseName = "_compilerTemplateClause_" + sClauseName + "_compilerInstantiation_";
		} else {
			sClauseName = "_compilerClause_" + sClauseName;
		}
		bool bReturnedValue = (!_listOfConstants.empty() && (myClause.getReturnType() == BNFClause::VALUE_RETURN_TYPE)) || ((_pVariableToAssign != NULL) && (myClause.getReturnType() != BNFClause::NO_RETURN_TYPE));
		if (bReturnedValue) {
			CW_BODY_INDENT << "CppParsingTree_value _compilerClauseReturnedValue;";
			CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "_compilerClauseSuccess = " << DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(theCompilerEnvironment.getClauseModule(myClause.getName()))) << "::" << sClauseName << "(";
		if (bTemplateDispatching) {
			_pTemplateExpression->compileCppString(theCompilerEnvironment);
			CW_BODY_STREAM << ", ";
		}
		if (bReturnedValue) {
			CW_BODY_STREAM << "_compilerClauseReturnedValue, ";
		}
		int iIndex = 0;
		for (std::vector<ExprScriptExpression*>::const_iterator i = _listOfParameters.begin(); i != _listOfParameters.end(); i++) {
			EXPRESSION_TYPE exprType = myClause.getParameterType(iIndex);
			if ((exprType == VALUE_EXPRTYPE) || (dynamic_cast<ExprScriptAlien*>(*i) != NULL)) (*i)->compileCpp(theCompilerEnvironment);
			else dynamic_cast<ExprScriptVariable*>(*i)->compileCppForGetOrCreateLocal(theCompilerEnvironment);
			CW_BODY_STREAM << ", ";
			iIndex++;
		}
		CW_BODY_STREAM << "theEnvironment);";CW_BODY_ENDL;
		if ((_pVariableToAssign != NULL) || !_listOfConstants.empty()) {
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";
			CW_BODY_ENDL;
			theCompilerEnvironment.incrementIndentation();
			if (!_listOfConstants.empty()) {
				CW_BODY_INDENT << "std::string _compilerClauseText_" << iCursor << " = ";
				if (myClause.getReturnType() == BNFClause::VALUE_RETURN_TYPE) {
					CW_BODY_STREAM << "_compilerClauseReturnedValue.getStringValue();";
				} else {
					CW_BODY_STREAM << "CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - _compilerClauseNotEmptyLocation_" << iCursor << ");";
				}
				CW_BODY_ENDL;
				CW_BODY_INDENT << "_compilerClauseSuccess = ";
				for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
					if (i != _listOfConstants.begin()) CW_BODY_STREAM << " || ";
					CW_BODY_STREAM << "(";
					if (_bNoCase) {
						CW_BODY_STREAM << "stricmp(_compilerClauseText_" << iCursor << ".c_str(), ";
						CW_BODY_STREAM.writeString(*i);
						CW_BODY_STREAM << ") == 0";
					} else {
						CW_BODY_STREAM << "_compilerClauseText_" << iCursor << " == ";
						CW_BODY_STREAM.writeString(*i);
					}
					CW_BODY_STREAM << ")";
				}
				CW_BODY_STREAM << ";";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
				CPP_COMPILER_BNF_SYMBOL_HAS_FAILED;
				CW_BODY_INDENT << "}";CW_BODY_ENDL;
			}
			if (_pVariableToAssign != NULL) {
				if (!_listOfConstants.empty() && !_bContinue) {
					theCompilerEnvironment.incrementIndentation();
					CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
				}
				if (myClause.getReturnType() == BNFClause::NODE_RETURN_TYPE) {
					CW_BODY_INDENT;
					_pVariableToAssign->compileCppForBNFSet(theCompilerEnvironment);
					CW_BODY_STREAM << ".setAll(_compilerClauseReturnedValue);";CW_BODY_ENDL;
				} else {
					if (_listOfConstants.empty()) {
						CW_BODY_INDENT << "std::string _compilerClauseText_" << iCursor << " = ";
						if (myClause.getReturnType() == BNFClause::VALUE_RETURN_TYPE) {
							CW_BODY_STREAM << "_compilerClauseReturnedValue.getStringValue();";
						} else {
							CW_BODY_STREAM << "CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - _compilerClauseNotEmptyLocation_" << iCursor << ");";
						}
						CW_BODY_ENDL;
					}
					char tcText[32];
					sprintf(tcText, "_compilerClauseText_%d", iCursor);
					_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, tcText);
				}
				if (!_listOfConstants.empty() && !_bContinue) {
					theCompilerEnvironment.decrementIndentation();
					CW_BODY_INDENT << "}";CW_BODY_ENDL;
				}
			}
			theCompilerEnvironment.decrementIndentation();
			if (_bContinue) {
				CW_BODY_INDENT << "} else CGRuntime::throwBNFExecutionError(";
				CW_BODY_STREAM.writeString(toString());
				CW_BODY_STREAM << ");";
			} else {
				CW_BODY_INDENT << "} else CGRuntime::setInputLocation(" << tcLocation << ");";
			}
			CW_BODY_ENDL;
		} else {
			CW_BODY_INDENT << "if (!_compilerClauseSuccess) ";
			if (_bContinue) {
				CW_BODY_STREAM << "CGRuntime::throwBNFExecutionError(";
				CW_BODY_STREAM.writeString(toString());
				CW_BODY_STREAM << ");";
			} else {
				CW_BODY_STREAM << "CGRuntime::setInputLocation(" << tcLocation << ");";
			}
			CW_BODY_ENDL;
		}
	}

	std::string BNFClauseCall::toString() const {
		std::string sString;
		if (_bContinue) sString = "#continue ";
		sString += getClause().getName();
		if (!getClause().getTemplateKey().empty()) sString += "<\"" + getClause().getTemplateKey() + "\">";
		if (!_listOfParameters.empty()) {
			if (_pTemplateExpression != NULL) sString += "<" + _pTemplateExpression->toString() + ">";
			sString += "(";
			for (std::vector<ExprScriptExpression*>::const_iterator i = _listOfParameters.begin(); i != _listOfParameters.end(); i++) {
				if (i != _listOfParameters.begin()) sString += ", ";
				sString += (*i)->toString();
			}
			sString += ")";
		}
		return sString + DtaBNFScript::constantsToString(_listOfConstants) + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
	}



	BNFRootClauseCall::BNFRootClauseCall(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bNoCase)
		: BNFClauseCall(pBNFScript, pParent, false, bNoCase) {
	}
	BNFRootClauseCall::~BNFRootClauseCall() {}

	void BNFRootClauseCall::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::string sIgnoreClauseName;
		if (getBNFScript().getIgnoreClause() != NULL) {
			sIgnoreClauseName = getBNFScript().getIgnoreClause()->getName();
			if (sIgnoreClauseName[0] == '#') sIgnoreClauseName = "_compilerDirectiveClause_" + sIgnoreClauseName.substr(1);
			sIgnoreClauseName = DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(theCompilerEnvironment.getClauseModule(getBNFScript().getIgnoreClause()->getName()))) + "::" + sIgnoreClauseName;
		} else if (getClause().getPreprocessingIgnoreMode() != UNDEFINED_IGNORE) {
			IGNORE_MODE eMode = (IGNORE_MODE) getClause().getPreprocessingIgnoreMode();
			if (eMode != NOT_IGNORE) {
				if (eMode == IGNORE_CLAUSE) sIgnoreClauseName = "_compilerDirectiveClause_" + sIgnoreClauseName.substr(1);
				sIgnoreClauseName = DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(theCompilerEnvironment.getClauseModule(getBNFScript().getIgnoreClause()->getName()))) + "::" + sIgnoreClauseName;
			}
		}
		if (sIgnoreClauseName.empty()) {
			sIgnoreClauseName = "NULL";
		} else {
			sIgnoreClauseName = "new CodeWorker::DEFAULT_EXECUTE_CLAUSE(" + sIgnoreClauseName + ")";
		}
		std::string sClauseName = getClause().getName();
		if (getClause().isATemplateInstantiation()) sClauseName = "_compilerTemplateClause_" + sClauseName + "_compilerInstantiation_" + theCompilerEnvironment.convertTemplateKey(getClause().getTemplateKey());
		else if (sClauseName[0] == '#') sClauseName = "_compilerDirectiveClause_" + sClauseName.substr(1);
		else sClauseName = "_compilerClause_" + sClauseName;
		CW_BODY_INDENT << "CGBNFRuntimeEnvironment theEnvironment(" << sIgnoreClauseName << ", " << getBNFScript().getIgnoreMode() << ", " << ((getBNFScript().implicitCopy()) ? "true" : "false") << ");";CW_BODY_ENDL;
		if (getBNFScript().hasCoverageRecording()) {
			CW_BODY_INDENT << "theEnvironment.activateMatchingAreas();";CW_BODY_ENDL;
			const std::map<std::string, std::list<BNFClause*> >& allClauses = getBNFScript().getClauses();
			for (std::map<std::string, std::list<BNFClause*> >::const_iterator i = allClauses.begin(); i != allClauses.end(); ++i) {
				for (std::list<BNFClause*>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
					CW_BODY_INDENT << "theEnvironment.addClauseSignature(\"" << CGRuntime::composeCLikeString((*j)->getSignature()) << "\");";CW_BODY_ENDL;
				}
			}
		}
		CW_BODY_INDENT << DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(theCompilerEnvironment.getClauseModule(getClause().getName()))) << "::" << sClauseName << "(";
		int iIndex = 0;
		for (std::vector<ExprScriptExpression*>::const_iterator i = getParameters().begin(); i != getParameters().end(); i++) {
			EXPRESSION_TYPE exprType = getClause().getParameterType(iIndex);
			if ((exprType == VALUE_EXPRTYPE) || (dynamic_cast<ExprScriptAlien*>(*i) != NULL)) (*i)->compileCpp(theCompilerEnvironment);
			else dynamic_cast<ExprScriptVariable*>(*i)->compileCppForGetOrCreateLocal(theCompilerEnvironment);
			CW_BODY_STREAM << ", ";
			iIndex++;
		}
		CW_BODY_STREAM << "theEnvironment);";CW_BODY_ENDL;
		if (getBNFScript().hasCoverageRecording()) {
			CW_BODY_INDENT << "theEnvironment.storeMatchingAreas(";
			getBNFScript().getMatchingAreasContainer()->compileCppForSet(theCompilerEnvironment);
			CW_BODY_STREAM << ");";CW_BODY_ENDL;
		}
	}
}
