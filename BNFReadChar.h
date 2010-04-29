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

#ifndef _BNFReadChar_h_
#define _BNFReadChar_h_

#include "GrfBlock.h"

namespace CodeWorker {
	class DtaBNFScript;
	class BNFClause;
	class ExprScriptVariable;

	class BNFReadChar : public GrfCommand {
	protected:
		DtaBNFScript* _pBNFScript;
		ExprScriptVariable* _pVariableToAssign;
		bool _bConcatVariable;
		std::vector<std::string> _listOfConstants;
		int _iClauseReturnType;
		bool _bContinue;

	public:
		BNFReadChar(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue);
		virtual ~BNFReadChar();

		virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

		virtual bool isABNFCommand() const;

		void setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause);
		virtual void setConstantsToMatch(const std::vector<std::string>& listOfConstants);

		virtual std::string toString() const;

		void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
		virtual void postExecutionAssignment(DtaScriptVariable& variable, unsigned char cChar);

		virtual void compileCppAssignment(CppCompilerEnvironment& theCompilerEnvironment, int iCursor) const;
	};
}

#endif
