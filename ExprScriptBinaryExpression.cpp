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
#include "ExprScriptBinaryExpression.h"


namespace CodeWorker {
	ExprScriptBinaryExpression::~ExprScriptBinaryExpression() {
		delete _pLeft;
		delete _pRight;
	}



	ExprScriptConcatenation::~ExprScriptConcatenation() {
	}

	std::string ExprScriptConcatenation::getValue(DtaScriptVariable& visibility) const {
		std::string sValue = getLeft()->getValue(visibility);
		sValue += getRight()->getValue(visibility);
		return sValue;
	}

	std::string ExprScriptConcatenation::toString() const {
		return getLeft()->toString() + " + " + getRight()->toString();
	}

	EXPRESSION_RETURN_TYPE ExprScriptConcatenation::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		ExprScriptConcatenation::compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	bool ExprScriptConcatenation::compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		getLeft()->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << " + ";
		getRight()->compileCppString(theCompilerEnvironment);
		return true;
	}



	ExprScriptArithmetic::~ExprScriptArithmetic() {}

	std::string ExprScriptArithmetic::getValue(DtaScriptVariable& visibility) const {
		if (_cOperator == '%') return CGRuntime::toString(getIntValue(visibility));
		return CGRuntime::toString(getDoubleValue(visibility));
	}

	int ExprScriptArithmetic::getIntValue(DtaScriptVariable& visibility) const {
		int iValue1 = getLeft()->getIntValue(visibility);
		int iValue2 = getRight()->getIntValue(visibility);
		switch(_cOperator) {
			case '+' : return iValue1 + iValue2;
			case '-' : return iValue1 - iValue2;
			case '*' : return iValue1 * iValue2;
			case '/' : return iValue1 / iValue2;
			case '%' : return iValue1 % iValue2;
		}
		throw UtlException("ExprScriptArithmetic::getDoubleValue(): undefined arithmetic type");
	}

	double ExprScriptArithmetic::getDoubleValue(DtaScriptVariable& visibility) const {
		if (_cOperator == '%') return (double) getIntValue(visibility);
		double dValue1 = getLeft()->getDoubleValue(visibility);
		double dValue2 = getRight()->getDoubleValue(visibility);
		switch(_cOperator) {
			case '+' : return dValue1 + dValue2;
			case '-' : return dValue1 - dValue2;
			case '*' : return dValue1 * dValue2;
			case '/' : return dValue1 / dValue2;
		}
		throw UtlException("ExprScriptArithmetic::getDoubleValue(): undefined arithmetic type");
	}

	std::string ExprScriptArithmetic::toString() const {
		return "(" + getLeft()->toString() + std::string(1, _cOperator) + getRight()->toString() + ")";
	}

	EXPRESSION_RETURN_TYPE ExprScriptArithmetic::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	bool ExprScriptArithmetic::compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (_cOperator == '%') {
			CW_BODY_STREAM << "(";
			getLeft()->compileCppInt(theCompilerEnvironment);
			CW_BODY_STREAM << " " << _cOperator << " ";
			getRight()->compileCppInt(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		} else {
			CW_BODY_STREAM << "(";
			getLeft()->compileCppDouble(theCompilerEnvironment);
			CW_BODY_STREAM << " " << _cOperator << " ";
			getRight()->compileCppDouble(theCompilerEnvironment);
			CW_BODY_STREAM << ")";
		}
		return true;
	}



	ExprScriptShift::~ExprScriptShift() {}

	std::string ExprScriptShift::getValue(DtaScriptVariable& visibility) const {
		return CGRuntime::toString(getIntValue(visibility));
	}

	int ExprScriptShift::getIntValue(DtaScriptVariable& visibility) const {
		int iValue1 = getLeft()->getIntValue(visibility);
		int iValue2 = getRight()->getIntValue(visibility);
		switch(_cOperator) {
			case '<' : return iValue1 << iValue2;
			case '>' : return iValue1 >> iValue2;
		}
		throw UtlException("ExprScriptShift::getIntValue(): undefined Shift type");
	}

	std::string ExprScriptShift::toString() const {
		return "(" + getLeft()->toString() + std::string(2, _cOperator) + getRight()->toString() + ")";
	}

	EXPRESSION_RETURN_TYPE ExprScriptShift::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	bool ExprScriptShift::compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(";
		getLeft()->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << " " << _cOperator << _cOperator << " ";
		getRight()->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}



	ExprScriptComparison::~ExprScriptComparison() {}

	std::string ExprScriptComparison::getValue(DtaScriptVariable& visibility) const {
		std::string sValue;
		std::string sValue1 = getLeft()->getValue(visibility);
		std::string sValue2 = getRight()->getValue(visibility);
		switch(_cOperator) {
			case '<' : if (sValue1.compare(sValue2) < 0) sValue = "true";break;
			case 'i' : if (sValue1.compare(sValue2) <= 0) sValue = "true";break;
			case '=' : if (sValue1 == sValue2) sValue = "true";break;
			case '!' : if (sValue1 != sValue2) sValue = "true";break;
			case 's' : if (sValue1.compare(sValue2) >= 0) sValue = "true";break;
			case '>' : if (sValue1.compare(sValue2) > 0) sValue = "true";break;
			default: throw UtlException("ExprScriptComparison::getValue(): undefined comparison type");
		}
		return sValue;
	}

	EXPRESSION_RETURN_TYPE ExprScriptComparison::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	std::string ExprScriptComparison::toString() const {
		std::string sText;
		std::string sOperator;
		switch(_cOperator) {
			case '<' : sOperator = "<";break;
			case 'i' : sOperator = "<=";break;
			case '=' : sOperator = "==";break;
			case '!' : sOperator = "!=";break;
			case 's' : sOperator = ">=";break;
			case '>' : sOperator = ">";break;
			default: throw UtlException("ExprScriptComparison::toString(): undefined comparison operator");
		}
		sText = "(" + getLeft()->toString() + ") " + sOperator + " (" + getRight()->toString() + ")";
		return sText;
	}

	bool ExprScriptComparison::compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		std::string sOperator;
		switch(_cOperator) {
			case '<' : if (bNegative) sOperator = ">="; else sOperator = "<";break;
			case 'i' : if (bNegative) sOperator = ">";  else sOperator = "<=";break;
			case '=' : if (bNegative) sOperator = "!="; else sOperator = "==";break;
			case '!' : if (bNegative) sOperator = "=="; else sOperator = "!=";break;
			case 's' : if (bNegative) sOperator = "<";  else sOperator = ">=";break;
			case '>' : if (bNegative) sOperator = "<="; else sOperator = ">";break;
			default: throw UtlException("ExprScriptComparison::compileCppBoolean(): undefined comparison type");
		}
		EXPRESSION_RETURN_TYPE returnType = getLeft()->compileCpp(theCompilerEnvironment);
		if (returnType == TREE_TYPE) {
			CW_BODY_STREAM << ".getStringValue()";
		}
		CW_BODY_STREAM << " " << sOperator << " ";
		switch(returnType) {
			case BOOL_TYPE:   getRight()->compileCppBoolean(theCompilerEnvironment, false);break;
			case INT_TYPE:    getRight()->compileCppInt(theCompilerEnvironment);break;
			case DOUBLE_TYPE: getRight()->compileCppDouble(theCompilerEnvironment);break;
			case STRING_TYPE: getRight()->compileCppString(theCompilerEnvironment);break;
			case ULONG_TYPE:  getRight()->compileCppUnsignedLong(theCompilerEnvironment);break;
			case USHORT_TYPE: getRight()->compileCppUnsignedShort(theCompilerEnvironment);break;
			case TREE_TYPE:
				getRight()->compileCppString(theCompilerEnvironment);
				break;
			default:
				getRight()->compileCppString(theCompilerEnvironment);
		}
		return true;
	}



	ExprScriptNumericComparison::~ExprScriptNumericComparison() {}

	std::string ExprScriptNumericComparison::getValue(DtaScriptVariable& visibility) const {
		std::string sValue;
		double dValue1 = getLeft()->getDoubleValue(visibility);
		double dValue2 = getRight()->getDoubleValue(visibility);
		switch(_cOperator) {
			case '<' : if (dValue1  < dValue2) sValue = "true";break;
			case 'i' : if (dValue1 <= dValue2) sValue = "true";break;
			case '=' : if (dValue1 == dValue2) sValue = "true";break;
			case '!' : if (dValue1 != dValue2) sValue = "true";break;
			case 's' : if (dValue1 >= dValue2) sValue = "true";break;
			case '>' : if (dValue1  > dValue2) sValue = "true";break;
			default: throw UtlException("ExprScriptNumericComparison::getValue(): undefined comparison type");
		}
		return sValue;
	}

	std::string ExprScriptNumericComparison::toString() const {
		return "$" + ExprScriptComparison::toString() + "$";
	}

	bool ExprScriptNumericComparison::compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		std::string sOperator;
		switch(_cOperator) {
			case '<' : if (bNegative) sOperator = ">="; else sOperator = "<";break;
			case 'i' : if (bNegative) sOperator = ">";  else sOperator = "<=";break;
			case '=' : if (bNegative) sOperator = "!="; else sOperator = "==";break;
			case '!' : if (bNegative) sOperator = "=="; else sOperator = "!=";break;
			case 's' : if (bNegative) sOperator = "<";  else sOperator = ">=";break;
			case '>' : if (bNegative) sOperator = "<="; else sOperator = ">";break;
			default: throw UtlException("ExprScriptNumericComparison::compileCppBoolean(): undefined comparison type");
		}
		getLeft()->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << " " << sOperator << " ";
		getRight()->compileCppDouble(theCompilerEnvironment);
		return true;
	}


	ExprScriptTernaryOperator::~ExprScriptTernaryOperator() {
		delete _pCondition;
	}

	std::string ExprScriptTernaryOperator::getValue(DtaScriptVariable& visibility) const {
		std::string sCondition = _pCondition->getValue(visibility);
		if (sCondition.empty()) return getRight()->getValue(visibility);
		return getLeft()->getValue(visibility);
	}

	std::string ExprScriptTernaryOperator::toString() const {
		return "(" + _pCondition->toString() + ") ? " + getLeft()->toString() + " : " + getRight()->toString();
	}

	EXPRESSION_RETURN_TYPE ExprScriptTernaryOperator::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	bool ExprScriptTernaryOperator::compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "((";
		_pCondition->compileCppBoolean(theCompilerEnvironment, false);
		CW_BODY_STREAM << ") ? (";
		getLeft()->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ") : (";
		getRight()->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << "))";
		return true;
	}



	ExprScriptBoolean::~ExprScriptBoolean() {
	}

	std::string ExprScriptBoolean::getValue(DtaScriptVariable& visibility) const {
		std::string sValue;
		std::string sValue1 = getLeft()->getValue(visibility);
		switch(_cOperator) {
			case '&' : if (sValue1.empty()) return "";break;
			case '|' : if (!sValue1.empty()) return "true";break;
		}
		std::string sValue2 = getRight()->getValue(visibility);
		switch(_cOperator) {
			case '&' : if (!sValue2.empty()) sValue = "true";break;
			case '|' : if (!sValue2.empty()) sValue = "true";break;
			case '^' : if (sValue1.empty() != sValue2.empty()) sValue = "true";break;
			default: throw UtlException("ExprScriptBoolean::getValue(): undefined comparison type");
		}
		return sValue;
	}

	std::string ExprScriptBoolean::toString() const {
		std::string sText;
		std::string sOperator;
		switch(_cOperator) {
			case '&' : sOperator = "&&";break;
			case '|' : sOperator = "||";break;
			case '^' : sOperator = "!=";break;
			default: throw UtlException("ExprScriptBoolean::toString(): undefined boolean operator");
		}
		ExprScriptBinaryExpression* pBinary = dynamic_cast<ExprScriptBinaryExpression*>(getLeft());
		ExprScriptBoolean* pBoolean = dynamic_cast<ExprScriptBoolean*>(pBinary);
		bool bParenthesis = (pBinary != NULL) && ((pBoolean == NULL) || (pBoolean->_cOperator != _cOperator) || (_cOperator == '^'));
		if (bParenthesis) sText = "(";
		sText += getLeft()->toString();
		if (bParenthesis) sText += ")";
		pBinary = dynamic_cast<ExprScriptBinaryExpression*>(getRight());
		pBoolean = dynamic_cast<ExprScriptBoolean*>(pBinary);
		bParenthesis = (pBinary != NULL) && ((pBoolean == NULL) || (pBoolean->_cOperator != _cOperator) || (_cOperator == '^'));
		sText += " " + sOperator + " ";
		if (bParenthesis) sText += "(";
		sText += getRight()->toString();
		if (bParenthesis) sText += ")";
		return sText;
	}

	EXPRESSION_RETURN_TYPE ExprScriptBoolean::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	bool ExprScriptBoolean::compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		std::string sOperator;
		switch(_cOperator) {
			case '&' : if (bNegative) sOperator = "||"; else sOperator = "&&";break;
			case '|' : if (bNegative) sOperator = "&&"; else sOperator = "||";break;
			case '^' : if (bNegative) sOperator = "=="; else sOperator = "!=";break;
			default: throw UtlException("ExprScriptBoolean::compileCppBoolean(): undefined boolean operator");
		}
		ExprScriptBinaryExpression* pBinary = dynamic_cast<ExprScriptBinaryExpression*>(getLeft());
		ExprScriptBoolean* pBoolean = dynamic_cast<ExprScriptBoolean*>(pBinary);
		bool bParenthesis = (pBinary != NULL) && ((pBoolean == NULL) || (pBoolean->_cOperator != _cOperator) || (_cOperator == '^'));
		if (bParenthesis) CW_BODY_STREAM << "(";
		getLeft()->compileCppBoolean(theCompilerEnvironment, bNegative);
		if (bParenthesis) CW_BODY_STREAM << ")";
		pBinary = dynamic_cast<ExprScriptBinaryExpression*>(getRight());
		pBoolean = dynamic_cast<ExprScriptBoolean*>(pBinary);
		bParenthesis = (pBinary != NULL) && ((pBoolean == NULL) || (pBoolean->_cOperator != _cOperator) || (_cOperator == '^'));
		CW_BODY_STREAM << " " << sOperator << " ";
		if (bParenthesis) CW_BODY_STREAM << "(";
		getRight()->compileCppBoolean(theCompilerEnvironment, bNegative);
		if (bParenthesis) CW_BODY_STREAM << ")";
		return true;
	}


	ExprScriptInSet::~ExprScriptInSet() {
		delete _pLeft;
	}

	std::string ExprScriptInSet::getValue(DtaScriptVariable& visibility) const {
		std::string sValue = _pLeft->getValue(visibility);
		if (_listOfConstants.find(sValue) != _listOfConstants.end()) return "true";
		return "";
	}

	EXPRESSION_RETURN_TYPE ExprScriptInSet::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		int iCursor = theCompilerEnvironment.newCursor();
		char tcInSet[64];
		sprintf(tcInSet, "_compiler_internal_inset%d", iCursor);
		// definition
		ScpStream* pOwner = 0;
		CW_BODY_STREAM.insertText("bool " + std::string(tcInSet) + "(const std::string& sValue) {\n", CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner));
		CW_BODY_STREAM.insertText("\tstatic std::set<std::string> theSet;\n", CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner));
		CW_BODY_STREAM.insertText("\tif (theSet.empty()) {\n", CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner));
		for (std::set<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); ++i) {
			CW_BODY_STREAM.insertText("\t\ttheSet.insert(\"" + CGRuntime::composeCLikeString(*i) + "\");\n", CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner));
		}
		CW_BODY_STREAM.insertText("\t}\n\treturn (theSet.find(sValue) != theSet.end());\n}\n", CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner));
		// call
		CW_BODY_STREAM << tcInSet << "(";
		_pLeft->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		CW_BODY_ENDL;
		return BOOL_TYPE;
	}

	std::string ExprScriptInSet::toString() const {
		std::string sText = '(' + _pLeft->toString() + " in " + '{';
		for (std::set<std::string>::const_iterator i = _listOfConstants.begin(); i != _listOfConstants.end(); i++) {
			if (i != _listOfConstants.begin()) sText += ", ";
			sText += "\"" + CGRuntime::composeCLikeString(*i) + "\"";
		}
		return sText + '}';
	}
}
