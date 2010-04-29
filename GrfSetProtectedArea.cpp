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
#include <string>

//##protect##"INCLUDE FILES"
#include "DtaScriptVariable.h"
//##protect##"INCLUDE FILES"

#include "GrfSetProtectedArea.h"

namespace CodeWorker {
	GrfSetProtectedArea::~GrfSetProtectedArea() {
		delete _pProtectedAreaName;
	}

	SEQUENCE_INTERRUPTION_LIST GrfSetProtectedArea::executeInternal(DtaScriptVariable& visibility) {
		std::string sProtectedAreaName = _pProtectedAreaName->getValue(visibility);
//##protect##"execute"
		if (*_pOutputCoverage != NULL) {
			DtaScriptVariable* pCoverage = (*_pOutputCoverage)->pushItem("P");
			pCoverage->insertNode("script")->setValue(_iFileLocation);
			pCoverage->insertNode("output")->setValue(CGRuntime::getOutputLocation() + (*_pOutputCoverage)->getIntValue());
		}
//##protect##"execute"
		return CGRuntime::setProtectedArea(sProtectedAreaName);
	}

//##protect##"implementation"
	void GrfSetProtectedArea::prepareCoverage(DtaScriptVariable* const* pOutputCoverage) {
		_pOutputCoverage = pOutputCoverage;
	}
//##protect##"implementation"

	void GrfSetProtectedArea::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::setProtectedArea(";
		_pProtectedAreaName->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
