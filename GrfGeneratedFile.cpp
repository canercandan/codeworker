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
#include "GrfGeneratedFile.h"

namespace CodeWorker {
	GrfGeneratedFile::~GrfGeneratedFile() {
		delete _pOutputFile;
	}

	SEQUENCE_INTERRUPTION_LIST GrfGeneratedFile::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		DtaOutputFile generatedFile(_pPatternScript);
		std::string sFile = _pOutputFile->getValue(visibility);
		ScpStream* pOldOutputStream;
		generatedFile.openGenerate(true, sFile.c_str(), pOldOutputStream);
		try {
			result = GrfBlock::executeInternal(visibility);
		} catch(UtlException& exception) {
			generatedFile.catchGenerateExecution(true, pOldOutputStream, &exception);
		} catch(std::exception&) {
			generatedFile.catchGenerateExecution(true, pOldOutputStream, 0);
			throw;
		}
		generatedFile.closeGenerate(true, sFile.c_str(), pOldOutputStream);
		return result;
	}

	void GrfGeneratedFile::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "{";
		CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "CGRuntimeOutputFile outputFile(";
		_pOutputFile->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "try ";
		theCompilerEnvironment.bracketsToNextBlock(true);
		theCompilerEnvironment.carriageReturnAfterBlock(false);
		GrfBlock::compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << " catch(UtlException& exception) {";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "\toutputFile.onCatchedException(exception);";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "\toutputFile.closeGenerate();";
		CW_BODY_ENDL;
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
	}
}
