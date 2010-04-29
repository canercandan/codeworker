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

#ifndef _BNFMultiplicity_h_
#define _BNFMultiplicity_h_

#include "BNFStepper.h"

namespace CodeWorker {
	class DtaBNFScript;
	class BNFClause;
	class ExprScriptVariable;

	struct BNFMultiplicityBoundaries {
		bool bConstantBoundaries;
		union BOUNDARY_TYPE {
			struct CONSTANT_BOUNDARIES {
				int _iBegin;
				int _iEnd;
			};
			CONSTANT_BOUNDARIES constant;
			struct EXPRESSION_BOUNDARIES {
				ExprScriptExpression* _pBegin;
				ExprScriptExpression* _pEnd;
			};
			EXPRESSION_BOUNDARIES variable;
		};
		BOUNDARY_TYPE choice;

		inline BNFMultiplicityBoundaries() {}
		~BNFMultiplicityBoundaries();

		void setMultiplicity(int iBegin, int iEnd);
		void setMultiplicity(ExprScriptExpression* pBegin, ExprScriptExpression* pEnd);

		void computeBoundaries(DtaScriptVariable& visibility, int& iBegin, int& iEnd) const;

		std::string toString() const;
	};

	class BNFMultiplicity : public GrfBlock {
	private:
		DtaBNFScript* _pBNFScript;
		ExprScriptVariable* _pVariableToAssign;
		bool _bConcatVariable;
		std::vector<std::string> _listOfConstants;
		int _iClauseReturnType;
		bool _bContinue;
		BNFMultiplicityBoundaries _boundaries;

	public:
		BNFMultiplicity(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue);
		virtual ~BNFMultiplicity();

		virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

		virtual bool isABNFCommand() const;

		void setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause);
		inline void setConstantsToMatch(const std::vector<std::string>& listOfConstants) { _listOfConstants = listOfConstants; }

		inline BNFMultiplicityBoundaries& getBoundaries() { return _boundaries; }

		virtual SEQUENCE_INTERRUPTION_LIST execute(DtaScriptVariable& visibility);

		virtual std::string toString() const;
		void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
	};
}

#endif
