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

#ifndef _ExprScriptFunction_h_
#define _ExprScriptFunction_h_

#include <list>
#include <vector>
#include <map>
#include <string>

#include "ExprScriptExpression.h"

namespace CodeWorker {
	class ScpStream;

	class DtaScriptVariable;
	class GrfFunction;
	class GrfBlock;
	class ExprScriptFunction;

	enum EXPRESSION_TYPE {
		UNKNOWN_EXPRTYPE = -1,
		VALUE_EXPRTYPE = 0,
		NODE_EXPRTYPE = 1,
		ITERATOR_EXPRTYPE = 2,
		REFERENCE_EXPRTYPE = 3,
		SCRIPTFILE_EXPRTYPE = 8, // common script
		SCRIPTFILE_PATTERN_EXPRTYPE = 9,
		SCRIPTFILE_FREE_EXPRTYPE = 10,
		SCRIPTFILE_BNF_EXPRTYPE = 11,
		SCRIPTFILE_TRANSLATE_EXPRTYPE = 12,
		ARRAY_EXPRTYPE = 16 // for specifying an array type
	};

	typedef ExprScriptFunction* (*CREATE_FUNCTION)(GrfBlock&);


	class DtaFunctionInfo {
	public:
		CREATE_FUNCTION constructor;
		unsigned int* pCounter;
	};


	class ExprScriptFunction : public ExprScriptExpression {
	protected:
		std::vector<ExprScriptExpression*> _parameters;
		ExprScriptExpression* _pTemplate;
		GrfFunction* _pPrototype;
		bool _bIsExternal;

	public:
		ExprScriptFunction(GrfFunction* pPrototype = NULL);
		virtual ~ExprScriptFunction();

		virtual bool isAFunctionExpression() const { return true; }
		virtual bool isAGenerateFunction() const { return false; }
		virtual bool isAParseFunction() const { return false; }

		void addParameter(ExprScriptExpression* pParameter);
		void clearParameters();
		const std::vector<ExprScriptExpression*>& getParameters() const { return _parameters; }
		virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const;
		virtual ExprScriptExpression* getDefaultParameter(unsigned int iIndex) const;
		virtual unsigned int getArity() const;
		virtual unsigned int getMinArity() const;
		virtual const char* getName() const;
		inline GrfFunction* getUserBody() const { return _pPrototype; }
		inline void setTemplate(ExprScriptExpression* pTemplate) { _pTemplate = pTemplate; }
		virtual int getThisPosition() const;
		virtual void initializationDone();

		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const;
		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const;

		virtual std::string toString() const;

		static ExprScriptFunction* create(GrfBlock& block, ScpStream& script, const std::string& sFunction, const std::string& sTemplate, bool bGenericKey);
		static ExprScriptFunction* createMethod(GrfBlock& block, ScpStream& script, const std::string& sFunction, const std::string& sTemplate, bool bGenericKey);
		static std::map<std::string, DtaFunctionInfo>& getFunctionRegister();
		static void clearCounters();
	};
}

#endif

