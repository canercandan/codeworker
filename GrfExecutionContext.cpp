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
#include "DtaScript.h"
#include "DtaScriptVariable.h"
#include "GrfFunction.h"
#include "BNFClauseCall.h"
#include "GrfExecutionContext.h"

namespace CodeWorker {
	GrfExecutionContext::GrfExecutionContext(GrfBlock* pParent) : GrfBlock(pParent), _pLastCommand(NULL), _pLastExecutionContext(NULL) {}

	GrfExecutionContext::~GrfExecutionContext() {}

	void GrfExecutionContext::clearRecursively(GrfCommand* pCommand) {
		pCommand->clearCounter();
		pCommand->clearTimer();
	}

	void GrfExecutionContext::handleBeforeExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/) {
		_pLastCommand = pCommand;
	}

	void GrfExecutionContext::handleAfterExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/) {
		_pLastCommand = NULL;
	}

	void GrfExecutionContext::handleBeforeScriptExecutionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/) {
		applyRecursively(clearRecursively);
	}

	void GrfExecutionContext::handleAfterScriptExecutionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/) {}

	void GrfExecutionContext::handleStartingFunction(GrfFunction* pFunction) {
		if (pFunction->_sParsingFilePtr != NULL) {
			_stack.push_front(_pLastCommand);
		}
	}

	void GrfExecutionContext::handleEndingFunction(GrfFunction* pFunction) {
		if (pFunction->_sParsingFilePtr != NULL) {
			_stack.pop_front();
		}
	}

	void GrfExecutionContext::handleStartingBNFClause(BNFClauseCall* pClauseCall) {
		if (pClauseCall->_sParsingFilePtr != NULL) {
			_stack.push_front(_pLastCommand);
		}
	}

	void GrfExecutionContext::handleEndingBNFClause(BNFClauseCall* pClauseCall) {
		if (pClauseCall->_sParsingFilePtr != NULL) {
			_stack.pop_front();
		}
	}

	SEQUENCE_INTERRUPTION_LIST GrfExecutionContext::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		bool bRequiresSymbols = DtaScript::requiresParsingInformation();
		DtaScript::requiresParsingInformation(true);
		_pLastExecutionContext = GrfCommand::getCurrentExecutionContext();
		setCurrentExecutionContext(this);
		try {
			applyRecursively(clearRecursively);
			result = openSession(visibility);
		} catch(std::exception&/* exception*/) {
			setCurrentExecutionContext(_pLastExecutionContext);
			DtaScript::requiresParsingInformation(bRequiresSymbols);
			throw/* UtlException(exception)*/;
		}
		setCurrentExecutionContext(_pLastExecutionContext);
		DtaScript::requiresParsingInformation(bRequiresSymbols);
		return result;
	}
}
