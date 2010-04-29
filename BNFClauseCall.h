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

#ifndef _BNFClauseCall_h_
#define _BNFClauseCall_h_

#include "GrfBlock.h"

namespace CodeWorker {
	class DtaBNFScript;
	class BNFClause;
	class ExprScriptExpression;
	class ExprScriptVariable;
	class ASTCommand;

	class BNFClauseCall : public GrfCommand {
	private:
		DtaBNFScript* _pBNFScript;
		BNFClause* _pClause;
		std::vector<ExprScriptExpression*> _listOfParameters;
		ExprScriptExpression* _pTemplateExpression;
		ExprScriptVariable* _pVariableToAssign;
		bool _bConcatVariable;
		std::vector<std::string> _listOfConstants;
		int _iClauseReturnType;
		bool _bContinue;
		bool _bNoCase; // used only when a list of constants is present
		int _iSuperCallDepth;
		std::list<ASTCommand*>* _pASTcommands;

	public:
		BNFClauseCall(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue, bool bNoCase);
		virtual ~BNFClauseCall();

		virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

		virtual bool isABNFCommand() const;

		inline DtaBNFScript& getBNFScript() const { return *_pBNFScript; }
		inline BNFClause& getClause() const { return *_pClause; }
		inline void setClause(BNFClause* pClause) { _pClause = pClause; }
		inline const std::vector<ExprScriptExpression*>& getParameters() const { return _listOfParameters; }
		inline void addParameter(ExprScriptExpression* pParameter) { _listOfParameters.push_back(pParameter); }
		inline ExprScriptExpression* getTemplateExpression() const { return _pTemplateExpression; }
		inline void setTemplateExpression(ExprScriptExpression* pTemplateExpression) { _pTemplateExpression = pTemplateExpression; }
		BNFClause& getInstantiatedClause(DtaScriptVariable& visibility, std::string& sInstantiationKey) const;
		inline int getSuperCallDepth() const { return _iSuperCallDepth; }
		inline void setSuperCallDepth(int iSuperCallDepth) { _iSuperCallDepth = iSuperCallDepth; }

		void addASTCommand(ASTCommand* pCommand);

		void setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause);
		void setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat);
		inline void setConstantsToMatch(const std::vector<std::string>& listOfConstants) { _listOfConstants = listOfConstants; }

		virtual std::string toString() const;

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual void callBeforeExecutionCBK(DtaScriptVariable& visibility);
		virtual void callRecursiveBeforeExecutionCBK(GrfExecutionContext* pContext, DtaScriptVariable& visibility);
		virtual void callAfterExecutionCBK(DtaScriptVariable& visibility);

		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
	};


	class BNFRootClauseCall : public BNFClauseCall {
	public:
		BNFRootClauseCall(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bNoCase);
		virtual ~BNFRootClauseCall();

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
	};
}

#endif
