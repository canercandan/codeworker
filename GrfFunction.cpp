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
#include "ScpStream.h"

#include "DtaProject.h"
#include "DtaClass.h"
#include "ExprScriptVariable.h"
#include "ExprScriptFunction.h"
#include "GrfExecutionContext.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"
#include "DtaBNFScript.h"
#include "DtaPatternScript.h"
#include "GrfReturn.h"
#include "GrfFunction.h"

namespace CodeWorker {
	int GrfFunction::_iMaxStackDepth = 1000;
	int GrfFunction::_iCurrentStackDepth = 0;

	class GrfFunctionDepth {
	public:
		GrfFunctionDepth() {
			GrfFunction::_iCurrentStackDepth++;
			if (GrfFunction::_iCurrentStackDepth > GrfFunction::_iMaxStackDepth) {
				throw UtlException("stack overflow! You may increase the stack reserved for function calls with option '-stack'.");
			}
		}
		inline ~GrfFunctionDepth() {
			GrfFunction::_iCurrentStackDepth--;
		}
	};

	GrfFunction::PatternTemplateStruct::~PatternTemplateStruct() { delete _pPattern; }


	GrfFunction::GrfFunction(GrfBlock* pParent, const std::string& sName, const std::string& sTemplateInstantiation, bool bGenericKey) : GrfBlock(pParent), _sName(sName), _sTemplateInstantiation(sTemplateInstantiation), _bGenericKey(bGenericKey), _bIsBodyDefined(false), _pFinally(NULL), _pTemplateFunction(NULL), _bIsExternal(false), _pExternalFunction(NULL), _pGenericTemplateFunction(NULL), _pPatternTemplateBody(NULL) {
		if (!sTemplateInstantiation.empty()) {
			_pTemplateFunction = pParent->getFunction(sName, "", false);
			if (_pTemplateFunction == NULL) _pTemplateFunction = pParent->addFunction(sName, "", false);
			_pTemplateFunction->_mapOfInstantiatedFunctions[sTemplateInstantiation] = this;
			if (_bGenericKey) _pTemplateFunction->_pGenericTemplateFunction = this;
		}
	}

	GrfFunction::~GrfFunction() {
		if (_pFinally != NULL) delete _pFinally;
		if (_pTemplateFunction != NULL) {
			_pTemplateFunction->_mapOfInstantiatedFunctions.erase(_sTemplateInstantiation);
		} else {
			for (std::map<std::string, GrfFunction*>::const_iterator i = _mapOfInstantiatedFunctions.begin(); i != _mapOfInstantiatedFunctions.end(); i++) {
				i->second->_pTemplateFunction = NULL;
				getParent()->removeFunction(getName(), i->first);
				getParent()->removeGenericTemplateFunction(getName());
			}
		}
		delete _pPatternTemplateBody;
		for (std::vector<ExprScriptExpression*>::iterator i = _defaults.begin(); i != _defaults.end(); ++i) {
			delete *i;
		}
	}

	bool GrfFunction::addParameterAndType(const char* sParameter, EXPRESSION_TYPE exprType, ExprScriptExpression* pDefault) {
		for (std::list<std::string>::const_iterator i = _parameters.begin(); i != _parameters.end(); i++) {
			if ((*i) == sParameter) return false;
		}
		_parameters.push_back(sParameter);
		_parameterTypes.push_back(exprType);
		if (pDefault != NULL) _defaults.push_back(pDefault);
		return true;
	}

	EXPRESSION_TYPE GrfFunction::getParameterType(unsigned int iIndex) const {
		if (iIndex >= getArity()) return UNKNOWN_EXPRTYPE;
		return _parameterTypes[iIndex];
	}

	ExprScriptExpression* GrfFunction::getDefaultParameter(unsigned int iIndex) const {
		if ((iIndex < getMinArity()) || (iIndex >= getArity())) return NULL;
		return _defaults[iIndex - getMinArity()];
	}


	bool GrfFunction::addBNFLocalVariable(const std::string& sVarName, EXPRESSION_TYPE varType) {
		return false;
	}

	EXPRESSION_TYPE GrfFunction::getLocalVariable(const std::string& sVarName) const {
		EXPRESSION_TYPE result = GrfBlock::getLocalVariable(sVarName);
		if (result == UNKNOWN_EXPRTYPE) {
			int iIndex = 0;
			for (std::list<std::string>::const_iterator i = _parameters.begin(); i != _parameters.end(); i++) {
				if ((*i) == sVarName) {
					result = getParameterType(iIndex);
					break;
				}
				++iIndex;
			}
			result = getParameterType(iIndex);
		}
		return result;
	}

	EXPRESSION_TYPE GrfFunction::getVariable(const std::string& sVarName) const {
		EXPRESSION_TYPE result = getLocalVariable(sVarName);
		if (result == UNKNOWN_EXPRTYPE) {
			if ((sVarName == "this") || (sVarName == "project") || (sVarName == "null") || (DtaProject::getInstance().getGlobalVariableType(sVarName) != UNKNOWN_EXPRTYPE)) return NODE_EXPRTYPE;
		}
		return result;
	}


	void GrfFunction::setPatternTemplateBody(DtaScriptFactory::SCRIPT_TYPE eParentScriptType, DtaPatternScript* pScript) {
		if (_pTemplateFunction != NULL) _pTemplateFunction->setPatternTemplateBody(eParentScriptType, pScript);
		else _pPatternTemplateBody = new PatternTemplateStruct(eParentScriptType, pScript);
	}

	GrfFunction* GrfFunction::getInstantiatedFunction(DtaScriptVariable& visibility, const std::string& sInstantiationKey) const {
		std::map<std::string, GrfFunction*>::const_iterator cursor = _mapOfInstantiatedFunctions.find(sInstantiationKey);
		if (cursor == _mapOfInstantiatedFunctions.end()) {
			if (sInstantiationKey.empty()) return const_cast<GrfFunction*>(this);
			if (_pPatternTemplateBody != NULL) {
				std::string sOutput;
				DtaScriptVariable localVariables(&CGRuntime::getThisInternalNode(), "##stack## specialize");
				localVariables.setValueAtVariable(_pGenericTemplateFunction->getTemplateInstantiation().c_str(), sInstantiationKey.c_str());
				if (_pPatternTemplateBody->_pPattern->generateString(sOutput, localVariables) == NO_INTERRUPTION) {
					ScpStream theStream("{" + sOutput + "}");
					GrfBlock* pBlock = _pPatternTemplateBody->_pPattern->getBlock().getParent();
					std::auto_ptr<DtaScript> pScript(DtaScriptFactory::create(_pPatternTemplateBody->_eParentScriptType, *pBlock));
					if ((pScript->getType() == DtaScriptFactory::BNF_SCRIPT) || (pScript->getType() == DtaScriptFactory::TRANSLATE_SCRIPT)) {
						DtaBNFScript* pBNFScript = dynamic_cast<DtaBNFScript*>(pScript.get());
						pBNFScript->setBNFMode(false);
						pBNFScript->setExecMode(true);
					} else if (pScript->getType() == DtaScriptFactory::PATTERN_SCRIPT) {
						DtaPatternScript* pPatternScript = dynamic_cast<DtaPatternScript*>(pScript.get());
						pPatternScript->setExecMode(true);
					}
					GrfFunction* pNewTemplateFunction = pBlock->addFunction(getFunctionName(), sInstantiationKey, false);
					pNewTemplateFunction->_parameterTypes = _parameterTypes;
					pNewTemplateFunction->_parameters = _parameters;
					pNewTemplateFunction->isBodyDefined(true);
					pScript->parseBlock(theStream, *pNewTemplateFunction);
					return pNewTemplateFunction;
				}
			}
			return _pGenericTemplateFunction;
		}
		return cursor->second;
	}

	void GrfFunction::applyRecursively(APPLY_ON_COMMAND_FUNCTION apply) {
		GrfBlock::applyRecursively(apply);
		if (_pFinally != NULL) _pFinally->applyRecursively(apply);
	}

	void GrfFunction::callBeforeExecutionCBK(DtaScriptVariable& visibility) {
		callRecursiveBeforeExecutionCBK(getCurrentExecutionContext(), visibility);
	}

	void GrfFunction::callRecursiveBeforeExecutionCBK(GrfExecutionContext* pContext, DtaScriptVariable& visibility) {
		if (pContext != NULL) {
			callRecursiveBeforeExecutionCBK(pContext->getLastExecutionContext(), visibility);
			pContext->handleStartingFunction(this);
			pContext->handleBeforeExecutionCBK(this, visibility);
			pContext = pContext->getLastExecutionContext();
		}
	}

	void GrfFunction::callAfterExecutionCBK(DtaScriptVariable& visibility) {
		GrfExecutionContext* pContext = getCurrentExecutionContext();
		while (pContext != NULL) {
			pContext->handleAfterExecutionCBK(this, visibility);
			pContext->handleEndingFunction(this);
			pContext = pContext->getLastExecutionContext();
		}
	}

	std::string GrfFunction::launchExecution(DtaScriptVariable& visibility, const ExprScriptFunction& functionCall, const std::string& sInstantiationKey) {
		UTLTRACE_STACK_FUNCTION(_sParsingFilePtr, getFunctionName(), _iFileLocation);
		GrfFunctionDepth newFunctionCall;
		if (!_bIsBodyDefined && !_bIsExternal) {
			throw UtlException("function '" + getSignature() + "' hasn't been implemented");
		}
		std::string sName = "##stack## function '";
		sName += _sName + "'";
		DtaScriptVariable stackFunction(&visibility, sName.c_str());
		if (!sInstantiationKey.empty() && isGenericKey()) {
			stackFunction.setValueAtVariable(getTemplateInstantiation().c_str(), sInstantiationKey.c_str());
		}
		const std::vector<ExprScriptExpression*>& parameters = functionCall.getParameters();
		std::vector<ExprScriptExpression*>::const_iterator i;
		std::list<std::string>::const_iterator j = _parameters.begin();
		std::vector<EXPRESSION_TYPE>::const_iterator k = _parameterTypes.begin();
		stackFunction.getNonRecursiveVariable(_sName.c_str());
		std::string sResult;
		SEQUENCE_INTERRUPTION_LIST result;
		if (_bIsExternal) {
			if (_pExternalFunction == NULL) {
				std::string sKey = getSignature();
				_pExternalFunction = CGRuntime::getExternalFunction(sKey);
				if (_pExternalFunction == NULL) {
					throw UtlException("external function '" + sKey + "' hasn't been registered ; call to function 'init()' missing?");
				}
			}
			CppParsingTree_var** tParameters = new CppParsingTree_var*[_parameters.size()];
			std::string::size_type iIndex;
			try {
				iIndex = 0;
				for (i = parameters.begin(); i != parameters.end(); i++) {
					DtaScriptVariable* pVariable = NULL;
					switch(*k & 0x00FF) {
						case VALUE_EXPRTYPE:
							{
								std::string sValue = (*i)->getValue(visibility);
								stackFunction.setValueAtVariable((*j).c_str(), sValue.c_str());
								pVariable = stackFunction.getVariable(j->c_str());
							}
							break;
						case ITERATOR_EXPRTYPE:
						case REFERENCE_EXPRTYPE:
						case NODE_EXPRTYPE:
							register const ExprScriptVariable* pVarExpr = dynamic_cast<const ExprScriptVariable*>(*i);
							if (pVarExpr == NULL) {
								ExprScriptResolvedVariable* pResolvedVariable = dynamic_cast<ExprScriptResolvedVariable*>(*i);
								if (pResolvedVariable == NULL) {
									std::string sValue = (*i)->getValue(visibility);
									sValue = CGRuntime::composeCLikeString(sValue);
									throw UtlException("in function '" + getSignature() + "', parameter '" + *j + ":node' expects a tree node, not the expression \"" + sValue + "\"");
								} else {
									pVariable = pResolvedVariable->getVariable();
								}
							} else {
								pVariable = visibility.getVariable(*pVarExpr);
							}
							stackFunction.createNodeArgument(j->c_str(), pVariable);
							break;
					}
					tParameters[iIndex++] = new CppParsingTree_var(pVariable);
					j++;
					k++;
				}
				sResult = _pExternalFunction(tParameters);
			} catch(std::exception&/* exception*/) {
				for (iIndex = 0; iIndex < _parameters.size(); iIndex++) delete tParameters[iIndex];
				delete [] tParameters;
				throw/* UtlException(exception)*/;
			}
			for (iIndex = 0; iIndex < _parameters.size(); iIndex++) delete tParameters[iIndex];
			delete [] tParameters;
			result = NO_INTERRUPTION;
		} else {
			for (i = parameters.begin(); i != parameters.end(); i++) {
				switch(*k & 0x00FF) {
					case VALUE_EXPRTYPE:
						{
							std::string sValue = (*i)->getValue(visibility);
							stackFunction.setValueAtVariable((*j).c_str(), sValue.c_str());
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
								pVariable = pResolvedVariable->getVariable();
							} else if ((*k & 0x00FF) == ITERATOR_EXPRTYPE) {
								pVariable = visibility.getIterator(pVarExpr->getName().c_str());
							} else {
								pVariable = visibility.getVariable(*pVarExpr);
							}
							DtaScriptVariable* pParameter = stackFunction.createNodeArgument(j->c_str(), pVariable);
						}
						break;
				}
				j++;
				k++;
			}
			result = execute(stackFunction);
		}
		DtaScriptVariable* pVariable;
		DtaScriptVariable* pReference;
		switch(result) {
			case CONTINUE_INTERRUPTION:
			case BREAK_INTERRUPTION:
			case RETURN_INTERRUPTION:
			case NO_INTERRUPTION:
				result = NO_INTERRUPTION;
				j = _parameters.begin();
				k = _parameterTypes.begin();
				for (i = parameters.begin(); i != parameters.end(); i++) {
					if ((*k & 0x00FF) == REFERENCE_EXPRTYPE) {
						pVariable = visibility.getVariableForReferenceAssignment((const ExprScriptVariable&) (*(*i)));
						pReference = stackFunction.getNodeArgument((*j).c_str())->getReferencedVariable();
						if (pReference != pVariable) pVariable->setValue(pReference);
					}
					j++;
					k++;
				}
				if (!_bIsExternal) {
					const char* tcResult = stackFunction.getVariable(_sName.c_str())->getValue();
					if (tcResult != NULL) sResult = tcResult;
				}
				break;
		}
		return sResult;
	}

	SEQUENCE_INTERRUPTION_LIST GrfFunction::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		if (getFinally() != NULL) {
			try {
				result = GrfBlock::executeInternal(visibility);
			} catch(UtlException&/* exception*/) {
				result = getFinally()->execute(visibility);
				throw/* UtlException(exception)*/;
			}
			result = getFinally()->execute(visibility);
		} else {
			result = GrfBlock::executeInternal(visibility);
		}
		switch(result) {
			case RETURN_INTERRUPTION:
			case BREAK_INTERRUPTION:
			case CONTINUE_INTERRUPTION:
				result = NO_INTERRUPTION;
				break;
		}
		return result;
	}

	std::string GrfFunction::getSignature() const {
		std::string sFunctionName = getFunctionName();
		if (!_sTemplateInstantiation.empty()) {
			if (_bGenericKey) sFunctionName += "<" + _sTemplateInstantiation + ">";
			else sFunctionName += "<\"" + _sTemplateInstantiation + "\">";
		}
		if (!_parameters.empty()) {
			sFunctionName += "(";
			int iIndex = 0;
			for (std::list<std::string>::const_iterator i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
				if (i != _parameters.begin()) sFunctionName += ", ";
				sFunctionName += (*i);
				EXPRESSION_TYPE exprType = getParameterType(iIndex);
				switch(exprType & 0x00FF) {
					case VALUE_EXPRTYPE: sFunctionName += " : value";break;
					case ITERATOR_EXPRTYPE: sFunctionName += " : iterator";break;
					case REFERENCE_EXPRTYPE: sFunctionName += " : reference";break;
					case NODE_EXPRTYPE:	sFunctionName += " : node";break;
					default:
						throw UtlException("internal error in GrfFunction::getSignature(): unhandled enum");
				}
				DtaClass* pTypeSpecifier = DtaClass::getClass(exprType);
				if (pTypeSpecifier != NULL) {
					sFunctionName += "<" + pTypeSpecifier->getName() + ">";
				}
			}
			sFunctionName += ")";
		}
		return sFunctionName;
	}

	void GrfFunction::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		const GrfFunction* pParentFunction = theCompilerEnvironment.getCurrentFunction();
		int iPointerToDeclarations = theCompilerEnvironment.getPointerToDeclarations();
		std::string sFunctionName = getFunctionName();
		if (isATemplateInstantiation()) sFunctionName = "_compilerTemplateFunction_" + sFunctionName + "_compilerInstantiation_" + theCompilerEnvironment.convertTemplateKey(_sTemplateInstantiation);
		else sFunctionName = "_compilerFunction_" + sFunctionName;
		theCompilerEnvironment.getHeader() << theCompilerEnvironment.getIndentation() << "\t\tstatic std::string " << sFunctionName << "(";
		if (!_mapOfInstantiatedFunctions.empty()) theCompilerEnvironment.getHeader() << "const std::string& _compilerTemplateFunction_dispatching";
		int iIndex = 0;
		std::list<std::string>::const_iterator i;
		for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
			if ((i != _parameters.begin()) || !_mapOfInstantiatedFunctions.empty()) theCompilerEnvironment.getHeader() << ", ";
			switch(getParameterType(iIndex) & 0x00FF) {
				case VALUE_EXPRTYPE: theCompilerEnvironment.getHeader() << "CodeWorker::CppParsingTree_value ";break;
				case ITERATOR_EXPRTYPE:
				case REFERENCE_EXPRTYPE: theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
				case NODE_EXPRTYPE:	theCompilerEnvironment.getHeader() << "const CodeWorker::CppParsingTree_var& ";break;
			}
			theCompilerEnvironment.getHeader() << (*i);
		}
		theCompilerEnvironment.getHeader() << ");";
		theCompilerEnvironment.getHeader().endl();

		CW_BODY_INDENT << "std::string " << DtaScript::convertFilenameAsIdentifier(theCompilerEnvironment.getRadical()) << "::" << sFunctionName << "(";
		if (!_mapOfInstantiatedFunctions.empty()) CW_BODY_STREAM << "const std::string& _compilerTemplateFunction_dispatching";
		iIndex = 0;
		for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
			if ((i != _parameters.begin()) || !_mapOfInstantiatedFunctions.empty()) CW_BODY_STREAM << ", ";
			switch(getParameterType(iIndex) & 0x00FF) {
				case VALUE_EXPRTYPE: CW_BODY_STREAM << "CppParsingTree_value ";break;
				case ITERATOR_EXPRTYPE:
				case REFERENCE_EXPRTYPE: CW_BODY_STREAM << "const CppParsingTree_var& ";break;
				case NODE_EXPRTYPE:	CW_BODY_STREAM << "const CppParsingTree_var& ";break;
			}
			CW_BODY_STREAM << (*i);
		}
		CW_BODY_STREAM << ") {";
		CW_BODY_ENDL;
		if (!_mapOfInstantiatedFunctions.empty()) {
			for (std::map<std::string, GrfFunction*>::const_iterator j = _mapOfInstantiatedFunctions.begin(); j != _mapOfInstantiatedFunctions.end(); j++) {
				CW_BODY_INDENT << "\tif (_compilerTemplateFunction_dispatching == \"";
				CW_BODY_STREAM << j->first << "\") return " << sFunctionName << CppCompilerEnvironment::convertTemplateKey(j->first) << "(";
				for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
					if (i != _parameters.begin()) CW_BODY_STREAM << ", ";
					CW_BODY_STREAM << (*i);
				}
				CW_BODY_STREAM << ");";
				CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "\tif (!_compilerTemplateFunction_dispatching.empty()) throw UtlException(\"template function '" << getName() << "<\\\"\" + _compilerTemplateFunction_dispatching + \"\\\">' hasn't been implemented\");";
			CW_BODY_ENDL;
		}
		if (_bIsBodyDefined) {
			theCompilerEnvironment.setPointerToDeclarations(CW_BODY_STREAM.getOutputLocation());
			CppNewVariableScopeEnvironment newVariableScope(theCompilerEnvironment);
			theCompilerEnvironment.incrementIndentation();
			for (i = _parameters.begin(); i != _parameters.end(); i++, iIndex++) {
				theCompilerEnvironment.addVariable((*i));
			}
			CW_BODY_INDENT << "CppParsingTree_value " << getFunctionName() << ";";
			theCompilerEnvironment.addVariable(getFunctionName());
			CW_BODY_ENDL;
			if (_pFinally != NULL) {
				CW_BODY_INDENT << "try {";
				CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
			}
			theCompilerEnvironment.bracketsToNextBlock(false);
			GrfBlock::compileCpp(theCompilerEnvironment);
			if (_pFinally != NULL) {
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_INDENT << "} catch(UtlException& _compilerException_finally) {";
				CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				theCompilerEnvironment.bracketsToNextBlock(false);
				_pFinally->compileCpp(theCompilerEnvironment);
				CW_BODY_INDENT << "throw UtlException(_compilerException_finally);";
				CW_BODY_ENDL;
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_STREAM << "}";
				CW_BODY_ENDL;
				theCompilerEnvironment.bracketsToNextBlock(false);
				_pFinally->compileCpp(theCompilerEnvironment);
			}
			if ((getNbCommands() == 0) || ((getNbCommands() > 0) && (dynamic_cast<GrfReturn*>(getCommands().back()) == NULL))) {
				CW_BODY_INDENT << "return " << getFunctionName() << ".getStringValue();";
				CW_BODY_ENDL;
			}
			theCompilerEnvironment.decrementIndentation();
		} else {
			if (_sTemplateInstantiation.empty()) CW_BODY_INDENT << "\tthrow UtlException(\"function '" << _sName << "' hasn't been implemented\");";
			else CW_BODY_INDENT << "\tthrow UtlException(\"function '" << _sName << "<\\\"" << _sTemplateInstantiation << "\\\">' hasn't been implemented\");";
			CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
		CW_BODY_ENDL;
		theCompilerEnvironment.setCurrentFunction(pParentFunction);
		theCompilerEnvironment.setPointerToDeclarations(iPointerToDeclarations);
	}

	void GrfFunction::compileCppFunctionNameForCalling(CppCompilerEnvironment& theCompilerEnvironment) {
		CW_BODY_STREAM << DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(theCompilerEnvironment.getFunctionModule(getFunctionName()))) << "::_compilerFunction_" << getFunctionName();
	}
}
