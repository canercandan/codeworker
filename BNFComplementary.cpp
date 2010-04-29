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
#include "BNFComplementary.h"

namespace CodeWorker {
	BNFComplementary::BNFComplementary(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : BNFStepper(pBNFScript, pParent, bContinue) {}

	BNFComplementary::~BNFComplementary() {}

	void BNFComplementary::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFComplementary(*this, env);
	}

	SEQUENCE_INTERRUPTION_LIST BNFComplementary::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		int iNotEmptyLocation = CGRuntime::getInputLocation();
		_iStepLocation = 0;
		SEQUENCE_INTERRUPTION_LIST result = GrfBlock::executeInternal(visibility);
		if (_pBNFScript->getParentClauseMatching() != NULL) {
			// if the coverage recording in ON, purge it from here
			_pBNFScript->getParentClauseMatching()->purgeChildsAfterPosition(iLocation);
		}
		if (result == NO_INTERRUPTION) {
			BNF_SYMBOL_HAS_FAILED
		}
		if (result != CONTINUE_INTERRUPTION) {
	//		CGRuntime::setInputLocation(iNotEmptyLocation);
			std::string sText;
			if (_iStepLocation > iNotEmptyLocation) {
				CGRuntime::setInputLocation(_iStepLocation);
				sText = CGRuntime::getLastReadChars(_iStepLocation - iNotEmptyLocation);
			} else {
				int iChar = CGRuntime::readCharAsInt();
				if (iChar < 0) {
					BNF_SYMBOL_HAS_FAILED
				}
				sText = std::string(1, (char) iChar);
			}
			result = NO_INTERRUPTION;
			if (!_listOfConstants.empty()) {
				bool bMatch = false;
				for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
					bMatch = ((*i) == sText);
					if (bMatch) break;
				}
				if (!bMatch) {
					BNF_SYMBOL_HAS_FAILED
				}
			}
			if (_pVariableToAssign != NULL) {
				DtaScriptVariable* pVariable = visibility.getVariable(*_pVariableToAssign);
				if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
					pVariable->pushItem(sText);
				} else {
					if (_bConcatVariable) pVariable->concatValue(sText.c_str());
					else pVariable->setValue(sText.c_str());
				}
			}
			if (iImplicitCopyPosition >= 0) _pBNFScript->writeBinaryData(sText.c_str(), sText.size());
		}
		return result;
	}

	void BNFComplementary::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			CW_BODY_INDENT << "// " << toString();
			CW_BODY_ENDL;
			int iCursor = theCompilerEnvironment.newCursor();
			CW_BODY_INDENT << "int _compilerClauseLocation_" << iCursor << " = CGRuntime::getInputLocation();";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "int _compilerClauseImplicitCopy_" << iCursor << " = theEnvironment.skipEmptyChars();";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "int _compilerClauseNotEmptyLocation_" << iCursor << " = CGRuntime::getInputLocation();";
			CW_BODY_ENDL;
			int iOldBNFStepperCursor = theCompilerEnvironment.getBNFStepperCursor();
			theCompilerEnvironment.setBNFStepperCursor(iCursor);
			CW_BODY_INDENT << "int _compilerClauseNextLocation_" << iCursor << " = 0;";CW_BODY_ENDL;
			GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
			if (_pBNFScript->hasCoverageRecording()) {
				// coverage recording required
				CW_BODY_INDENT << "_compilerClauseMatching.purgeChildsAfterPosition(_compilerClauseLocation_" << iCursor << ");";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "_compilerClauseSuccess = !_compilerClauseSuccess;";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tCGRuntime::setInputLocation(_compilerClauseNotEmptyLocation_" << iCursor << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tint iChar = CGRuntime::readCharAsInt();";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\t_compilerClauseSuccess = (iChar >= 0);";
			CW_BODY_ENDL;
			if (!_listOfConstants.empty()) {
				CW_BODY_INDENT << "\tif (_compilerClauseSuccess) switch(iChar) {";
				CW_BODY_ENDL;
				for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
					CW_BODY_INDENT << "\t\tcase " << (int) ((unsigned char) ((*i)[0])) << ": // '" << CGRuntime::composeCLikeString(*i) << "'";
					CW_BODY_ENDL;
				}
				CW_BODY_INDENT << "\t\t\tbreak;";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "\t\tdefault: _compilerClauseSuccess = false;";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "\t}";
				CW_BODY_ENDL;
			}
			theCompilerEnvironment.incrementIndentation();
			if (_pVariableToAssign != NULL) {
				CW_BODY_INDENT << "if (_compilerClauseSuccess) {";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "\tchar tcText[] = {'\\0', '\\0'};";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "\ttcText[0] = (char) iChar;";
				CW_BODY_ENDL;
				_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, "tcText");
				CW_BODY_INDENT << "}";CW_BODY_ENDL;
			}

			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\tif (_compilerClauseImplicitCopy_" << iCursor << " >= 0) {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\tunsigned char cChar = (unsigned char) iChar;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\tCGRuntime::writeBinaryData(cChar);";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t}";CW_BODY_ENDL;
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
			CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
			if (_bContinue) {
				CW_BODY_INDENT << "\tRuntime::throwBNFExecutionError(";
				CW_BODY_STREAM.writeString(toString());
				CW_BODY_STREAM << ");";
			} else {
				CW_BODY_INDENT << "\tCGRuntime::setInputLocation(_compilerClauseLocation_" << iCursor << ");";CW_BODY_ENDL;
				CW_BODY_INDENT << "\tif (_compilerClauseImplicitCopy_" << iCursor << " >= 0) CGRuntime::resizeOutputStream(_compilerClauseImplicitCopy_" << iCursor << ");";
			}
			CW_BODY_ENDL;
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
			theCompilerEnvironment.setBNFStepperCursor(iOldBNFStepperCursor);
		}
	}

	std::string BNFComplementary::toString() const {
		std::string sText = "~" + getCommands()[0]->toString() + DtaBNFScript::constantsToString(_listOfConstants) + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
