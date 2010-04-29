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
#include "BNFString.h"

namespace CodeWorker {
	BNFString::BNFString(DtaBNFScript* pBNFScript, GrfBlock* pParent, const std::string& sText, bool bContinue, bool bNoCase) : _pBNFScript(pBNFScript), GrfCommand(pParent), _pVariableToAssign(NULL), _bContinue(bContinue), _iClauseReturnType(BNFClause::NO_RETURN_TYPE), _bNoCase(bNoCase) {
		if (_bNoCase) {
			_bNoCase = false;
			for (std::string::size_type i = 0; i < sText.size(); i++) {
				char a = sText[i];
				if ((a >= 'A') && (a <= 'Z')) {
					a += ' ';
					_bNoCase = true;
				} else if (!_bNoCase && (a >= 'a') && (a <= 'z')) _bNoCase = true;
				_sText += a;
			}
		} else {
			_sText = sText;
		}
	}

	BNFString::~BNFString() {
		delete _pVariableToAssign;
	}

	void BNFString::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFString(*this, env);
	}

	bool BNFString::isABNFCommand() const { return true; }

	void BNFString::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatVariable = bConcat;
		}
	}

	SEQUENCE_INTERRUPTION_LIST BNFString::executeInternal(DtaScriptVariable& visibility) {
		bool bSuccess;
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		if (_bNoCase) bSuccess = CGRuntime::readIfEqualToIgnoreCase(_sText);
		else bSuccess = CGRuntime::readIfEqualTo(_sText);
		if (!bSuccess) {
			BNF_SYMBOL_HAS_FAILED
		}
		if (_pVariableToAssign != NULL) {
			DtaScriptVariable* pVariable = visibility.getVariable(*_pVariableToAssign);
			if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
				pVariable->pushItem(_sText);
			} else {
				if (_bConcatVariable) pVariable->concatValue(_sText.c_str());
				else pVariable->setValue(_sText.c_str());
			}
		}
		if (iImplicitCopyPosition >= 0) _pBNFScript->writeBinaryData(_sText.c_str(), _sText.size());
		return NO_INTERRUPTION;
	}

	void BNFString::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "// " << toString();
		CW_BODY_ENDL;
		int iCursor = theCompilerEnvironment.newCursor();
		CW_BODY_INDENT << "int _compilerClauseLocation_" << iCursor << " = CGRuntime::getInputLocation();";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "int _compilerClauseImplicitCopy_" << iCursor << " = theEnvironment.skipEmptyChars();";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "_compilerClauseSuccess = CGRuntime::readIfEqualTo";
		if (_bNoCase) CW_BODY_STREAM << "IgnoreCase";
		CW_BODY_STREAM << "(";
		CW_BODY_STREAM.writeString(_sText);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
		if (_pVariableToAssign != NULL) {
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
			theCompilerEnvironment.incrementIndentation();
			std::string sText = "\"" + CGRuntime::composeCLikeString(_sText) + "\"";
			_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, sText.c_str());
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
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
		CW_BODY_INDENT << "} else if (_compilerClauseImplicitCopy_" << iCursor << " >= 0) CGRuntime::writeBinaryData(";
		CW_BODY_STREAM.writeString(_sText);
		CW_BODY_STREAM << ", " << (int) _sText.size() << ");";
		CW_BODY_ENDL;
	}

	std::string BNFString::toString() const {
		std::string sText = "\"" + _sText + "\"" + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
