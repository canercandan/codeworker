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
#include <list>
#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "GrfCutString.h"

namespace CodeWorker {
	GrfCutString::~GrfCutString() {
		delete _pText;
		delete _pSeparator;
		delete _pList;
	}

	SEQUENCE_INTERRUPTION_LIST GrfCutString::executeInternal(DtaScriptVariable& visibility) {
		std::string sText = _pText->getValue(visibility);
		std::string sSeparator = _pSeparator->getValue(visibility);
		DtaScriptVariable* pList = visibility.getExistingVariable(*_pList);
		if (pList == NULL) throw UtlException("parameter 'list' of procedure 'cutString' takes unexisting variable '" + _pList->toString() + "' as argument");
		std::list<std::string> slList;
		SEQUENCE_INTERRUPTION_LIST result = CGRuntime::cutString(sText, sSeparator, slList);
		pList->setValue(slList);
		return result;
	}

	void GrfCutString::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::cutString(";
		_pText->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pSeparator->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pList->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
