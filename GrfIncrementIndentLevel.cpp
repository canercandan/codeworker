/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2010 Cédric Lemaire

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
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"
#include "ExprScriptExpression.h"
#include "GrfIncrementIndentLevel.h"

namespace CodeWorker {
	GrfIncrementIndentLevel::~GrfIncrementIndentLevel() {
		delete _pLevel;
	}

	SEQUENCE_INTERRUPTION_LIST GrfIncrementIndentLevel::executeInternal(DtaScriptVariable& visibility) {
		std::string sLevel = _pLevel->getValue(visibility);
		int iLevel = atoi(sLevel.c_str());
		return CGRuntime::incrementIndentLevel(iLevel);
	}

	void GrfIncrementIndentLevel::populateDefaultParameters() {
		if (_pLevel == NULL) _pLevel = new ExprScriptConstant(1);
	}

	void GrfIncrementIndentLevel::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::incrementIndentLevel(";
		_pLevel->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
