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
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "DtaBNFScript.h"
#include "BNFClause.h"
#include "DtaVisitor.h"
#include "BNFScanWindow.h"

namespace CodeWorker {
	BNFScanWindow::BNFScanWindow(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : _pBNFScript(pBNFScript), GrfBlock(pParent), _bContinue(bContinue), _pWindowSequence(NULL) {}

	BNFScanWindow::~BNFScanWindow() {
		delete _pWindowSequence;
	}

	void BNFScanWindow::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFScanWindow(*this, env);
	}

	bool BNFScanWindow::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFScanWindow::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		SEQUENCE_INTERRUPTION_LIST result = _pWindowSequence->execute(visibility);
		if (result != CONTINUE_INTERRUPTION) {
			int iFinalLocation = CGRuntime::getInputLocation();
			CGRuntime::setInputLocation(iLocation);
			if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
			if (iFinalLocation < iLocation) result = BREAK_INTERRUPTION;
			if (result == NO_INTERRUPTION) {
				ScpStream::SizeAttributes sizeAttrs(CGRuntime::_pInputStream->resize(iFinalLocation));
				result = GrfBlock::executeInternal(visibility);
				CGRuntime::_pInputStream->restoreSize(sizeAttrs);
				if ((result == NO_INTERRUPTION) || (result == CONTINUE_INTERRUPTION)) {
					CGRuntime::setInputLocation(iFinalLocation);
				} else {
					BNF_SYMBOL_HAS_FAILED
				}
			} else {
				if (iFinalLocation < iLocation) throw UtlException("Left member of '" + CGRuntime::composeCLikeString(toString()) + "' shouldn't set the cursor back in the sentence");
				if (_bContinue) CGRuntime::throwBNFExecutionError(toString());
			}
		}
		return result;
	}

	void BNFScanWindow::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CPP_COMPILER_BNF_SYMBOL_BEGIN;
		_pWindowSequence->compileCpp(theCompilerEnvironment);
		CW_BODY_INDENT << "int _compilerClauseFinalLocation" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
		CW_BODY_INDENT << "CGRuntime::setInputLocation(" << tcLocation << ");";CW_BODY_ENDL;
		CW_BODY_INDENT << "if (" << tcImplicitCopy << " >= 0) CGRuntime::resizeOutputStream(" << tcImplicitCopy << ");";CW_BODY_ENDL;
		CW_BODY_INDENT << "if (_compilerClauseFinalLocation" << iCursor << " < " << tcLocation << ") _compilerClauseSuccess = false;";CW_BODY_ENDL;
		CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "{";CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "CGBNFRuntimeResizeInput _compilerClauseSizeAttrs(_compilerClauseFinalLocation" << iCursor << ");";CW_BODY_ENDL;
		GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
		CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
		CW_BODY_INDENT << "\tCGRuntime::setInputLocation(_compilerClauseFinalLocation" << iCursor << ");";CW_BODY_ENDL;
		CW_BODY_INDENT << "} else {";CW_BODY_ENDL;
		CPP_COMPILER_BNF_SYMBOL_HAS_FAILED;
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "} else {";CW_BODY_ENDL;
		CW_BODY_INDENT << "\tif (_compilerClauseFinalLocation" << iCursor << " < " << tcLocation << ") throw UtlException(\"Left member of '" << CGRuntime::composeCLikeString(toString()) << "' shouldn't set the cursor back in the sentence\");";CW_BODY_ENDL;
		if (_bContinue) {
			CW_BODY_INDENT << "\tCGRuntime::throwBNFExecutionError(";
			CW_BODY_STREAM.writeString(toString());
			CW_BODY_STREAM << ");";
			CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
	}

	std::string BNFScanWindow::toString() const {
		std::string sText = "[" + _pWindowSequence->toString() + "] |> [" + getCommands()[0]->toString() + "]";
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
