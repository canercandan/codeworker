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

#ifndef _DynFunction_h_
#define _DynFunction_h_

#include <string>

#include "GrfFunction.h"

namespace CodeWorker {
#ifndef _DynPackage_h_
	typedef void* Parameter;
#endif

	class Interpreter;


	class DynFunction : public GrfFunction {
	private:
		Interpreter* _pInterpreter;
		std::string _sModuleName;
		void* _pfFunctionCall;

	public:
		inline DynFunction(Interpreter* pInterpreter, const std::string& sName) : GrfFunction(NULL, sName, "", false), _pInterpreter(pInterpreter) { init(); }
		DynFunction(Interpreter* pInterpreter, const char* tcName, bool bIsP1Node);
		DynFunction(Interpreter* pInterpreter, const char* tcName, bool bIsP1Node, bool bIsP2Node);
		DynFunction(Interpreter* pInterpreter, const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node);
		DynFunction(Interpreter* pInterpreter, const char* tcName, bool bIsP1Node, bool bIsP2Node, bool bIsP3Node, bool bIsP4Node);
		DynFunction(Interpreter* pInterpreter, const char* tcName, int iNbParams,  int* tbNodeParams);
		virtual ~DynFunction();

		inline void setFunctionCall(void* pfFunctionCall) { _pfFunctionCall = pfFunctionCall; }

		virtual std::string launchExecution(DtaScriptVariable& visibility, const ExprScriptFunction& functionCall, const std::string& sInstantiationKey = "");

		virtual void compileCppFunctionNameForCalling(CppCompilerEnvironment& theCompilerEnvironment);

	private:
		void init();

		std::string call(DtaScriptVariable& pVisibility);
		std::string call(DtaScriptVariable& pVisibility, Parameter p1);
		std::string call(DtaScriptVariable& pVisibility, Parameter p1, Parameter p2);
		std::string call(DtaScriptVariable& pVisibility, Parameter p1, Parameter p2, Parameter p3);
		std::string call(DtaScriptVariable& pVisibility, Parameter p1, Parameter p2, Parameter p3, Parameter p4);
		std::string call(DtaScriptVariable& pVisibility, Parameter* tParams);
	};
}

#endif
