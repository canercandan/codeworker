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

#ifndef _DynPackage_h_
#define _DynPackage_h_

#include <string>
#include <map>

namespace CodeWorker {
	class GrfBlock;

	typedef void* LIBRARY_HANDLE;
#ifndef _DynFunction_h_
	typedef void* Parameter;
#endif

	class Interpreter;
	class DynFunction;
	class DtaScriptVariable;

	typedef const char* (*CALL0_FUNCTION)(Interpreter*);
	typedef const char* (*CALL1_FUNCTION)(Interpreter*, Parameter);
	typedef const char* (*CALL2_FUNCTION)(Interpreter*, Parameter, Parameter);
	typedef const char* (*CALL3_FUNCTION)(Interpreter*, Parameter, Parameter, Parameter);
	typedef const char* (*CALL4_FUNCTION)(Interpreter*, Parameter, Parameter, Parameter, Parameter);
	typedef const char* (*CALLN_FUNCTION)(Interpreter*, Parameter*);


	class DynPackage {
	private:
		static std::map<std::string, DynPackage*> _packages;

		GrfBlock* _pBlock;
		std::map<std::string, DynFunction*> _functions;
		std::map<std::string, DtaScriptVariable*> _variables;

	protected:
		std::string _sPackage;
		LIBRARY_HANDLE _hHandle;
		Interpreter* _pInterpreter;

	public:
		~DynPackage();

		inline const std::string& getName() const { return _sPackage; }
		inline GrfBlock* getBlock() const { return _pBlock; }

		static inline const std::map<std::string, DynPackage*>& allPackages() { return _packages; }
		inline const std::map<std::string, DynFunction*>& allFunctions() const { return _functions; }

		DynFunction* getFunction(const std::string& sFunction) const;
		bool addFunction(const char* tcName);
		bool addFunction(const char* tcName, bool bIsP1Node);
		bool addFunction(const char* tcName, bool bIsP1Node, bool bIsP2Node);
		bool addFunction(const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node);
		bool addFunction(const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node, bool bIsP4Node);
		bool addFunction(const char* tcName, int  iNbParams, int* tbNodeParams);

		DtaScriptVariable* getVariable(const std::string& sVariable) const;
		DtaScriptVariable* addVariable(const char* tcName);

		static DynPackage* addPackage(const std::string& sPackage, GrfBlock& block);
		static DynPackage* getPackage(const std::string& sPackage);

		static LIBRARY_HANDLE loadLibrary(const std::string& sLibrary);
		static LIBRARY_HANDLE loadPackage(const std::string& sLibrary);
		static void freeLibrary(LIBRARY_HANDLE hHandle);
		static void* findFunction(LIBRARY_HANDLE hHandle, const std::string& sFunction);

	protected:
		DynPackage(const std::string& sPackage);

	private:
		DynPackage(const std::string& sPackage, GrfBlock& block);
		void initialize();
		bool addFunction(DynFunction* pFunction);
	};

}

#endif
