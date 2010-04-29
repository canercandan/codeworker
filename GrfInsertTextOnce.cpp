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
#include "GrfInsertText.h"
//##protect##"INCLUDE FILES"

#include "GrfInsertTextOnce.h"

namespace CodeWorker {
	GrfInsertTextOnce::~GrfInsertTextOnce() {
		delete _pLocation;
		delete _pText;
	}

	SEQUENCE_INTERRUPTION_LIST GrfInsertTextOnce::executeInternal(DtaScriptVariable& visibility) {
		std::string sLocation = _pLocation->getValue(visibility);
		int iLocation = atoi(sLocation.c_str());
		std::string sText = _pText->getValue(visibility);
//##protect##"execute"
		if ((*_pOutputCoverage != NULL) && CGRuntime::getOutputStream()->insertTextOnce(sText, iLocation)) {
			int iCoverageLocation = iLocation + (*_pOutputCoverage)->getIntValue();
			DtaScriptVariable* pCoverage = GrfInsertText::getCoverageItem(*_pOutputCoverage, sText.size(), iCoverageLocation, "I");
			pCoverage->insertNode("script")->setValue(_iFileLocation);
			pCoverage->insertNode("output")->setValue(iCoverageLocation);
			return NO_INTERRUPTION;
		}
//##protect##"execute"
		return CGRuntime::insertTextOnce(iLocation, sText);
	}

//##protect##"implementation"
	void GrfInsertTextOnce::prepareCoverage(DtaScriptVariable* const* pOutputCoverage) {
		_pOutputCoverage = pOutputCoverage;
	}
//##protect##"implementation"

	void GrfInsertTextOnce::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::insertTextOnce(";
		_pLocation->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pText->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
