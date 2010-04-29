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

#ifndef _GrfFunction_h_
#define _GrfFunction_h_

#pragma warning (disable : 4786)

#include <string>
#include <list>
#include <vector>
#include <map>

#include "GrfBlock.h"
#include "DtaScriptFactory.h"
#include "ExprScriptFunction.h"

namespace CodeWorker {
	#ifndef EXTERNAL_FUNCTION_TYPE
	#define EXTERNAL_FUNCTION_TYPE
	class CppParsingTree_var;
	typedef std::string (*EXTERNAL_FUNCTION)(CppParsingTree_var**);
	#endif

	class DtaPatternScript;

	class GrfFunction : public GrfBlock {
	private:
		std::string _sName;
		std::string _sTemplateInstantiation;
		bool _bGenericKey; // means that it is a generic template function where '_sTemplateInstantiation' gives the generic key
		GrfFunction* _pTemplateFunction; // root function for templates
		std::map<std::string, GrfFunction*> _mapOfInstantiatedFunctions; // belongs to the root template function
		GrfFunction* _pGenericTemplateFunction; // belongs to the root template function
		struct PatternTemplateStruct {
			DtaScriptFactory::SCRIPT_TYPE _eParentScriptType;
			DtaPatternScript* _pPattern;
			PatternTemplateStruct(DtaScriptFactory::SCRIPT_TYPE eParentScriptType, DtaPatternScript* pPattern) : _eParentScriptType(eParentScriptType), _pPattern(pPattern) {}
			~PatternTemplateStruct();
		};
		PatternTemplateStruct* _pPatternTemplateBody; // belongs to the root template function when it defines a pattern script
		bool _bIsBodyDefined;
		GrfBlock* _pFinally;
		bool _bIsExternal;
		EXTERNAL_FUNCTION _pExternalFunction;

	protected:
		std::vector<EXPRESSION_TYPE> _parameterTypes;
		std::list<std::string> _parameters;
		std::vector<ExprScriptExpression*> _defaults;

	public:
		static int _iMaxStackDepth;
		static int _iCurrentStackDepth;

	public:
		GrfFunction(GrfBlock* pParent, const std::string& sName, const std::string& sTemplateInstantiation, bool bGenericKey);
		virtual ~GrfFunction();

		virtual const char* getFunctionName() const { return _sName.c_str(); }
		virtual bool isAPredefinedFunction() const { return false; }

		bool addParameterAndType(const char* sParameter, EXPRESSION_TYPE exprType, ExprScriptExpression* pDefault);
		inline const std::vector<EXPRESSION_TYPE>& getParameterTypes() const { return _parameterTypes; }
		inline const std::list<std::string>& getParameters() const { return _parameters; }
		EXPRESSION_TYPE getParameterType(unsigned int iIndex) const;
		ExprScriptExpression* getDefaultParameter(unsigned int iIndex) const;

		virtual bool addBNFLocalVariable(const std::string& sVarName, EXPRESSION_TYPE varType);
		virtual EXPRESSION_TYPE getLocalVariable(const std::string& sVarName) const;
		virtual EXPRESSION_TYPE getVariable(const std::string& sVarName) const;

		inline const std::string& getName() const { return _sName; }
		std::string getSignature() const;

		inline unsigned int getArity() const { return _parameters.size(); }
		inline unsigned int getMinArity() const { return getArity() - _defaults.size(); }
		inline bool isATemplateInstantiation() const { return (_pTemplateFunction != NULL) || !_mapOfInstantiatedFunctions.empty() || (_pGenericTemplateFunction != NULL); }
		inline bool isATemplateDispatcher() const { return !_mapOfInstantiatedFunctions.empty(); }
		inline const std::string& getTemplateInstantiation() const { return _sTemplateInstantiation; }
		inline bool isGenericKey() const { return _bGenericKey; }
		inline GrfFunction* getTemplateFunction() const { return _pTemplateFunction; }
		inline const std::map<std::string, GrfFunction*>& getInstantiatedFunctions() const { return _mapOfInstantiatedFunctions; }
		GrfFunction* getInstantiatedFunction(DtaScriptVariable& visibility, const std::string& sInstantiationKey) const;
		void setPatternTemplateBody(DtaScriptFactory::SCRIPT_TYPE eParentScriptType, DtaPatternScript* pScript);

		inline bool isBodyDefined() const { return _bIsBodyDefined; }
		inline void isBodyDefined(bool bIsBodyDefined) { _bIsBodyDefined = bIsBodyDefined; }
		inline bool isExternal() const { return _bIsExternal; }
		inline void isExternal(bool bIsExternal) { _bIsExternal = bIsExternal; }
		inline GrfBlock* getFinally() const { return _pFinally; }
		inline void setFinally(GrfBlock* pFinally) { _pFinally = pFinally; }

		virtual void applyRecursively(APPLY_ON_COMMAND_FUNCTION apply);

		virtual std::string launchExecution(DtaScriptVariable& visibility, const ExprScriptFunction& functionCall, const std::string& sInstantiationKey = "");

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual void compileCppFunctionNameForCalling(CppCompilerEnvironment& theCompilerEnvironment);

	protected:
		virtual void callBeforeExecutionCBK(DtaScriptVariable& visibility);
		virtual void callRecursiveBeforeExecutionCBK(GrfExecutionContext* pContext, DtaScriptVariable& visibility);
		virtual void callAfterExecutionCBK(DtaScriptVariable& visibility);

		// called by launchExecution
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
	};
}

#endif
