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
#include "BNFSkipIgnore.h"

namespace CodeWorker {
	BNFSkipIgnore::BNFSkipIgnore(DtaBNFScript* pBNFScript, GrfBlock* pParent, IGNORE_MODE eMode, BNFClause* pIgnoreClause, bool bContinue) : _pBNFScript(pBNFScript), GrfCommand(pParent), _iClauseReturnType(BNFClause::NO_RETURN_TYPE), _eMode(eMode), _pIgnoreClause(pIgnoreClause), _bContinue(bContinue), _pVariableToAssign(NULL) {}
	BNFSkipIgnore::~BNFSkipIgnore() {}

	void BNFSkipIgnore::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFSkipIgnore(*this, env);
	}

	bool BNFSkipIgnore::isABNFCommand() const { return true; }

	void BNFSkipIgnore::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatVariable = bConcat;
		}
	}

	SEQUENCE_INTERRUPTION_LIST BNFSkipIgnore::executeInternal(DtaScriptVariable& visibility) {
		BNFIgnoreScope theScope(_pBNFScript, _eMode, _pIgnoreClause);
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		std::string sValue;
		if (!_listOfConstants.empty()) {
			sValue = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - iLocation);
			bool bMatch = false;
			for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
				bMatch = (*i == sValue);
				if (bMatch) break;
			}
			if (!bMatch) {
				BNF_SYMBOL_HAS_FAILED
			}
		}
		if (_pVariableToAssign != NULL) {
			if (_listOfConstants.empty()) {
				sValue = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - iLocation);
			}
			DtaScriptVariable* pVariable = visibility.getVariable(*_pVariableToAssign);
			if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
				pVariable->pushItem(sValue);
			} else {
				if (_bConcatVariable) pVariable->concatValue(sValue.c_str());
				else pVariable->setValue(sValue.c_str());
			}
		}
		return NO_INTERRUPTION;
	}

	void BNFSkipIgnore::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		int iCursor = theCompilerEnvironment.newCursor();
		char tcLocation[64];
		sprintf(tcLocation, "_compilerClauseLocation_%d", iCursor);
		char tcImplicitCopy[64];
		sprintf(tcImplicitCopy, "_compilerClauseImplicitCopy_%d", iCursor);
		CW_BODY_INDENT << "{";CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "// " << toString();CW_BODY_ENDL;
		CW_BODY_INDENT << "int " << tcLocation << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
		BNFIgnore::compileCppRuntimeIgnore(theCompilerEnvironment, _eMode, _pIgnoreClause, iCursor);
		CW_BODY_INDENT << "int " << tcImplicitCopy << " = theEnvironment.skipEmptyChars();";CW_BODY_ENDL;
		CW_BODY_INDENT << "std::string _compilerClauseText_" << iCursor << ";";CW_BODY_ENDL;
		if (!_listOfConstants.empty()) {
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t_compilerClauseText_" << iCursor << " = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - " << tcLocation << ");";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t_compilerClauseSuccess = ";
			for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
				if (i != _listOfConstants.begin()) CW_BODY_STREAM << " || ";
				CW_BODY_STREAM << "(_compilerClauseText_" << iCursor << " == ";
				CW_BODY_STREAM.writeString(*i);
				CW_BODY_STREAM << ")";
			}
			CW_BODY_STREAM << ";";CW_BODY_ENDL;
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
		}
		if (_pVariableToAssign != NULL) {
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
			if (_listOfConstants.empty()) {
				CW_BODY_INDENT << "\t_compilerClauseText_" << iCursor << " = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - " << tcLocation << ");";CW_BODY_ENDL;
			}
			theCompilerEnvironment.incrementIndentation();
			char tcText[32];
			sprintf(tcText, "_compilerClauseText_%d", iCursor);
			_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, tcText);
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
		if (_bContinue) {
			CW_BODY_INDENT << "\tCGRuntime::throwBNFExecutionError(";
			CW_BODY_STREAM.writeString(toString());
			CW_BODY_STREAM << ");";
		} else {
			CW_BODY_INDENT << "\tCGRuntime::setInputLocation(" << tcLocation << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tif (" << tcImplicitCopy << " >= 0) CGRuntime::resizeOutputStream(" << tcImplicitCopy << ");";
		}
		CW_BODY_ENDL;
		CW_BODY_INDENT << "} else if (" << tcImplicitCopy << " >= 0) {";CW_BODY_ENDL;
		if (_listOfConstants.empty() && (_pVariableToAssign == NULL)) {
			CW_BODY_INDENT << "\t_compilerClauseText_" << iCursor << " = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - " << tcLocation << ");";CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "\tCGRuntime::writeBinaryData(_compilerClauseText_" << iCursor << ".c_str(), _compilerClauseText_" << iCursor << ".size());";CW_BODY_ENDL;
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
	}

	std::string BNFSkipIgnore::toString() const {
		std::string sString;
		if (_bContinue) sString = "#continue ";
		sString += "#skipIgnore";
		return sString + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
	}
}
