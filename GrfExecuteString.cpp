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

#include <string>

#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"
#include "DtaScript.h"
#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "ExprScriptExpression.h"
#include "GrfExecuteString.h"

namespace CodeWorker {
	GrfExecuteString::~GrfExecuteString() {
		delete _pThis;
		delete _pCommand;
	}

	SEQUENCE_INTERRUPTION_LIST GrfExecuteString::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pThis = visibility.getVariable(*_pThis);
		std::string sCommand = _pCommand->getValue(visibility);
		ScpStream theCommand(sCommand);
		DtaScript script(getParent());
		script.parseStream(theCommand);
		return script.execute(*pThis);
	}

	void GrfExecuteString::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::executeString(";
		_pThis->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pCommand->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
