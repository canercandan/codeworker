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
#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "GrfInvertArray.h"

namespace CodeWorker {
	GrfInvertArray::~GrfInvertArray() {
		delete _pArray;
	}

	SEQUENCE_INTERRUPTION_LIST GrfInvertArray::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pArray = visibility.getExistingVariable(*_pArray);
		return CGRuntime::invertArray(pArray);
	}

	void GrfInvertArray::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::invertArray(";
		_pArray->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
