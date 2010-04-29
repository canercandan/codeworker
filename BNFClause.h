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

#ifndef _BNFClause_h_
#define _BNFClause_h_

#include <vector>
#include <string>

#include "GrfBlock.h"
#include "ExprScriptFunction.h"

namespace CodeWorker {
	class DtaBNFScript;
	class BNFClause;

	struct BNFClauseMatchingArea {
		BNFClause* clause;
		int beginPosition;
		int endPosition;
		std::list<BNFClauseMatchingArea*> childs;

		BNFClauseMatchingArea(BNFClause* p, int i) : clause(p), beginPosition(i), endPosition(-1) {}
		~BNFClauseMatchingArea();

		inline void pushChild(BNFClauseMatchingArea* pChild) { childs.push_back(pChild); }

		void purgeChildsAfterPosition(int iPosition);
	};


	class BNFClause : public GrfBlock {
	private:
		DtaBNFScript* _pBNFScript;
		std::string _sName;
		std::string _sTemplateKey;
		bool _bGenericKey;
		BNFClause* _pTemplateClause;
		std::map<std::string, BNFClause*> _mapOfTemplateInstantiations;
		BNFClause* _pGenericTemplateClause;
		int _iReturnType;
		std::vector<EXPRESSION_TYPE> _parameterTypes;
		std::vector<std::string> _parameters;
		bool _bPropagatedParameters;
		int _iPreprocessingIgnoreMode;
		BNFClause* _pPreprocessingIgnoreClause;
		BNFClause* _pOverloadClause;

	public:
		static int NO_RETURN_TYPE;
		static int LIST_RETURN_TYPE;
		static int NODE_RETURN_TYPE;
		static int VALUE_RETURN_TYPE;

	public:
		BNFClause(DtaBNFScript* pBNFScript, GrfBlock* pParent, const std::string& sName, const std::string& sTemplateKey, bool bGenericKey, const std::vector<std::string>& listOfParameters, const std::vector<EXPRESSION_TYPE>& listOfParameterTypes);
		BNFClause(DtaBNFScript* pBNFScript, GrfBlock* pParent, const std::string& sName, int iArity);
		virtual ~BNFClause();

		virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

		virtual bool isABNFCommand() const;

		inline const std::string& getName() const { return _sName; }
		inline const std::string& getTemplateKey() const { return _sTemplateKey; }
		inline BNFClause* getTemplateClause() const { return _pTemplateClause; }
		inline bool isGenericKey() const { return _bGenericKey; }
		void setTemplateClause(BNFClause* pTemplateClause);
		BNFClause* getInstantiatedClause(const std::string& sInstantiationKey) const;
		inline bool isATemplateInstantiation() const { return (_pTemplateClause != NULL) || !_mapOfTemplateInstantiations.empty() || (_pGenericTemplateClause != NULL); }
		inline BNFClause* getGenericTemplateClause() const { return _pGenericTemplateClause; }
		inline int getReturnType() const { return _iReturnType; }
		inline void setReturnType(int iReturnType) { _iReturnType = iReturnType; }

		inline unsigned int getArity() const { return _parameters.size(); }
		inline const std::vector<std::string>& getParameters() const { return _parameters; }
		inline const std::vector<EXPRESSION_TYPE>& getParameterTypes() const { return _parameterTypes; }
		inline const std::string& getParameter(int i) const { return _parameters[i]; }
		inline const EXPRESSION_TYPE getParameterType(int i) const { return _parameterTypes[i]; }
		void setParameter(int i, const std::string& sParameter, EXPRESSION_TYPE iType);

		virtual bool addBNFLocalVariable(const std::string& sVarName, EXPRESSION_TYPE varType);
		virtual EXPRESSION_TYPE getLocalVariable(const std::string& sVarName) const;
		virtual EXPRESSION_TYPE getVariable(const std::string& sVarName) const;

		inline bool isPropagatedParameter() const { return _bPropagatedParameters; }
		inline int/*IGNORE_MODE*/ getPreprocessingIgnoreMode() const { return _iPreprocessingIgnoreMode; }
		void setPreprocessingIgnoreMode(int/*IGNORE_MODE*/ iPreprocessingIgnoreMode, BNFClause* pIgnoreClause);
		inline BNFClause* getOverloadClause() const { return _pOverloadClause; }
		void setOverloadClause(BNFClause* pOverloadClause);

		std::string getSignature() const;
		bool propagateParameters(ExprScriptExpression& theFilter, const std::string& sFunctionQuantity, const std::vector<std::string>& listOfParameters, const std::vector<EXPRESSION_TYPE>& listOfParameterTypes);

		virtual SEQUENCE_INTERRUPTION_LIST execute(DtaScriptVariable& visibility);
		virtual SEQUENCE_INTERRUPTION_LIST executeClause(DtaScriptVariable& visibility, int iSuperCallDepth);

		virtual std::string toString() const;
		virtual void compileCppFunction(CppCompilerEnvironment& theCompilerEnvironment) const;

	private:
		SEQUENCE_INTERRUPTION_LIST executeInternalSuperClause(DtaScriptVariable& visibility, int &iSuperCallDepth);
	};
}

#endif
