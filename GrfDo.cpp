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

#include "DtaScriptVariable.h"
#include "ExprScriptExpression.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "GrfDo.h"

namespace CodeWorker {
	GrfDo::~GrfDo() {
		delete _pCondition;
	}

	SEQUENCE_INTERRUPTION_LIST GrfDo::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		std::string sCondition;
		do {
			result = GrfBlock::executeInternal(visibility);
			switch(result) {
				case CONTINUE_INTERRUPTION:
					result = NO_INTERRUPTION;
				case NO_INTERRUPTION: break;
				case BREAK_INTERRUPTION: return NO_INTERRUPTION;
				default:
					return result;

			}
			sCondition = _pCondition->getValue(visibility);
		} while (!sCondition.empty());
		return result;
	}

	void GrfDo::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "do ";
		theCompilerEnvironment.carriageReturnAfterBlock(false);
		GrfBlock::compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << " while (";
		_pCondition->compileCppBoolean(theCompilerEnvironment, false);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
