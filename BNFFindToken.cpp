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
#include "BNFMultiplicity.h" // for 'BNFMultiplicityBoundaries'
#include "DtaVisitor.h"
#include "BNFFindToken.h"

namespace CodeWorker {

	BNFFindToken::BNFFindToken(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : BNFStepper(pBNFScript, pParent, bContinue), _pIntermediateSequence(NULL), _pIntermediateVariableToAssign(NULL), _pFinalVariableToAssign(NULL), _pBoundaries(NULL) {}

	BNFFindToken::~BNFFindToken() {
		delete _pIntermediateSequence;
		delete _pIntermediateVariableToAssign;
		delete _pFinalVariableToAssign;
		delete _pBoundaries;
	}

	void BNFFindToken::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFFindToken(*this, env);
	}

	GrfBlock& BNFFindToken::createIntermediateSequence() {
		_pIntermediateSequence = new GrfBlock(this);
		return *_pIntermediateSequence;
	}

	void BNFFindToken::setIntermediateVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pIntermediateVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pIntermediateVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatIntermediateVariable = bConcat;
		}
	}

	void BNFFindToken::setFinalVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pFinalVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pFinalVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatFinalVariable = bConcat;
		}
	}

	SEQUENCE_INTERRUPTION_LIST BNFFindToken::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		int iNotEmptyLocation = CGRuntime::getInputLocation();
		int iIntermediateLocation = iNotEmptyLocation; // for 'intermediate code' only
		_iStepLocation = 0;
		int iBegin;
		int iEnd;
		if (_pBoundaries != NULL) {
			_pBoundaries->computeBoundaries(visibility, iBegin, iEnd);
		} else {
			iBegin = 0;
			iEnd = -1;
		}
		int iLoop;
		for (iLoop = 0; (iLoop <= iBegin) || (iEnd < 0) || (iLoop <= iEnd); iLoop++) {
			_pBNFScript->skipEmptyChars(visibility);
			result = GrfBlock::executeInternal(visibility);
			if (result != BREAK_INTERRUPTION) break;
			int iStartToken = CGRuntime::getInputLocation();
			if (_iStepLocation > iStartToken) {
				CGRuntime::setInputLocation(_iStepLocation);
				iIntermediateLocation = _iStepLocation;
				if (iImplicitCopyPosition >= 0) {
					std::string sText = CGRuntime::getLastReadChars(_iStepLocation - iStartToken);
					_pBNFScript->writeBinaryData(sText.c_str(), sText.size());
				}
				_iStepLocation = 0;
			} else {
				int iChar = CGRuntime::readCharAsInt();
				if (iChar < 0) break;
				iIntermediateLocation = iStartToken + 1;
				if (iImplicitCopyPosition >= 0) {
					char cChar = (char) iChar;
					_pBNFScript->writeBinaryData(&cChar, 1);
				}
			}
		}
		if (result == NO_INTERRUPTION) {
			if (iLoop >= iBegin) {
				int iCurrentLocation = CGRuntime::getInputLocation();
				result = resolveAssignment(visibility, _listOfConstants, _pVariableToAssign, _bConcatVariable, iImplicitCopyPosition, iLocation, iNotEmptyLocation, iCurrentLocation);
				if (result == NO_INTERRUPTION) {
					result = resolveAssignment(visibility, _listOfFinalConstants, _pFinalVariableToAssign, _bConcatFinalVariable, iImplicitCopyPosition, iLocation, iIntermediateLocation, iCurrentLocation);
					if (result == NO_INTERRUPTION) {
						CGRuntime::setInputLocation(iIntermediateLocation);
						result = resolveAssignment(visibility, _listOfIntermediateConstants, _pIntermediateVariableToAssign, _bConcatIntermediateVariable, iImplicitCopyPosition, iLocation, iNotEmptyLocation, iIntermediateLocation);
						if ((_pIntermediateSequence != NULL) && (result == NO_INTERRUPTION)) {
							result = _pIntermediateSequence->execute(visibility);
						}
						if (result == NO_INTERRUPTION) CGRuntime::setInputLocation(iCurrentLocation);
					}
				}
			} else {
				result = BREAK_INTERRUPTION;
			}
			if (result == BREAK_INTERRUPTION) {
				BNF_SYMBOL_HAS_FAILED
			}
		} else if (result != CONTINUE_INTERRUPTION) {
			BNF_SYMBOL_HAS_FAILED
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST BNFFindToken::resolveAssignment(DtaScriptVariable& visibility, const std::vector<std::string>& listOfConstants, ExprScriptVariable* pVariableToAssign, bool bConcatVariable, int iImplicitCopyPosition, int iLocation, int iBeginLocation, int iEndLocation) {
		std::string sTextToMatch;
		if (!listOfConstants.empty()) {
			bool bMatch = false;
			sTextToMatch = CGRuntime::getLastReadChars(iEndLocation - iBeginLocation);
			for (std::vector<std::string>::const_iterator i = listOfConstants.begin(); i != listOfConstants.end(); i++) {
				bMatch = ((*i) == sTextToMatch);
				if (bMatch) break;
			}
			if (!bMatch) {
				BNF_SYMBOL_HAS_FAILED
			}
		}
		if (pVariableToAssign != NULL) {
			DtaScriptVariable* pVariable = visibility.getVariable(*pVariableToAssign);
			if (listOfConstants.empty()) sTextToMatch = CGRuntime::getLastReadChars(iEndLocation - iBeginLocation);
			if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
				pVariable->pushItem(sTextToMatch);
			} else {
				if (bConcatVariable) pVariable->concatValue(sTextToMatch.c_str());
				else pVariable->setValue(sTextToMatch.c_str());
			}
		}
		return NO_INTERRUPTION;
	}

	void BNFFindToken::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			CPP_COMPILER_BNF_SYMBOL_BEGIN;
			if (!_listOfConstants.empty() || (_pVariableToAssign != 0) || !_listOfIntermediateConstants.empty() || (_pIntermediateVariableToAssign != 0) || !_listOfFinalConstants.empty() || (_pFinalVariableToAssign != 0)) {
				CW_BODY_INDENT << "int _compilerClauseNotEmptyLocation_" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
				if (!_listOfIntermediateConstants.empty() || (_pIntermediateVariableToAssign != 0) || !_listOfFinalConstants.empty() || (_pFinalVariableToAssign != 0)) {
					CW_BODY_INDENT << "int _compilerClauseIntermediateLocation_" << iCursor << " = _compilerClauseNotEmptyLocation_" << iCursor << ";";CW_BODY_ENDL;
				}
			}
			int iOldBNFStepperCursor = theCompilerEnvironment.getBNFStepperCursor();
			theCompilerEnvironment.setBNFStepperCursor(iCursor);
			CW_BODY_INDENT << "int _compilerClauseNextLocation_" << iCursor << " = 0;";CW_BODY_ENDL;
			if (_pBoundaries != 0) {
				if (!_pBoundaries->bConstantBoundaries) throw UtlException("BNFFindToken::compileCpp(): variable boundaries not handled yet!");
				CW_BODY_INDENT << "int _compilerClauseLoop_" << iCursor << ";";CW_BODY_ENDL;
				CW_BODY_INDENT << "int _compilerClauseBegin_" << iCursor << " = " << _pBoundaries->choice.constant._iBegin << ";";CW_BODY_ENDL;
				CW_BODY_INDENT << "int _compilerClauseEnd_" << iCursor << " = " << _pBoundaries->choice.constant._iEnd << ";";CW_BODY_ENDL;
				CW_BODY_INDENT << "for (_compilerClauseLoop_" << iCursor << " = 0; (_compilerClauseLoop_" << iCursor << " <= _compilerClauseBegin_" << iCursor << ") || (_compilerClauseEnd_" << iCursor << " < 0) || (_compilerClauseLoop_" << iCursor << " <= _compilerClauseEnd_" << iCursor << "); ++_compilerClauseLoop_" << iCursor << ") {";CW_BODY_ENDL;
			} else {
				CW_BODY_INDENT << "for (;;) {";CW_BODY_ENDL;
			}
			theCompilerEnvironment.incrementIndentation();
			CW_BODY_INDENT << "theEnvironment.skipEmptyChars();";CW_BODY_ENDL;
			for (;;) {
				(*i)->compileCpp(theCompilerEnvironment);
				i++;
				if (i == getCommands().end()) break;
				CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
			}
			int j = getNbCommands() - 1;
			while (j > 0) {
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_INDENT << "}";CW_BODY_ENDL;
				j--;
			}
			CW_BODY_INDENT << "if (_compilerClauseSuccess) break;";CW_BODY_ENDL;
			CW_BODY_INDENT << "int _compilerClauseStartToken_" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
			CW_BODY_INDENT << "if (_compilerClauseNextLocation_" << iCursor << " > _compilerClauseStartToken_" << iCursor << ") {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\tCGRuntime::setInputLocation(_compilerClauseNextLocation_" << iCursor << ");";CW_BODY_ENDL;
			if (!_listOfIntermediateConstants.empty() || (_pIntermediateVariableToAssign != 0) || !_listOfFinalConstants.empty() || (_pFinalVariableToAssign != 0)) {
				CW_BODY_INDENT << "\t_compilerClauseIntermediateLocation_" << iCursor << " = _compilerClauseNextLocation_" << iCursor << ";";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "\tif (" << tcImplicitCopy << " >= 0) {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\tstd::string sText = CGRuntime::getLastReadChars(_compilerClauseNextLocation_" << iCursor << " - _compilerClauseStartToken_" << iCursor << ");";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\ttheEnvironment.writeBinaryData(sText.c_str(), sText.size());";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t}";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t_compilerClauseNextLocation_" << iCursor << " = 0;";CW_BODY_ENDL;
			CW_BODY_INDENT << "} else {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\tint iChar = CGRuntime::readCharAsInt();";CW_BODY_ENDL;
			CW_BODY_INDENT << "\tif (iChar < 0) break;";CW_BODY_ENDL;
			if (!_listOfIntermediateConstants.empty() || (_pIntermediateVariableToAssign != 0) || !_listOfFinalConstants.empty() || (_pFinalVariableToAssign != 0)) {
				CW_BODY_INDENT << "\t_compilerClauseIntermediateLocation_" << iCursor << " = _compilerClauseStartToken_" << iCursor << " + 1;";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "\tif (" << tcImplicitCopy << " >= 0) {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\tunsigned char cChar = (unsigned char) iChar;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\ttheEnvironment.writeBinaryData(cChar);";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t}";CW_BODY_ENDL;
			CW_BODY_INDENT << "}";	CW_BODY_ENDL;
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
			if (_pBoundaries != 0) {
				CW_BODY_INDENT << "_compilerClauseSuccess = _compilerClauseSuccess && (_compilerClauseLoop_" << iCursor << " >= _compilerClauseBegin_" << iCursor << ");";CW_BODY_ENDL;
			}
			if (!_listOfConstants.empty() || (_pVariableToAssign != 0) || !_listOfIntermediateConstants.empty() || (_pIntermediateVariableToAssign != 0) || !_listOfFinalConstants.empty() || (_pFinalVariableToAssign != 0)) {
				CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "int _compilerClauseCurrentLocation_" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
				if (!_listOfConstants.empty() || (_pVariableToAssign != 0)) {
					compileCppResolveAssignment(theCompilerEnvironment, iCursor, _listOfConstants, _pVariableToAssign, _bConcatVariable, "NotEmptyLocation", "CurrentLocation");
				}
				if (!_listOfFinalConstants.empty() || (_pFinalVariableToAssign != 0)) {
					if (!_listOfConstants.empty()) {
						CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
						theCompilerEnvironment.incrementIndentation();
					}
					compileCppResolveAssignment(theCompilerEnvironment, iCursor, _listOfFinalConstants, _pFinalVariableToAssign, _bConcatFinalVariable, "IntermediateLocation", "CurrentLocation");
					if (!_listOfConstants.empty()) {
						theCompilerEnvironment.decrementIndentation();
						CW_BODY_INDENT << "}";CW_BODY_ENDL;
					}
				}
				if (!_listOfIntermediateConstants.empty() || (_pIntermediateVariableToAssign != 0)) {
					if (!_listOfConstants.empty() || !_listOfFinalConstants.empty()) {
						CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
						theCompilerEnvironment.incrementIndentation();
					}
					CW_BODY_INDENT << "CGRuntime::setInputLocation(_compilerClauseIntermediateLocation_" << iCursor << ");";CW_BODY_ENDL;
					compileCppResolveAssignment(theCompilerEnvironment, iCursor, _listOfIntermediateConstants, _pIntermediateVariableToAssign, _bConcatIntermediateVariable, "NotEmptyLocation", "IntermediateLocation");
					if (_pIntermediateSequence != NULL) {
						CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
						theCompilerEnvironment.incrementIndentation();
						_pIntermediateSequence->compileCpp(theCompilerEnvironment);
						theCompilerEnvironment.decrementIndentation();
						CW_BODY_INDENT << "}";CW_BODY_ENDL;
					}
					CW_BODY_INDENT;
					if (!_listOfIntermediateConstants.empty()) {
						CW_BODY_STREAM << "if (_compilerClauseSuccess) ";
					}
					CW_BODY_STREAM << "CGRuntime::setInputLocation(_compilerClauseCurrentLocation_" << iCursor << ");";CW_BODY_ENDL;
					if (!_listOfConstants.empty() || !_listOfFinalConstants.empty()) {
						theCompilerEnvironment.decrementIndentation();
						CW_BODY_INDENT << "}";CW_BODY_ENDL;
					}
				}
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_INDENT << "}";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
			CPP_COMPILER_BNF_SYMBOL_HAS_FAILED;
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
			theCompilerEnvironment.setBNFStepperCursor(iOldBNFStepperCursor);
		}
	}

	void BNFFindToken::compileCppResolveAssignment(CppCompilerEnvironment& theCompilerEnvironment, int iCursor, const std::vector<std::string>& listOfConstants, ExprScriptVariable* pVariableToAssign, bool bConcatVariable, const std::string& sBeginLocation, const std::string& sEndLocation) const {
		if (!listOfConstants.empty()) {
			CW_BODY_INDENT << "std::string _compilerClauseText_" << iCursor << " = CGRuntime::getLastReadChars(_compilerClause" << sEndLocation << "_" << iCursor << " - _compilerClause" << sBeginLocation << "_" << iCursor << ");";CW_BODY_ENDL;
			CW_BODY_INDENT << "_compilerClauseSuccess = ";
			for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
				if (i != _listOfConstants.begin()) CW_BODY_STREAM << " || ";
				CW_BODY_STREAM << "(";
//				if (_bNoCase) {
//					CW_BODY_STREAM << "stricmp(_compilerClauseText_" << iCursor << ".c_str(), ";
//					CW_BODY_STREAM.writeString(*i);
//					CW_BODY_STREAM << ") == 0";
//				} else {
					CW_BODY_STREAM << "_compilerClauseText_" << iCursor << " == ";
					CW_BODY_STREAM.writeString(*i);
//				}
				CW_BODY_STREAM << ")";
			}
			CW_BODY_STREAM << ";";CW_BODY_ENDL;
		}
		if (pVariableToAssign != NULL) {
			if (!listOfConstants.empty()) {
				CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
			} else {
				CW_BODY_INDENT << "std::string _compilerClauseText_" << iCursor << " = CGRuntime::getLastReadChars(_compilerClause" << sEndLocation << "_" << iCursor << " - _compilerClause" << sBeginLocation << "_" << iCursor << ");";CW_BODY_ENDL;
			}
			char tcText[32];
			sprintf(tcText, "_compilerClauseText_%d", iCursor);
			_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, tcText);
			if (!listOfConstants.empty()) {
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_INDENT << "}";CW_BODY_ENDL;
			}
		}
	}

	std::string BNFFindToken::toString() const {
		std::string sText = "->";
		if (_pBoundaries != NULL) {
			sText += _pBoundaries->toString();
			if (sText.size() == 2) sText += "1";
		}
		if (hasIntermediateCode()) {
			std::string sConstVar = DtaBNFScript::constantsToString(_listOfIntermediateConstants) + DtaBNFScript::assignmentToString(_pIntermediateVariableToAssign, _bConcatVariable);
			if (!_listOfFinalConstants.empty() || (_pFinalVariableToAssign != NULL)) {
				sConstVar += "-" + DtaBNFScript::constantsToString(_listOfFinalConstants) + DtaBNFScript::assignmentToString(_pFinalVariableToAssign, _bConcatVariable);
			}
			sText += "(" + sConstVar;
			if (_pIntermediateSequence != NULL) {
				if (!sConstVar.empty()) sText += " ";
				sText += _pIntermediateSequence->toString();
			}
			sText += ")";
		}
		sText += getCommands()[0]->toString() + DtaBNFScript::constantsToString(_listOfConstants) + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
		if (_bContinue) sText = "#continue " + sText;
		return sText;
	}
}
