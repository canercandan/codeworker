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

#ifndef _BNFTryCatch_h_
#define _BNFTryCatch_h_

#include "GrfTryCatch.h"

namespace CodeWorker {
	class DtaBNFScript;

	class BNFTryCatch : public GrfTryCatch {
		public:
			BNFTryCatch(DtaBNFScript* pBNFScript);
			BNFTryCatch(DtaBNFScript* pBNFScript, GrfBlock* pParent);
			virtual ~BNFTryCatch();

			virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

			virtual bool isABNFCommand() const;

			virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

			virtual std::string toString() const;

		protected:
			DtaBNFScript* _pBNFScript;

			SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
			virtual SEQUENCE_INTERRUPTION_LIST executeCatchStatement(DtaScriptVariable& visibility, int iLocation, int iImplicitCopyPosition, const std::string& sCatchedMessage, const std::string& sCatchedStack = "");
			virtual SEQUENCE_INTERRUPTION_LIST executeCatchStatement(DtaScriptVariable& visibility, int iLocation, int iImplicitCopyPosition, UtlException& exception);
	};


	class ExprScriptExpression;

	class BNFContinue : public BNFTryCatch {
		private:
			ExprScriptExpression* _pErrorMessage;
		public:
			BNFContinue(DtaBNFScript* pBNFScript, GrfBlock* pParent, ExprScriptVariable* pVariable, ExprScriptExpression* pMessage);
			virtual ~BNFContinue();

			virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

			virtual std::string toString() const;
		protected:
			virtual SEQUENCE_INTERRUPTION_LIST executeCatchStatement(DtaScriptVariable& visibility, int iLocation, int iImplicitCopyPosition, UtlException& exception);
	};
}

#endif
