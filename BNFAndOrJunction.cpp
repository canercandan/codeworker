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

#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include "UtlException.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "DtaBNFScript.h"
#include "BNFClause.h"
#include "DtaVisitor.h"
#include "BNFAndOrJunction.h"

namespace CodeWorker {
	BNFAndOrJunction::BNFAndOrJunction(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : _pBNFScript(pBNFScript), GrfBlock(pParent), _bContinue(bContinue), _pLeftMember(NULL) {}

	BNFAndOrJunction::~BNFAndOrJunction() {
		delete _pLeftMember;
	}

	void BNFAndOrJunction::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFAndOrJunction(*this, env);
	}

	bool BNFAndOrJunction::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFAndOrJunction::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		SEQUENCE_INTERRUPTION_LIST result = _pLeftMember->execute(visibility);
		if (result != CONTINUE_INTERRUPTION) {
			SEQUENCE_INTERRUPTION_LIST result2 = GrfBlock::executeInternal(visibility);
			if ((result == BREAK_INTERRUPTION) && (result2 == BREAK_INTERRUPTION)) {
				BNF_SYMBOL_HAS_FAILED
			} else if (result2 != BREAK_INTERRUPTION) {
				result = result2;
			}
		}
		return result;
	}

	void BNFAndOrJunction::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CPP_COMPILER_BNF_SYMBOL_BEGIN;
		_pLeftMember->compileCpp(theCompilerEnvironment);
		CW_BODY_INDENT << "bool _compilerClauseSuccess" << iCursor << " = _compilerClauseSuccess;";CW_BODY_ENDL;
		GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
		CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "_compilerClauseSuccess = _compilerClauseSuccess" << iCursor << ";";CW_BODY_ENDL;
		CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
		CPP_COMPILER_BNF_SYMBOL_HAS_FAILED;
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
	}

	std::string BNFAndOrJunction::toString() const {
		std::string sText = _pLeftMember->toString() + " &| " + getCommands()[0]->toString();
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
