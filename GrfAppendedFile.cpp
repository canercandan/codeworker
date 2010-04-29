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
#include "GrfAppendedFile.h"

namespace CodeWorker {
	GrfAppendedFile::~GrfAppendedFile() {
		delete _pOutputFile;
	}

	SEQUENCE_INTERRUPTION_LIST GrfAppendedFile::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		DtaOutputFile AppendedFile(_pPatternScript);
		std::string sFile = _pOutputFile->getValue(visibility);
		ScpStream* pOldOutputStream;
		AppendedFile.openAppend(true, sFile.c_str(), pOldOutputStream);
		try {
			result = GrfBlock::executeInternal(visibility);
		} catch(UtlException& exception) {
			AppendedFile.catchGenerateExecution(true, pOldOutputStream, &exception);
		} catch(std::exception&) {
			AppendedFile.catchGenerateExecution(true, pOldOutputStream, 0);
			throw;
		}
		AppendedFile.closeGenerate(true, sFile.c_str(), pOldOutputStream);
		return result;
	}

	void GrfAppendedFile::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "{";
		CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "CGRuntimeOutputFile outputFile(";
		_pOutputFile->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", true);";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "try ";
		theCompilerEnvironment.bracketsToNextBlock(true);
		theCompilerEnvironment.carriageReturnAfterBlock(false);
		GrfBlock::compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << " catch(UtlException& exception) {";CW_BODY_ENDL;
		CW_BODY_INDENT << "\toutputFile.onCatchedException(exception);";CW_BODY_ENDL;
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
		CW_BODY_INDENT << "\toutputFile.closeGenerate();";CW_BODY_ENDL;
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
	}
}
