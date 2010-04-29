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

#include "ExprScriptExpression.h"
#include "DtaScriptVariable.h"
#include "GrfBlock.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "GrfIfThenElse.h"

namespace CodeWorker {
	GrfIfThenElse::~GrfIfThenElse() {
		delete _pCondition;
		delete _pThenBlock;
		delete _pElseBlock;
	}

	void GrfIfThenElse::applyRecursively(APPLY_ON_COMMAND_FUNCTION apply) {
		GrfCommand::applyRecursively(apply);
		if (_pThenBlock != NULL) _pThenBlock->applyRecursively(apply);
		if (_pElseBlock != NULL) _pElseBlock->applyRecursively(apply);
	}


	SEQUENCE_INTERRUPTION_LIST GrfIfThenElse::executeInternal(DtaScriptVariable& visibility) {
		std::string sCondition = _pCondition->getValue(visibility);
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		if (!sCondition.empty()) result = _pThenBlock->execute(visibility);
		else if (_pElseBlock != NULL) result = _pElseBlock->execute(visibility);
		return result;
	}

	void GrfIfThenElse::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "if (";
		_pCondition->compileCppBoolean(theCompilerEnvironment, false);
		CW_BODY_STREAM << ") ";
		theCompilerEnvironment.carriageReturnAfterBlock(_pElseBlock == NULL);
		_pThenBlock->compileCpp(theCompilerEnvironment);
		if (_pElseBlock != NULL) {
			CW_BODY_STREAM << " else ";
			_pElseBlock->compileCpp(theCompilerEnvironment);
		}
	}
}
