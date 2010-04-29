/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2003 Cédric Lemaire

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
#include "CGRuntime.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "GrfBreak.h"
#include "GrfError.h"
#include "GrfBlock.h"
#include "DtaBNFScript.h"
#include "CppCompilerEnvironment.h"
#include "BNFClause.h" // for the coverage recording
#include "DtaVisitor.h"
#include "BNFTryCatch.h"

namespace CodeWorker {
	BNFTryCatch::BNFTryCatch(DtaBNFScript* pBNFScript) : _pBNFScript(pBNFScript) {
	}

	BNFTryCatch::BNFTryCatch(DtaBNFScript* pBNFScript, GrfBlock* pParent) : _pBNFScript(pBNFScript) {
		setTryBlock(new GrfBlock(pParent));
		setCatchBlock(new GrfBlock(pParent));
		getCatchBlock()->add(new GrfBreak);
	}

	BNFTryCatch::~BNFTryCatch() {}

	void BNFTryCatch::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFTryCatch(*this, env);
	}

	bool BNFTryCatch::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFTryCatch::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		try {
			result = getTryBlock()->execute(visibility);
		} catch(UtlException& exception) {
			result = executeCatchStatement(visibility, iLocation, iImplicitCopyPosition, exception);
		} catch(std::exception& exceptSTL) {
			result = executeCatchStatement(visibility, iLocation, iImplicitCopyPosition, exceptSTL.what());
		} catch(...) {
			result = executeCatchStatement(visibility, iLocation, iImplicitCopyPosition, "ellipsis exception (internal error)");
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST BNFTryCatch::executeCatchStatement(DtaScriptVariable& visibility, int iLocation, int iImplicitCopyPosition, UtlException& exception) {
		return executeCatchStatement(visibility, iLocation, iImplicitCopyPosition, exception.getMessage(), exception.getTraceStack());
	}

	SEQUENCE_INTERRUPTION_LIST BNFTryCatch::executeCatchStatement(DtaScriptVariable& visibility, int iLocation, int iImplicitCopyPosition, const std::string& sCatchedMessage, const std::string& sCatchedStack) {
		SEQUENCE_INTERRUPTION_LIST result;
		if (_pBNFScript->getParentClauseMatching() != NULL) {
			// if the coverage recording in ON, purge it from here
			_pBNFScript->getParentClauseMatching()->purgeChildsAfterPosition(iLocation);
		}
		int iLine = CGRuntime::_pInputStream->getLineCount();
		int iCol = CGRuntime::_pInputStream->getColCount();
		char tcNumber[64];
		sprintf(tcNumber, "line %d, col %d:", iLine, iCol);
		std::string sMessage = tcNumber + CGRuntime::endl() + sCatchedMessage;
		sMessage += CGRuntime::endl() + sCatchedStack;
		DtaScriptVariable* pVariable = visibility.getVariable(*getErrorVariable());
		pVariable->setValue(sMessage.c_str());
		result = getCatchBlock()->execute(visibility);
		CGRuntime::setInputLocation(iLocation);
		if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
		return result;
	}

	void BNFTryCatch::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CPP_COMPILER_BNF_SYMBOL_BEGIN;
		CW_BODY_INDENT << "try ";
		theCompilerEnvironment.bracketsToNextBlock(true);
		theCompilerEnvironment.carriageReturnAfterBlock(false);
		getTryBlock()->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << " catch(UtlException& _compilerException_" << getErrorVariable()->getName() << ") {";
		CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		if (_pBNFScript->hasCoverageRecording()) {
			// coverage recording required
			CW_BODY_INDENT << "_compilerClauseMatching.purgeChildsAfterPosition(" << tcLocation << ");";CW_BODY_ENDL;
		}
		CW_BODY_INDENT;
		getErrorVariable()->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ".setValue(_compilerException_" << getErrorVariable()->getName() << ".getMessage());";
		CW_BODY_ENDL;
		theCompilerEnvironment.bracketsToNextBlock(false);
		CW_BODY_INDENT << "_compilerClauseSuccess = false;";CW_BODY_ENDL;
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
	}

	std::string BNFTryCatch::toString() const {
		std::string sText = "#try ";
		sText += getTryBlock()->toString();
		sText += " #catch(";
		sText += getErrorVariable()->toString();
		sText += ")";
		return sText;
	}


	BNFContinue::BNFContinue(DtaBNFScript* pBNFScript, GrfBlock* pParent, ExprScriptVariable* pVariable, ExprScriptExpression* pMessage) : BNFTryCatch(pBNFScript), _pErrorMessage(pMessage) {
		setTryBlock(new GrfBlock(pParent));
		setCatchBlock(new GrfBlock(pParent));
		setErrorVariable(pVariable);
	}

	BNFContinue::~BNFContinue() {
		delete _pErrorMessage;
	}

	void BNFContinue::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFContinue(*this, env);
	}

	SEQUENCE_INTERRUPTION_LIST BNFContinue::executeCatchStatement(DtaScriptVariable& visibility, int iLocation, int iImplicitCopyPosition, UtlException& exception) {
		if (_pBNFScript->getParentClauseMatching() != NULL) {
			// if the coverage recording in ON, purge it from here
			_pBNFScript->getParentClauseMatching()->purgeChildsAfterPosition(iLocation);
		}
		if (exception.isFinalMessage()) {
			throw;
		}
		std::string sMessage = _pErrorMessage->getValue(visibility);
		throw UtlException(exception.getTraceStack(), sMessage, true);
	}

	std::string BNFContinue::toString() const {
		std::string sText = "#continue(";
		sText += getErrorVariable()->toString();
		sText += ", ";
		sText += _pErrorMessage->toString();
		sText += ")";
		return sText;
	}
}
