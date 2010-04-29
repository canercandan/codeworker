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
#include "GrfExit.h"

#include "ExprScriptVariable.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"

namespace CodeWorker {
	GrfExit::~GrfExit() {
		delete _pCodeExpression;
	}

	SEQUENCE_INTERRUPTION_LIST GrfExit::executeInternal(DtaScriptVariable& visibility) {
		int iValue = _pCodeExpression->getIntValue(visibility);
		throw UtlExitException(iValue);
	}

	void GrfExit::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "throw UtlExitException(";
		_pCodeExpression->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ");";CW_BODY_ENDL;
	}
}
