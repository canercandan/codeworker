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
#include "BNFForeach.h"

namespace CodeWorker {
	BNFForeach::BNFForeach(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : _pBNFScript(pBNFScript), GrfForeach(pParent), _bContinue(bContinue) {}

	BNFForeach::~BNFForeach() {
	}

	void BNFForeach::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFForeach(*this, env);
	}

	bool BNFForeach::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFForeach::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = GrfForeach::executeInternal(visibility);
		if (result != NO_INTERRUPTION) {
			if (_bContinue) CGRuntime::throwBNFExecutionError(toString());
			result = BREAK_INTERRUPTION;
		}
		return result;
	}

	void BNFForeach::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "// " << toString();
		CW_BODY_ENDL;
		// TO DO!
	}

	std::string BNFForeach::toString() const {
		std::string sText = "#foreach()";
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
