/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2004 Cédric Lemaire

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

#ifndef WIN32
#	include "UtlString.h" // for Debian/gcc 2.95.4
#endif

#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "DtaBNFScript.h"
#include "BNFClause.h"
#include "DtaVisitor.h"
#include "BNFReadCChar.h"

namespace CodeWorker {
	BNFReadCChar::BNFReadCChar(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue, bool bNoCase) : _pBNFScript(pBNFScript), GrfCommand(pParent), _pVariableToAssign(NULL), _bContinue(bContinue), _iClauseReturnType(BNFClause::NO_RETURN_TYPE), _bNoCase(bNoCase) {}

	BNFReadCChar::~BNFReadCChar() {
		delete _pVariableToAssign;
	}

	void BNFReadCChar::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFReadCChar(*this, env);
	}

	bool BNFReadCChar::isABNFCommand() const { return true; }

	void BNFReadCChar::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatVariable = bConcat;
		}
	}

	SEQUENCE_INTERRUPTION_LIST BNFReadCChar::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		int iNotEmptyLocation = CGRuntime::getInputLocation();
		std::string sValue = CGRuntime::readCChar();
		if (iNotEmptyLocation == CGRuntime::getInputLocation()) {
			BNF_SYMBOL_HAS_FAILED
		}
		if (!_listOfConstants.empty()) {
			bool bMatch = false;
			for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
				if (_bNoCase) bMatch = (stricmp(i->c_str(), sValue.c_str()) == 0);
				else bMatch = (*i == sValue);
				if (bMatch) break;
			}
			if (!bMatch) {
				BNF_SYMBOL_HAS_FAILED
			}
		}
		if (_pVariableToAssign != NULL) {
			DtaScriptVariable* pVariable = visibility.getVariable(*_pVariableToAssign);
			if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
				pVariable->pushItem(sValue);
			} else {
				if (_bConcatVariable) pVariable->concatValue(sValue.c_str());
				else pVariable->setValue(sValue.c_str());
			}
		}
		if (iImplicitCopyPosition >= 0) {
			std::string sText = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - iNotEmptyLocation);
			_pBNFScript->writeBinaryData(sText.c_str(), sText.size());
		}
		return NO_INTERRUPTION;
	}

	void BNFReadCChar::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "// " << toString();
		CW_BODY_ENDL;
		int iCursor = theCompilerEnvironment.newCursor();
		CW_BODY_INDENT << "int _compilerClauseLocation_" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
		CW_BODY_INDENT << "int _compilerClauseImplicitCopy_" << iCursor << " = theEnvironment.skipEmptyChars();";CW_BODY_ENDL;
		CW_BODY_INDENT << "int _compilerClauseNotEmptyLocation_" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
		CW_BODY_INDENT << "std::string _compilerClauseText_" << iCursor << " = CGRuntime::readCChar();";CW_BODY_ENDL;
		CW_BODY_INDENT << "_compilerClauseSuccess = (_compilerClauseNotEmptyLocation_" << iCursor << " != _compilerClauseLocation_" << iCursor << ");";CW_BODY_ENDL;
		if (!_listOfConstants.empty()) {
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\t_compilerClauseSuccess = ";
			for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
				if (i != _listOfConstants.begin()) CW_BODY_STREAM << " || ";
				CW_BODY_STREAM << "(";
				if (_bNoCase) {
					CW_BODY_STREAM << "stricmp(_compilerClauseText_" << iCursor << ".c_str(), ";
					CW_BODY_STREAM.writeString(*i);
					CW_BODY_STREAM << ") == 0";
				} else {
					CW_BODY_STREAM << "_compilerClauseText_" << iCursor << " == ";
					CW_BODY_STREAM.writeString(*i);
				}
				CW_BODY_STREAM << ")";
			}
			CW_BODY_STREAM << ";";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "}";
			CW_BODY_ENDL;
		}
		if (_pVariableToAssign != NULL) {
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
			theCompilerEnvironment.incrementIndentation();
			char tcText[32];
			sprintf(tcText, "_compilerClauseText_%d", iCursor);
			_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, tcText);
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";
		CW_BODY_ENDL;
		if (_bContinue) {
			CW_BODY_INDENT << "\tCGRuntime::throwBNFExecutionError(";
			CW_BODY_STREAM.writeString(toString());
			CW_BODY_STREAM << ");";
		} else {
			CW_BODY_INDENT << "\tCGRuntime::setInputLocation(_compilerClauseLocation_" << iCursor << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tif (_compilerClauseImplicitCopy_" << iCursor << " >= 0) CGRuntime::resizeOutputStream(_compilerClauseImplicitCopy_" << iCursor << ");";
		}
		CW_BODY_ENDL;
		CW_BODY_INDENT << "} else if (_compilerClauseImplicitCopy_" << iCursor << " >= 0) CGRuntime::writeBinaryData(_compilerClauseText_" << iCursor << ".c_str(), _compilerClauseText_" << iCursor << ".size());";
		CW_BODY_ENDL;
	}

	std::string BNFReadCChar::toString() const {
		std::string sText = "#readCChar" + DtaBNFScript::constantsToString(_listOfConstants) + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
