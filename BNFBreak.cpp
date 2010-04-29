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

#include "DtaVisitor.h"
#include "BNFBreak.h"

namespace CodeWorker {

	BNFBreak::BNFBreak(GrfBlock* pParent) : GrfCommand(pParent), _pCondition(NULL) {}

	BNFBreak::~BNFBreak() {
		delete _pCondition;
	}

	void BNFBreak::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFBreak(*this, env);
	}

	bool BNFBreak::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFBreak::executeInternal(DtaScriptVariable& visibility) {
		std::string sValue = (_pCondition != NULL)?_pCondition->getValue(visibility):"true";
		if (sValue.empty()) {
			return NO_INTERRUPTION;
		}
		return CONTINUE_INTERRUPTION;
	}

	void BNFBreak::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (theCompilerEnvironment.getLastRepeatCursor(true) < 0) throw UtlException("while translating a BNF directive #break to C++: #break must be included in a repeated sequence");
		CW_BODY_INDENT << "// " << toString();
		CW_BODY_ENDL;
		if (_pCondition != NULL) {
			CW_BODY_INDENT << "if (";
			_pCondition->compileCppBoolean(theCompilerEnvironment, false);
			CW_BODY_STREAM << ") goto _compilerRepeatLabel" << theCompilerEnvironment.getLastRepeatCursor(true) << ";";
			CW_BODY_ENDL;
		} else {
			CW_BODY_INDENT << "goto _compilerRepeatLabel" << theCompilerEnvironment.getLastRepeatCursor(true) << ";";CW_BODY_ENDL;
		}
	}

	std::string BNFBreak::toString() const {
		std::string sText = "#break";
		if (_pCondition != NULL) sText += "(" + _pCondition->toString() + ")";
		return sText;
	}
}
