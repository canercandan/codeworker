/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2003 Cédric Lemaire

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
#	pragma warning (disable : 4786)
#	include <windows.h>
#else
#	include <dlfcn.h>
#endif

#include "ScpStream.h"
#include "UtlException.h"
#include "UtlTrace.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h" // CGRuntime::toString(double)

#include "DynFunction.h"
#include "DynPackage.h"

namespace CodeWorker {
	std::map<std::string, DynPackage*> DynPackage::_packages;

//--------------------------------------------------------------------------------
//                                    Callbacks
//--------------------------------------------------------------------------------
	typedef DtaScriptVariable  Tree;
	typedef ExprScriptFunction Function;

	class ExternalValueNode;


	/** class 'Parameter2CW'
	 * Type of parameters to pass to the interpreter when calling a CodeWorker's
	 * function. See Interpreter::callFunction(...).
	 **/
	class Parameter2CW {
		private:
			bool _bValue;
			union PARAMETER2CW_TYPE {
				const char* _tcValue;
				Tree* _pNode;
			};
			PARAMETER2CW_TYPE _param;

		public:
			inline Parameter2CW(const char* tcValue) : _bValue(true) { _param._tcValue = tcValue; }
			inline Parameter2CW(Tree* pNode) : _bValue(false) { _param._pNode = pNode; }

			inline bool isValue() const { return _bValue; }

			inline const char* getValue() const { return ((_bValue) ? _param._tcValue : NULL); }
			inline Tree* getNode() const { return ((_bValue) ? NULL : _param._pNode); }
	};


	typedef bool (*CREATE_COMMAND0)(DynPackage*, const char*);
	typedef bool (*CREATE_COMMAND1)(DynPackage*, const char*, bool);
	typedef bool (*CREATE_COMMAND2)(DynPackage*, const char*, bool, bool);
	typedef bool (*CREATE_COMMAND3)(DynPackage*, const char*, bool, bool, bool);
	typedef bool (*CREATE_COMMAND4)(DynPackage*, const char*, bool, bool, bool, bool);
	typedef bool (*CREATE_COMMANDN)(DynPackage*, const char*, int,  int*);
	typedef Tree* (*CREATE_VARIABLE)(DynPackage*, const char*);
	typedef void (*ERROR_COMMAND)(const char*);
	typedef const char* (*COPY_LOCAL_STRING)(Interpreter*, const char*);
	typedef const char* (*COPY_LOCAL_BOOLEAN)(Interpreter* pInterpreter, bool);
	typedef const char* (*COPY_LOCAL_INT)(Interpreter* pInterpreter, int);
	typedef const char* (*COPY_LOCAL_DOUBLE)(Interpreter* pInterpreter, double);

	typedef Tree*		(*CREATE_TREE)(const char*);
	typedef const char*	(*GET_VALUE)(Tree*);
	typedef int			(*GET_INT_VALUE)(Tree*);
	typedef double		(*GET_DOUBLE_VALUE)(Tree*);
	typedef bool		(*GET_BOOLEAN_VALUE)(Tree*);
	typedef ExternalValueNode*	(*GET_EXTERNAL_VALUE)(Tree*);
	typedef void		(*SET_VALUE)(Tree*, const char*);
	typedef void		(*SET_EXTERNAL_VALUE)(Tree*, ExternalValueNode*);
	typedef void		(*SET_INT_VALUE)(Tree*, int);
	typedef void		(*SET_DOUBLE_VALUE)(Tree*, double);
	typedef void		(*SET_BOOLEAN_VALUE)(Tree*, bool);
	typedef Tree*		(*NEXT_NODE)(Tree*, const char*);
	typedef Tree*		(*INSERT_NODE)(Tree*, const char*);
	typedef void		(*CLEAR_NODE)(Tree*);
	typedef int			(*SIZE_ARRAY)(Tree*);
	typedef int			(*ALL_KEYS)(Tree*, const char**);
	typedef int			(*ALL_VALUES)(Tree*, const char**);
	typedef Tree*		(*GET_ITEM_FROM_KEY)(Tree*, const char*);
	typedef Tree*		(*GET_ITEM_FROM_POS)(Tree*, int);
	typedef const char*	(*GET_ITEM_KEY)(Tree*, int);
	typedef Tree*		(*PUSH_ITEM)(Tree*, const char*);
	typedef Tree*		(*INSERT_ITEM)(Tree*, const char*, const char*);

	typedef Function*	(*FIND_FUNCTION)(Interpreter*, const char*);

	typedef const char*	(*CALL_FUNCTION0)(Interpreter* pInterpreter, Function*);
	typedef const char*	(*CALL_FUNCTION1)(Interpreter* pInterpreter, Function*, Parameter2CW);
	typedef const char*	(*CALL_FUNCTION2)(Interpreter* pInterpreter, Function*, Parameter2CW, Parameter2CW);
	typedef const char*	(*CALL_FUNCTION3)(Interpreter* pInterpreter, Function*, Parameter2CW, Parameter2CW, Parameter2CW);
	typedef const char*	(*CALL_FUNCTION4)(Interpreter* pInterpreter, Function*, Parameter2CW, Parameter2CW, Parameter2CW, Parameter2CW);

	typedef void		(*RELEASE_FUNCTION)(Function*);

	bool pfCreateCommand0(DynPackage* pPackage, const char* tcName) {
		return pPackage->addFunction(tcName);
	}

	bool pfCreateCommand1(DynPackage* pPackage, const char* tcName, bool bIsP1Node) {
		return pPackage->addFunction(tcName, bIsP1Node);
	}

	bool pfCreateCommand2(DynPackage* pPackage, const char* tcName, bool bIsP1Node, bool bIsP2Node) {
		return pPackage->addFunction(tcName, bIsP1Node, bIsP2Node);
	}

	bool pfCreateCommand3(DynPackage* pPackage, const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node) {
		return pPackage->addFunction(tcName, bIsP1Node, bIsP2Node, bIsP3Node);
	}

	bool pfCreateCommand4(DynPackage* pPackage, const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node, bool bIsP4Node) {
		return pPackage->addFunction(tcName, bIsP1Node, bIsP2Node, bIsP3Node, bIsP4Node);
	}

	bool pfCreateCommandN(DynPackage* pPackage, const char* tcName, int iNbParams, int* tbNodeParams) {
		return pPackage->addFunction(tcName, iNbParams, tbNodeParams);
	}

	Tree* pfCreateVariable(DynPackage* pPackage, const char* tcName) {
		return pPackage->addVariable(tcName);
	}

	void pfError(const char* tcError) {
		throw UtlException(tcError);
	}

	const char* pfCopyLocalString(Interpreter* pInterpreter, const char* tcText);
	const char* pfCopyLocalBoolean(Interpreter* pInterpreter, bool bValue);
	const char* pfCopyLocalDouble(Interpreter* pInterpreter, double dValue);
	const char* pfCopyLocalInt(Interpreter* pInterpreter, int iValue);


	Tree*	pfCreateTree(const char* tcName) {
		return new Tree(NULL, tcName);
	}

	const char*	pfGetValue(Tree* pNode) {
		return pNode->getValue();
	}

	int	pfGetIntValue(Tree* pNode) {
		return pNode->getIntValue();
	}

	double pfGetDoubleValue(Tree* pNode) {
		return pNode->getDoubleValue();
	}

	bool pfGetBooleanValue(Tree* pNode) {
		return pNode->getBooleanValue();
	}

	ExternalValueNode*	pfGetExternalValue(Tree* pNode) {
		return pNode->getExternalValueNode();
	}

	void pfSetValue(Tree* pNode, const char* tcValue) {
		pNode->setValue(tcValue);
	}

	void pfSetExternalValue(Tree* pNode, ExternalValueNode* pValue) {
		pNode->setValue(pValue);
	}

	void pfSetIntValue(Tree* pNode, int iValue) {
		pNode->setValue(iValue);
	}

	void pfSetDoubleValue(Tree* pNode, double dValue) {
		pNode->setValue(dValue);
	}

	void pfSetBooleanValue(Tree* pNode, bool bValue) {
		pNode->setValue(bValue);
	}

	Tree* pfNextNode(Tree* pNode, const char* tcAttribute) {
		return pNode->getNode(tcAttribute);
	}

	Tree* pfInsertNode(Tree* pNode, const char* tcAttribute) {
		return pNode->insertNode(tcAttribute);
	}

	void pfClearNode(Tree* pNode) {
		pNode->clearContent();
	}

	int pfSizeArray(Tree* pArray) {
		return pArray->getArraySize();
	}

	int pfAllKeys(Tree* pArray, const char** tcKeys) {
		const std::list<DtaScriptVariable*>* pArrayList = pArray->getArray();
		if (pArrayList == NULL) return 0;
		int iIndex = 0;
		for (std::list<DtaScriptVariable*>::const_iterator i = pArrayList->begin(); i != pArrayList->end(); i++) {
			tcKeys[iIndex++] = (*i)->getName();
		}
		return iIndex;
	}

	int pfAllValues(Tree* pArray, const char** tcValues) {
		const std::list<DtaScriptVariable*>* pArrayList = pArray->getArray();
		if (pArrayList == NULL) return 0;
		int iIndex = 0;
		for (std::list<DtaScriptVariable*>::const_iterator i = pArrayList->begin(); i != pArrayList->end(); i++) {
			tcValues[iIndex++] = (*i)->getValue();
		}
		return iIndex;
	}

	Tree* pfGetItemFromKey(Tree* pArray, const char* tcKey) {
		return pArray->getArrayElement(tcKey);
	}

	Tree* pfGetItemFromPos(Tree* pArray, int iKey) {
		return pArray->getArrayElement(iKey);
	}

	const char* pfGetItemKey(Tree* pArray, int iKey) {
		DtaScriptVariable* pItem = pArray->getArrayElement(iKey);
		if (pItem == NULL) return NULL;
		return pItem->getName();
	}

	Tree* pfPushItem(Tree* pArray, const char* tcValue) {
		if (tcValue == NULL) return pArray->pushItem("");
		return pArray->pushItem(tcValue);
	}

	Tree* pfInsertItem(Tree* pArray, const char* tcKey, const char* tcValue) {
		DtaScriptVariable* pItem = pArray->addElement(tcKey);
		if (tcValue != NULL) pItem->setValue(tcValue);
		return pItem;
	}

	Function* pfFindFunction(Interpreter* pInterpreter, const char* sFunctionName);

	const char* pfCallFunction0(Interpreter* pInterpreter, Function* pFunction);
	const char* pfCallFunction1(Interpreter* pInterpreter, Function* pFunction, Parameter2CW p1);
	const char* pfCallFunction2(Interpreter* pInterpreter, Function* pFunction, Parameter2CW p1, Parameter2CW p2);
	const char* pfCallFunction3(Interpreter* pInterpreter, Function* pFunction, Parameter2CW p1, Parameter2CW p2, Parameter2CW p3);
	const char* pfCallFunction4(Interpreter* pInterpreter, Function* pFunction, Parameter2CW p1, Parameter2CW p2, Parameter2CW p3, Parameter2CW p4);

	void pfReleaseFunction(Function* pFunction) {
		delete pFunction;
	}



//--------------------------------------------------------------------------------
//                                   Interpreter
//--------------------------------------------------------------------------------
	class Interpreter {
		private:
			CREATE_COMMAND0 _createCommand0;
			CREATE_COMMAND1 _createCommand1;
			CREATE_COMMAND2 _createCommand2;
			CREATE_COMMAND3 _createCommand3;
			CREATE_COMMAND4 _createCommand4;
			CREATE_COMMANDN _createCommandN;
			CREATE_VARIABLE _createVariable;
			ERROR_COMMAND	_error;
			COPY_LOCAL_STRING	_copyLocalString;
			COPY_LOCAL_BOOLEAN	_copyLocalBoolean;
			COPY_LOCAL_INT		_copyLocalInt;
			COPY_LOCAL_DOUBLE	_copyLocalDouble;

			CREATE_TREE	 _createTree;
			GET_VALUE	 _getValue;
			GET_INT_VALUE	 _getIntValue;
			GET_DOUBLE_VALUE _getDoubleValue;
			GET_BOOLEAN_VALUE	 _getBooleanValue;
			GET_EXTERNAL_VALUE	 _getExternalValue;
			SET_VALUE	 _setValue;
			SET_EXTERNAL_VALUE	 _setExternalValue;
			SET_INT_VALUE	 _setIntValue;
			SET_DOUBLE_VALUE _setDoubleValue;
			SET_BOOLEAN_VALUE _setBooleanValue;
			NEXT_NODE	 _nextNode;
			INSERT_NODE	 _insertNode;
			CLEAR_NODE	 _clearNode;
			SIZE_ARRAY	 _sizeArray;
			ALL_KEYS	 _allKeys;
			ALL_VALUES	 _allValues;
			GET_ITEM_FROM_KEY _getItemFromKey;
			GET_ITEM_FROM_POS _getItemFromPos;
			GET_ITEM_KEY _getItemKey;
			PUSH_ITEM	 _pushItem;
			INSERT_ITEM	 _insertItem;

			FIND_FUNCTION _findFunction;

			CALL_FUNCTION0 _callFunction0;
			CALL_FUNCTION1 _callFunction1;
			CALL_FUNCTION2 _callFunction2;
			CALL_FUNCTION3 _callFunction3;
			CALL_FUNCTION4 _callFunction4;

			RELEASE_FUNCTION _releaseFunction;

			DynPackage* _pPackage;

			Tree* _pVisibility;
			char* _tcLocalString;

		public:
			Interpreter(DynPackage* pPackage);

			inline DynPackage* getPackage() const { return _pPackage; }
			inline DtaScriptVariable* getVisibility() const { return _pVisibility; }
			inline void setVisibitity(DtaScriptVariable* pVisibility) { _pVisibility = pVisibility; }

			void prepare(DtaScriptVariable& pVisibility);
			inline const char* copyLocalString(const char* tcText) {
#ifdef WIN32
				return _tcLocalString = ::_strdup(tcText);
#else
				return _tcLocalString = ::strdup(tcText);
#endif
			}

		private:
			Interpreter(const Interpreter&);
	};

	const char* pfCopyLocalString(Interpreter* pInterpreter, const char* tcText) {
		return pInterpreter->copyLocalString(tcText);
	}

	const char* pfCopyLocalBoolean(Interpreter* pInterpreter, bool bValue) {
		return pInterpreter->copyLocalString(bValue ? "true" : "");
	}

	const char* pfCopyLocalDouble(Interpreter* pInterpreter, double dValue) {
		return pInterpreter->copyLocalString(CGRuntime::toString(dValue).c_str());
	}

	const char* pfCopyLocalInt(Interpreter* pInterpreter, int iValue) {
		char tcNumber[32];
		sprintf(tcNumber, "%d", iValue);
		return pInterpreter->copyLocalString(tcNumber);
	}

	Function* pfFindFunction(Interpreter* pInterpreter, const char* sFunctionName) {
		if (pInterpreter->getPackage()->getBlock() == NULL) return NULL;
		ScpStream script("");
		ExprScriptFunction* pFunction = ExprScriptFunction::create(*pInterpreter->getPackage()->getBlock(), script, sFunctionName, "", false);
		return pFunction;
	}

	const char* pfCallFunction0(Interpreter* pInterpreter, Function* pFunction) {
		static std::string sValue;
		if (pFunction == NULL) return NULL;
		if (pFunction->getArity() != 0) {
			std::string sMessage = "CodeWorker's function '";
			sMessage += pFunction->getName();
			sMessage += "' requires parameters";
			throw UtlException(sMessage);
		}
		sValue = pFunction->getValue(*pInterpreter->getVisibility());
		return sValue.c_str();
	}


	void populateFunctionWithParameterExpression(Function* pFunction, EXPRESSION_TYPE eType, Parameter2CW& p) {
		if ((eType & 0x00FF) == VALUE_EXPRTYPE) {
			if (p.isValue()) {
				const char* tcValue = p.getValue();
				ExprScriptConstant* pConstant;
				if (tcValue == NULL) pConstant = new ExprScriptConstant;
				else pConstant = new ExprScriptConstant(tcValue);
				pFunction->addParameter(pConstant);
			} else {
				pFunction->addParameter(new ExprScriptResolvedVariable(p.getNode()));
			}
		} else {
			if (p.isValue()) {
				std::string sMessage = "CodeWorker's function '";
				sMessage += pFunction->getName();
				sMessage += "' requires a node parameter, which isn't passed as one";
				throw UtlException(sMessage);
			}
			pFunction->addParameter(new ExprScriptResolvedVariable(p.getNode()));
		}
	}

	const char* pfCallFunction1(Interpreter* pInterpreter, Function* pFunction, Parameter2CW p1) {
		static std::string sValue;
		if (pFunction == NULL) return NULL;
		if (pFunction->getArity() != 1) {
			std::string sMessage = "CodeWorker's function '";
			sMessage += pFunction->getName();
			sMessage += "' doesn't require one parameter";
			throw UtlException(sMessage);
		}
		pFunction->clearParameters();
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(0), p1);
		sValue = pFunction->getValue(*pInterpreter->getVisibility());
		return sValue.c_str();
	}

	const char* pfCallFunction2(Interpreter* pInterpreter, Function* pFunction, Parameter2CW p1, Parameter2CW p2) {
		static std::string sValue;
		if (pFunction == NULL) return NULL;
		if (pFunction->getArity() != 2) {
			std::string sMessage = "CodeWorker's function '";
			sMessage += pFunction->getName();
			sMessage += "' doesn't require two parameters";
			throw UtlException(sMessage);
		}
		pFunction->clearParameters();
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(0), p1);
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(1), p2);
		sValue = pFunction->getValue(*pInterpreter->getVisibility());
		return sValue.c_str();
	}

	const char* pfCallFunction3(Interpreter* pInterpreter, Function* pFunction, Parameter2CW p1, Parameter2CW p2, Parameter2CW p3) {
		static std::string sValue;
		if (pFunction == NULL) return NULL;
		if (pFunction->getArity() != 3) {
			std::string sMessage = "CodeWorker's function '";
			sMessage += pFunction->getName();
			sMessage += "' doesn't require three parameters";
			throw UtlException(sMessage);
		}
		pFunction->clearParameters();
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(0), p1);
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(1), p2);
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(2), p3);
		sValue = pFunction->getValue(*pInterpreter->getVisibility());
		return sValue.c_str();
	}

	const char* pfCallFunction4(Interpreter* pInterpreter, Function* pFunction, Parameter2CW p1, Parameter2CW p2, Parameter2CW p3, Parameter2CW p4) {
		static std::string sValue;
		if (pFunction == NULL) return NULL;
		if (pFunction->getArity() != 4) {
			std::string sMessage = "CodeWorker's function '";
			sMessage += pFunction->getName();
			sMessage += "' doesn't require four parameters";
			throw UtlException(sMessage);
		}
		pFunction->clearParameters();
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(0), p1);
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(1), p2);
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(2), p3);
		populateFunctionWithParameterExpression(pFunction, pFunction->getParameterType(3), p4);
		sValue = pFunction->getValue(*pInterpreter->getVisibility());
		return sValue.c_str();
	}


	Interpreter::Interpreter(DynPackage* pPackage) : _pPackage(pPackage), _tcLocalString(NULL) {
		_createCommand0 = pfCreateCommand0;
		_createCommand1 = pfCreateCommand1;
		_createCommand2 = pfCreateCommand2;
		_createCommand3 = pfCreateCommand3;
		_createCommand4 = pfCreateCommand4;
		_createCommandN = pfCreateCommandN;
		_createVariable = pfCreateVariable;
		_error			= pfError;
		_copyLocalString	= pfCopyLocalString;
		_copyLocalBoolean	= pfCopyLocalBoolean;
		_copyLocalInt		= pfCopyLocalInt;
		_copyLocalDouble	= pfCopyLocalDouble;

		_createTree = pfCreateTree;
		_getValue = pfGetValue;
		_getIntValue = pfGetIntValue;
		_getBooleanValue = pfGetBooleanValue;
		_getDoubleValue = pfGetDoubleValue;
		_getExternalValue = pfGetExternalValue;
		_setValue = pfSetValue;
		_setExternalValue = pfSetExternalValue;
		_setIntValue = pfSetIntValue;
		_setDoubleValue = pfSetDoubleValue;
		_setBooleanValue = pfSetBooleanValue;
		_nextNode = pfNextNode;
		_insertNode = pfInsertNode;
		_clearNode = pfClearNode;
		_sizeArray = pfSizeArray;
		_allKeys = pfAllKeys;
		_allValues = pfAllValues;
		_getItemFromKey = pfGetItemFromKey;
		_getItemFromPos = pfGetItemFromPos;
		_getItemKey = pfGetItemKey;
		_pushItem   = pfPushItem;
		_insertItem = pfInsertItem;

		_findFunction = pfFindFunction;

		_callFunction0 = pfCallFunction0;
		_callFunction1 = pfCallFunction1;
		_callFunction2 = pfCallFunction2;
		_callFunction3 = pfCallFunction3;
		_callFunction4 = pfCallFunction4;

		_releaseFunction = pfReleaseFunction;
	}

	void Interpreter::prepare(DtaScriptVariable& pVisibility) {
		_pVisibility = &pVisibility;
		free(_tcLocalString);
		_tcLocalString = NULL;
	}



//--------------------------------------------------------------------------------
//                                  Dynamic function
//--------------------------------------------------------------------------------
	
	DynFunction::DynFunction(Interpreter* pInterpreter, const char* tcName, bool bIsP1Node) : GrfFunction(NULL, tcName, "", false), _pInterpreter(pInterpreter) {
		init();
		addParameterAndType("p1", ((bIsP1Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
	}

	DynFunction::DynFunction(Interpreter* pInterpreter, const char* tcName, bool bIsP1Node, bool bIsP2Node) : GrfFunction(NULL, tcName, "", false), _pInterpreter(pInterpreter) {
		init();
		addParameterAndType("p1", ((bIsP1Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
		addParameterAndType("p2", ((bIsP2Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
	}

	DynFunction::DynFunction(Interpreter* pInterpreter, const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node) : GrfFunction(NULL, tcName, "", false), _pInterpreter(pInterpreter) {
		init();
		addParameterAndType("p1", ((bIsP1Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
		addParameterAndType("p2", ((bIsP2Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
		addParameterAndType("p3", ((bIsP3Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
	}

	DynFunction::DynFunction(Interpreter* pInterpreter, const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node, bool bIsP4Node) : GrfFunction(NULL, tcName, "", false), _pInterpreter(pInterpreter) {
		init();
		addParameterAndType("p1", ((bIsP1Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
		addParameterAndType("p2", ((bIsP2Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
		addParameterAndType("p3", ((bIsP3Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
		addParameterAndType("p4", ((bIsP4Node) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
	}

	DynFunction::DynFunction(Interpreter* pInterpreter, const char* tcName, int iNbParams, int* tbNodeParams) : GrfFunction(NULL, tcName, "", false), _pInterpreter(pInterpreter) {
		init();
		char tcParam[16];
		for (int i = 0; i < iNbParams; i++) {
			sprintf(tcParam, "p%d", i+1);
			addParameterAndType(tcParam, ((tbNodeParams[i] != 0) ? NODE_EXPRTYPE : VALUE_EXPRTYPE), NULL);
		}
	}

	DynFunction::~DynFunction() {}

	void DynFunction::init() {
#ifdef WIN32
		_sModuleName = _pInterpreter->getPackage()->getName() + "cw.dll";
#else
		_sModuleName = _pInterpreter->getPackage()->getName() + "cw.so";
#endif
	}

	std::string DynFunction::launchExecution(DtaScriptVariable& visibility, const ExprScriptFunction& functionCall, const std::string& sInstantiationKey) {
		UTLTRACE_STACK_FUNCTION(_sModuleName.c_str(), getFunctionName(), 0);

		std::string sResult;

		Parameter* tParameters = new Parameter[getArity()];
		try {
			const std::vector<ExprScriptExpression*>& parameters = functionCall.getParameters();
			std::vector<EXPRESSION_TYPE>::const_iterator k = _parameterTypes.begin();
			int iIndex = 0;
			for (std::vector<ExprScriptExpression*>::const_iterator i = parameters.begin(); i != parameters.end(); i++) {
				DtaScriptVariable* pVariable = NULL;
				switch(*k & 0x00FF) {
					case VALUE_EXPRTYPE:
						{
							std::string sValue = (*i)->getValue(visibility);
							if (sValue.empty()) {
								ExprScriptVariable* pExprVar = dynamic_cast<ExprScriptVariable*>(*i);
								if ((pExprVar != NULL) && pExprVar->getName() == "null") {
									tParameters[iIndex++] = NULL;
								} else {
#ifdef WIN32
									tParameters[iIndex++] = ::_strdup("");
#else
									tParameters[iIndex++] = ::strdup("");
#endif
								}
							} else {
#ifdef WIN32
								tParameters[iIndex++] = ::_strdup(sValue.c_str());
#else
								tParameters[iIndex++] = ::strdup(sValue.c_str());
#endif
							}
						}
						break;
					case NODE_EXPRTYPE:
						{
							ExprScriptResolvedVariable* pResolvedVariable = dynamic_cast<ExprScriptResolvedVariable*>(*i);
							if (pResolvedVariable == NULL) {
								ExprScriptVariable* pVarExpr = dynamic_cast<ExprScriptVariable*>(*i);
								if (pVarExpr == NULL) {
									throw UtlException("bad parameter passed to the function '" + _pInterpreter->getPackage()->getName() + "::" + std::string(getFunctionName()) + "()'");
								}
								DtaScriptVariable* pVariable = visibility.getVariable((const ExprScriptVariable&) *(*i));
								tParameters[iIndex++] = pVariable;
							} else {
								tParameters[iIndex++] = pResolvedVariable->getVariable();
							}
						}
						break;
				}
				k++;
			}
			switch(getArity()) {
				case 0:
					sResult = call(visibility);
					break;
				case 1:
					sResult = call(visibility, tParameters[0]);
					break;
				case 2:
					sResult = call(visibility, tParameters[0], tParameters[1]);
					break;
				case 3:
					sResult = call(visibility, tParameters[0], tParameters[1], tParameters[2]);
					break;
				case 4:
					sResult = call(visibility, tParameters[0], tParameters[1], tParameters[2], tParameters[3]);
					break;
				default:
					sResult = call(visibility, tParameters);
			}
		} catch(std::exception&) {
			for (std::string::size_type iIndex = 0; iIndex < getArity(); iIndex++) {
				if ((getParameterType(iIndex) & 0x00FF) == VALUE_EXPRTYPE) free(tParameters[iIndex]);
			}
			delete [] tParameters;
			throw;
		} catch(...) {
			for (std::string::size_type iIndex = 0; iIndex < getArity(); iIndex++) {
				if ((getParameterType(iIndex) & 0x00FF) == VALUE_EXPRTYPE) free(tParameters[iIndex]);
			}
			delete [] tParameters;
			std::string sMessage = "an ellipsis exception has occurred in " + _sModuleName + ", function " + getName();
			throw UtlException(sMessage);
		}
		for (std::string::size_type iIndex = 0; iIndex < getArity(); iIndex++) {
			if ((getParameterType(iIndex) & 0x00FF) == VALUE_EXPRTYPE) free(tParameters[iIndex]);
		}
		delete [] tParameters;
		return sResult;
	}

	std::string DynFunction::call(DtaScriptVariable& pVisibility) {
		_pInterpreter->prepare(pVisibility);
		CALL0_FUNCTION pfCall0 = (CALL0_FUNCTION) _pfFunctionCall;
		const char* tcResult = pfCall0(_pInterpreter);
		if (tcResult == NULL) return "";
		return tcResult;
	}

	std::string DynFunction::call(DtaScriptVariable& pVisibility, Parameter p1) {
		_pInterpreter->prepare(pVisibility);
		CALL1_FUNCTION pfCall1 = (CALL1_FUNCTION) _pfFunctionCall;
		const char* tcResult = pfCall1(_pInterpreter, p1);
		if (tcResult == NULL) return "";
		return tcResult;
	}

	std::string DynFunction::call(DtaScriptVariable& pVisibility, Parameter p1, Parameter p2) {
		_pInterpreter->prepare(pVisibility);
		CALL2_FUNCTION pfCall2 = (CALL2_FUNCTION) _pfFunctionCall;
		const char* tcResult = pfCall2(_pInterpreter, p1, p2);
		if (tcResult == NULL) return "";
		return tcResult;
	}

	std::string DynFunction::call(DtaScriptVariable& pVisibility, Parameter p1, Parameter p2, Parameter p3) {
		_pInterpreter->prepare(pVisibility);
		CALL3_FUNCTION pfCall3 = (CALL3_FUNCTION) _pfFunctionCall;
		const char* tcResult = pfCall3(_pInterpreter, p1, p2, p3);
		if (tcResult == NULL) return "";
		return tcResult;
	}

	std::string DynFunction::call(DtaScriptVariable& pVisibility, Parameter p1, Parameter p2, Parameter p3, Parameter p4) {
		_pInterpreter->prepare(pVisibility);
		CALL4_FUNCTION pfCall4 = (CALL4_FUNCTION) _pfFunctionCall;
		const char* tcResult = pfCall4(_pInterpreter, p1, p2, p3, p4);
		if (tcResult == NULL) return "";
		return tcResult;
	}

	std::string DynFunction::call(DtaScriptVariable& pVisibility, Parameter* tParams) {
		_pInterpreter->prepare(pVisibility);
		CALLN_FUNCTION pfCallN = (CALLN_FUNCTION) _pfFunctionCall;
		const char* tcResult = pfCallN(_pInterpreter, tParams);
		if (tcResult == NULL) return "";
		return tcResult;
	}


	void DynFunction::compileCppFunctionNameForCalling(CppCompilerEnvironment& theCompilerEnvironment) {
		CW_BODY_STREAM << _pInterpreter->getPackage()->getName() << "::instance()._compilerFunction_" << getFunctionName();
		if (!theCompilerEnvironment.getIncludeParentScript().empty()) {
			// not the leader script, which initializes the dynamic packages
			ScpStream* pOwner = NULL;
			int iInsertAreaLocation = CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner);
			CW_BODY_STREAM.insertTextOnce("// header file for the dynamic module '" + _pInterpreter->getPackage()->getName() + "'" + ScpStream::ENDL + "#include \"" + _pInterpreter->getPackage()->getName() + ".h\"" + ScpStream::ENDL + ScpStream::ENDL, iInsertAreaLocation);
		}
	}



//--------------------------------------------------------------------------------
//                                  Dynamic package
//--------------------------------------------------------------------------------
	typedef void (*LIBRARY_INIT_FUNCTION)(Interpreter*);


	DynPackage::DynPackage(const std::string& sPackage) : _sPackage(sPackage), _pBlock(NULL), _hHandle(0), _pInterpreter(NULL) {
		initialize();
	}

	DynPackage::DynPackage(const std::string& sPackage, GrfBlock& block) : _sPackage(sPackage), _pBlock(&block), _hHandle(0), _pInterpreter(NULL) {
		initialize();
	}
	void DynPackage::initialize() {
		_hHandle = loadPackage(_sPackage + "cw");
		_pInterpreter = new Interpreter(this);

		LIBRARY_INIT_FUNCTION pfInit = (LIBRARY_INIT_FUNCTION) findFunction(_hHandle, _sPackage + "_Init");
		if (pfInit == NULL) {
			throw UtlException("unable to find the init function " + _sPackage + "_Init(CW4dl::Interpreter*) in module " + _sPackage + "cw");
		}
		try {
			pfInit(_pInterpreter);
		} catch(...) {
			throw UtlException("ellipsis exception while initializing the module " + _sPackage + "cw: is the interpreter's version the same as the module?");
		}
		_packages[_sPackage] = this;
	}

	DynPackage::~DynPackage() {
		freeLibrary(_hHandle);
		delete _pInterpreter;
		for (std::map<std::string, DynFunction*>::iterator i = _functions.begin(); i != _functions.end(); i++) {
			delete i->second;
		}
		for (std::map<std::string, DtaScriptVariable*>::iterator j = _variables.begin(); j != _variables.end(); j++) {
			delete j->second;
		}
		_packages.erase(_sPackage);
	}

	DynFunction* DynPackage::getFunction(const std::string& sFunction) const {
		std::map<std::string, DynFunction*>::const_iterator cursor = _functions.find(sFunction);
		if (cursor == _functions.end()) return NULL;
		return cursor->second;
	}

	bool DynPackage::addFunction(const char* tcName) {
		return addFunction(new DynFunction(_pInterpreter, tcName));
	}

	bool DynPackage::addFunction(const char* tcName, bool bIsP1Node) {
		return addFunction(new DynFunction(_pInterpreter, tcName, bIsP1Node));
	}

	bool DynPackage::addFunction(const char* tcName, bool bIsP1Node, bool bIsP2Node) {
		return addFunction(new DynFunction(_pInterpreter, tcName, bIsP1Node, bIsP2Node));
	}

	bool DynPackage::addFunction(const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node) {
		return addFunction(new DynFunction(_pInterpreter, tcName, bIsP1Node, bIsP2Node, bIsP3Node));
	}

	bool DynPackage::addFunction(const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node, bool bIsP4Node) {
		return addFunction(new DynFunction(_pInterpreter, tcName, bIsP1Node, bIsP2Node, bIsP3Node, bIsP4Node));
	}

	bool DynPackage::addFunction(const char* tcName, int iNbParams, int* tbNodeParams) {
		return addFunction(new DynFunction(_pInterpreter, tcName, iNbParams, tbNodeParams));
	}

	bool DynPackage::addFunction(DynFunction* pFunction) {
		if (getFunction(pFunction->getFunctionName()) != NULL) {
			delete pFunction;
			std::string sError = "function '" + _sPackage + "::";
			sError += pFunction->getFunctionName();
			sError += "' already exists";
			throw UtlException(sError);
		}
		void* pfFunctionCall = findFunction(_hHandle, pFunction->getFunctionName());
		if (pfFunctionCall == NULL) {
			std::string sError = "the module '" + _sPackage + "' doesn't export the function '" + _sPackage + "::";
			sError += pFunction->getFunctionName();
			sError += "'";
			delete pFunction;
			throw UtlException(sError);
		}
		pFunction->setFunctionCall(pfFunctionCall);
		_functions[pFunction->getFunctionName()] = pFunction;
		return true;
	}

	DtaScriptVariable* DynPackage::getVariable(const std::string& sVariable) const {
		std::map<std::string, DtaScriptVariable*>::const_iterator cursor = _variables.find(sVariable);
		if (cursor == _variables.end()) return NULL;
		return cursor->second;
	}

	DtaScriptVariable* DynPackage::addVariable(const char* tcName) {
		std::map<std::string, DtaScriptVariable*>::const_iterator cursor = _variables.find(tcName);
		if (cursor != _variables.end()) return cursor->second;
		DtaScriptVariable* pVariable = new Tree(NULL, tcName);
		_variables[tcName] = pVariable;
		return pVariable;
	}


	DynPackage* DynPackage::getPackage(const std::string& sPackage) {
		std::map<std::string, DynPackage*>::iterator cursor = _packages.find(sPackage);
		if (cursor == _packages.end()) return NULL;
		return cursor->second;
	}

	DynPackage* DynPackage::addPackage(const std::string& sPackage, GrfBlock& block) {
		DynPackage* pPackage = getPackage(sPackage);
		if (pPackage == NULL) {
			pPackage = new DynPackage(sPackage, block);
		}
		return pPackage;
	}

	LIBRARY_HANDLE DynPackage::loadPackage(const std::string& sLibrary) {
		LIBRARY_HANDLE hHandle;
		std::string sLibraryWithExt;
#ifdef WIN32
		sLibraryWithExt = sLibrary + ".dll";
		HMODULE hModule = LoadLibrary(sLibraryWithExt.c_str());
		if (hModule == NULL) {
			sLibraryWithExt = sLibrary + "_d.dll";
			hModule = LoadLibrary(sLibraryWithExt.c_str());
			if (hModule == NULL) {
				std::string sCompleteFileName;
				if (ScpStream::existInputFileFromIncludePath(sLibraryWithExt.c_str(), sCompleteFileName)) {
					sCompleteFileName = CGRuntime::canonizePath(sCompleteFileName);
					hModule = LoadLibrary(sCompleteFileName.c_str());
				}
				if (hModule == NULL) throw UtlException("unable to load library '" + sLibraryWithExt + "'");
			}
		}
		hHandle = hModule;
#else
		sLibraryWithExt = sLibrary + ".so";
		hHandle = dlopen(sLibraryWithExt.c_str(), RTLD_NOW);
		if (hHandle == NULL) {
			sLibraryWithExt = sLibrary + "_d.so";
			hHandle = dlopen(sLibraryWithExt.c_str(), RTLD_NOW);
			throw UtlException("unable to load library '" + sLibraryWithExt + "': " + dlerror());
		}
#endif
		return hHandle;
	}

	LIBRARY_HANDLE DynPackage::loadLibrary(const std::string& sLibrary) {
		LIBRARY_HANDLE hHandle;
#ifdef WIN32
		HMODULE hModule = LoadLibrary(sLibrary.c_str());
		hHandle = hModule;
#else
		hHandle = dlopen(sLibrary.c_str(), RTLD_NOW);
#endif
		return hHandle;
	}

	void DynPackage::freeLibrary(LIBRARY_HANDLE hHandle) {
#ifdef WIN32
		FreeLibrary((HMODULE) hHandle);
#else
		dlclose(hHandle);
#endif
	}

	void* DynPackage::findFunction(LIBRARY_HANDLE hHandle, const std::string& sFunction) {
#ifdef WIN32
		return GetProcAddress((HMODULE) hHandle, sFunction.c_str());
#else
		return dlsym(hHandle, sFunction.c_str());
#endif
	}
}
