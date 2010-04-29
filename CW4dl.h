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

#ifndef _CodeWorker_for_dynamic_libraries_h_
#define _CodeWorker_for_dynamic_libraries_h_

/**
 *  -------------------------------------------------------
 *  CW4dl: CodeWorker header for building dynamic libraries
 *  -------------------------------------------------------
 *
 * This C++ header provides all what you need to construct a new package
 * intended to CodeWorker.
 * There is no C++ body for this file. Just include this file in your
 * project, and it is sufficient.
 *
 *
 * What you have to know for building a new package.
 * -------------------------------------------------
 *
 * Let say that the package's name is 'XXX'. The corresponding executable
 * module (dynamic library) will be "XXXcw.dll" under Windows, and
 * "XXXcw.so" under Linux or other.
 *
 * When the interpreter will found the preprocessor directive '#use XXX'
 * in a script, it will load the executable module and execute the
 * exported C-like function 'void XXX_Init(CW4dl::Interpreter*)'.
 *
 * This C-like function MUST be present! 'C-like' means that it is declared
 * extern "C".
 *
 * Initializing the module that way is useful for registering new functions
 * in the engine, via the function 'createCommand()' of the interpreter
 * (see below in the declaration of the class 'Interpreter' for learning
 * more about it).
 *
 * Every function to export must start its declaration with the preprocessor
 * definition 'CW4DL_EXPORT_SYMBOL' (means 'extern "C"', but a little more
 * under Windows).
 *
 * # Up to 4 parameters, the signature of such a function looks like:
 *     CW4DL_EXPORT_SYMBOL const char* selectList(CW4dl::Interpreter*,
 *												  CW4dl::Parameter p1,
 *												  CW4dl::Parameter p2);
 * where 'selectList' is a function expecting 2 parameters.
 *
 * The initializer 'XXX_Init' informs the engine about the existence
 * of this function in the package:
 *     createCommand("selectList", VALUE_PARAMETER, NODE_PARAMETER);
 * which means that 'selectList' expects a string followed by a tree.
 *
 * In the body of the function 'selectList(...)', the C++ binding is
 * obtained easily by a cast of 'CW4dl::Parameter':
 *   - '(const char*)  p1' for the value parameter 'p1',
 *   - '(CW4dl::Tree*) p2' for the node  parameter 'p2',
 *
 * # if a function contains strictly more than 4 parameter, its signature
 * # changes and requires a variable number of parameters:
 *     CW4DL_EXPORT_SYMBOL const char* myFunction(CW4dl::Interpreter*,
 *												  int nbParams,
 *												  CW4dl::Parameter* tParams);
 * where 'tParams' is an array of parameter types, and where 'nbParams' gives
 * the size.
 *
 * The initializer 'XXX_Init' informs the engine about the existence
 * of this function in the package differently too:
 *     createCommand("myFunction", 6, tParams);
 * which means that 'myFunction' has 6 parameters whose types are provided
 * in 'tParams'.
 *
 * Note that every function returns 'const char*'.
 **/


/**
 *  -------------------------------------------------------
 *  CW4dl: CodeWorker header for building dynamic libraries
 *  -------------------------------------------------------
 *
 * This C++ header provides all what you need to construct a new package
 * intended to CodeWorker.
 * There is no C++ body for this file. Just include this file in your
 * project, and it is sufficient.
 *
 *
 * What you have to know for building a new package.
 * -------------------------------------------------
 *
 * Let say that the package's name is 'XXX'. The corresponding executable
 * module (dynamic library) will be "XXXcw.dll" under Windows, and
 * "XXXcw.so" under Linux or other.
 *
 * When the interpreter will found the preprocessor directive '#use XXX'
 * in a script, it will load the executable module and execute the
 * exported C-like function 'void XXX_Init(CW4dl::Interpreter*)'.
 *
 * This C-like function MUST be present! 'C-like' means that it is declared
 * extern "C".
 *
 * Initializing the module that way is useful for registering new functions
 * in the engine, via the function 'createCommand()' of the interpreter
 * (see below in the declaration of the class 'Interpreter' for learning
 * more about it).
 *
 * Every function to export must start its declaration with the preprocessor
 * definition 'CW4DL_EXPORT_SYMBOL' (means 'extern "C"', but a little more
 * under Windows).
 *
 * # Up to 4 parameters, the signature of such a function looks like:
 *     CW4DL_EXPORT_SYMBOL const char* selectList(CW4dl::Interpreter*,
 *												  CW4dl::Parameter p1,
 *												  CW4dl::Parameter p2);
 * where 'selectList' is a function expecting 2 parameters.
 *
 * The initializer 'XXX_Init' informs the engine about the existence
 * of this function in the package:
 *     createCommand("selectList", VALUE_PARAMETER, NODE_PARAMETER);
 * which means that 'selectList' expects a string followed by a tree.
 *
 * In the body of the function 'selectList(...)', the C++ binding is
 * obtained easily by a cast of 'CW4dl::Parameter':
 *   - '(const char*)  p1' for the value parameter 'p1',
 *   - '(CW4dl::Tree*) p2' for the node  parameter 'p2',
 *
 * # if a function contains strictly more than 4 parameter, its signature
 * # changes and requires a variable number of parameters:
 *     CW4DL_EXPORT_SYMBOL const char* myFunction(CW4dl::Interpreter*,
 *												  int nbParams,
 *												  CW4dl::Parameter* tParams);
 * where 'tParams' is an array of parameter types, and where 'nbParams' gives
 * the size.
 *
 * The initializer 'XXX_Init' informs the engine about the existence
 * of this function in the package differently too:
 *     createCommand("myFunction", 6, tParams);
 * which means that 'myFunction' has 6 parameters whose types are provided
 * in 'tParams'.
 *
 * Note that every function returns 'const char*'.
 **/

namespace CodeWorker {
	// type of an external instance, of a type defined by the plugin developer.
	class ExternalValueNode;
}


namespace CW4dl {
	/**
	 * One may share 2 kinds of parameter between the interpreter and
	 * the executable module:
	 *    - value: binds to a 'const char*'
	 *    - node:  binds to a 'CW4dl::Tree*', which refers to the classical
	 *             tree structure of CodeWorker
	 **/
	enum PARAMETER_TYPE {
		VALUE_PARAMETER,
		NODE_PARAMETER
	};

	// see further to understand the role of 'Interpreter'
	class Interpreter;

	/**
	 * The preprocessor definition 'CW4DL_EXPORT_SYMBOL' must appear just
	 * before the declaration of functions to export.
	 **/
#	ifdef WIN32
#		define CW4DL_EXPORT_SYMBOL extern "C" _declspec(dllexport)
#	else
#		define CW4DL_EXPORT_SYMBOL extern "C"
#	endif

	typedef void* Parameter;
	typedef void* Tree;
	typedef void* Function;

	typedef void* DynPackage; // private declaration. Don't care about it.


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
	};


	// Private callback signatures for calling CodeWorker's features from
	// the module.
	// You aren't concerned by them.
	typedef bool (*CREATE_COMMAND0)(DynPackage*, const char*);
	typedef bool (*CREATE_COMMAND1)(DynPackage*, const char*, bool);
	typedef bool (*CREATE_COMMAND2)(DynPackage*, const char*, bool, bool);
	typedef bool (*CREATE_COMMAND3)(DynPackage*, const char*, bool, bool, bool);
	typedef bool (*CREATE_COMMAND4)(DynPackage*, const char*, bool, bool, bool, bool);
	typedef bool (*CREATE_COMMANDN)(DynPackage*, const char*, int,  int*);
	typedef Tree* (*CREATE_VARIABLE)(DynPackage*, const char*);
	typedef void (*ERROR_COMMAND)(const char*);
	typedef const char* (*COPY_LOCAL_STRING)(Interpreter* pInterpreter, const char*);
	typedef const char* (*COPY_LOCAL_BOOLEAN)(Interpreter* pInterpreter, bool);
	typedef const char* (*COPY_LOCAL_INT)(Interpreter* pInterpreter, int);
	typedef const char* (*COPY_LOCAL_DOUBLE)(Interpreter* pInterpreter, double);

	typedef Tree*		(*CREATE_TREE)(const char*);
	typedef const char*	(*GET_VALUE)(Tree*);
	typedef int			(*GET_INT_VALUE)(Tree*);
	typedef double		(*GET_DOUBLE_VALUE)(Tree*);
	typedef bool		(*GET_BOOLEAN_VALUE)(Tree*);
	typedef CodeWorker::ExternalValueNode*	(*GET_EXTERNAL_VALUE)(Tree*);
	typedef void		(*SET_VALUE)(Tree*, const char*);
	typedef void		(*SET_EXTERNAL_VALUE)(Tree*, CodeWorker::ExternalValueNode*);
	typedef void		(*SET_INT_VALUE)(Tree*, int);
	typedef void		(*SET_BOOLEAN_VALUE)(Tree*, bool);
	typedef void		(*SET_DOUBLE_VALUE)(Tree*, double);
	typedef Tree*		(*NEXT_NODE)(Tree*, const char*);
	typedef Tree*		(*INSERT_NODE)(Tree*, const char*);
	typedef void		(*CLEAR_NODE)(Tree*);
	typedef int			(*SIZE_ARRAY)(Tree*);
	typedef	int			(*ALL_KEYS)(Tree*, const char**);
	typedef	int			(*ALL_VALUES)(Tree*, const char**);
	typedef Tree*		(*GET_ITEM_FROM_KEY)(Tree*, const char*);
	typedef Tree*		(*GET_ITEM_FROM_POS)(Tree*, int);
	typedef const char*	(*GET_ITEM_KEY)(Tree*, int);
	typedef Tree*		(*PUSH_ITEM)(Tree*, const char*);
	typedef Tree*		(*INSERT_ITEM)(Tree*, const char*, const char*);

	typedef Function*	(*FIND_FUNCTION)(Interpreter* pInterpreter, const char*);

	typedef const char*	(*CALL_FUNCTION0)(Interpreter* pInterpreter, Function*);
	typedef const char*	(*CALL_FUNCTION1)(Interpreter* pInterpreter, Function*, Parameter2CW);
	typedef const char*	(*CALL_FUNCTION2)(Interpreter* pInterpreter, Function*, Parameter2CW, Parameter2CW);
	typedef const char*	(*CALL_FUNCTION3)(Interpreter* pInterpreter, Function*, Parameter2CW, Parameter2CW, Parameter2CW);
	typedef const char*	(*CALL_FUNCTION4)(Interpreter* pInterpreter, Function*, Parameter2CW, Parameter2CW, Parameter2CW, Parameter2CW);

	typedef void		(*RELEASE_FUNCTION)(Function*);


	/** class 'Interpreter'
	 * It represents the runtime context of CodeWorker. It is the unavoidable
	 * intermediary between the module you are building and CodeWorker.
	 * Use it for:
	 *   - registering new functions into the CodeWorker's engine,
	 *   - throwing an error,
	 *   - handling parse trees,
	 **/
	class Interpreter {
		private:
			// Private callbacks for calling CodeWorker's features.
			// Don't care about it
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
			COPY_LOCAL_INT	_copyLocalInt;
			COPY_LOCAL_DOUBLE	_copyLocalDouble;

			CREATE_TREE	 _createTree;
			GET_VALUE	 _getValue;
			GET_INT_VALUE	 _getIntValue;
			GET_DOUBLE_VALUE _getDoubleValue;
			GET_BOOLEAN_VALUE _getBooleanValue;
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
			ALL_KEYS     _allKeys;
			ALL_VALUES     _allValues;
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

			// Reference to the CodeWorker's package that manages this module.
			// Don't care about it.
			DynPackage* _pPackage;

			// Variable scope of the interpreter.
			// Not used, but might be in future callbacks.
			Tree* _pVisibility;

			// Contains a string allocated locally. Don't care about this field.
			char* _tcLocalString;

		public:
			//------------------------------------------------------------------
			// Creating new functions to register in CodeWorker.
			//--------------------------------------------------
			//
			// Enriching the engine of CodeWorker with new functions is done by
			// calling 'createCommand()'.
			//
			// These calls are done just after loading the module in
			// CodeWorker (preprocessor statement '#use'), when the engine
			// initializes the library, running the exported special function
			// 'xxx_Init' (where 'xxx' is the name of the package).
			//
			// The signature of the initializer conforms to:
			// 'CW4DL_EXPORT_SYMBOL void xxx_Init(CW4dl::Interpreter* interpreter)'
			//------------------------------------------------------------------

			/** function 'createCommand()'
			 * createCommand() serves to inform the engine of CodeWorker about
			 * the functions exported by the package, giving their signature.
			 * Each createCommand() takes the name of the function as the first
			 * parameter. Then, it describes the type of parameters:
			 *   - VALUE_PARAMETER: 'const char*'  in the C++ binding,
			 *   - NODE_PARAMETER:  'CW4dl::Tree*' in the C++ binding.
			 *
			 * To register functions belonging up to 4 parameters, use the function
			 * with the fixed number of parameters (5 first functions).
			 * To register functions expecting strictly more that 4 parameters,
			 * use the 'createCommand()' with variable number of parameters.
			 **/
			inline bool createCommand(const char* tcName) { return _createCommand0(_pPackage, tcName); }
			inline bool createCommand(const char* tcName, PARAMETER_TYPE p1) { return _createCommand1(_pPackage, tcName, (p1 == NODE_PARAMETER)); }
			inline bool createCommand(const char* tcName, PARAMETER_TYPE p1, PARAMETER_TYPE p2) { return _createCommand2(_pPackage, tcName, (p1 == NODE_PARAMETER), (p2 == NODE_PARAMETER)); }
			inline bool createCommand(const char* tcName, PARAMETER_TYPE p1, PARAMETER_TYPE p2, PARAMETER_TYPE p3) { return _createCommand3(_pPackage, tcName, (p1 == NODE_PARAMETER), (p2 == NODE_PARAMETER), (p3 == NODE_PARAMETER)); }
			inline bool createCommand(const char* tcName, PARAMETER_TYPE p1, PARAMETER_TYPE p2, PARAMETER_TYPE p3, PARAMETER_TYPE p4) { return _createCommand4(_pPackage, tcName, (p1 == NODE_PARAMETER), (p2 == NODE_PARAMETER), (p3 == NODE_PARAMETER), (p4 == NODE_PARAMETER)); }
			inline bool createCommand(const char* tcName, int iNbParams, PARAMETER_TYPE* tParams) { return _createCommandN(_pPackage, tcName, iNbParams, (int*) tParams); }

			inline Tree* createVariable(const char* tcName) { return _createVariable(_pPackage, tcName); }

			/** function 'copyLocalString(...)'
			 * Both CodeWorker and the shared library have their own heap
			 * management.
			 * You can't allocate memory in the module and desallocate
			 * it in CodeWorker.
			 * To pass a string as a result of function, it must be either
			 * a constant string or a string you have to delete after its
			 * use.
			 * But when, and how?
			 * The string will be allocated in the body of the function and
			 * desallocated by the interpreter after the call. You allocate
			 * the string by running 'copyLocalString()':
			 *   Parameter:
			 *       - tcText: a constant string to copy,
			 *   Return value:
			 *       The duplicated input string, allocated in the scope of
			 *       the interpreter.
			 **/ 
			inline const char* copyLocalString(const char* tcText) { return _copyLocalString(this, tcText); }
			inline const char* copyLocalBoolean(bool bValue) { return _copyLocalBoolean(this, bValue); }
			inline const char* copyLocalInt(int iValue) { return _copyLocalInt(this, iValue); }
			inline const char* copyLocalDouble(double dValue) { return _copyLocalDouble(this, dValue); }

			/** function 'error(...)'
			 * Throws an error.
			 *
			 * Equivalent to the function 'error(...)' available in the
			 * scripting language.
			 **/
			inline void error(const char* tcError) { _error(tcError); }

			/** function 'trueConstant()'
			 * Returns the boolean value 'true'.
			 **/
			inline static const char* trueConstant() { return "true"; }

			//------------------------------------------------------------------
			// Node creation
			//------------------------------------------------------------------

			/** function 'createTree(...)'
			 * Returns a new local tree. The caller has to delete this tree after
			 * use.
			 **/
			inline Tree* createTree(const char* tcName) { return _createTree(tcName); }

			//------------------------------------------------------------------
			// Node value
			//------------------------------------------------------------------

			/** function 'getValue(...)'
			 * Returns the value of a node.
			 **/
			inline const char* getValue(Tree* pTree) { return _getValue(pTree); }

			/** function 'getIntValue(...)'
			 * Returns the value of a node as an integer.
			 **/
			inline int getIntValue(Tree* pTree) { return _getIntValue(pTree); }

			/** function 'getDoubleValue(...)'
			 * Returns the value of a node as a double.
			 **/
			inline double getDoubleValue(Tree* pTree) { return _getDoubleValue(pTree); }

			/** function 'getBooleanValue(...)'
			 * Returns the value of a node as a boolean.
			 **/
			inline bool getBooleanValue(Tree* pTree) { return _getBooleanValue(pTree); }

			/** function 'getExternalValue(...)'
			 * Returns the external instance of a node.
			 **/
			inline CodeWorker::ExternalValueNode* getExternalValue(Tree* pTree) { return _getExternalValue(pTree); }

			/** function 'setValue(...)'
			 * Assigns a value to a node.
			 **/
			inline void setValue(Tree* pTree, const char* tcValue) { _setValue(pTree, tcValue); }

			/** function 'setIntValue(...)'
			 * Assigns an integer value to a node.
			 **/
			inline void setIntValue(Tree* pTree, int iValue) { _setIntValue(pTree, iValue); }

			/** function 'setDoubleValue(...)'
			 * Assigns a double value to a node.
			 **/
			inline void setDoubleValue(Tree* pTree, double dValue) { _setDoubleValue(pTree, dValue); }

			/** function 'setBooleanValue(...)'
			 * Assigns a boolean value to a node.
			 **/
			inline void setBooleanValue(Tree* pTree, bool bValue) { _setBooleanValue(pTree, bValue); }

			/** function 'setExternalValue(...)'
			 * Assigns an external object type to a node.
			 **/
			inline void setExternalValue(Tree* pTree, CodeWorker::ExternalValueNode* pInstance) { _setExternalValue(pTree, pInstance); }

			//------------------------------------------------------------------
			// Node attribute
			//------------------------------------------------------------------

			/** function 'nextNode(...)'
			 * Navigates to the node an attribute is pointing to.
			 * If the node doesn't exist, it returns a null pointer.
			 *
			 * Example:
			 *   CW4dl::Tree* pClassName = interpreter->nextNode(pClass, "name");
			 **/
			inline Tree* nextNode(Tree* pTree, const char* tcAttribute) { return _nextNode(pTree, tcAttribute); }

			/** function 'insertNode(...)'
			 * Inserts a subnode to a node.
			 * If the node already exists, it does nothing.
			 *
			 * Example:
			 *   CW4dl::Tree* pMethods = interpreter->insertNode(pClass, "methods");
			 **/
			inline Tree* insertNode(Tree* pTree, const char* tcAttribute) { return _insertNode(pTree, tcAttribute); }

			/** function 'clearNode(...)'
			 * Clears the value and erases all subnodes.
			 **/
			inline void  clearNode(Tree* pTree) { _clearNode(pTree); }

			//------------------------------------------------------------------
			// Node array
			//------------------------------------------------------------------

			/** function 'size(...)'
			 * Returns the size of an array (i.e the number of items).
			 **/
			inline int size(Tree* pTree) { return _sizeArray(pTree); }

			/** function 'allKeys(..., const char**)'
			 * Populates a list of all keys of the array owned by the node, and
			 * returns the size.
			 * Note: you must allocate sufficient memory for the storage of keys.
			 *
			 * Example:
			 *   const char** tcKeys = new const char*[interpreter->size(listOfRooms)];
			 *   interpreter->allKeys(listOfRooms, "kitchen");
			 *   ...
			 *   delete [] tcKeys;
			 **/
			inline int allKeys(Tree* pTree, const char** tcKeys) { return _allKeys(pTree, tcKeys); }

			/** function 'allValues(..., const char**)'
			 * Populates a list of all values of the array owned by the node, and
			 * returns the size.
			 * Note: you must allocate sufficient memory for the storage of values.
			 *
			 * Example:
			 *   const char** tcValues = new const char*[interpreter->size(listOfRooms)];
			 *   interpreter->allValues(listOfRooms, "kitchen");
			 *   ...
			 *   delete [] tcValues;
			 **/
			inline int allValues(Tree* pTree, const char** tcValues) { return _allValues(pTree, tcValues); }

			/** function 'getItem(..., const char*)'
			 * Returns the item of an array, whose key is passed as parameter.
			 *
			 * Example:
			 *   CW4dl::Tree* pItem = interpreter->getItem(listOfRooms, "kitchen");
			 * In CodeWorker:
			 *   localref pItem = listOfRooms["kitchen"];
			 **/
			inline Tree* getItem(Tree* pTree, const char* tcKey) { return _getItemFromKey(pTree, tcKey); }

			/** function 'getItem(..., int)'
			 * Returns the item of an array, whose rank is passed as parameter.
			 * It starts counting a 0.
			 *
			 * Example:
			 *   CW4dl::Tree* pItem = getItem(listOfRooms, 2);
			 * In CodeWorker:
			 *   localref pItem = listOfRooms#[2];
			 **/
			inline Tree* getItem(Tree* pTree, int iKey) { return _getItemFromPos(pTree, iKey); }

			/** function 'getItemKey(..., int)'
			 * Returns the key of an array's item, whose rank is passed as parameter.
			 * It starts counting a 0.
			 *
			 * Example:
			 *   interpreter->insertItem(listOfRooms, "kitchen", "2nd floor");
			 *   interpreter->insertItem(listOfRooms, "bathroom");
			 *   const char* tcKey0 = getItemKey(listOfRooms, 0);
			 *   const char* tcKey1 = getItemKey(listOfRooms, 1);
			 * with:
			 *   tcKey0 equals "kitchen"
			 *   tcKey1 equals "bathroom"
			 **/
			inline const char* getItemKey(Tree* pTree, int iKey) { return _getItemKey(pTree, iKey); }

			/** function 'pushItem(...)'
			 * Adds a new item in a list, where the key is automatically computed as
			 * the rank of the item in the list (starting at 0), and returns it.
			 * A value may be assigned to the new item.
			 *
			 * Example:
			 *   CW4dl::Tree* pItem = interpreter->pushItem(pList, "myFile.txt");
			 * In CodeWorker:
			 *   pushItem pList = "myFile.txt";
			 *   localref pItem = pList#back;
			 **/
			inline Tree* pushItem(Tree* pArray, const char* tcValue = NULL) { return _pushItem(pArray, tcValue); }

			/** function 'insertItem(...)'
			 * Inserts a new item in a list, associated to a given key.
			 * A value may be assigned to the new item.
			 *
			 * Example:
			 *   interpreter->insertItem(pList, "kitchen", "2nd floor");
			 *   interpreter->insertItem(pList, "bathroom");
			 * In CodeWorker:
			 *   insert pList["kitchen"] = "2nd floor";
			 *   insert pList["bathroom"];
			 **/
			inline Tree* insertItem(Tree* pArray, const char* tcKey, const char* tcValue = NULL) { return _insertItem(pArray, tcKey, tcValue); }

			/** function 'findFunction(...)'
			 * Searches a function implemented in CodeWorker, at the current
			 * instruction scope.
			 * This function may be either predefined or user defined.
			 *
			 * Example:
			 *   CW4dl::Function* pFunction = interpreter->findFunction("traceObject");
			 *   interpreter->callFunction(pFunction, pList);
			 **/
			inline Function* findFunction(const char* tcFunctionName) { return _findFunction(this, tcFunctionName); }

			/** function 'callFunction(...)'
			 * Calls a CodeWorker's function previously obtained with 'findFunction()'.
			 * This function may be either predefined or user defined.
			 *
			 * Important: you have to delete the function's pointer after use, with
			 *   'releaseFunction()'.
			 *
			 * Example:
			 *   CW4dl::Function* pFunction = interpreter->findFunction("traceObject");
			 *   interpreter->callFunction(pFunction, pList);
			 *   ...
			 *   interpreter->releaseFunction(pFunction); // not used anymore
			 **/
			inline const char* callFunction(Function* pfFunction) { return _callFunction0(this, pfFunction); }
			inline const char* callFunction(Function* pfFunction, Parameter2CW p1) { return _callFunction1(this, pfFunction, p1); }
			inline const char* callFunction(Function* pfFunction, Parameter2CW p1, Parameter2CW p2) { return _callFunction2(this, pfFunction, p1, p2); }
			inline const char* callFunction(Function* pfFunction, Parameter2CW p1, Parameter2CW p2, Parameter2CW p3) { return _callFunction3(this, pfFunction, p1, p2, p3); }
			inline const char* callFunction(Function* pfFunction, Parameter2CW p1, Parameter2CW p2, Parameter2CW p3, Parameter2CW p4) { return _callFunction4(this, pfFunction, p1, p2, p3, p4); }

			inline void releaseFunction(Function* pfFunction) { _releaseFunction(pfFunction); }

		private:
			Interpreter();
			Interpreter(const Interpreter&);
	};
}

#endif
