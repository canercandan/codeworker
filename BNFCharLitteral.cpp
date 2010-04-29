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
#include "BNFCharLitteral.h"

namespace CodeWorker {
	BNFCharLitteral::BNFCharLitteral(DtaBNFScript* pBNFScript, GrfBlock* pParent, int iChar, bool bContinue, bool bNoCase) : _pBNFScript(pBNFScript), GrfCommand(pParent), _iChar(iChar), _pVariableToAssign(NULL), _bContinue(bContinue), _iClauseReturnType(BNFClause::NO_RETURN_TYPE), _bNoCase(bNoCase) {
		if (_bNoCase) {
			if ((_iChar >= (int) 'A') && (_iChar <= (int) 'Z')) _iChar += 32;
			else if ((iChar < (int) 'a') || (iChar > (int) 'z')) _bNoCase = false;
		}
	}

	BNFCharLitteral::~BNFCharLitteral() {
		delete _pVariableToAssign;
	}

	void BNFCharLitteral::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFCharLitteral(*this, env);
	}

	bool BNFCharLitteral::isABNFCommand() const { return true; }

	void BNFCharLitteral::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatVariable = bConcat;
		}
	}

	SEQUENCE_INTERRUPTION_LIST BNFCharLitteral::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		int iChar = CGRuntime::readCharAsInt();
		if ((iChar != _iChar) && (!_bNoCase || (iChar + 32 != _iChar))) {
			BNF_SYMBOL_HAS_FAILED
		}
		if (_pVariableToAssign != NULL) {
			DtaScriptVariable* pVariable = visibility.getVariable(*_pVariableToAssign);
			char tcText[] = {'\0', '\0'};
			tcText[0] = (char) _iChar;
			if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
				pVariable->pushItem(tcText);
			} else {
				if (_bConcatVariable) pVariable->concatValue(tcText);
				else pVariable->setValue(tcText);
			}
		}
		if (iImplicitCopyPosition >= 0) {
			char cChar = (char) _iChar;
			_pBNFScript->writeBinaryData(&cChar, 1);
		}
		return NO_INTERRUPTION;
	}

	void BNFCharLitteral::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CPP_COMPILER_BNF_SYMBOL_BEGIN;
		char tcChar[64];
		sprintf(tcChar, "_compilerClauseChar_%d", iCursor);
		CW_BODY_INDENT << "int " << tcChar << " = CGRuntime::readCharAsInt();";CW_BODY_ENDL;
		if (_bNoCase) {
			CW_BODY_INDENT << "if ((" << tcChar << " != (int) ";
			CW_BODY_STREAM.writeQuotedChar(_iChar);
			CW_BODY_STREAM << ") && (" << tcChar << " != (int) ";
			CW_BODY_STREAM.writeQuotedChar(_iChar - 32);
			CW_BODY_STREAM << ")) {";
		} else {
			CW_BODY_INDENT << "if (" << tcChar << " != (int) ";
			CW_BODY_STREAM.writeQuotedChar(_iChar);
			CW_BODY_STREAM << ") {";
		}
		CW_BODY_ENDL;
		CPP_COMPILER_BNF_SYMBOL_HAS_FAILED;
		CW_BODY_INDENT << "} else {";CW_BODY_ENDL;
		// assign the variable, if required
		if (_pVariableToAssign != NULL) {
			CW_BODY_INDENT << "\tchar tcText[] = {'\\0', '\\0'};";CW_BODY_ENDL;
			CW_BODY_INDENT << "\ttcText[0] = (char) " << tcChar << ";";CW_BODY_ENDL;
			_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, "tcText");
		}
		// write to the output stream, for translations
		CW_BODY_INDENT << "\tif (" << tcImplicitCopy << " >= 0) {";CW_BODY_ENDL;
		CW_BODY_INDENT << "\t\tunsigned char cChar = (unsigned char) " << tcChar << ";";CW_BODY_ENDL;
		CW_BODY_INDENT << "\t\ttheEnvironment.writeBinaryData(cChar);";CW_BODY_ENDL;
		CW_BODY_INDENT << "\t}";CW_BODY_ENDL;
		CW_BODY_INDENT << "\t_compilerClauseSuccess = true;";CW_BODY_ENDL;
		CW_BODY_INDENT << "}";	CW_BODY_ENDL;
	}

	std::string BNFCharLitteral::toString() const {
		std::string sString;
		if (_bContinue) sString = "#continue ";
		sString += "'";
		char c = (char) _iChar;
		switch(c) {
			case '\\': sString += "\\\\";break;
			case '\"': sString += "\\\"";break;
			case '\'': sString += "\\'";break;
			case '\a': sString += "\\a";break;
			case '\b': sString += "\\b";break;
			case '\f': sString += "\\f";break;
			case '\n': sString += "\\n";break;
			case '\r': sString += "\\r";break;
			case '\t': sString += "\\t";break;
			case '\v': sString += "\\v";break;
			default:
				sString += c;
		}
		sString += "'";
		return sString + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
	}
}
