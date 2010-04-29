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
#include "DtaScriptVariable.h"
#include "ExprScriptExpression.h"
#include "GrfSwitch.h"


namespace CodeWorker {
	GrfSwitch::~GrfSwitch() {
		delete _pExpression;
	}

	SEQUENCE_INTERRUPTION_LIST GrfSwitch::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		DtaScriptVariable stackSwitch(&visibility, "##stack## switch");
		std::string sExpression = _pExpression->getValue(visibility);
		if (equalString(sExpression.c_str())) result = GrfBlock::executeAt(stackSwitch, _directionEqualList[sExpression]);
		else {
			bool bFound = false;
			int iRank;
			for (std::map<std::string, int>::const_iterator i = _directionStartList.begin(); i != _directionStartList.end(); i++) {
				if (strncmp(i->first.c_str(), sExpression.c_str(), i->first.size()) == 0) {
					bFound = true;
					iRank = i->second;
				} else if (bFound) {
					break;
				}
			}
			if (bFound) {
				result = GrfBlock::executeAt(stackSwitch, iRank);
			} else {
				if (_iDefault >= 0) {
					result = GrfBlock::executeAt(stackSwitch, _iDefault);
				} else {
					throw UtlException("switch internal error: unknown '" + sExpression + "' case encountered, it lacks either a 'case' or a 'default' statement");
				}
			}
		}
		switch(result) {
			case BREAK_INTERRUPTION:
			case CONTINUE_INTERRUPTION:
				result = NO_INTERRUPTION;
			case NO_INTERRUPTION: break;
		}
		return result;
	}

	void GrfSwitch::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::map<int, std::string> mapOfStatements;
		int iSwitchNumber = theCompilerEnvironment.newSwitch();
		CW_BODY_INDENT << "std::string _compilerSwitch_key" << iSwitchNumber << " = ";
		_pExpression->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ";";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "int _compilerSwitch_statement" << iSwitchNumber << ";";
		CW_BODY_ENDL;
		if (!_directionEqualList.empty()) {
			CW_BODY_INDENT << "static std::map<std::string, int> _compilerSwitch_table" << iSwitchNumber << ";";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "if (_compilerSwitch_table" << iSwitchNumber << ".empty()) {";
			CW_BODY_ENDL;
			theCompilerEnvironment.incrementIndentation();
			for (std::map<std::string, int>::const_iterator i = _directionEqualList.begin(); i != _directionEqualList.end(); i++) {
				mapOfStatements[i->second] = i->first;
				CW_BODY_INDENT << "_compilerSwitch_table" << iSwitchNumber << "[";
				CW_BODY_STREAM.writeString(i->first);
				CW_BODY_STREAM << "] = " << i->second << ";";
				CW_BODY_ENDL;
			}
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "std::map<std::string, int>::const_iterator _compilerSwitch_cursor" << iSwitchNumber << " = _compilerSwitch_table" << iSwitchNumber << ".find(_compilerSwitch_key" << iSwitchNumber << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "if (_compilerSwitch_cursor" << iSwitchNumber << " != _compilerSwitch_table" << iSwitchNumber << ".end()) {";
			CW_BODY_ENDL;
			theCompilerEnvironment.incrementIndentation();
			CW_BODY_INDENT << "_compilerSwitch_statement" << iSwitchNumber << " = _compilerSwitch_cursor" << iSwitchNumber << "->second;";
			CW_BODY_ENDL;
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";
		}
		if (!_directionStartList.empty()) {
			if (_directionEqualList.empty()) {
				CW_BODY_INDENT;
			} else {
				CW_BODY_STREAM << " else ";
			}
			for (std::map<std::string, int>::const_iterator i = _directionStartList.begin(); i != _directionStartList.end(); i++) {
				if (i != _directionStartList.begin()) {
					CW_BODY_STREAM << " else ";
				}
				mapOfStatements[i->second] = i->first;
				CW_BODY_STREAM << "if (strncmp(_compilerSwitch_key" << iSwitchNumber << ".c_str(), ";
				CW_BODY_STREAM.writeString(i->first);
				CW_BODY_STREAM << ", " << (int) i->first.size() << ") == 0) {";
				CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "_compilerSwitch_statement" << iSwitchNumber << " = " << i->second << ";";
				CW_BODY_ENDL;
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_INDENT << "}";
			}
		}
		if (_directionStartList.empty() && _directionEqualList.empty()) {
			throw UtlException("empty 'switch' statement encountered into file \"" + std::string(_sParsingFilePtr) + "\"");
		}
		CW_BODY_STREAM << " else {";
		CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "_compilerSwitch_statement" << iSwitchNumber << " = " << _iDefault << ";";
		CW_BODY_ENDL;
		if (_iDefault >= 0) mapOfStatements[_iDefault] = "default";
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "switch(_compilerSwitch_statement" << iSwitchNumber << ") {";
		CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		int iLastPosition = -1;
		for (std::map<int, std::string>::const_iterator i = mapOfStatements.begin(); i != mapOfStatements.end(); i++) {
			if (iLastPosition >= 0) {
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "{";
				CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				compileCppBetween(theCompilerEnvironment, iLastPosition, i->first);
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_INDENT << "}";
				CW_BODY_ENDL;
				theCompilerEnvironment.decrementIndentation();
			}
			if (i->first == _iDefault) {
				CW_BODY_INDENT << "default:";
			} else {
				CW_BODY_INDENT << "case " << i->first << ":";
			}
			CW_BODY_ENDL;
			iLastPosition = i->first;
		}
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "{";
		CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		compileCppAt(theCompilerEnvironment, iLastPosition);
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
		theCompilerEnvironment.decrementIndentation();
		if (_iDefault < 0) {
			CW_BODY_INDENT << "default:";
			CW_BODY_ENDL;
			theCompilerEnvironment.incrementIndentation();
			CW_BODY_INDENT << "throw UtlException(\"switch internal error: unknown '\" + _compilerSwitch_key" << iSwitchNumber << " + \"' case encountered into script \\";
			CW_BODY_STREAM.writeString(_sParsingFilePtr);
			CW_BODY_STREAM.setOutputLocation(CW_BODY_STREAM.getOutputLocation() - 1);
			CW_BODY_STREAM << "\\\", it lacks either a 'case' or a 'default' statement\");";
			CW_BODY_ENDL;
			theCompilerEnvironment.decrementIndentation();
		}
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";
		CW_BODY_ENDL;
	}
}
