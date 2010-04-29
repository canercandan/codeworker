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

#include "DtaScriptVariable.h"
#include "ExprScriptExpression.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "GrfFunction.h"
#include "GrfReturn.h"

namespace CodeWorker {
	GrfReturn::~GrfReturn() {
		delete _pExpression;
	}

	SEQUENCE_INTERRUPTION_LIST GrfReturn::executeInternal(DtaScriptVariable& visibility) {
		if (_pExpression != NULL) {
			std::string sValue = _pExpression->getValue(visibility);
			visibility.setValueAtVariable(_sFunctionName.c_str(), sValue.c_str(), true, true);
		}
		return RETURN_INTERRUPTION;
	}

	void GrfReturn::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (_pExpression != NULL) {
			CW_BODY_INDENT << _sFunctionName << ".setValue(";
			_pExpression->compileCpp(theCompilerEnvironment);
			CW_BODY_STREAM << ");";
			CW_BODY_ENDL;
		}
		if (theCompilerEnvironment.getCurrentFunction()->getFinally() != NULL) {
			theCompilerEnvironment.bracketsToNextBlock(false);
			theCompilerEnvironment.getCurrentFunction()->getFinally()->compileCpp(theCompilerEnvironment);
		}
		CW_BODY_INDENT << "return " << _sFunctionName << ".getStringValue();";
		CW_BODY_ENDL;
	}
}
