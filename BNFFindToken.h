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

#ifndef _BNFFindToken_h_
#define _BNFFindToken_h_

#include "BNFStepper.h"

namespace CodeWorker {
	struct BNFMultiplicityBoundaries;

	class BNFFindToken : public BNFStepper {
	private:
			ExprScriptVariable* _pIntermediateVariableToAssign;
			bool _bConcatIntermediateVariable;
			std::vector<std::string> _listOfIntermediateConstants;
			ExprScriptVariable* _pFinalVariableToAssign;
			bool _bConcatFinalVariable;
			std::vector<std::string> _listOfFinalConstants;
			GrfBlock*	_pIntermediateSequence;
			BNFMultiplicityBoundaries* _pBoundaries;

	public:
		BNFFindToken(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue);
		virtual ~BNFFindToken();

		virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

		inline GrfBlock* getIntermediateSequence() const { return _pIntermediateSequence; }
		GrfBlock& createIntermediateSequence();
		void setIntermediateVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause);
		inline void setIntermediateConstantsToMatch(const std::vector<std::string>& listOfConstants) { _listOfIntermediateConstants = listOfConstants; }
		void setFinalVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause);
		inline void setFinalConstantsToMatch(const std::vector<std::string>& listOfConstants) { _listOfFinalConstants = listOfConstants; }
		inline bool hasIntermediateCode() const { return (_pIntermediateVariableToAssign != NULL) || !_listOfIntermediateConstants.empty() || (_pIntermediateSequence != NULL) || !_listOfFinalConstants.empty() || (_pFinalVariableToAssign != NULL);}
		inline void setBoundaries(BNFMultiplicityBoundaries* pBoundaries) { _pBoundaries = pBoundaries; }

		virtual std::string toString() const;

		void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
		void compileCppResolveAssignment(CppCompilerEnvironment& theCompilerEnvironment, int iCursor, const std::vector<std::string>& listOfConstants, ExprScriptVariable* pVariableToAssign, bool bConcatVariable, const std::string& sBeginLocation, const std::string& sEndLocation) const;

	private:
		SEQUENCE_INTERRUPTION_LIST resolveAssignment(DtaScriptVariable& visibility, const std::vector<std::string>& listOfConstants, ExprScriptVariable* pVariableToAssign, bool bConcatVariable, int iImplicitCopyPosition, int iLocation, int iBeginLocation, int iEndLocation);
	};
}

#endif
