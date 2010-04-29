/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2006 Cédric Lemaire

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

//##protect##"INCLUDES"
#include "BNFAndOrJunction.h"
#include "BNFClause.h"
#include "BNFClauseCall.h"
#include "BNFConjunction.h"
#include "BNFDisjunction.h"
#include "BNFFindToken.h"
#include "BNFIgnore.h"
#include "BNFInsert.h"
#include "BNFMoveAhead.h"
#include "BNFMultiplicity.h"
#include "BNFPushItem.h"
#include "BNFRatchet.h"
#include "BNFScanWindow.h"
#include "BNFTransformationMode.h"
#include "BNFTryCatch.h"

#include "ScpStream.h"
#include "DtaScriptVariable.h"
#include "CGRuntime.h"
#include "ASTCommand.h"
#include "DtaBNFScript.h"
//##protect##"INCLUDES"

#include "DtaASTNavigator.h"

namespace CodeWorker {
	DtaASTNavigator::~DtaASTNavigator() {}

	void DtaASTNavigator::visitBNFAndOrJunction(BNFAndOrJunction& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFAndOrJunction"
		if (theObject.getLeftMember()->isABNFCommand()) theObject.getLeftMember()->accept(*this, env);
		iterateSequence(theObject, env);
//##protect##"BNFAndOrJunction"
	}

	void DtaASTNavigator::visitBNFBreak(BNFBreak& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFBreak"
//##protect##"BNFBreak"
	}

	void DtaASTNavigator::visitBNFCharBoundaries(BNFCharBoundaries& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFCharBoundaries"
//##protect##"BNFCharBoundaries"
	}

	void DtaASTNavigator::visitBNFCharLitteral(BNFCharLitteral& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFCharLitteral"
//##protect##"BNFCharLitteral"
	}

	void DtaASTNavigator::visitBNFCheck(BNFCheck& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFCheck"
//##protect##"BNFCheck"
	}

	void DtaASTNavigator::visitBNFClause(BNFClause& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFClause"
		iterateSequence(theObject, env);
//##protect##"BNFClause"
	}

	void DtaASTNavigator::visitBNFClauseCall(BNFClauseCall& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFClauseCall"
		DtaASTNavigatorEnvironment& ASTenv = (DtaASTNavigatorEnvironment&) env;
		std::string sNonTerminal = theObject.getClause().getName();
		DtaASTStructure::Clause::Sequence::NonTerminal* pNonTerminalParams = ASTenv._pCurrentSequence->_allNonTerminals[sNonTerminal];
		if (pNonTerminalParams == NULL) {
			ASTenv._pCurrentSequence->_allNonTerminals[sNonTerminal] = new DtaASTStructure::Clause::Sequence::NonTerminal;
		} else {
			pNonTerminalParams->addCallInSequence(1);
		}
//##protect##"BNFClauseCall"
	}

	void DtaASTNavigator::visitBNFComplementary(BNFComplementary& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFComplementary"
		// nothing to do! Consuming by a non-terminal will never happen!
//##protect##"BNFComplementary"
	}

	void DtaASTNavigator::visitBNFConjunction(BNFConjunction& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFConjunction"
		iterateSequence(theObject, env);
//##protect##"BNFConjunction"
	}

	void DtaASTNavigator::visitBNFDisjunction(BNFDisjunction& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFDisjunction"
		std::auto_ptr<DtaASTStructure::Clause::Sequence> pSequence;
		const std::vector<GrfCommand*>& commands = theObject.getCommands();
		for (std::vector<GrfCommand*>::const_iterator i = commands.begin(); i != commands.end(); ++i) {
			if ((*i)->isABNFCommand()) {
				DtaASTStructure::Clause::Sequence* pAlternativeSequence = new DtaASTStructure::Clause::Sequence;
				DtaASTNavigatorEnvironment subEnv(*pAlternativeSequence);
				(*i)->accept(*this, subEnv);
				if (pSequence.get() != NULL) {
					pSequence->merge(*pAlternativeSequence);
					delete pAlternativeSequence;
				} else {
					pSequence = std::auto_ptr<DtaASTStructure::Clause::Sequence>(pAlternativeSequence);
				}
			}
		}
		if (pSequence.get() != NULL) {
			((DtaASTNavigatorEnvironment&) env)._pCurrentSequence->addSequence(*pSequence);
		}
//##protect##"BNFDisjunction"
	}

	void DtaASTNavigator::visitBNFEndOfFile(BNFEndOfFile& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFEndOfFile"
//##protect##"BNFEndOfFile"
	}

	void DtaASTNavigator::visitBNFEndOfLine(BNFEndOfLine& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFEndOfLine"
//##protect##"BNFEndOfLine"
	}

	void DtaASTNavigator::visitBNFFindToken(BNFFindToken& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFFindToken"
		iterateSequence(theObject, env);
//##protect##"BNFFindToken"
	}

	void DtaASTNavigator::visitBNFIgnore(BNFIgnore& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFIgnore"
		iterateSequence(theObject, env);
//##protect##"BNFIgnore"
	}

	void DtaASTNavigator::visitBNFInsert(BNFInsert& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFInsert"
		iterateSequence(theObject, env);
//##protect##"BNFInsert"
	}

	void DtaASTNavigator::visitBNFMoveAhead(BNFMoveAhead& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFMoveAhead"
		iterateSequence(theObject, env);
//##protect##"BNFMoveAhead"
	}

	void DtaASTNavigator::visitBNFMultiplicity(BNFMultiplicity& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFMultiplicity"
		DtaASTNavigatorEnvironment subEnv((DtaASTNavigatorEnvironment&) env);
		iterateSequence(theObject, subEnv);
		BNFMultiplicityBoundaries& boundaries = theObject.getBoundaries();
		int iMin, iMax;
		if (!boundaries.bConstantBoundaries) {
			iMin = -1;
			iMax = -1;
		} else {
			iMin = boundaries.choice.constant._iBegin;
			iMax = boundaries.choice.constant._iEnd;
		}
		subEnv._pCurrentSequence->repeatCalls(iMin, iMax);
//##protect##"BNFMultiplicity"
	}

	void DtaASTNavigator::visitBNFNextStep(BNFNextStep& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFNextStep"
//##protect##"BNFNextStep"
	}

	void DtaASTNavigator::visitBNFNot(BNFNot& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFNot"
		// nothing to do! Consuming by a non-terminal will never happen!
//##protect##"BNFNot"
	}

	void DtaASTNavigator::visitBNFPushItem(BNFPushItem& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFPushItem"
		iterateSequence(theObject, env);
//##protect##"BNFPushItem"
	}

	void DtaASTNavigator::visitBNFRatchet(BNFRatchet& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFRatchet"
		iterateSequence(theObject, env);
//##protect##"BNFRatchet"
	}

	void DtaASTNavigator::visitBNFReadAdaString(BNFReadAdaString& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadAdaString"
//##protect##"BNFReadAdaString"
	}

	void DtaASTNavigator::visitBNFReadByte(BNFReadByte& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadByte"
//##protect##"BNFReadByte"
	}

	void DtaASTNavigator::visitBNFReadBytes(BNFReadBytes& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadBytes"
//##protect##"BNFReadBytes"
	}

	void DtaASTNavigator::visitBNFReadCChar(BNFReadCChar& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadCChar"
//##protect##"BNFReadCChar"
	}

	void DtaASTNavigator::visitBNFReadChar(BNFReadChar& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadChar"
//##protect##"BNFReadChar"
	}

	void DtaASTNavigator::visitBNFReadChars(BNFReadChars& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadChars"
//##protect##"BNFReadChars"
	}

	void DtaASTNavigator::visitBNFReadCompleteIdentifier(BNFReadCompleteIdentifier& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadCompleteIdentifier"
//##protect##"BNFReadCompleteIdentifier"
	}

	void DtaASTNavigator::visitBNFReadCString(BNFReadCString& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadCString"
//##protect##"BNFReadCString"
	}

	void DtaASTNavigator::visitBNFReadIdentifier(BNFReadIdentifier& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadIdentifier"
//##protect##"BNFReadIdentifier"
	}

	void DtaASTNavigator::visitBNFReadInteger(BNFReadInteger& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadInteger"
//##protect##"BNFReadInteger"
	}

	void DtaASTNavigator::visitBNFReadNumeric(BNFReadNumeric& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadNumeric"
//##protect##"BNFReadNumeric"
	}

	void DtaASTNavigator::visitBNFReadPythonString(BNFReadPythonString& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadCString"
//##protect##"BNFReadCString"
	}

	void DtaASTNavigator::visitBNFReadText(BNFReadText& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadText"
//##protect##"BNFReadText"
	}

	void DtaASTNavigator::visitBNFReadToken(BNFReadToken& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadToken"
//##protect##"BNFReadToken"
	}

	void DtaASTNavigator::visitBNFReadUptoIgnore(BNFReadUptoIgnore& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFReadUptoIgnore"
//##protect##"BNFReadUptoIgnore"
	}

	void DtaASTNavigator::visitBNFScanWindow(BNFScanWindow& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFScanWindow"
		if (theObject.isABNFCommand()) theObject.getWindow()->accept(*this, env);
//##protect##"BNFScanWindow"
	}

	void DtaASTNavigator::visitBNFSkipIgnore(BNFSkipIgnore& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFSkipIgnore"
//##protect##"BNFSkipIgnore"
	}

	void DtaASTNavigator::visitBNFStepintoHook(BNFStepintoHook& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFStepintoHook"
//##protect##"BNFStepintoHook"
	}

	void DtaASTNavigator::visitBNFStepoutHook(BNFStepoutHook& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFStepoutHook"
//##protect##"BNFStepoutHook"
	}

	void DtaASTNavigator::visitBNFStepper(BNFStepper& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFStepper"
		// should never be called: always passed to a sub-class
//##protect##"BNFStepper"
	}

	void DtaASTNavigator::visitBNFString(BNFString& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFString"
//##protect##"BNFString"
	}

	void DtaASTNavigator::visitBNFTransformationMode(BNFTransformationMode& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFTransformationMode"
		iterateSequence(theObject, env);
//##protect##"BNFTransformationMode"
	}

	void DtaASTNavigator::visitBNFTryCatch(BNFTryCatch& theObject, DtaVisitorEnvironment& env) {
//##protect##"BNFTryCatch"
		std::auto_ptr<DtaASTStructure::Clause::Sequence> pSequence;
		if (theObject.getTryBlock()->isABNFCommand()) {
			pSequence = std::auto_ptr<DtaASTStructure::Clause::Sequence>(new DtaASTStructure::Clause::Sequence);
			DtaASTNavigatorEnvironment subEnv(*pSequence);
			theObject.getTryBlock()->accept(*this, subEnv);
		}
		if (theObject.getCatchBlock()->isABNFCommand()) {
			DtaASTStructure::Clause::Sequence* pCatchSequence = new DtaASTStructure::Clause::Sequence;
			DtaASTNavigatorEnvironment subEnv(*pCatchSequence);
			theObject.getCatchBlock()->accept(*this, subEnv);
			if (pSequence.get() != NULL) {
				pSequence->merge(*pCatchSequence);
				delete pCatchSequence;
			} else {
				pSequence = std::auto_ptr<DtaASTStructure::Clause::Sequence>(pCatchSequence);
			}
		}
		if (pSequence.get() != NULL) {
			((DtaASTNavigatorEnvironment&) env)._pCurrentSequence->addSequence(*pSequence);
		}
//##protect##"BNFTryCatch"
	}

//##protect##"implementations"
	DtaASTStructure::~DtaASTStructure() {
		for (std::map<std::string, Clause*>::iterator i = _allClauses.begin(); i != _allClauses.end(); ++i) {
			delete i->second;
		}
	}

	void DtaASTStructure::build(DtaScriptVariable& theStorage) {
		DtaScriptVariable* pRules = theStorage.insertNode("rules");
		for (std::map<std::string, Clause*>::iterator i = _allClauses.begin(); i != _allClauses.end(); ++i) {
			DtaScriptVariable* pClauseNode = pRules->addElement(i->first);
			pClauseNode->setValue(i->first.c_str());
			i->second->build(*pClauseNode);
		}
	}

	void DtaASTStructure::populateAST(DtaScriptVariable& theStorage, BNFClauseMatchingArea& clauseMatching, ScpStream& inputStream, bool bBranchDone) {
		ASTCommandEnvironment env;
		populateAST(env, theStorage, clauseMatching, inputStream, bBranchDone);
	}

	void DtaASTStructure::populateAST(ASTCommandEnvironment& env, DtaScriptVariable& theStorage, BNFClauseMatchingArea& clauseMatching, ScpStream& inputStream, bool bBranchDone) {
		std::string sClauseName = clauseMatching.clause->getName();
		std::map<std::string, Clause*>::const_iterator cursorClause = _allClauses.find(clauseMatching.clause->getSignature());
		if (cursorClause != _allClauses.end()) {
			DtaScriptVariable* pClauseNode;
			if (!bBranchDone && confirmASTnodeCreation(env, clauseMatching, *(cursorClause->second))) {
				pClauseNode = theStorage.insertNode(sClauseName.c_str());
			} else {
				pClauseNode = &theStorage;
			}
			if (clauseMatching.childs.empty()) {
				inputStream.setInputLocation(clauseMatching.beginPosition);
				std::string sValue;
				inputStream.readChars(clauseMatching.endPosition - clauseMatching.beginPosition, sValue);
				pClauseNode->setValue(sValue.c_str());
			} else {
				const std::map<std::string, Clause::Sequence::NonTerminal*>& allCalls = cursorClause->second->_structure._allNonTerminals;
				for (std::list<BNFClauseMatchingArea*>::iterator i = clauseMatching.childs.begin(); i != clauseMatching.childs.end(); ++i) {
					std::string sNonTerminal = (*i)->clause->getName();
					std::map<std::string, Clause::Sequence::NonTerminal*>::const_iterator cursorCall = allCalls.find(sNonTerminal);
					if (cursorCall == allCalls.end()) continue;
					if (!optimizeASTchild(env, *pClauseNode, clauseMatching, cursorClause->second->_structure, *(*i), *(cursorCall->second), inputStream)) {
						if (cursorCall->second->_iMax == 1) {
							populateAST(env, *pClauseNode, *(*i), inputStream);
						} else {
							DtaScriptVariable* pElt = pClauseNode->insertNode(sNonTerminal.c_str())->pushItem("");
							populateAST(env, *pElt, *(*i), inputStream, true);
						}
					}
				}
				std::string sTemplateKey = clauseMatching.clause->getTemplateKey();
				if (!sTemplateKey.empty() && !clauseMatching.clause->isGenericKey()) {
					pClauseNode->setValue(sTemplateKey.c_str());
				}
				refactorASTnode(env, *pClauseNode, clauseMatching, cursorClause->second->_structure);
			}
		}
	}

	bool DtaASTStructure::confirmASTnodeCreation(ASTCommandEnvironment& env, BNFClauseMatchingArea& clauseMatching, DtaASTStructure::Clause& clauseType) {
		if (clauseType._structure._allNonTerminals.size() == 1) {
			std::string sClauseName = clauseMatching.clause->getName();
			std::string sNonTerminalName = clauseType._structure._allNonTerminals.begin()->first;
			return sClauseName != sNonTerminalName + "s";
		}
		return true;
	}

	bool DtaASTStructure::optimizeASTchild(ASTCommandEnvironment& env, DtaScriptVariable& clauseNode, BNFClauseMatchingArea& clauseMatching, DtaASTStructure::Clause::Sequence& sequenceType, BNFClauseMatchingArea& nonTerminalMatching, DtaASTStructure::Clause::Sequence::NonTerminal& nonTerminalType, ScpStream& inputStream) {
		if (nonTerminalType._iMax == 1) {
			if (sequenceType._allNonTerminals.size() == 1) {
				// optimization: the child takes the name of the parent
				populateAST(env, clauseNode, nonTerminalMatching, inputStream, true);
				return true;
			}
		}
		return false;
	}

	bool DtaASTStructure::refactorASTnode(ASTCommandEnvironment& env, DtaScriptVariable& clauseNode, BNFClauseMatchingArea& clauseMatching, DtaASTStructure::Clause::Sequence& sequenceType) {
/*		DtaScriptVariableList* pAttributes = clauseNode.getAttributes();
		if ((pAttributes != NULL) && (pAttributes->getNext() == NULL) && (clauseNode.getValue() == NULL) && (clauseNode.getArray() == NULL)) {
			// one and only one attribute
			const std::list<DtaScriptVariable*>* pList = pAttributes->getNode()->getArray();
			std::string sClauseName = clauseMatching.clause->getName();
			std::string sNonTerminalName = pAttributes->getNode()->getName();
			// refactor if X_Z ::= Y_Z or if Z ::= Y_Z, Z being a production rule
			std::string sSuffix; // Z
			std::string::size_type iIndex = sClauseName.find_last_of('_');
			if (iIndex == std::string::npos) sSuffix = sClauseName;
			else sSuffix = sClauseName.substr(iIndex + 1);
			if (_allClauses.find(sSuffix) != _allClauses.end()) {
				std::string::size_type iIndex = sNonTerminalName.find_last_of('_');
				if (((iIndex != std::string::npos) && (sNonTerminalName.substr(iIndex + 1) == sSuffix)) || (sNonTerminalName == sSuffix)) {
					// move the attribute content to the clause node
					DtaScriptVariable* pAttribute = clauseNode.detachAttribute(sNonTerminalName.c_str());
					pAttribute->moveToNode(clauseNode);
					delete pAttribute;
					if ((clauseNode.getParent() != NULL) && (clauseNode.getParent()->getNode(sNonTerminalName.c_str()) == &clauseNode)) {
						clauseNode.changeName(sSuffix.c_str());
					}
					if (clauseNode.getValue() == NULL) {
						std::string sValue = sNonTerminalName.substr(0, iIndex);
						clauseNode.setValue(sValue.c_str());
					}
					return true;
				}
			}
		}
*/		return false;
	}


	void DtaASTStructure::Clause::build(DtaScriptVariable& theStorage) {
		if (_structure._allNonTerminals.empty()) return;
		DtaScriptVariable* pCalls = theStorage.insertNode("calls");
		for (std::map<std::string, Sequence::NonTerminal*>::const_iterator i = _structure._allNonTerminals.begin(); i != _structure._allNonTerminals.end(); ++i) {
			DtaScriptVariable* pCallNode = pCalls->addElement(i->first);
			i->second->build(*pCallNode);
		}
	}

	DtaASTStructure::Clause::Sequence::~Sequence() {
		for (std::map<std::string, NonTerminal*>::iterator i = _allNonTerminals.begin(); i != _allNonTerminals.end(); ++i) {
			delete i->second;
		}
	}

	void DtaASTStructure::Clause::Sequence::repeatCalls(int iMin, int iMax) {
		for (std::map<std::string, NonTerminal*>::iterator i = _allNonTerminals.begin(); i != _allNonTerminals.end(); ++i) {
			i->second->repeatCall(iMin, iMax);
		}
	}

	void DtaASTStructure::Clause::Sequence::merge(Sequence& seq) {
		for (std::map<std::string, NonTerminal*>::iterator i = _allNonTerminals.begin(); i != _allNonTerminals.end(); ++i) {
			std::map<std::string, NonTerminal*>::const_iterator cursor = seq._allNonTerminals.find(i->first);
			if (cursor != seq._allNonTerminals.end()) {
				i->second->merge(*(cursor->second));
			}
		}
		for (std::map<std::string, NonTerminal*>::iterator j = seq._allNonTerminals.begin(); j != seq._allNonTerminals.end(); ++j) {
			std::map<std::string, NonTerminal*>::iterator cursor = _allNonTerminals.find(j->first);
			if (cursor == _allNonTerminals.end()) {
				_allNonTerminals[j->first] = j->second;
				j->second = NULL;
			}
		}
	}

	void DtaASTStructure::Clause::Sequence::addSequence(Sequence& seq) {
		std::map<std::string, DtaASTStructure::Clause::Sequence::NonTerminal*>& allNonTerminals = seq._allNonTerminals;
		for (std::map<std::string, DtaASTStructure::Clause::Sequence::NonTerminal*>::iterator i = allNonTerminals.begin(); i != allNonTerminals.end(); ++i) {
			std::map<std::string, DtaASTStructure::Clause::Sequence::NonTerminal*>::iterator cursor = _allNonTerminals.find(i->first);
			if (cursor == _allNonTerminals.end()) {
				_allNonTerminals[i->first] = i->second;
				i->second = NULL;
			} else {
				cursor->second->addCallInSequence(*(i->second));
			}
		}
	}


	void DtaASTStructure::Clause::Sequence::NonTerminal::addCallInSequence(const NonTerminal& call) {
		if (_iMin >= 0) {
			if (call._iMin >= 0) _iMin += call._iMin;
			else _iMin = -1;
		}
		if (_iMax >= 0) {
			if (call._iMax >= 0) _iMax += call._iMax;
			else _iMax = -1;
		}
	}

	void DtaASTStructure::Clause::Sequence::NonTerminal::addCallInSequence(int iNbCalls) {
		if (_iMin >= 0) {
			if (iNbCalls >= 0) _iMin += iNbCalls;
			else _iMin = -1;
		}
		if (_iMax >= 0) {
			if (iNbCalls >= 0) _iMax += iNbCalls;
			else _iMax = -1;
		}
	}

	void DtaASTStructure::Clause::Sequence::NonTerminal::repeatCall(int iMin, int iMax) {
		if (iMin > 0) {
			if (_iMin >= 0) _iMin *= iMin;
			else _iMin = -1;
		} else if (iMin < 0) {
			if (_iMin > 0) _iMin = -1;
		} else {
			_iMin = 0;
		}
		if (iMax > 0) {
			if (_iMax >= 0) _iMax *= iMax;
			else _iMax = -1;
		} else if (iMax < 0) {
			if (_iMax > 0) _iMax = -1;
		} else {
			_iMax = 0;
		}
	}

	void DtaASTStructure::Clause::Sequence::NonTerminal::merge(const NonTerminal& call) {
		if (_iMin > call._iMin) {
			if (call._iMin >= 0) _iMin = call._iMin;
		} else if (_iMin < call._iMin) {
			if (_iMin < 0) _iMin = call._iMin;
		}
		if (_iMax > call._iMax) {
			if (call._iMax < 0) _iMax = -1;
		} else if (_iMax < call._iMax) {
			if (_iMax >= 0) _iMax = call._iMax;
		}
	}

	void DtaASTStructure::Clause::Sequence::NonTerminal::build(DtaScriptVariable& theStorage) {
		if (_iMin >= 0) theStorage.insertNode("min")->setValue(_iMin);
		else theStorage.insertNode("min")->setValue("*");
		if (_iMax >= 0) theStorage.insertNode("max")->setValue(_iMax);
		else theStorage.insertNode("max")->setValue("*");
	}


	DtaASTNavigatorEnvironment::~DtaASTNavigatorEnvironment() {
		if (_pParentSequence != NULL) {
			_pParentSequence->addSequence(*_pCurrentSequence);
			delete _pCurrentSequence;
		}
	}


	DtaASTStructure* DtaASTNavigator::detectASTStructure(DtaBNFScript& theBNFScript) {
		std::auto_ptr<DtaASTStructure> pASTStructure(new DtaASTStructure);
		const std::map<std::string, std::list<BNFClause*> >& listOfClauses = theBNFScript.getClauses();
		for (std::map<std::string, std::list<BNFClause*> >::const_iterator i = listOfClauses.begin(); i != listOfClauses.end(); ++i) {
			for (std::list<BNFClause*>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				std::string sSignature = (*j)->getSignature();
				pASTStructure->_allClauses[sSignature] = new DtaASTStructure::Clause;
				DtaASTNavigatorEnvironment env(pASTStructure->_allClauses[sSignature]->_structure);
				(*j)->accept(*this, env);
			}
		}
		return pASTStructure.release();
	}

	void DtaASTNavigator::iterateSequence(GrfBlock& theObject, DtaVisitorEnvironment& env) {
		const std::vector<GrfCommand*>& commands = theObject.getCommands();
		for (std::vector<GrfCommand*>::const_iterator i = commands.begin(); i != commands.end(); ++i) {
			if ((*i)->isABNFCommand()) {
				(*i)->accept(*this, env);
			}
		}
	}
//##protect##"implementations"
}
