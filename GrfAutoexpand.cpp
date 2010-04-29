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

#include "ScpStream.h"

#include "ExprScriptVariable.h"
#include "DtaScriptVariable.h"
#include "CGRuntime.h"
#include "CppCompilerEnvironment.h"
#include "GrfAutoexpand.h"

namespace CodeWorker {
	GrfAutoexpand::~GrfAutoexpand() {
		delete _pClass;
		delete _pFileName;
	}

	SEQUENCE_INTERRUPTION_LIST GrfAutoexpand::executeInternal(DtaScriptVariable& visibility) {
		std::string sOutputFile = _pFileName->getValue(visibility);
		DtaScriptVariable* pClass = visibility.getExistingVariable(*_pClass);
		if (pClass == NULL) {
			throw UtlException("runtime error: variable '" + _pClass->toString() + "' doesn't exist while calling procedure 'autoexpand()'");
		}
		CGRuntime::autoexpand(sOutputFile, CppParsingTree_var(pClass));
		return NO_INTERRUPTION;
	}

	void GrfAutoexpand::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "// warning: the behaviour of 'autoexpand' may vary of the interpreted mode";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "CGRuntime::" << getFunctionName() << "(";
		_pFileName->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pClass->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
