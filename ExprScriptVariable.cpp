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
#pragma warning(disable: 4786)
#endif

#include "UtlException.h"
#include "DtaScriptVariable.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "DynPackage.h" // for 'ExprModuleScriptVariable'
#include "ExprScriptVariable.h"


namespace CodeWorker {
	unsigned char ExprScriptVariable::ATTRIBUTE_ACCESS = 1;
	unsigned char ExprScriptVariable::ARRAY_ACCESS = 2;
	unsigned char ExprScriptVariable::ARRAY_KEY_ACCESS = 6;
	unsigned char ExprScriptVariable::ARRAY_POSITION_ACCESS = 10;
	unsigned char ExprScriptVariable::NEXT_ACCESS = 16;
	unsigned char ExprScriptVariable::EVALUATION_ACCESS = 32;

	ExprScriptVariable::~ExprScriptVariable() {
		if (_pNext != NULL) delete _pNext;
		if (_pExpression != NULL) delete _pExpression;
	}

	std::string ExprScriptVariable::getValue(DtaScriptVariable& visibility) const {
		DtaScriptVariable* pVariable = visibility.getExistingVariable(*this);
		if (pVariable == NULL) return "";
		const char* tcValue = pVariable->getValue();
		if (tcValue == NULL) return "";
		return tcValue;
	}

	ExprScriptExpression* ExprScriptVariable::clone() const {
		if ((_pNext != NULL) || (_pExpression != NULL) || (_pPackage != NULL) || (_iAccess != ATTRIBUTE_ACCESS)) {
			throw UtlException("internal error: ExprScriptVariable::clone() not implemented completely!");
		}
		return new ExprScriptVariable(_sName.c_str());
	}

	EXPRESSION_RETURN_TYPE ExprScriptVariable::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (isEvaluation()) {
			theCompilerEnvironment.hasEvaluatedExpressionInScope(true);
			if (theCompilerEnvironment.getCurrentClause() != NULL) CW_BODY_STREAM << "_compilerClauseScope.getEvaluatedNode(";
			else CW_BODY_STREAM << "CGRuntime::getThisTree().getEvaluatedNode(";
			_pExpression->compileCppString(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		} else if (_sName == "this") {
			CW_BODY_STREAM << "CGRuntime::getThisTree()";
		} else if (_sName == "project") {
			CW_BODY_STREAM << "CGRuntime::getRootTree()";
		} else if (_sName == "null") {
			CW_BODY_STREAM << "NULL";
		} else if (theCompilerEnvironment.existVariable(_sName)) {
			CW_BODY_STREAM << theCompilerEnvironment.convertToCppVariable(_sName);
		} else if (theCompilerEnvironment.getCurrentClause() != NULL) {
			if (theCompilerEnvironment.isClauseReturnValue(_sName)) {
				CW_BODY_STREAM << "_compilerClause_returnValue";
			} else {
				CW_BODY_STREAM << "_compilerClauseScope.getNode(\"" << _sName << "\")";
			}
		} else {
			CW_BODY_STREAM << "CGRuntime::getThisTree().getNode(\"" << _sName << "\")";
		}
		if (isArray()) {
			compileCppFollowing(theCompilerEnvironment, true);
		} else if (_pNext != NULL) {
			_pNext->compileCppFollowing(theCompilerEnvironment);
		}
		return TREE_TYPE;
	}

	bool ExprScriptVariable::compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (!bNegative) CW_BODY_STREAM << "!";
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ".getStringValue().empty()";
		return true;
	}

	bool ExprScriptVariable::compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ".getIntValue()";
		return true;
	}

	bool ExprScriptVariable::compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ".getDoubleValue()";
		return true;
	}

	bool ExprScriptVariable::compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ".getStringValue()";
		return true;
	}

	void ExprScriptVariable::compileCppFollowing(CppCompilerEnvironment& theCompilerEnvironment, bool bFirstCall) const {
		if (!_sName.empty() && !bFirstCall) {
			CW_BODY_STREAM << ".getNode(\"" << _sName << "\")";
		}
		if (isEvaluation()) {
			theCompilerEnvironment.hasEvaluatedExpressionInScope(true);
			CW_BODY_STREAM << ".getEvaluatedNode(";
			_pExpression->compileCppString(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		} else if (isArrayKey()) {
			CW_BODY_STREAM << ".getArrayNodeFromKey(";
			_pExpression->compileCppString(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		} else if (isArrayPosition()) {
			ExprScriptConstant* pConstant = dynamic_cast<ExprScriptConstant*>(_pExpression);
			bool bResolved = (pConstant != NULL);
			if (pConstant != NULL) {
				std::string sConstant = pConstant->getConstant();
				if ((sConstant == "front") || (sConstant == "0")) CW_BODY_STREAM << ".getFirstArrayNode()";
				else if (sConstant == "back") CW_BODY_STREAM << ".getLastArrayNode()";
				else if (sConstant == "parent") CW_BODY_STREAM << ".getParentNode()";
				else if (sConstant == "root") CW_BODY_STREAM << ".getRootNode()";
				else bResolved = false;
			}
			if (!bResolved) {
				CW_BODY_STREAM << ".getArrayNodeFromPosition(";
				_pExpression->compileCppInt(theCompilerEnvironment);
				CW_BODY_STREAM << ")";
			}
		}
		if (_pNext != NULL) {
			_pNext->compileCppFollowing(theCompilerEnvironment);
		}
	}

	void ExprScriptVariable::compileCppForGetOrCreateLocal(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (isEvaluation()) {
			theCompilerEnvironment.hasEvaluatedExpressionInScope(true);
			if (theCompilerEnvironment.getCurrentClause() != NULL) CW_BODY_STREAM << "_compilerClauseScope.getOrCreateLocalEvaluatedNode(";
			else CW_BODY_STREAM << "CGRuntime::getThisTree().getOrCreateLocalEvaluatedNode(";
			_pExpression->compileCppString(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		} else if (_sName == "this") {
			CW_BODY_STREAM << "CGRuntime::getThisTree()";
		} else if (_sName == "project") {
			CW_BODY_STREAM << "CGRuntime::getRootTree()";
		} else if (_sName == "null") {
			CW_BODY_STREAM << "NULL";
		} else if (theCompilerEnvironment.existVariable(_sName)) {
			CW_BODY_STREAM << theCompilerEnvironment.convertToCppVariable(_sName);
		} else if (theCompilerEnvironment.getCurrentClause() != NULL) {
			if (theCompilerEnvironment.isClauseReturnValue(_sName)) {
				CW_BODY_STREAM << "_compilerClause_returnValue";
			} else {
				CW_BODY_STREAM << "_compilerClauseScope.getOrCreateLocalNode(\"" << _sName << "\")";
			}
		} else {
			CW_BODY_STREAM << "CGRuntime::getThisTree().getOrCreateLocalNode(\"" << _sName << "\")";
		}
		if (isArray()) {
			compileCppFollowingForSet(theCompilerEnvironment, true);
		} else if (_pNext != NULL) {
			_pNext->compileCppFollowingForSet(theCompilerEnvironment);
		}
	}

	void ExprScriptVariable::compileCppForSet(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (isEvaluation()) {
			theCompilerEnvironment.hasEvaluatedExpressionInScope(true);
			if (theCompilerEnvironment.getCurrentClause() != NULL) CW_BODY_STREAM << "_compilerClauseScope.insertClassicalEvaluatedNode(";
			else CW_BODY_STREAM << "CGRuntime::getThisTree().insertEvaluatedNode(";
			_pExpression->compileCppString(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		} else if (_sName == "this") {
			CW_BODY_STREAM << "CGRuntime::getThisTree()";
		} else if (_sName == "project") {
			CW_BODY_STREAM << "CGRuntime::getRootTree()";
		} else if (_sName == "null") {
			CW_BODY_STREAM << "NULL";
		} else if (theCompilerEnvironment.existVariable(_sName)) {
			CW_BODY_STREAM << theCompilerEnvironment.convertToCppVariable(_sName);
		} else if (theCompilerEnvironment.getCurrentClause() != NULL) {
			if (theCompilerEnvironment.isClauseReturnValue(_sName)) {
				CW_BODY_STREAM << "_compilerClause_returnValue";
			} else {
				CW_BODY_STREAM << "_compilerClauseScope.insertClassicalNode(\"" << _sName << "\")";
			}
		} else {
			CW_BODY_STREAM << "CGRuntime::getThisTree().insertNode(\"" << _sName << "\")";
		}
		if (isArray()) {
			compileCppFollowingForSet(theCompilerEnvironment, true);
		} else if (_pNext != NULL) {
			_pNext->compileCppFollowingForSet(theCompilerEnvironment);
		}
	}

	void ExprScriptVariable::compileCppForBNFSet(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (isEvaluation()) {
			theCompilerEnvironment.hasEvaluatedExpressionInScope(true);
			CW_BODY_STREAM << "_compilerClauseScope.insertBNFEvaluatedNode(";
			_pExpression->compileCppString(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		} else if (_sName == "this") {
			CW_BODY_STREAM << "CGRuntime::getThisTree()";
		} else if (_sName == "project") {
			CW_BODY_STREAM << "CGRuntime::getRootTree()";
		} else if (_sName == "null") {
			CW_BODY_STREAM << "NULL";
		} else if (theCompilerEnvironment.existVariable(_sName)) {
			CW_BODY_STREAM << theCompilerEnvironment.convertToCppVariable(_sName);
		} else if (theCompilerEnvironment.getCurrentClause() != NULL) {
			if (theCompilerEnvironment.isClauseReturnValue(_sName)) {
				CW_BODY_STREAM << "_compilerClause_returnValue";
			} else {
				CW_BODY_STREAM << "_compilerClauseScope.getOrCreateLocalNode(\"" << _sName << "\")";
			}
		} else {
			CW_BODY_STREAM << "_compilerClauseScope.getOrCreateLocalNode(\"" << _sName << "\")";
		}
		if (isArray()) {
			compileCppFollowingForSet(theCompilerEnvironment, true);
		} else if (_pNext != NULL) {
			_pNext->compileCppFollowingForSet(theCompilerEnvironment);
		}
	}

	void ExprScriptVariable::compileCppFollowingForSet(CppCompilerEnvironment& theCompilerEnvironment, bool bFirstCall) const {
		if (!_sName.empty() && !bFirstCall) {
			CW_BODY_STREAM << ".insertNode(\"" << _sName << "\")";
		}
		if (isEvaluation()) {
			theCompilerEnvironment.hasEvaluatedExpressionInScope(true);
			CW_BODY_STREAM << ".insertEvaluatedNode(";
			_pExpression->compileCppString(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		} else if (isArrayKey()) {
			CW_BODY_STREAM << ".insertArrayNodeFromKey(";
			_pExpression->compileCppString(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		} else if (isArrayPosition()) {
			ExprScriptConstant* pConstant = dynamic_cast<ExprScriptConstant*>(_pExpression);
			bool bResolved = (pConstant != NULL);
			if (pConstant != NULL) {
				std::string sConstant = pConstant->getConstant();
				if ((sConstant == "front") || (sConstant == "0")) CW_BODY_STREAM << ".getFirstArrayNode()";
				else if (sConstant == "back") CW_BODY_STREAM << ".getLastArrayNode()";
				else if (sConstant == "parent") CW_BODY_STREAM << ".getParentNode()";
				else if (sConstant == "root") CW_BODY_STREAM << ".getRootNode()";
				else bResolved = false;
			}
			if (!bResolved) {
				CW_BODY_STREAM << ".insertArrayNodeFromPosition(";
				_pExpression->compileCppInt(theCompilerEnvironment);
				CW_BODY_STREAM << ")";
			}
		}
		if (_pNext != NULL) {
			_pNext->compileCppFollowingForSet(theCompilerEnvironment);
		}
	}

	bool ExprScriptVariable::compileCppExpr(CppCompilerEnvironment& theCompilerEnvironment) const {
		char tcNumber[32];
		sprintf(tcNumber, "%d", CW_BODY_STREAM.getOutputLocation());
		std::string sVariable = "_compilerVariable_";
		sVariable += tcNumber;
		std::string sDeclaration = "\tstatic CppParsingTree_varexpr " + sVariable + "(\"" + toString() + "\");" + ScpStream::ENDL;
		CW_BODY_STREAM.insertText(sDeclaration, theCompilerEnvironment.getPointerToDeclarations());
		CW_BODY_STREAM << sVariable;
		return false;
	}

	std::string ExprScriptVariable::toString() const {
		std::string sText;
		if (isEvaluation()) {
			sText = "#evaluateVariable(" + _pExpression->toString() + ")";
		} else {
			if (_pPackage != NULL) sText = _pPackage->getName() + "::" + _sName;
			else sText = _sName;
			if (isArrayKey()) {
				sText += "[" + _pExpression->toString() + "]";
			} else if (isArrayPosition()) {
				std::string sPosition = _pExpression->toString();
				if ((sPosition == "front") || (sPosition == "back") || (sPosition == "parent") || (sPosition == "root")) sText += "#" + sPosition;
				else sText += "#[" + sPosition + "]";
			}
		}
		if (_pNext != NULL) {
			sText += "." + _pNext->toString();
		}
		return sText;
	}


	ExprScriptAlienVariable::~ExprScriptAlienVariable() {}

	std::string ExprScriptAlienVariable::getValue(DtaScriptVariable& visibility) const {
		throw UtlException("internal error: an alien variable expression shouldn't be executed in CodeWorker!");
	}

	ExprScriptExpression* ExprScriptAlienVariable::clone() const {
		return new ExprScriptAlienVariable(_sName.c_str(), _sTargetLanguage);
	}

	EXPRESSION_RETURN_TYPE ExprScriptAlienVariable::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "TARGET_LANGUAGE_VARIABLE(" << _sName << ")";
		return ALIEN_TYPE;
	}

	bool ExprScriptAlienVariable::compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		compileCpp(theCompilerEnvironment);
		return true;
	}

	bool ExprScriptAlienVariable::compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
		return true;
	}

	bool ExprScriptAlienVariable::compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
		return true;
	}

	bool ExprScriptAlienVariable::compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
		return true;
	}

	void ExprScriptAlienVariable::compileCppFollowing(CppCompilerEnvironment& theCompilerEnvironment, bool bFirstCall) const {
		throw UtlException("internal error: an alien variable expression can't be navigable!");
	}

	void ExprScriptAlienVariable::compileCppForGetOrCreateLocal(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
	}

	void ExprScriptAlienVariable::compileCppForSet(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
	}

	void ExprScriptAlienVariable::compileCppForBNFSet(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
	}

	void ExprScriptAlienVariable::compileCppFollowingForSet(CppCompilerEnvironment& theCompilerEnvironment, bool bFirstCall) const {
		compileCpp(theCompilerEnvironment);
	}

	bool ExprScriptAlienVariable::compileCppExpr(CppCompilerEnvironment& theCompilerEnvironment) const {
		throw UtlException("internal error: an alien variable expression can't be translated to a CodeWorker expression!");
	}

	std::string ExprScriptAlienVariable::toString() const {
		return _sName;
	}
}
