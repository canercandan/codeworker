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

#include "ExprScriptVariable.h"
#include "CGExternalHandling.h"
#include "CppCompilerEnvironment.h"
#include "GrfStringAsStandardInput.h"

namespace CodeWorker {
	GrfStringAsStandardInput::~GrfStringAsStandardInput() {
		delete _pText;
	}

	SEQUENCE_INTERRUPTION_LIST GrfStringAsStandardInput::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		std::string sInput = _pText->getValue(visibility);
		CGStandardInputOutput theInput(sInput);
		result = GrfBlock::executeInternal(visibility);
		return result;
	}

	void GrfStringAsStandardInput::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "{";
		CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "CGStandardInputOutput theInput(";
		_pText->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
		CW_BODY_INDENT;
		theCompilerEnvironment.bracketsToNextBlock(true);
		GrfBlock::compileCpp(theCompilerEnvironment);
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
	}
}
