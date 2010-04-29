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
#include "BNFMultiplicity.h"

namespace CodeWorker {

	BNFMultiplicityBoundaries::~BNFMultiplicityBoundaries() {
		if (!bConstantBoundaries) {
			delete choice.variable._pBegin;
			delete choice.variable._pEnd;
		}
	}

	void BNFMultiplicityBoundaries::setMultiplicity(int iBegin, int iEnd) {
		bConstantBoundaries = true;
		choice.constant._iBegin = iBegin;
		choice.constant._iEnd = iEnd;
	}

	void BNFMultiplicityBoundaries::setMultiplicity(ExprScriptExpression* pBegin, ExprScriptExpression* pEnd) {
		bConstantBoundaries = false;
		choice.variable._pBegin = pBegin;
		choice.variable._pEnd = pEnd;
	}

	void BNFMultiplicityBoundaries::computeBoundaries(DtaScriptVariable& visibility, int& iBegin, int& iEnd) const {
		if (bConstantBoundaries) {
			iBegin = choice.constant._iBegin;
			iEnd   = choice.constant._iEnd;
		} else {
			iBegin = choice.variable._pBegin->getIntValue(visibility);
			if (choice.variable._pEnd == NULL) iEnd = iBegin;
			else iEnd = choice.variable._pEnd->getIntValue(visibility);
		}
	}

	std::string BNFMultiplicityBoundaries::toString() const {
		std::string sString;
		if (bConstantBoundaries) {
			if (choice.constant._iBegin == 0) {
				if (choice.constant._iEnd < 0) sString = "*";
				else if (choice.constant._iEnd == 1) sString = "?";
				else {
					char tcNumber[32];
					sprintf(tcNumber, "0..%d", choice.constant._iEnd);
					sString = tcNumber;
				}
			} else if (choice.constant._iBegin == 1) {
				if (choice.constant._iEnd < 0) sString = "+";
				else if (choice.constant._iEnd != 1) {
					char tcNumber[32];
					sprintf(tcNumber, "1..%d", choice.constant._iEnd);
					sString = tcNumber;
				}
			} else {
				if (choice.constant._iBegin == choice.constant._iEnd) {
					char tcNumber[32];
					sprintf(tcNumber, "%d", choice.constant._iBegin);
					sString = tcNumber;
				} else {
					char tcNumber[32];
					sprintf(tcNumber, "%d..%d", choice.constant._iBegin, choice.constant._iEnd);
					sString = tcNumber;
				}
			}
		} else {
			sString = "#repeat(" + choice.variable._pBegin->toString();
			if (choice.variable._pEnd != NULL) {
				sString += ", " + choice.variable._pEnd->toString();
			}
			sString += ")";
		}
		return sString;
	}


	BNFMultiplicity::BNFMultiplicity(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : _pBNFScript(pBNFScript), GrfBlock(pParent), _pVariableToAssign(NULL), _bContinue(bContinue), _iClauseReturnType(BNFClause::NO_RETURN_TYPE), _bConcatVariable(false) {}

	BNFMultiplicity::~BNFMultiplicity() {
		delete _pVariableToAssign;
	}

	void BNFMultiplicity::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFMultiplicity(*this, env);
	}

	bool BNFMultiplicity::isABNFCommand() const { return true; }

	void BNFMultiplicity::setVariableToAssign(ExprScriptVariable* pVariableToAssign, bool bConcat, BNFClause& theClause) {
		if (pVariableToAssign != NULL) {
			_pVariableToAssign = pVariableToAssign;
			std::string sVariableName = _pVariableToAssign->toString();
			if (sVariableName == theClause.getName()) _iClauseReturnType = theClause.getReturnType();
			_bConcatVariable = bConcat;
		}
	}

	SEQUENCE_INTERRUPTION_LIST BNFMultiplicity::execute(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = -1;
		int iNotEmptyLocation;
		if (!_listOfConstants.empty() || (_pVariableToAssign != NULL)) {
			iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
			iNotEmptyLocation = CGRuntime::getInputLocation();
		}
		int iBegin;
		int iEnd;
		_boundaries.computeBoundaries(visibility, iBegin, iEnd);
		int i;
		for (i = 0; i < iBegin; i++) {
			result = GrfBlock::executeInternal(visibility);
			if (result != NO_INTERRUPTION) {
				if (result == CONTINUE_INTERRUPTION) {
					if (i != iBegin - 1) result = BREAK_INTERRUPTION;
				}
				break;
			}
			iLocation = CGRuntime::getInputLocation();
		}
		if (result == NO_INTERRUPTION) {
			int iInfiniteLoopCounter = 0;
			for (i = iEnd - iBegin; i != 0; i--) {
				result = GrfBlock::executeInternal(visibility);
				if (result != NO_INTERRUPTION) {
					if (result != CONTINUE_INTERRUPTION) {
						CGRuntime::setInputLocation(iLocation);
						if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
						result = NO_INTERRUPTION;
					}
					break;
				}
				register int iNewLocation = CGRuntime::getInputLocation();
/*				if (iLocation == iNewLocation) {
					iInfiniteLoopCounter++;
					if (iInfiniteLoopCounter > 10) throw UtlException("infinite loop expected: the parse cursor hasn't move after repeating a sequence 10 times");
				}
*/				iLocation = iNewLocation;
			}
		}
		if (result == NO_INTERRUPTION) {
			if (!_listOfConstants.empty()) {
				bool bMatch = false;
				std::string sTextToMatch = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - iNotEmptyLocation);
				for (std::vector<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
					bMatch = ((*i) == sTextToMatch);
					if (bMatch) break;
				}
				if (!bMatch) {
					BNF_SYMBOL_HAS_FAILED
				}
			}
			if (_pVariableToAssign != NULL) {
				DtaScriptVariable* pVariable = visibility.getVariable(*_pVariableToAssign);
				std::string sText = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - iNotEmptyLocation);
				if (_iClauseReturnType == BNFClause::LIST_RETURN_TYPE) {
					pVariable->pushItem(sText);
				} else {
					if (_bConcatVariable) pVariable->concatValue(sText.c_str());
					else pVariable->setValue(sText.c_str());
				}
			}
		} else if (result == CONTINUE_INTERRUPTION) {
			if ((iBegin != 1) || (iEnd != 1)) result = NO_INTERRUPTION;
		} else {
			BNF_SYMBOL_HAS_FAILED
		}
		return result;
	}

	void BNFMultiplicity::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (!_boundaries.bConstantBoundaries) throw UtlException("BNFMultiplicity::compileCpp(): variable boundaries not handled yet!");
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			CW_BODY_INDENT << "// " << toString();
			CW_BODY_ENDL;
			int iCursor = theCompilerEnvironment.newCursor();
			theCompilerEnvironment.pushLastRepeatCursor(iCursor);
			CW_BODY_INDENT << "int _compilerClauseCursor_" << iCursor << " = CGRuntime::getInputLocation();";
			CW_BODY_ENDL;
			char tcImplicitCopy[64]; // populated only if ':var' or ':{...}'
			char tcNotEmptyLocation[64]; // populated only if ':var' or ':{...}'
			if (!_listOfConstants.empty() || (_pVariableToAssign != NULL)) {
				sprintf(tcImplicitCopy, "_compilerClauseImplicitCopy_%d", iCursor);
				CW_BODY_INDENT << "int " << tcImplicitCopy << " = theEnvironment.skipEmptyChars();";
				CW_BODY_ENDL;
				sprintf(tcNotEmptyLocation, "_compilerClauseNotEmptyLocation_%d", iCursor);
				CW_BODY_INDENT << "int " << tcNotEmptyLocation << " = CGRuntime::getInputLocation();";
				CW_BODY_ENDL;
			}
			if (_boundaries.choice.constant._iEnd != 1) {
				if ((_boundaries.choice.constant._iBegin > 0) || (_boundaries.choice.constant._iEnd > 0)) {
					CW_BODY_INDENT << "int _compilerClause_iteration_" << iCursor << " = 0;";
					CW_BODY_ENDL;
				}
				CW_BODY_INDENT << "do {";
				CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
			}
			GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
			if (_boundaries.choice.constant._iEnd != 1) {
				CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "	CGRuntime::setInputLocation(_compilerClauseCursor_" << iCursor << ");";
				CW_BODY_ENDL;
				if (_boundaries.choice.constant._iBegin == 0) {
					CW_BODY_INDENT << "	_compilerClauseSuccess = true;";
					CW_BODY_ENDL;
					CW_BODY_INDENT << "	break;";
					CW_BODY_ENDL;
				}
				CW_BODY_INDENT << "} else {";
				CW_BODY_ENDL;
				if ((_boundaries.choice.constant._iBegin > 0) || (_boundaries.choice.constant._iEnd > 0)) {
					CW_BODY_INDENT << "	++_compilerClause_iteration_" << iCursor << ";";
					CW_BODY_ENDL;
				}
				CW_BODY_INDENT << "	";
				if (_boundaries.choice.constant._iBegin > 1) CW_BODY_STREAM << "if (_compilerClause_iteration_" << iCursor << " >= " << _boundaries.choice.constant._iBegin << ") ";
				CW_BODY_STREAM << "_compilerClauseCursor_" << iCursor << " = CGRuntime::getInputLocation();";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "}";
				CW_BODY_ENDL;
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_INDENT << "} while (_compilerClauseSuccess";
				if (_boundaries.choice.constant._iEnd > 0) CW_BODY_STREAM << " && (_compilerClause_iteration_" << iCursor << " < " << _boundaries.choice.constant._iEnd << ")";
				CW_BODY_STREAM << ");";
				CW_BODY_ENDL;
				if (_boundaries.choice.constant._iBegin > 0) {
					CW_BODY_INDENT << "if (!_compilerClauseSuccess && (_compilerClause_iteration_" << iCursor << " >= " << _boundaries.choice.constant._iBegin << ")) _compilerClauseSuccess = true;";
					CW_BODY_ENDL;
				}
			} else if (_boundaries.choice.constant._iBegin == 0) {
				CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "	CGRuntime::setInputLocation(_compilerClauseCursor_" << iCursor << ");";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "	_compilerClauseSuccess = true;";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "}";
				CW_BODY_ENDL;
			} else {
				CW_BODY_INDENT << "if (!_compilerClauseSuccess) CGRuntime::setInputLocation(_compilerClauseCursor_" << iCursor << ");";
				CW_BODY_ENDL;
			}
			if (_bContinue) {
				CW_BODY_INDENT << "if (!_compilerClauseSuccess) CGRuntime::throwBNFExecutionError(";
				CW_BODY_STREAM.writeString(toString());
				CW_BODY_STREAM << ");";CW_BODY_ENDL;
			}
			if (theCompilerEnvironment.popLastRepeatCursor()) {
				CW_BODY_INDENT << "_compilerRepeatLabel" << iCursor << ":";CW_BODY_ENDL;
			}
			if (_pVariableToAssign != NULL) {
				if (!_bContinue) {
					CW_BODY_INDENT << "if (_compilerClauseSuccess) {";CW_BODY_ENDL;
					theCompilerEnvironment.incrementIndentation();
				}
				CW_BODY_INDENT << "std::string sText = CGRuntime::getLastReadChars(CGRuntime::getInputLocation() - " << tcNotEmptyLocation << ");";CW_BODY_ENDL;
				_pBNFScript->compileCppBNFAssignment(theCompilerEnvironment, _iClauseReturnType, *_pVariableToAssign, _bConcatVariable, "sText");
				if (!_bContinue) {
					theCompilerEnvironment.decrementIndentation();
					CW_BODY_INDENT << "}";CW_BODY_ENDL;
				}
			}
		}
	}

	std::string BNFMultiplicity::toString() const {
		std::string sString;
		if (_bContinue) sString = "#continue ";
		sString += "[";
		for (std::vector<GrfCommand*>::const_iterator i = getCommands().begin(); i != getCommands().end(); ++i) {
			if (i != getCommands().begin()) sString += " ";
			sString += (*i)->toString();
		}
		sString += "]";
		sString += _boundaries.toString();
		return sString + DtaBNFScript::assignmentToString(_pVariableToAssign, _bConcatVariable);
	}
}
