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
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "DtaBNFScript.h"
#include "BNFClause.h"
#include "DtaVisitor.h"
#include "BNFNot.h"

namespace CodeWorker {
	BNFNot::BNFNot(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : _pBNFScript(pBNFScript), GrfBlock(pParent), _bContinue(bContinue) {}

	BNFNot::~BNFNot() {
	}

	void BNFNot::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFNot(*this, env);
	}

	bool BNFNot::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFNot::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		SEQUENCE_INTERRUPTION_LIST result = GrfBlock::executeInternal(visibility);
		if (_pBNFScript->getParentClauseMatching() != NULL) {
			// if the coverage recording in ON, purge it from here
			_pBNFScript->getParentClauseMatching()->purgeChildsAfterPosition(iLocation);
		}
		if (result == NO_INTERRUPTION) {
			if (_bContinue) CGRuntime::throwBNFExecutionError(toString());
			result = BREAK_INTERRUPTION;
		} else if (result != CONTINUE_INTERRUPTION) {
			result = NO_INTERRUPTION;
		}
		CGRuntime::setInputLocation(iLocation);
		if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
		return result;
	}

	void BNFNot::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			CW_BODY_INDENT << "// " << toString();
			CW_BODY_ENDL;
			int iCursor = theCompilerEnvironment.newCursor();
			CW_BODY_INDENT << "int _compilerClauseCursor_" << iCursor << " = CGRuntime::getInputLocation();";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "int _compilerClauseImplicitCopy_" << iCursor << " = theEnvironment.skipEmptyChars();";
			CW_BODY_ENDL;
			GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
			if (_pBNFScript->hasCoverageRecording()) {
				// coverage recording required
				CW_BODY_INDENT << "_compilerClauseMatching.purgeChildsAfterPosition(_compilerClauseCursor_" << iCursor << ");";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "if (_compilerClauseSuccess) ";
			if (_bContinue) {
				CW_BODY_STREAM << "CGRuntime::throwBNFExecutionError(";
				CW_BODY_STREAM.writeString(toString());
				CW_BODY_STREAM << ");";
			} else {
				CW_BODY_STREAM << "_compilerClauseSuccess = false;";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "else _compilerClauseSuccess = true;";
			}
			CW_BODY_ENDL;
			CW_BODY_INDENT << "if (_compilerClauseImplicitCopy_" << iCursor << " >= 0) CGRuntime::resizeOutputStream(_compilerClauseImplicitCopy_" << iCursor << ");";CW_BODY_ENDL;
			CW_BODY_INDENT << "CGRuntime::setInputLocation(_compilerClauseCursor_" << iCursor << ");";CW_BODY_ENDL;
		}
	}

	std::string BNFNot::toString() const {
		std::string sText = "!" + getCommands()[0]->toString();
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
