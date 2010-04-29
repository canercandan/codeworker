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
#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "GrfMerge.h"

namespace CodeWorker {
	GrfMerge::~GrfMerge() {
		if (_pVariable != NULL) delete _pVariable;
		if (_pSource != NULL) delete _pSource;
	}

	SEQUENCE_INTERRUPTION_LIST GrfMerge::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pVariable = visibility.getVariable(*_pVariable);
		DtaScriptVariable* pSource = visibility.getExistingVariable(*_pSource);
		if (pSource == NULL) {
			std::string sVariableName = pVariable->getCompleteName();
			throw UtlException("runtime error: variable '" + _pSource->toString() + "' doesn't exist while merging all of it ('merge' command) to variable '" + sVariableName + "'");
		}
		pVariable->copyAll(*pSource, true);
		return NO_INTERRUPTION;
	}

	void GrfMerge::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT;
		_pVariable->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ".merge(";
		_pSource->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
