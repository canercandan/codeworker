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
#include "BNFReadChar.h"

namespace CodeWorker {
	BNFReadChar::BNFReadChar(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : _pBNFScript(pBNFScript), GrfCommand(pParent), _pVariableToAssign(NULL), _bContinue(bContinue), _iClauseReturnType(BNFClause::NO_RETURN_TYPE) {}

	BNFReadChar::~BNFReadChar() {
		delete _pVariableToAssign;
	}

	void BNFReadChar::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFReadChar(*this, env);
	}

	bool BNFReadChar::isABNFCommand() const { return true; }

	void BNFReadChar::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatVariable = bConcat;
		}
	}

	void BNFReadChar::setConstantsToMatch(const std::vector<std::string>& listOfConstants) {
		_listOfConstants = listOfConstants;
	}

	SEQUENCE_INTERRUPTION_LIST BNFReadChar::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		int iChar = CGRuntime::readCharAsInt();
		if (iChar < 0) {
			BNF_SYMBOL_HAS_FAILED
		}
		if (!_listOfConstants.empty()) {
			bool bMatch = false;
			for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
				bMatch = ((int) ((*i)[0]) == iChar);
				if (bMatch) break;
			}
			if (!bMatch) {
				BNF_SYMBOL_HAS_FAILED
			}
		}
		if (_pVariableToAssign != NULL) {
			DtaScriptVariable* pVariable = visibility.getVariable(*_pVariableToAssign);
			postExecutionAssignment(*pVariable, (unsigned char) iChar);
		}
		if (iImplicitCopyPosition >= 0) {
			register char cChar = (char) iChar;
			_pBNFScript->writeBinaryData((const char*) &cChar, 1);
		}
		return NO_INTERRUPTION;
	}

	void BNFReadChar::postExecutionAssignment(DtaScriptVariable& variable, unsigned char cChar) {
		char tcText[] = {'\0', '\0'};
		tcText[0] = cChar;
		if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
			variable.pushItem(tcText);
		} else {
			if (_bConcatVariable) variable.concatValue(tcText);
			else variable.setValue(tcText);
		}
	}

	void BNFReadChar::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "// " << toString();
		CW_BODY_ENDL;
		int iCursor = theCompilerEnvironment.newCursor();
		CW_BODY_INDENT << "int _compilerClauseLocation_" << iCursor << " = CGRuntime::getInputLocation();";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "int _compilerClauseImplicitCopy_" << iCursor << " = theEnvironment.skipEmptyChars();";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "int _compilerClauseChar_" << iCursor << " = CGRuntime::readCharAsInt();";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "if (_compilerClauseChar_" << iCursor << " < 0) {";
		CW_BODY_ENDL;
		if (_bContinue) {
			CW_BODY_INDENT << "\tCGRuntime::throwBNFExecutionError(";
			CW_BODY_STREAM.writeString(toString());
			CW_BODY_STREAM << ");";
		} else {
			CW_BODY_INDENT << "\t_compilerClauseSuccess = false;";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tCGRuntime::setInputLocation(_compilerClauseLocation_" << iCursor << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tif (_compilerClauseImplicitCopy_" << iCursor << " >= 0) CGRuntime::resizeOutputStream(_compilerClauseImplicitCopy_" << iCursor << ");";
		}
		CW_BODY_ENDL;
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "else {";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "\t_compilerClauseSuccess = true;";
		CW_BODY_ENDL;
		if (!_listOfConstants.empty()) {
			CW_BODY_INDENT << "\tswitch(_compilerClauseChar_" << iCursor << ") {";
			CW_BODY_ENDL;
			for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
				CW_BODY_INDENT << "\t\tcase " << (int) ((unsigned char) ((*i)[0])) << ": // '" << CGRuntime::composeCLikeString(*i) << "'";
				CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "\t\t\tbreak;";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\tdefault: ";
			if (_bContinue) {
				CW_BODY_STREAM << "CGRuntime::throwBNFExecutionError(";
				CW_BODY_STREAM.writeString(toString());
				CW_BODY_STREAM << ");";
			} else {
				CW_BODY_STREAM << "_compilerClauseSuccess = false;";
			}
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\t}";
			CW_BODY_ENDL;
		}
		if (_pVariableToAssign != NULL) {
			if (!_listOfConstants.empty() && !_bContinue) {
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
			}
			compileCppAssignment(theCompilerEnvironment, iCursor);
			theCompilerEnvironment.incrementIndentation();
			_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, "tcText");
			theCompilerEnvironment.decrementIndentation();
			if (!_listOfConstants.empty() && !_bContinue) {
				CW_BODY_INDENT << "}";CW_BODY_ENDL;
				theCompilerEnvironment.decrementIndentation();
			}
		}
		theCompilerEnvironment.incrementIndentation();
		if (!_listOfConstants.empty() && !_bContinue) {
			CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tCGRuntime::setInputLocation(_compilerClauseLocation_" << iCursor << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tif (_compilerClauseImplicitCopy_" << iCursor << " >= 0) CGRuntime::resizeOutputStream(_compilerClauseImplicitCopy_" << iCursor << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "} else ";
		} else {
			CW_BODY_INDENT;
		}
		CW_BODY_STREAM << "if (_compilerClauseImplicitCopy_" << iCursor << " >= 0) {";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "\tunsigned char cChar = (unsigned char) _compilerClauseChar_" << iCursor << ";";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "\tCGRuntime::writeBinaryData(cChar);";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
	}

	void BNFReadChar::compileCppAssignment(CppCompilerEnvironment& theCompilerEnvironment, int iCursor) const {
		CW_BODY_INDENT << "\tchar tcText[] = {'\\0', '\\0'};";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "\ttcText[0] = (char) _compilerClauseChar_" << iCursor << ";";
		CW_BODY_ENDL;
	}

	std::string BNFReadChar::toString() const {
		std::string sText = "#readChar" + DtaBNFScript::constantsToString(_listOfConstants) + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
