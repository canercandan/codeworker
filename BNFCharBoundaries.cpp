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
#include "BNFCharBoundaries.h"


namespace CodeWorker {
	BNFCharBoundaries::BNFCharBoundaries(DtaBNFScript* pBNFScript, GrfBlock* pParent, int iStartChar, int iEndChar, bool bContinue, bool bNoCase) : GrfCommand(pParent), _pBNFScript(pBNFScript), _iStartChar(iStartChar), _iEndChar(iEndChar), _pVariableToAssign(NULL), _bContinue(bContinue), _iClauseReturnType(BNFClause::NO_RETURN_TYPE), _bNoCase(bNoCase) {
		if (_bNoCase) {
			if ((iStartChar >= (int) 'A') && (_iStartChar <= (int) 'Z')) iStartChar += 32;
			if ((iEndChar >= (int) 'A') && (_iEndChar <= (int) 'Z')) iEndChar += 32;
			if ((iStartChar >= (int) 'a') && (iStartChar <= (int) 'z') &&
				(iEndChar >= (int) 'a') && (iEndChar <= (int) 'z')) {
				_iStartChar = iStartChar;
				_iEndChar = iEndChar;
			} else {
				_bNoCase = false;
			}
		}
	}

	BNFCharBoundaries::~BNFCharBoundaries() {
		delete _pVariableToAssign;
	}

	void BNFCharBoundaries::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFCharBoundaries(*this, env);
	}

	bool BNFCharBoundaries::isABNFCommand() const { return true; }

	void BNFCharBoundaries::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatVariable = bConcat;
		}
	}

	SEQUENCE_INTERRUPTION_LIST BNFCharBoundaries::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		int iOrgChar = CGRuntime::readCharAsInt();
		int iChar = iOrgChar;
		if (_bNoCase && (iChar <= (int) 'Z') && (iChar >= (int) 'A')) iChar += 32;
		if ((iChar < 0) || (iChar < _iStartChar) || (iChar > _iEndChar)) {
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
			char tcText[] = {'\0', '\0'};
			tcText[0] = (char) iOrgChar;
			if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
				pVariable->pushItem(tcText);
			} else {
				if (_bConcatVariable) pVariable->concatValue(tcText);
				else pVariable->setValue(tcText);
			}
		}
		if (iImplicitCopyPosition >= 0) {
			char cChar = (char) iOrgChar;
			_pBNFScript->writeBinaryData(&cChar, 1);
		}
		return NO_INTERRUPTION;
	}

	void BNFCharBoundaries::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CPP_COMPILER_BNF_SYMBOL_BEGIN;
		char tcOrgChar[64];
		sprintf(tcOrgChar, "_compilerOrgClauseChar_%d", iCursor);
		char tcChar[64];
		sprintf(tcChar, "_compilerClauseChar_%d", iCursor);
		CW_BODY_INDENT << "int " << tcOrgChar << " = CGRuntime::readCharAsInt();";CW_BODY_ENDL;
		CW_BODY_INDENT << "int " << tcChar << " = " << tcOrgChar << ";";CW_BODY_ENDL;
		if (_bNoCase) {
			CW_BODY_INDENT << "\tif ((" << tcChar << " <= (int) 'Z') && (" << tcChar << " >= (int) 'A')) " << tcChar << " += 32;";
			CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "if ((" << tcChar << " < 0) || (" << tcChar << " < ";
		CW_BODY_STREAM.writeQuotedChar(_iStartChar);
		CW_BODY_STREAM << ") || (" << tcChar << " > ";
		CW_BODY_STREAM.writeQuotedChar(_iEndChar);
		CW_BODY_STREAM << ")) {";
		CW_BODY_ENDL;
		CPP_COMPILER_BNF_SYMBOL_HAS_FAILED;
		CW_BODY_INDENT << "} else {";
		CW_BODY_ENDL;
		// compare to the set of constant characters, if any
		if (!_listOfConstants.empty()) {
			CW_BODY_INDENT << "\t_compilerClauseSuccess = true;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\tswitch(" << tcChar << ") {";CW_BODY_ENDL;
			for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
				CW_BODY_INDENT << "\t\tcase " << (int) ((*i)[0]) << ":";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "\t\t\tbreak;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\tdefault: ";
			CPP_COMPILER_BNF_SYMBOL_HAS_FAILED;
			CW_BODY_INDENT << "\t}";CW_BODY_ENDL;
		}
		// assign the variable, if required
		if (_pVariableToAssign != NULL) {
			if (!_listOfConstants.empty() && !_bContinue) {
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "\tchar tcText[] = {'\\0', '\\0'};";CW_BODY_ENDL;
			CW_BODY_INDENT << "\ttcText[0] = (char) " << tcOrgChar << ";";CW_BODY_ENDL;
			theCompilerEnvironment.incrementIndentation();
			_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, "tcText");
			theCompilerEnvironment.decrementIndentation();
			if (!_listOfConstants.empty() && !_bContinue) {
				CW_BODY_INDENT << "}";
				CW_BODY_ENDL;
				theCompilerEnvironment.decrementIndentation();
			}
		}
		// write to the output stream, for translations
		if (!_listOfConstants.empty() && !_bContinue) {
			theCompilerEnvironment.incrementIndentation();
			CW_BODY_INDENT << "if (_compilerClauseSuccess) {";
			CW_BODY_ENDL;
		}
		CW_BODY_INDENT << "\tif (" << tcImplicitCopy << " >= 0) {";CW_BODY_ENDL;
		CW_BODY_INDENT << "\t\tunsigned char cChar = (unsigned char) " << tcOrgChar << ";";CW_BODY_ENDL;
		CW_BODY_INDENT << "\t\ttheEnvironment.writeBinaryData(cChar);";CW_BODY_ENDL;
		CW_BODY_INDENT << "\t}";CW_BODY_ENDL;
		if (!_listOfConstants.empty() && !_bContinue) {
			CW_BODY_INDENT << "}";
			CW_BODY_ENDL;
			theCompilerEnvironment.decrementIndentation();
		}
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
	}

	std::string BNFCharBoundaries::toString() const {
		std::string sString;
		if (_bContinue) sString = "#continue ";
		sString += "'";
		char a = (char) _iStartChar;
		switch(a) {
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
				sString += a;
		}
		sString += "'..'";
		a = (char) _iEndChar;
		switch(a) {
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
				sString += a;
		}
		sString += "'";
		return sString + DtaBNFScript::constantsToString(_listOfConstants) + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
	}
}
