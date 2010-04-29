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
#include "UtlTrace.h"
#include "ScpStream.h"
#include "UtlTimer.h"

#include "DtaScriptVariable.h"
#include "GrfExecutionContext.h"
#include "CppCompilerEnvironment.h"
#include "GrfCommand.h"

namespace CodeWorker {
	GrfExecutionContext* GrfCommand::_pExecutionContext = NULL;

	GrfCommand::GrfCommand(GrfBlock* pParent) : _pParent(pParent), _sParsingFilePtr(NULL), _iFileLocation(-1), _iCounter(0), _pTimer(NULL) {}

	GrfCommand::~GrfCommand() {}

	void GrfCommand::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		throw UtlException("internal error: GrfCommand::accept() MUST be overloaded in sub classes!");
	}

	const char* GrfCommand::getFunctionName() const { return NULL; }
	bool GrfCommand::isAPredefinedFunction() const { return (getFunctionName() != NULL); }
	bool GrfCommand::isABNFCommand() const { return false; }

	void GrfCommand::applyRecursively(APPLY_ON_COMMAND_FUNCTION apply) {
		apply(this);
	}

	void GrfCommand::setParsingInformation(const char* sName, ScpStream& stream) {
		if (sName != NULL) {
			_sParsingFilePtr = sName;
			_iFileLocation = stream.getInputLocation();
		}
	}

	void GrfCommand::callBeforeExecutionCBK(DtaScriptVariable& visibility) {
		callRecursiveBeforeExecutionCBK(getCurrentExecutionContext(), visibility);
	}

	void GrfCommand::callRecursiveBeforeExecutionCBK(GrfExecutionContext* pContext, DtaScriptVariable& visibility) {
		if (pContext != NULL) {
			callRecursiveBeforeExecutionCBK(pContext->getLastExecutionContext(), visibility);
			pContext->handleBeforeExecutionCBK(this, visibility);
		}
	}

	void GrfCommand::callAfterExecutionCBK(DtaScriptVariable& visibility) {
		GrfExecutionContext* pContext = getCurrentExecutionContext();
		while (pContext != NULL) {
			pContext->handleAfterExecutionCBK(this, visibility);
			pContext = pContext->getLastExecutionContext();
		}
	}

	void GrfCommand::callAfterExceptionCBK(DtaScriptVariable& visibility, UtlException& exception) {
		GrfExecutionContext* pContext = getCurrentExecutionContext();
		while (pContext != NULL) {
			pContext->handleAfterExceptionCBK(this, visibility, exception);
			pContext = pContext->getLastExecutionContext();
		}
	}

	SEQUENCE_INTERRUPTION_LIST GrfCommand::execute(DtaScriptVariable& visibility) {
		UTLTRACE_STACK_INSTRUCTION(_sParsingFilePtr, _iFileLocation);
		SEQUENCE_INTERRUPTION_LIST result;
		if (_pExecutionContext != NULL) {
			callBeforeExecutionCBK(visibility);
			try {
				result = executeInternal(visibility);
			} catch(UtlException& exception) {
				callAfterExceptionCBK(visibility, exception);
				throw/* UtlException(exception)*/;
			}
			callAfterExecutionCBK(visibility);
		} else {
			result = executeInternal(visibility);
		}
		return result;
	}

	void GrfCommand::clearTimer() {
		if (_pTimer != NULL) _pTimer->clear();
	}

	void GrfCommand::startTimer() {
		if (_pTimer == NULL) _pTimer = new UtlTimer;
		_pTimer->start();
	}

	void GrfCommand::stopTimer() {
		if (_pTimer != NULL) _pTimer->stop();
	}

	long GrfCommand::getTimeInMillis() const {
		if ((_iCounter == 0) || (_pTimer == NULL)) return 0L; // little bug when the timer has never started
		return _pTimer->getTimeInMillis();
	}

	std::string GrfCommand::toString() const {
		return "<unknown command>";
	}

	void GrfCommand::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "<unhandled command '" << getFunctionName() << "'>";
		CW_BODY_ENDL;
	}
}
