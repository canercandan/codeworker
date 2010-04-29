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
#include "BNFCheck.h"

namespace CodeWorker {
	BNFCheck::BNFCheck(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : _pBNFScript(pBNFScript), GrfCommand(pParent), _bContinue(bContinue), _pCondition(NULL) {}

	BNFCheck::~BNFCheck() {
		delete _pCondition;
	}

	void BNFCheck::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFCheck(*this, env);
	}

	bool BNFCheck::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFCheck::executeInternal(DtaScriptVariable& visibility) {
		std::string sValue = _pCondition->getValue(visibility);
		if (sValue.empty()) {
			if (_bContinue) CGRuntime::throwBNFExecutionError(toString());
			return BREAK_INTERRUPTION;
		}
		return NO_INTERRUPTION;
	}

	void BNFCheck::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "// " << toString();
		CW_BODY_ENDL;
		CW_BODY_INDENT << "_compilerClauseSuccess = ";
		_pCondition->compileCppBoolean(theCompilerEnvironment, false);
		CW_BODY_STREAM << ";";
		CW_BODY_ENDL;
		if (_bContinue) {
			CW_BODY_INDENT << "if (!_compilerClauseSuccess) CGRuntime::throwBNFExecutionError(";
			CW_BODY_STREAM.writeString(toString());
			CW_BODY_STREAM << ");";
			CW_BODY_ENDL;
		}
	}

	std::string BNFCheck::toString() const {
		std::string sText = "#check(" + _pCondition->toString() + ")";
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
