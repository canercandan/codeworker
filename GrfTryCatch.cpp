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
#include "ExprScriptVariable.h"
#include "DtaScriptVariable.h"
#include "GrfBlock.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"
#include "GrfTryCatch.h"

namespace CodeWorker {
	GrfTryCatch::~GrfTryCatch() {
		delete _pVariable;
		delete _pTryBlock;
		delete _pCatchBlock;
	}

	void GrfTryCatch::applyRecursively(APPLY_ON_COMMAND_FUNCTION apply) {
		GrfCommand::applyRecursively(apply);
		if (_pTryBlock != NULL) _pTryBlock->applyRecursively(apply);
		if (_pCatchBlock != NULL) _pCatchBlock->applyRecursively(apply);
	}


	SEQUENCE_INTERRUPTION_LIST GrfTryCatch::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		try {
			result = _pTryBlock->execute(visibility);
		} catch(UtlExitException&) {
			throw;
		} catch(UtlException& exception) {
			std::string sError = exception.getMessage() + CGRuntime::endl() + exception.getTraceStack();
			result = handleCatch(visibility, sError);
		} catch(std::exception& except) {
			std::string sError(except.what());
			if (sError.empty() || (sError[sError.size() - 1] != '\n')) sError += CGRuntime::endl();
			result = handleCatch(visibility, sError);
		} catch(...) {
			result = handleCatch(visibility, "CodeWorker warning: Ellipsis exception");
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST  GrfTryCatch::handleCatch(DtaScriptVariable& visibility, const std::string& sError) {
		DtaScriptVariable stackCatch(&visibility, "##stack## catch");
		DtaScriptVariable* pVariable = stackCatch.getNonRecursiveVariable(stackCatch, *_pVariable);
		pVariable->setValue(sError.c_str());
		return _pCatchBlock->execute(stackCatch);
	}

	void GrfTryCatch::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "try ";
		theCompilerEnvironment.carriageReturnAfterBlock(false);
		_pTryBlock->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << " catch(UtlException& _compilerException_" << _pVariable->getName() << ") {";
		CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "CppParsingTree_value " << _pVariable->getName() << "(_compilerException_" << _pVariable->getName() << ".getMessage());";
		CW_BODY_ENDL;
		theCompilerEnvironment.bracketsToNextBlock(false);
		_pCatchBlock->compileCpp(theCompilerEnvironment);
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
	}
}
