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

#include <math.h>

#include "UtlException.h"
#include "DtaScriptVariable.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"
#include "ExprScriptVariable.h"
#include "ExprScriptExpression.h"
#include "DtaScript.h"


namespace CodeWorker {
	ExprScriptExpression::~ExprScriptExpression() {}

	DtaScriptVariable* ExprScriptExpression::getReferencedVariable() const { return NULL; }

	double ExprScriptExpression::getDoubleValue(DtaScriptVariable& visibility) const {
		std::string sValue = getValue(visibility);
		return atof(sValue.c_str());
	}

	int ExprScriptExpression::getIntValue(DtaScriptVariable& visibility) const {
		std::string sValue = getValue(visibility);
		return atoi(sValue.c_str());
	}

	EXPRESSION_RETURN_TYPE ExprScriptExpression::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "<unhandled expression>";
		return NO_TYPE;
	}

	ExprScriptExpression* ExprScriptExpression::clone() const {
		throw UtlException("internal error: ExprScriptExpression::clone() not implemented yet!");
	}

	bool ExprScriptExpression::compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		ScpStream* pOwner;
		int iInsertAreaLocation = CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner);
		int iLocation = CW_BODY_STREAM.getOutputLocation();
		if (bNegative) CW_BODY_STREAM << "!";
		switch(compileCpp(theCompilerEnvironment)) {
			case BOOL_TYPE:
				if (bNegative) CW_BODY_STREAM.insertText("!", iLocation + (CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner) - iInsertAreaLocation));
				break;
			case INT_TYPE:
			case DOUBLE_TYPE:
			case ULONG_TYPE:
			case USHORT_TYPE:
				CW_BODY_STREAM.insertText("(", iLocation + (CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner) - iInsertAreaLocation));
				if (bNegative) CW_BODY_STREAM << " == 0)";
				else CW_BODY_STREAM << " != 0)";
				break;
			case STRING_TYPE:
				if (bNegative) CW_BODY_STREAM.insertText("!CGRuntime::toBoolean(", iLocation + (CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner) - iInsertAreaLocation));
				else CW_BODY_STREAM.insertText("CGRuntime::toBoolean(", iLocation + (CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner) - iInsertAreaLocation));
				CW_BODY_STREAM << ")";
				break;
			case TREE_TYPE:
				if (!bNegative) CW_BODY_STREAM.insertText("!", iLocation + (CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner) - iInsertAreaLocation));
				CW_BODY_STREAM << ".getStringValue().empty()";
				break;
		}
		return false;
	}

	bool ExprScriptExpression::compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toInteger(";
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return false;
	}

	bool ExprScriptExpression::compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toDouble(";
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return false;
	}

	bool ExprScriptExpression::compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return false;
	}

	bool ExprScriptExpression::compileCppUnsignedLong(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toUnsignedLong(";
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return false;
	}

	bool ExprScriptExpression::compileCppUnsignedShort(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toUnsignedShort(";
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return false;
	}

	bool ExprScriptExpression::compileCppExpr(CppCompilerEnvironment& theCompilerEnvironment) const {
		char tcNumber[32];
		sprintf(tcNumber, "%d", CW_BODY_STREAM.getOutputLocation());
		std::string sVariable = "_compilerExpression_";
		sVariable += tcNumber;
		std::string sDeclaration = "\tstatic CppParsingTree_expr " + sVariable + "(\"" + toString() + "\");" + ScpStream::ENDL;
		CW_BODY_STREAM.insertText(sDeclaration, theCompilerEnvironment.getPointerToDeclarations());
		CW_BODY_STREAM << sVariable;
		return false;
	}

	std::string ExprScriptExpression::toString() const {
		throw UtlException("ExprScriptExpression::toString() <to redefine in subclasses>");
	}


	ExprScriptReference::~ExprScriptReference() {}
	std::string ExprScriptReference::getValue(DtaScriptVariable& visibility) const {
		if (_pRef != NULL) return _pRef->getValue(visibility);
		const char* tcValue = _pVariable->getValue();
		if (tcValue == NULL) return "";
		return tcValue;
	}

	DtaScriptVariable* ExprScriptReference::getReferencedVariable() const { return _pVariable; }



	ExprScriptUnaryMinus::~ExprScriptUnaryMinus() { delete _pExpression; }

	std::string ExprScriptUnaryMinus::getValue(DtaScriptVariable& visibility) const {
		return CGRuntime::toString(getDoubleValue(visibility));
	}

	double ExprScriptUnaryMinus::getDoubleValue(DtaScriptVariable& visibility) const {
		return -(_pExpression->getDoubleValue(visibility));
	}

	std::string ExprScriptUnaryMinus::toString() const {
		return "-(" + _pExpression->toString() + ")";
	}

	EXPRESSION_RETURN_TYPE ExprScriptUnaryMinus::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	bool ExprScriptUnaryMinus::compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "-(";
		_pExpression->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}


	ExprScriptBitwiseNot::~ExprScriptBitwiseNot() { delete _pExpression; }

	std::string ExprScriptBitwiseNot::getValue(DtaScriptVariable& visibility) const {
		return CGRuntime::toString(getIntValue(visibility));
	}

	int ExprScriptBitwiseNot::getIntValue(DtaScriptVariable& visibility) const {
		return ~(_pExpression->getIntValue(visibility));
	}

	std::string ExprScriptBitwiseNot::toString() const {
		return "~(" + _pExpression->toString() + ")";
	}

	EXPRESSION_RETURN_TYPE ExprScriptBitwiseNot::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	bool ExprScriptBitwiseNot::compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "~(";
		_pExpression->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}


	ExprScriptConstant::ExprScriptConstant(bool bValue) {
		if (bValue) _sValue = "true";
	}

	ExprScriptConstant::ExprScriptConstant(int iValue) {
		_sValue = CGRuntime::toString(iValue);
	}

	ExprScriptConstant::ExprScriptConstant(double dValue) {
		_sValue = CGRuntime::toString(dValue);
	}

	ExprScriptConstant::~ExprScriptConstant() {}
	std::string ExprScriptConstant::getValue(DtaScriptVariable&) const { return _sValue; }
	double ExprScriptConstant::getDoubleValue(DtaScriptVariable& visibility) const { return atof(_sValue.c_str()); }
	int ExprScriptConstant::getIntValue(DtaScriptVariable& visibility) const { return atoi(_sValue.c_str()); }

	ExprScriptExpression* ExprScriptConstant::clone() const {
		return new ExprScriptConstant(_sValue.c_str());
	}

	std::string ExprScriptConstant::toString() const {
		std::string sText;
		char* u = (char*) _sValue.c_str();
		sText = "\"";
		while (*u != '\0') {
			switch(*u) {
				case '\\': sText += "\\\\";break;
				case '\"': sText += "\\\"";break;
				case '\'': sText += "\\\'";break;
				case '\a': sText += "\\a";break;
				case '\b': sText += "\\b";break;
				case '\f': sText += "\\f";break;
				case '\n': sText += "\\n";break;
				case '\r': sText += "\\r";break;
				case '\t': sText += "\\t";break;
				case '\v': sText += "\\v";break;
				default:
					sText += *u;
			}
			u++;
		}
		sText += "\"";
		return sText;
	}

	EXPRESSION_RETURN_TYPE ExprScriptConstant::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	bool ExprScriptConstant::compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (_sValue.empty() == bNegative) CW_BODY_STREAM << "true";
		else CW_BODY_STREAM << "false";
		return true;
	}

	bool ExprScriptConstant::compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (_sValue.empty()) CW_BODY_STREAM << 0;
		else CW_BODY_STREAM << _sValue;
		return true;
	}

	bool ExprScriptConstant::compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (_sValue.empty()) CW_BODY_STREAM << "0.0";
		else CW_BODY_STREAM << _sValue;
		return true;
	}

	bool ExprScriptConstant::compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM.writeString(_sValue);
		return true;
	}



	ExprScriptConstantTree::~ExprScriptConstantTree() {
		for (std::list<ExprScriptExpression*>::iterator i = _listOfItems.begin(); i != _listOfItems.end(); ++i) {
			delete *i;
		}
	}
	
	void ExprScriptConstantTree::setValueExpression(ExprScriptExpression* pValue) {
		_pValue = pValue;
	}

	void ExprScriptConstantTree::addAttribute(const std::string& sName, ExprScriptExpression* pTree) {
		if (_listOfAttributes.find(sName) != _listOfAttributes.end()) {
			delete _listOfAttributes[sName];
		}
		_listOfAttributes[sName] = pTree;
	}

	void ExprScriptConstantTree::addItem(ExprScriptExpression* pKey, ExprScriptExpression* pItem) {
		_listOfKeys.push_back(pKey);
		_listOfItems.push_back(pItem);
	}

	void ExprScriptConstantTree::addItem(ExprScriptExpression* pItem) {
		_listOfKeys.push_back(NULL);
		_listOfItems.push_back(pItem);
	}

	std::string ExprScriptConstantTree::getValue(DtaScriptVariable& /*visibility*/) const {
		throw UtlException("ExprScriptConstantTree::getValue() -> no sense for a constant tree!");
	}

	void ExprScriptConstantTree::setTree(DtaScriptVariable& visibility, DtaScriptVariable&  theVariable) const {
		if (_pValue != NULL) {
			std::string sValue = _pValue->getValue(visibility);
			theVariable.setValue(sValue.c_str());
		}
		std::list<ExprScriptExpression*>::const_iterator iterKeys = _listOfKeys.begin();
		for (std::list<ExprScriptExpression*>::const_iterator iterItems = _listOfItems.begin(); iterItems != _listOfItems.end(); ++iterItems) {
			ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(*iterItems);
			if (pConstantTree != NULL) {
				if (*iterKeys != NULL) {
					std::string sKey = (*iterKeys)->getValue(visibility);
					DtaScriptVariable* pItem = theVariable.addElement(sKey);
					pConstantTree->setTree(visibility, *pItem);
				} else {
					DtaScriptVariable* pItem = theVariable.pushItem("");
					pConstantTree->setTree(visibility, *pItem);
				}
			} else {
				std::string sValue = (*iterItems)->getValue(visibility);
				if (*iterKeys != NULL) {
					std::string sKey = (*iterKeys)->getValue(visibility);
					DtaScriptVariable* pItem = theVariable.addElement(sKey);
					theVariable.setValue(sValue.c_str());
				} else {
					theVariable.pushItem(sValue.c_str());
				}
			}
			++iterKeys;
		}
		for (std::map<std::string, ExprScriptExpression*>::const_iterator iterVariables = _listOfAttributes.begin(); iterVariables != _listOfAttributes.end(); ++iterVariables) {
			std::string sAttributeName = iterVariables->first;
			if (sAttributeName[0] == '&') {
				sAttributeName = sAttributeName.substr(1);
			}
			DtaScriptVariable* pNode = theVariable.insertNode(sAttributeName.c_str());
			if (iterVariables->first[0] == '&') {
				ExprScriptVariable* pRefExpr = dynamic_cast<ExprScriptVariable*>(iterVariables->second);
				DtaScriptVariable* pReference = visibility.getExistingVariable(*pRefExpr);
				if (pReference == NULL) {
					throw UtlException(std::string("unknown reference node ") + pRefExpr->toString());
				}
				pNode->setValue(pReference);
			} else {
				ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(iterVariables->second);
				if (pConstantTree != NULL) {
					pConstantTree->setTree(visibility, *pNode);
				} else {
					std::string sValue = iterVariables->second->getValue(visibility);
					pNode->setValue(sValue.c_str());
				}
			}
		}
	}

	EXPRESSION_RETURN_TYPE ExprScriptConstantTree::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "<unhandled 'ExprScriptConstantTree' - should be initialized through 'compileCppInit'>";
		return NO_TYPE;
	}

	EXPRESSION_RETURN_TYPE ExprScriptConstantTree::compileCppInit(CppCompilerEnvironment& theCompilerEnvironment, ExprScriptVariable& theVariable) const {
		CW_BODY_INDENT << "{";CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "CodeWorker::CppParsingTree_var cstTree(";
		theVariable.compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ");";CW_BODY_ENDL;
		compileCppInit(theCompilerEnvironment, "cstTree");
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
		return TREE_TYPE;
	}

	EXPRESSION_RETURN_TYPE ExprScriptConstantTree::compileCppInit(CppCompilerEnvironment& theCompilerEnvironment, const std::string& sVariable) const {
		for (std::list<ExprScriptExpression*>::const_iterator i = _listOfItems.begin(); i != _listOfItems.end(); ++i) {
			ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(*i);
			if (pConstantTree != NULL) {
				pConstantTree->compileCppInit(theCompilerEnvironment, sVariable, 1);
			} else {
				CW_BODY_INDENT << sVariable << ".pushItem(";
				(*i)->compileCppString(theCompilerEnvironment);
				CW_BODY_STREAM << ");";CW_BODY_ENDL;
			}
		}
		return TREE_TYPE;
	}

	EXPRESSION_RETURN_TYPE ExprScriptConstantTree::compileCppInit(CppCompilerEnvironment& theCompilerEnvironment, const std::string& sVariable, int iDepth) const {
		CW_BODY_INDENT << "{";CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		char tcDepth[32];
		sprintf(tcDepth, "%d", iDepth);
		CW_BODY_INDENT << "CodeWorker::CppParsingTree_var " << sVariable << "_" << tcDepth << "(" << sVariable;
		if (iDepth > 1) CW_BODY_STREAM << "_" << (iDepth - 1);
		CW_BODY_STREAM << ".pushItem());";CW_BODY_ENDL;
		for (std::list<ExprScriptExpression*>::const_iterator i = _listOfItems.begin(); i != _listOfItems.end(); ++i) {
			ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(*i);
			if (pConstantTree != NULL) {
				pConstantTree->compileCppInit(theCompilerEnvironment, sVariable, iDepth + 1);
			} else {
				CW_BODY_INDENT << sVariable << "_" << tcDepth << ".pushItem(";
				(*i)->compileCppString(theCompilerEnvironment);
				CW_BODY_STREAM << ");";CW_BODY_ENDL;
			}
		}
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
		return TREE_TYPE;
	}

	std::string ExprScriptConstantTree::toString() const {
		std::string sText = "{";
		for (std::list<ExprScriptExpression*>::const_iterator i = _listOfItems.begin(); i != _listOfItems.end(); ++i) {
			if (i != _listOfItems.begin()) sText += ", ";
			sText += (*i)->toString();
		}
		return sText + "}";
	}


	ExprScriptResolvedVariable::~ExprScriptResolvedVariable() {
	}

	std::string ExprScriptResolvedVariable::getValue(DtaScriptVariable&) const {
		const char* tcValue = _pVariable->getValue();
		if (tcValue == NULL) return "";
		return tcValue;
	}

	std::string ExprScriptResolvedVariable::toString() const {
		return _pVariable->getCompleteName();
	}


	ExprScriptScriptFile::~ExprScriptScriptFile() {
		if (_bFileName) delete script._pFileName;
		else delete script._pBody;
	}

	std::string ExprScriptScriptFile::getValue(DtaScriptVariable& visibility) const {
		if (_bFileName) return script._pFileName->getValue(visibility);
		throw UtlException("Internal error: cannot ask for the value of a piece of script!");
	}

	std::string ExprScriptScriptFile::toString() const {
		if (_bFileName) return script._pFileName->toString();
		return "<piece of script>";
	}


	ExprScriptAlien::~ExprScriptAlien() {}
	std::string ExprScriptAlien::getValue(DtaScriptVariable&) const { throw UtlException("internal error: it is forbidden to execute an alien expression"); }
	double ExprScriptAlien::getDoubleValue(DtaScriptVariable& visibility) const { throw UtlException("internal error: it is forbidden to execute an alien expression"); }
	int ExprScriptAlien::getIntValue(DtaScriptVariable& visibility) const { throw UtlException("internal error: it is forbidden to execute an alien expression"); }

	ExprScriptExpression* ExprScriptAlien::clone() const {
		return new ExprScriptAlien(_sValue.c_str(), _sTargetLanguage);
	}

	std::string ExprScriptAlien::toString() const {
		return _sValue;
	}

	EXPRESSION_RETURN_TYPE ExprScriptAlien::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "TARGET_LANGUAGE_EXPRESSION(" << _sValue << ")";
		return ALIEN_TYPE;
	}

	bool ExprScriptAlien::compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		compileCpp(theCompilerEnvironment);
		return true;
	}

	bool ExprScriptAlien::compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
		return true;
	}

	bool ExprScriptAlien::compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
		return true;
	}

	bool ExprScriptAlien::compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCpp(theCompilerEnvironment);
		return true;
	}
}
