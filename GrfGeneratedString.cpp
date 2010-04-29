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

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "CppCompilerEnvironment.h"
#include "DtaPatternScript.h"
#include "GrfGeneratedString.h"

namespace CodeWorker {
	GrfGeneratedString::~GrfGeneratedString() {
		delete _pOutputString;
	}

	SEQUENCE_INTERRUPTION_LIST GrfGeneratedString::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		DtaOutputFile generatedString(_pPatternScript);
		DtaScriptVariable* pVariable = visibility.getExistingVariable(*_pOutputString);
		ScpStream* pOldOutputStream;
		generatedString.openGenerate(true, NULL, pOldOutputStream);
		try {
			result = GrfBlock::executeInternal(visibility);
		} catch(UtlException& exception) {
			generatedString.catchGenerateExecution(true, pOldOutputStream, &exception);
		} catch(std::exception&) {
			generatedString.catchGenerateExecution(true, pOldOutputStream, 0);
			throw;
		}
		std::string sResult = generatedString.closeGenerate(true, NULL, pOldOutputStream);
		if (pVariable != NULL) pVariable->setValue(sResult.c_str());
		return result;
	}

	void GrfGeneratedString::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "{";
		CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "CGRuntimeOutputString outputString;";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "try ";
		theCompilerEnvironment.bracketsToNextBlock(true);
		theCompilerEnvironment.carriageReturnAfterBlock(false);
		GrfBlock::compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << " catch(UtlException& exception) {";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "\toutputString.onCatchedException(exception);";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
		CW_BODY_INDENT;
		_pOutputString->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ".setValue(outputString.getResult());";
		CW_BODY_ENDL;
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
	}
}
