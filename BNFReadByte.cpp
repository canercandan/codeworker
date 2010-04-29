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

#include "ScpStream.h"
#include "CGRuntime.h"
#include "CppCompilerEnvironment.h"

#include "DtaScriptVariable.h"
#include "BNFClause.h"
#include "DtaBNFScript.h"
#include "DtaVisitor.h"
#include "BNFReadByte.h"

namespace CodeWorker {
	BNFReadByte::BNFReadByte(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : BNFReadChar(pBNFScript, pParent, bContinue) {}

	BNFReadByte::~BNFReadByte() {}

	void BNFReadByte::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFReadByte(*this, env);
	}

	void BNFReadByte::setConstantsToMatch(const std::vector<std::string>& listOfConstants) {
		_listOfConstants = std::vector<std::string>();
		_listOfConstants.reserve(listOfConstants.size());
		for (std::string::size_type i = 0; i < listOfConstants.size(); i++) {
			_listOfConstants.push_back(CGRuntime::byteToChar(listOfConstants[i]));
		}
	}

	void BNFReadByte::postExecutionAssignment(DtaScriptVariable& variable, unsigned char cChar) {
		char tcText[] = {'\0', '\0', '\0'};
		tcText[0] = CGRuntime::_tcHexa[cChar >> 4];
		tcText[1] = CGRuntime::_tcHexa[cChar & 0x0F];
		if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
			variable.pushItem(tcText);
		} else {
			variable.setValue(tcText);
		}
	}

	void BNFReadByte::compileCppAssignment(CppCompilerEnvironment& theCompilerEnvironment, int iCursor) const {
		CW_BODY_INDENT << "\tchar tcText[] = {'\\0', '\\0', '\\0'};";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "\ttcText[0] = CGRuntime::_tcHexa[_compilerClauseChar_" << iCursor << " >> 4];";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "\ttcText[1] = CGRuntime::_tcHexa[_compilerClauseChar_" << iCursor << " & 0x0F];";
		CW_BODY_ENDL;
	}

	std::string BNFReadByte::toString() const {
		std::string sText = "#readByte" + constantsToString(_listOfConstants) + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}

	std::string BNFReadByte::constantsToString(const std::vector<std::string>& listOfConstants) {
		std::string sText;
		if (listOfConstants.size() == 1) {
			sText = ":\"" + CGRuntime::charToByte(listOfConstants[0]) + "\"";
		} else if (!listOfConstants.empty()) {
			sText = ":{";
			for (std::vector<std::string>::const_iterator i = listOfConstants.begin(); i != listOfConstants.end(); ++i) {
				if (i != listOfConstants.begin()) sText += ", ";
				sText += "\"" + CGRuntime::charToByte(*i) + "\"";
			}
			sText += "}";
		}
		return sText;
	}
}
