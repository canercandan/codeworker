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
#include "CppCompilerEnvironment.h"

#include "CGRuntime.h"

#include "DtaScript.h"
#include "DtaProject.h"
#include "DtaClass.h"
#include "DtaBNFScript.h"
#include "DtaVisitor.h"
#include "BNFClause.h"


namespace CodeWorker {
	BNFClauseMatchingArea::~BNFClauseMatchingArea() {
		for (std::list<BNFClauseMatchingArea*>::iterator i = childs.begin(); i != childs.end(); ++i) {
			delete *i;
		}
	}

	void BNFClauseMatchingArea::purgeChildsAfterPosition(int iPosition) {
		while (!childs.empty() && (childs.back()->endPosition > iPosition)) {
			delete childs.back();
			childs.pop_back();
		}
	}

	class BNFClauseMatchingAreaValidator {
	private:
		DtaBNFScript* pBNFScript_;
		BNFClauseMatchingArea* pOld_;

	public:
		BNFClauseMatchingAreaValidator(BNFClause* pClause, DtaBNFScript* pBNFScript) {
			if (!pBNFScript->hasCoverageRecording()) {
				pBNFScript_ = NULL;
			} else {
				pBNFScript_ = pBNFScript;
				pOld_ = pBNFScript->getParentClauseMatching();
				pBNFScript->setParentClauseMatching(new BNFClauseMatchingArea(pClause, CGRuntime::getInputLocation()));
			}
		}
		~BNFClauseMatchingAreaValidator() {
			if (pBNFScript_ != NULL) {
				if (pBNFScript_->getParentClauseMatching()->endPosition < 0) {
					// the clause has failed!
					delete pBNFScript_->getParentClauseMatching();
//					if (pOld_ != NULL) pOld_->popChild();
					pBNFScript_->setParentClauseMatching(pOld_);
				} else {
					// the clause has succeeded, keep the matching
					// and don't loose the header clause matching!
					if (pOld_ != NULL) pBNFScript_->setParentClauseMatching(pOld_);
				}
			}
		}
		void validate() {
			if (pBNFScript_ != NULL) {
				BNFClauseMatchingArea* pNew = pBNFScript_->getParentClauseMatching();
				if (pNew->endPosition < 0) {
					// prevents against multiple call to 'validate()' (shouldn't)
					pNew->endPosition = CGRuntime::getInputLocation();
					if (pOld_ != NULL) pOld_->pushChild(pNew);
				}
			}
		}
	};


	int BNFClause::NO_RETURN_TYPE    = 0;
	int BNFClause::LIST_RETURN_TYPE  = 1;
	int BNFClause::NODE_RETURN_TYPE  = 2;
	int BNFClause::VALUE_RETURN_TYPE = 3;


	BNFClause::BNFClause(DtaBNFScript* pBNFScript, GrfBlock* pParent, const std::string& sName, const std::string& sTemplateKey, bool bGenericKey, const std::vector<std::string>& listOfParameters, const std::vector<EXPRESSION_TYPE>& listOfParameterTypes) : _pBNFScript(pBNFScript), GrfBlock(pParent), _sName(sName), _sTemplateKey(sTemplateKey), _bGenericKey(bGenericKey), _pTemplateClause(NULL), _pGenericTemplateClause(NULL), _iReturnType(0), _parameters(listOfParameters), _parameterTypes(listOfParameterTypes), _pOverloadClause(NULL), _bPropagatedParameters(false), _iPreprocessingIgnoreMode(0/*UNDEFINED_IGNORE*/), _pPreprocessingIgnoreClause(NULL) {
	}

	BNFClause::BNFClause(DtaBNFScript* pBNFScript, GrfBlock* pParent, const std::string& sName, int iArity) : _pBNFScript(pBNFScript), GrfBlock(pParent), _sName(sName), _bGenericKey(false), _pTemplateClause(NULL), _pGenericTemplateClause(NULL), _iReturnType(0), _pOverloadClause(NULL), _bPropagatedParameters(false), _iPreprocessingIgnoreMode(0/*UNDEFINED_IGNORE*/), _pPreprocessingIgnoreClause(NULL) {
		_parameters.resize(iArity);
		_parameterTypes.resize(iArity);
	}

	BNFClause::~BNFClause() {
		delete _pOverloadClause;
	}

	void BNFClause::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFClause(*this, env);
	}

	void BNFClause::setPreprocessingIgnoreMode(int/*IGNORE_MODE*/ iPreprocessingIgnoreMode, BNFClause* pIgnoreClause) {
		_iPreprocessingIgnoreMode = iPreprocessingIgnoreMode;
		_pPreprocessingIgnoreClause = pIgnoreClause;
		if (_pTemplateClause != NULL) {
			// the default template clause might have been created implicitly,
			// so the preprocessing ignore mode isn't initialized
			_pTemplateClause->_iPreprocessingIgnoreMode = _iPreprocessingIgnoreMode;
			_pTemplateClause->_pPreprocessingIgnoreClause = _pPreprocessingIgnoreClause;
		}
	}

	bool BNFClause::isABNFCommand() const { return true; }

	void BNFClause::setTemplateClause(BNFClause* pTemplateClause) {
		_pTemplateClause = pTemplateClause;
		if (isGenericKey()) _pTemplateClause->_pGenericTemplateClause = this;
		else _pTemplateClause->_mapOfTemplateInstantiations[_sTemplateKey] = this;
	}

	BNFClause* BNFClause::getInstantiatedClause(const std::string& sInstantiationKey) const {
		std::map<std::string, BNFClause*>::const_iterator cursor = _mapOfTemplateInstantiations.find(sInstantiationKey);
		if (cursor == _mapOfTemplateInstantiations.end()) {
			if (sInstantiationKey.empty() && (getNbCommands() != 0)) return const_cast<BNFClause*>(this);
			return _pGenericTemplateClause;
		}
		return cursor->second;
	}

	void BNFClause::setParameter(int i, const std::string& sParameter, EXPRESSION_TYPE iType) {
		_parameters[i] = sParameter;
		_parameterTypes[i] = iType;
	}

	void BNFClause::setOverloadClause(BNFClause* pOverloadClause) {
		if (_pOverloadClause != NULL) _pOverloadClause->setOverloadClause(pOverloadClause);
		else _pOverloadClause = pOverloadClause;
	}

	bool BNFClause::propagateParameters(ExprScriptExpression& theFilter, const std::string& sFunctionQuantity, const std::vector<std::string>& listOfParameters, const std::vector<EXPRESSION_TYPE>& listOfParameterTypes) {
		DtaScriptVariable theContext;
		std::string sSignature = getSignature();
		theContext.setValueAtVariable(sFunctionQuantity.c_str(), sSignature.c_str());
		std::string sResult = theFilter.getValue(theContext);
		bool bResult = (!sResult.empty());
		if (bResult) {
			_bPropagatedParameters = true;
			if (listOfParameters.size() < getArity()) throw UtlException("while propagating parameters on non terminals, not enough parameters assigned to " + sFunctionQuantity + "='" + sSignature + "'");
			std::string::size_type i;
			for (i = 0; i < getArity(); ++i) {
				if (getParameterType(i) != listOfParameterTypes[i]) throw UtlException("while propagating parameters on non terminals, incompatible parameter type assigned to " + sFunctionQuantity + "='" + sSignature + "'/parameter='" + listOfParameters[i] + "'");
			}
			for (i = getArity(); i < listOfParameterTypes.size(); ++i) {
				_parameters.push_back(listOfParameters[i]);
				_parameterTypes.push_back(listOfParameterTypes[i]);
			}
		}
		if ((_pGenericTemplateClause != NULL) && _pGenericTemplateClause->propagateParameters(theFilter, sFunctionQuantity, listOfParameters, listOfParameterTypes)) bResult = true;
		for (std::map<std::string, BNFClause*>::iterator i = _mapOfTemplateInstantiations.begin(); i != _mapOfTemplateInstantiations.end(); ++i) {
			if (i->second->propagateParameters(theFilter, sFunctionQuantity, listOfParameters, listOfParameterTypes)) bResult = true;
		}
		if ((_pOverloadClause != NULL) && _pOverloadClause->propagateParameters(theFilter, sFunctionQuantity, listOfParameters, listOfParameterTypes)) bResult = true;
		if (bResult && !_bPropagatedParameters) {
			ExprScriptConstant alwaysTrue(true);
			propagateParameters(alwaysTrue, sFunctionQuantity, listOfParameters, listOfParameterTypes);
		}
		return bResult;
	}


	bool BNFClause::addBNFLocalVariable(const std::string& sVarName, EXPRESSION_TYPE varType) {
		return addLocalVariable(sVarName, varType);
	}

	EXPRESSION_TYPE BNFClause::getLocalVariable(const std::string& sVarName) const {
		EXPRESSION_TYPE result = GrfBlock::getLocalVariable(sVarName);
		if (result == UNKNOWN_EXPRTYPE) {
			int iIndex = 0;
			for (std::vector<std::string>::const_iterator i = _parameters.begin(); i != _parameters.end(); i++) {
				if ((*i) == sVarName) {
					result = getParameterType(iIndex);
					break;
				}
				++iIndex;
			}
		}
		return result;
	}

	EXPRESSION_TYPE BNFClause::getVariable(const std::string& sVarName) const {
		EXPRESSION_TYPE result = getLocalVariable(sVarName);
		if (result == UNKNOWN_EXPRTYPE) {
			if ((sVarName == "this") || (sVarName == "project") || (sVarName == "null") || (DtaProject::getInstance().getGlobalVariableType(sVarName) != UNKNOWN_EXPRTYPE)) return NODE_EXPRTYPE;
		}
		return result;
	}


	SEQUENCE_INTERRUPTION_LIST BNFClause::execute(DtaScriptVariable& visibility) {
		throw UtlException("internal error: call 'BNFClause::executeClause(..., 0)' instead of 'BNFClause::execute(...)'");
	}

	SEQUENCE_INTERRUPTION_LIST BNFClause::executeClause(DtaScriptVariable& visibility, int iSuperCallDepth) {
		SEQUENCE_INTERRUPTION_LIST result;
		BNFClauseMatchingAreaValidator matchingAreaValidator(this, _pBNFScript);
		if (iSuperCallDepth == 0) {
			register BNFClause* pLastOverloadClause = this;
			while (pLastOverloadClause->_pOverloadClause != NULL) pLastOverloadClause = pLastOverloadClause->_pOverloadClause;
			result = pLastOverloadClause->GrfBlock::execute(visibility);
		} else {
			result = executeInternalSuperClause(visibility, iSuperCallDepth);
			if (result == CONTINUE_INTERRUPTION) result = NO_INTERRUPTION;
			if (iSuperCallDepth > 0) throw UtlException("'#super' has failed: no overloaded non-terminal found for '" + getSignature() + "'");
		}
		if (result != BREAK_INTERRUPTION) {
			matchingAreaValidator.validate();
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST BNFClause::executeInternalSuperClause(DtaScriptVariable& visibility, int &iSuperCallDepth) {
		SEQUENCE_INTERRUPTION_LIST result;
		if (_pOverloadClause != NULL) {
			result = _pOverloadClause->executeInternalSuperClause(visibility, iSuperCallDepth);
			iSuperCallDepth--;
			if (iSuperCallDepth == 0) result = GrfBlock::execute(visibility);
		} else {
			result = NO_INTERRUPTION;
		}
		return result;
	}

	std::string BNFClause::getSignature() const {
		std::string sText;
		BNFClause* pOverloadClause = _pOverloadClause;
		while (pOverloadClause != NULL) {
			sText += "super::";
			pOverloadClause = pOverloadClause->_pOverloadClause;
		}
		sText += getName();
		if (!_sTemplateKey.empty()) {
			if (_bGenericKey) sText += "<" + _sTemplateKey + ">";
			else sText += "<\"" + _sTemplateKey + "\">";
		}
		if (!_parameters.empty()) {
			sText += "(";
			int iIndex = 0;
			for (std::vector<std::string>::const_iterator i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
				if (i != _parameters.begin()) sText += ", ";
				sText += (*i);
				EXPRESSION_TYPE exprType = getParameterType(iIndex);
				switch(exprType & 0x00FF) {
					case VALUE_EXPRTYPE: sText += " : value";break;
					case REFERENCE_EXPRTYPE: sText += " : reference";break;
					case ITERATOR_EXPRTYPE: sText += " : iterator";break;
					case NODE_EXPRTYPE:	sText += " : node";break;
					default:
						throw UtlException("internal error in BNFClause::getSignature(): unhandled parameter type");
				}
				DtaClass* pTypeSpecifier = DtaClass::getClass(exprType);
				if (pTypeSpecifier != NULL) {
					sText += "<" + pTypeSpecifier->getName() + ">";
				}
			}
			sText += ")";
		}
		switch(getReturnType()) {
			case 0/*NO_RETURN_TYPE*/: break;
			case 1/*LIST_RETURN_TYPE*/: sText += " : list";break;
			case 2/*NODE_RETURN_TYPE*/: sText += " : node";break;
			case 3/*VALUE_RETURN_TYPE*/: sText += " : value";break;
			default:
				throw UtlException("internal error in BNFClause::toString(): unhandled enum");
		}
		return sText;
	}

	std::string BNFClause::toString() const {
		std::string sText = getSignature();
		sText += " ::= ";
		if (getNbCommands() == 1) {
			sText += getCommands()[0]->toString();
		} else {
			std::string sPrefix(sText.size(), ' ');
			for (std::vector<GrfCommand*>::const_iterator i = getCommands().begin(); i != getCommands().end(); i++) {
				if (i != getCommands().begin()) {
					sText += "\n";
					sText += sPrefix + "| ";
				} else {
					sText += "  ";
				}
				sText += "[";
				(*i)->toString();
				sText += "]";
			}
		}
		return sText + ";";
	}

	void BNFClause::compileCppFunction(CppCompilerEnvironment& theCompilerEnvironment) const {
		theCompilerEnvironment.setBNFStepperCursor(0);
		const BNFClause* pParentClause = theCompilerEnvironment.getCurrentClause();
		int iPointerToDeclarations = theCompilerEnvironment.getPointerToDeclarations();
		theCompilerEnvironment.newClause(this);
		// create the name of the clause
		std::string sClauseName = getName();
		if (isATemplateInstantiation()) {
			if (_bGenericKey) sClauseName = "_compilerTemplateClause_" + sClauseName + "_compilerGeneric";
			else sClauseName = "_compilerTemplateClause_" + sClauseName + "_compilerInstantiation_" + theCompilerEnvironment.convertTemplateKey(_sTemplateKey);
		} else if (sClauseName[0] == '#') sClauseName = "_compilerDirectiveClause_" + sClauseName.substr(1);
		else sClauseName = "_compilerClause_" + sClauseName;
		std::vector<std::string>::const_iterator i;
		int iIndex = 0;
		if (isATemplateInstantiation() && _sTemplateKey.empty()) {
			// prototype of the dispatcher for template clauses, in the C++ header
			theCompilerEnvironment.getHeader() << theCompilerEnvironment.getIndentation() << "\t\tstatic bool " << sClauseName << "(const std::string& _compilerTemplateClause_dispatching, ";
			for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
				switch(getParameterType(iIndex) & 0x00FF) {
					case VALUE_EXPRTYPE: theCompilerEnvironment.getHeader() << "CodeWorker::CppParsingTree_value ";break;
					case ITERATOR_EXPRTYPE:
					case REFERENCE_EXPRTYPE: theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
					case NODE_EXPRTYPE:	theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
					default:
						throw UtlException("internal error in BNFClause::compileCppFunction(): unhandled parameter type");
				}
				theCompilerEnvironment.getHeader() << (*i) << ", ";
			}
			theCompilerEnvironment.getHeader() << "CodeWorker::CGBNFRuntimeEnvironment& theEnvironment);";
			theCompilerEnvironment.getHeader().endl();
			iIndex = 0;
			theCompilerEnvironment.getHeader() << theCompilerEnvironment.getIndentation() << "\t\tstatic bool " << sClauseName << "(const std::string& _compilerTemplateClause_dispatching, const CodeWorker::CppParsingTree_var& _compilerClause_returnValue, ";
			for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
				EXPRESSION_TYPE exprType = getParameterType(iIndex);
				if ((exprType & ~0x00FF) == 0) {
					switch(exprType) {
						case VALUE_EXPRTYPE: theCompilerEnvironment.getHeader() << "CodeWorker::CppParsingTree_value ";break;
						case ITERATOR_EXPRTYPE:
						case REFERENCE_EXPRTYPE: theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
						case NODE_EXPRTYPE:	theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
						default:
							throw UtlException("internal error in BNFClause::compileCppFunction(): unhandled parameter type");
					}
				} else {
					DtaClass* pTypeSpecifier = DtaClass::getClass(exprType);
					theCompilerEnvironment.getHeader() << pTypeSpecifier->getCppTypeSpecifier() << " ";
				}
				theCompilerEnvironment.getHeader() << (*i) << ", ";
			}
			theCompilerEnvironment.getHeader() << "CodeWorker::CGBNFRuntimeEnvironment& theEnvironment);";
			theCompilerEnvironment.getHeader().endl();
		}
		// prototype in the C++ header, without return value
		iIndex = 0;
		theCompilerEnvironment.getHeader() << theCompilerEnvironment.getIndentation() << "\t\tstatic bool " << sClauseName << "(";
		for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
			EXPRESSION_TYPE exprType = getParameterType(iIndex);
			if ((exprType & ~0x00FF) == 0) {
				switch(exprType) {
					case VALUE_EXPRTYPE: theCompilerEnvironment.getHeader() << "CodeWorker::CppParsingTree_value ";break;
					case ITERATOR_EXPRTYPE:
					case REFERENCE_EXPRTYPE: theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
					case NODE_EXPRTYPE:	theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
					default:
						throw UtlException("internal error in BNFClause::compileCppFunction(): unhandled parameter type");
				}
			} else {
				DtaClass* pTypeSpecifier = DtaClass::getClass(exprType);
				theCompilerEnvironment.getHeader() << pTypeSpecifier->getCppTypeSpecifier() << " ";
			}
			theCompilerEnvironment.getHeader() << (*i) << ", ";
		}
		theCompilerEnvironment.getHeader() << "CodeWorker::CGBNFRuntimeEnvironment& theEnvironment);";
		theCompilerEnvironment.getHeader().endl();
		// prototype in the C++ header with return value
		iIndex = 0;
		theCompilerEnvironment.getHeader() << theCompilerEnvironment.getIndentation() << "\t\tstatic bool " << sClauseName << "(const CodeWorker::CppParsingTree_var& _compilerClause_returnValue, ";
		for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
			EXPRESSION_TYPE exprType = getParameterType(iIndex);
			if ((exprType & ~0x00FF) == 0) {
				switch(exprType) {
					case VALUE_EXPRTYPE: theCompilerEnvironment.getHeader() << "CodeWorker::CppParsingTree_value ";break;
					case ITERATOR_EXPRTYPE:
					case REFERENCE_EXPRTYPE: theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
					case NODE_EXPRTYPE:	theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
					default:
						throw UtlException("internal error in BNFClause::compileCppFunction(): unhandled parameter type");
				}
			} else {
				DtaClass* pTypeSpecifier = DtaClass::getClass(exprType);
				theCompilerEnvironment.getHeader() << pTypeSpecifier->getCppTypeSpecifier() << " ";
			}
			theCompilerEnvironment.getHeader() << (*i) << ", ";
		}
		theCompilerEnvironment.getHeader() << "CodeWorker::CGBNFRuntimeEnvironment& theEnvironment);";
		theCompilerEnvironment.getHeader().endl();

		// implementations of the clause in the C++ body
		CW_BODY_INDENT << "//**";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "// " << toString();
		CW_BODY_ENDL;
		CW_BODY_INDENT << "//**";
		CW_BODY_ENDL;
		if (isATemplateInstantiation() && _sTemplateKey.empty()) {
			// implementation of the dispatcher for template clauses, in the C++ body
			CW_BODY_INDENT << "bool " << DtaScript::convertFilenameAsIdentifier(theCompilerEnvironment.getRadical()) << "::" << sClauseName << "(const std::string& _compilerTemplateClause_dispatching, ";
			iIndex = 0;
			for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
				EXPRESSION_TYPE exprType = getParameterType(iIndex);
				if ((exprType & ~0x00FF) == 0) {
					switch(exprType) {
						case VALUE_EXPRTYPE: CW_BODY_STREAM << "CppParsingTree_value ";break;
						case ITERATOR_EXPRTYPE:
						case REFERENCE_EXPRTYPE: CW_BODY_STREAM << "const CppParsingTree_var& ";break;
						case NODE_EXPRTYPE:	CW_BODY_STREAM << "const CppParsingTree_var& ";break;
						default:
							throw UtlException("internal error in BNFClause::compileCppFunction(): unhandled parameter type");
					}
				} else {
					DtaClass* pTypeSpecifier = DtaClass::getClass(exprType);
					CW_BODY_STREAM << pTypeSpecifier->getCppTypeSpecifier() << " ";
				}
				CW_BODY_STREAM << (*i) << ", ";
			}
			CW_BODY_STREAM << "CGBNFRuntimeEnvironment& theEnvironment) {";
			CW_BODY_ENDL;
			CW_BODY_STREAM << "	CppParsingTree_value _compilerClauseThis;";
			CW_BODY_ENDL;
			CW_BODY_STREAM << "	return " << sClauseName << "(_compilerTemplateClause_dispatching, _compilerClauseThis, ";
			for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
				CW_BODY_STREAM << (*i) << ", ";
			}
			CW_BODY_STREAM << "theEnvironment);";
			CW_BODY_ENDL;
			CW_BODY_STREAM << "}";
			CW_BODY_ENDL;
			CW_BODY_ENDL;
			CW_BODY_INDENT << "bool " << DtaScript::convertFilenameAsIdentifier(theCompilerEnvironment.getRadical()) << "::" << sClauseName << "(const std::string& _compilerTemplateClause_dispatching, const CppParsingTree_var& _compilerClause_returnValue, ";
			iIndex = 0;
			for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
				EXPRESSION_TYPE exprType = getParameterType(iIndex);
				if ((exprType & ~0x00FF) == 0) {
					switch(exprType) {
						case VALUE_EXPRTYPE: CW_BODY_STREAM << "CppParsingTree_value ";break;
						case ITERATOR_EXPRTYPE:
						case REFERENCE_EXPRTYPE: CW_BODY_STREAM << "const CppParsingTree_var& ";break;
						case NODE_EXPRTYPE:	CW_BODY_STREAM << "const CppParsingTree_var& ";break;
						default:
							throw UtlException("internal error in BNFClause::compileCppFunction(): unhandled parameter type");
					}
				} else {
					DtaClass* pTypeSpecifier = DtaClass::getClass(exprType);
					CW_BODY_STREAM << pTypeSpecifier->getCppTypeSpecifier() << " ";
				}
				CW_BODY_STREAM << (*i) << ", ";
			}
			CW_BODY_STREAM << "CGBNFRuntimeEnvironment& theEnvironment) {";
			CW_BODY_ENDL;
			// core of the dispatcher
			if (_mapOfTemplateInstantiations.size() < 8) {
				// just a battery of if/else on the key for determining the correct function call
				for (std::map<std::string, BNFClause*>::const_iterator j = _mapOfTemplateInstantiations.begin(); j != _mapOfTemplateInstantiations.end(); ++j) {
					CW_BODY_STREAM << "\t";
					if (j != _mapOfTemplateInstantiations.begin()) CW_BODY_STREAM << "else ";
					CW_BODY_STREAM << "if (_compilerTemplateClause_dispatching == ";
					CW_BODY_STREAM.writeString(j->first);
					CW_BODY_STREAM << ") return " << sClauseName << theCompilerEnvironment.convertTemplateKey(j->first) << "(_compilerClause_returnValue, ";
					for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
						CW_BODY_STREAM << (*i) << ", ";
					}
					CW_BODY_STREAM << "theEnvironment);";
					CW_BODY_ENDL;
				}
			} else {
				// a switch/case depending on a hash code issued of the key
				std::map<long, std::list<std::string> > keysByCode;
				for (std::map<std::string, BNFClause*>::const_iterator j = _mapOfTemplateInstantiations.begin(); j != _mapOfTemplateInstantiations.end(); ++j) {
					long iHashCode = 0;
					char* u = (char*) (j->first.c_str());
					while (*u != '\0') iHashCode = iHashCode * 31 + (((long) *u++) % 31);
					keysByCode[iHashCode].push_back(j->first);
				}
				CW_BODY_STREAM << "\tlong _compilerInternal_hashCode = 0;";
				CW_BODY_ENDL;
				CW_BODY_STREAM << "\tchar* _compilerInternal_u = (char*) (_compilerTemplateClause_dispatching.c_str());";
				CW_BODY_ENDL;
				CW_BODY_STREAM << "\twhile (*_compilerInternal_u != '\\0') _compilerInternal_hashCode = _compilerInternal_hashCode * 31 + (((long) *_compilerInternal_u++) % 31);";
				CW_BODY_ENDL;
				CW_BODY_STREAM << "\tswitch(_compilerInternal_hashCode) {";
				CW_BODY_ENDL;
				for (std::map<long, std::list<std::string> >::const_iterator k = keysByCode.begin(); k != keysByCode.end(); ++k) {
					CW_BODY_STREAM << "\t\tcase " << k->first << ": ";
					CW_BODY_ENDL;
					for (std::list<std::string>::const_iterator l = k->second.begin(); l != k->second.end(); ++l) {
						CW_BODY_STREAM << "\t\t\tif (_compilerTemplateClause_dispatching == ";
						CW_BODY_STREAM.writeString(*l);
						CW_BODY_STREAM << ") return " << sClauseName << theCompilerEnvironment.convertTemplateKey(*l) << "(_compilerClause_returnValue, ";
						for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
							CW_BODY_STREAM << (*i) << ", ";
						}
						CW_BODY_STREAM << "theEnvironment);";
						CW_BODY_ENDL;
					}
					CW_BODY_STREAM << "\t\t\tbreak;";
					CW_BODY_ENDL;
				}
				CW_BODY_STREAM << "\t}";
				CW_BODY_ENDL;
			}
			if (_pGenericTemplateClause != NULL) {
				CW_BODY_STREAM << "\treturn _compilerTemplateClause_" + getName() + "_compilerGeneric(_compilerClause_returnValue, ";
				for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
					CW_BODY_STREAM << (*i) << ", ";
				}
				CW_BODY_STREAM << "theEnvironment);";CW_BODY_ENDL;
			} else {
				CW_BODY_STREAM << "\tif (!_compilerTemplateClause_dispatching.empty()) throw UtlException(\"template clause '" << getName() << "<\\\"\" + _compilerTemplateClause_dispatching + \"\\\">' hasn't been implemented\");";CW_BODY_ENDL;
				if (getNbCommands() == 0) {
					CW_BODY_STREAM << "\treturn true;";CW_BODY_ENDL;
				} else {
					CW_BODY_STREAM << "\treturn " << sClauseName << "(_compilerClause_returnValue, ";
					for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
						CW_BODY_STREAM << (*i) << ", ";
					}
					CW_BODY_STREAM << "theEnvironment);";
					CW_BODY_ENDL;
				}
			}
			CW_BODY_STREAM << "}";
			CW_BODY_ENDL;
			CW_BODY_ENDL;
		}
		// implementation of the clause in the C++ body, with a return value
		CW_BODY_INDENT << "bool " << DtaScript::convertFilenameAsIdentifier(theCompilerEnvironment.getRadical()) << "::" << sClauseName << "(const CppParsingTree_var& _compilerClause_returnValue, ";
		iIndex = 0;
		for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
			EXPRESSION_TYPE exprType = getParameterType(iIndex);
			if ((exprType & ~0x00FF) == 0) {
				switch(exprType) {
					case VALUE_EXPRTYPE: CW_BODY_STREAM << "CppParsingTree_value ";break;
					case ITERATOR_EXPRTYPE:
					case REFERENCE_EXPRTYPE: CW_BODY_STREAM << "const CppParsingTree_var& ";break;
					case NODE_EXPRTYPE:	CW_BODY_STREAM << "const CppParsingTree_var& ";break;
					default:
						throw UtlException("internal error in BNFClause::compileCppFunction(): unhandled parameter type");
				}
			} else {
				DtaClass* pTypeSpecifier = DtaClass::getClass(exprType);
				CW_BODY_STREAM << pTypeSpecifier->getCppTypeSpecifier() << " ";
			}
			CW_BODY_STREAM << (*i) << ", ";
		}
		CW_BODY_STREAM << "CGBNFRuntimeEnvironment& theEnvironment) {";CW_BODY_ENDL;
		theCompilerEnvironment.setPointerToDeclarations(CW_BODY_STREAM.getOutputLocation());
		theCompilerEnvironment.incrementIndentation();
		theCompilerEnvironment.pushVariableScope();
		if (_pBNFScript->hasCoverageRecording()) {
			// coverage recording required
			CW_BODY_INDENT << "CGBNFRuntimeClauseMatchingAreaValidator _compilerClauseMatching(\"" << CGRuntime::composeCLikeString(getSignature()) << "\", &theEnvironment);";CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "CGBNFClauseScope _compilerClauseScope;";CW_BODY_ENDL;
		CW_BODY_STREAM.newFloatingLocation("CLAUSE SCOPE DECLARATION");
		CW_BODY_INDENT << "bool _compilerClauseSuccess = true;";CW_BODY_ENDL;
		for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
			theCompilerEnvironment.addVariable((*i));
		}
		if (getReturnType() != 0/*NO_RETURN_TYPE*/) {
			// the clause admits a return value
			theCompilerEnvironment.setClauseReturnValue(getName());
		}
		theCompilerEnvironment.bracketsToNextBlock(false);
		GrfBlock::compileCpp(theCompilerEnvironment);
		if (theCompilerEnvironment.hasEvaluatedExpressionInScope()) {
			for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
				std::string sDynamicVariable = theCompilerEnvironment.getIndentation() + "_compilerClauseScope.insertNode(\"" + *i + "\").setReference(" + *i + ");" + ScpStream::ENDL;
				ScpStream* pOwner = 0;
				CW_BODY_STREAM.insertText(sDynamicVariable, CW_BODY_STREAM.getFloatingLocation("CLAUSE SCOPE DECLARATION", pOwner));
			}
		}
		if (_pBNFScript->hasCoverageRecording()) {
			// coverage recording required
			CW_BODY_INDENT << "if (_compilerClauseSuccess) _compilerClauseMatching.validate();";CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "return _compilerClauseSuccess;";CW_BODY_ENDL;
		theCompilerEnvironment.popVariableScope();
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
		CW_BODY_ENDL;
		// implementation of the clause in the C++ body, without a return value
		CW_BODY_INDENT << "bool " << DtaScript::convertFilenameAsIdentifier(theCompilerEnvironment.getRadical()) << "::" << sClauseName << "(";
		iIndex = 0;
		for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
			EXPRESSION_TYPE exprType = getParameterType(iIndex);
			if ((exprType & ~0x00FF) == 0) {
				switch(exprType) {
					case VALUE_EXPRTYPE: CW_BODY_STREAM << "CppParsingTree_value ";break;
					case ITERATOR_EXPRTYPE:
					case REFERENCE_EXPRTYPE: CW_BODY_STREAM << "const CppParsingTree_var& ";break;
					case NODE_EXPRTYPE:	CW_BODY_STREAM << "const CppParsingTree_var& ";break;
					default:
						throw UtlException("internal error in BNFClause::compileCppFunction(): unhandled parameter type");
				}
			} else {
				DtaClass* pTypeSpecifier = DtaClass::getClass(exprType);
				CW_BODY_STREAM << pTypeSpecifier->getCppTypeSpecifier() << " ";
			}
			CW_BODY_STREAM << (*i) << ", ";
		}
		CW_BODY_STREAM << "CGBNFRuntimeEnvironment& theEnvironment) {";
		CW_BODY_ENDL;
		CW_BODY_STREAM << "	CppParsingTree_value _compilerClauseThis;";
		CW_BODY_ENDL;
		CW_BODY_STREAM << "	return " << sClauseName << "(_compilerClauseThis, ";
		for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
			CW_BODY_STREAM << (*i) << ", ";
		}
		CW_BODY_STREAM << "theEnvironment);";
		CW_BODY_ENDL;
		CW_BODY_STREAM << "}";
		CW_BODY_ENDL;
		CW_BODY_ENDL;
		theCompilerEnvironment.setCurrentClause(pParentClause);
		theCompilerEnvironment.setPointerToDeclarations(iPointerToDeclarations);
	}
}
