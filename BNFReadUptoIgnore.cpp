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
#include "BNFIgnore.h"
#include "DtaVisitor.h"
#include "BNFReadUptoIgnore.h"

namespace CodeWorker {
	BNFReadUptoIgnore::BNFReadUptoIgnore(DtaBNFScript* pBNFScript, GrfBlock* pParent, IGNORE_MODE eMode, BNFClause* pIgnoreClause, bool bContinue) : _pBNFScript(pBNFScript), GrfCommand(pParent), _pVariableToAssign(NULL), _eMode(eMode), _pIgnoreClause(pIgnoreClause), _bContinue(bContinue), _iClauseReturnType(BNFClause::NO_RETURN_TYPE) {}

	BNFReadUptoIgnore::~BNFReadUptoIgnore() {
		delete _pVariableToAssign;
		delete _pText;
	}

	void BNFReadUptoIgnore::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFReadUptoIgnore(*this, env);
	}

	bool BNFReadUptoIgnore::isABNFCommand() const { return true; }

	void BNFReadUptoIgnore::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatVariable = bConcat;
		}
	}

	SEQUENCE_INTERRUPTION_LIST BNFReadUptoIgnore::executeInternal(DtaScriptVariable& visibility) {
		bool bSuccess;
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		int iFirstLocation = CGRuntime::getInputLocation();
		int iLastLocation = iFirstLocation;
		BNFIgnoreScope theScope(_pBNFScript, _eMode, _pIgnoreClause);
		while (CGRuntime::readCharAsInt() >= 0) {
			iLastLocation++;
			_pBNFScript->skipEmptyChars(visibility);
			if (iLastLocation != CGRuntime::getInputLocation()) break;
		}
		bSuccess = (iFirstLocation != iLastLocation);
		std::string sValue;
		if (bSuccess) {
			CGRuntime::setInputLocation(iLastLocation);
		} else {
			BNF_SYMBOL_HAS_FAILED
		}
		if (!_listOfConstants.empty()) {
			sValue = CGRuntime::getLastReadChars(iLastLocation - iFirstLocation);
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
			if (sValue.empty()) {
				sValue = CGRuntime::getLastReadChars(iLastLocation - iFirstLocation);
			}
			DtaScriptVariable* pVariable = visibility.getVariable(*_pVariableToAssign);
			if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
				pVariable->pushItem(sValue);
			} else {
				if (_bConcatVariable) pVariable->concatValue(sValue.c_str());
				else pVariable->setValue(sValue.c_str());
			}
		}
		if (iImplicitCopyPosition >= 0) {
			if (sValue.empty()) {
				sValue = CGRuntime::getLastReadChars(iLastLocation - iFirstLocation);
			}
			_pBNFScript->writeBinaryData(sValue.c_str(), sValue.size());
		}
		return NO_INTERRUPTION;
	}

	void BNFReadUptoIgnore::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CPP_COMPILER_BNF_SYMBOL_BEGIN;
		CW_BODY_INDENT << "int _compilerClauseFirstLocation_" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
		CW_BODY_INDENT << "int _compilerClauseLastLocation_" << iCursor << " = _compilerClauseFirstLocation_" << iCursor << ";";CW_BODY_ENDL;
		BNFIgnore::compileCppRuntimeIgnore(theCompilerEnvironment, _eMode, _pIgnoreClause, iCursor);
		CW_BODY_INDENT << "while (CGRuntime::readCharAsInt() >= 0) {";CW_BODY_ENDL;
		CW_BODY_INDENT << "\t++_compilerClauseLastLocation_" << iCursor << ";";CW_BODY_ENDL;
		CW_BODY_INDENT << "\ttheEnvironment.skipEmptyChars();";CW_BODY_ENDL;
		CW_BODY_INDENT << "\tif (_compilerClauseLastLocation_" << iCursor << " != CGRuntime::getInputLocation()) break;";CW_BODY_ENDL;
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
		CW_BODY_INDENT << "_compilerClauseSuccess = (_compilerClauseFirstLocation_" << iCursor << " != _compilerClauseLastLocation_" << iCursor << ");";CW_BODY_ENDL;
		CW_BODY_INDENT << "std::string _compilerClauseText_" << iCursor << ";";CW_BODY_ENDL;
		if (!_listOfConstants.empty()) {
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t_compilerClauseText_" << iCursor << " = CGRuntime::getLastReadChars(_compilerClauseLastLocation_" << iCursor << " - _compilerClauseFirstLocation_" << iCursor << ");";CW_BODY_ENDL;
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
				CW_BODY_INDENT << "\t_compilerClauseText_" << iCursor << " = CGRuntime::getLastReadChars(_compilerClauseLastLocation_" << iCursor << " - _compilerClauseFirstLocation_" << iCursor << ");";CW_BODY_ENDL;
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
			CW_BODY_INDENT << "\t_compilerClauseText_" << iCursor << " = CGRuntime::getLastReadChars(_compilerClauseLastLocation_" << iCursor << " - _compilerClauseFirstLocation_" << iCursor << ");";CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "\tCGRuntime::writeBinaryData(_compilerClauseText_" << iCursor << ".c_str(), _compilerClauseText_" << iCursor << ".size());";CW_BODY_ENDL;
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
	}

	std::string BNFReadUptoIgnore::toString() const {
		std::string sText = "#readUptoIgnore";
		sText += DtaBNFScript::constantsToString(_listOfConstants) + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
