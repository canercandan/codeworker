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
#pragma warning(disable : 4786)
#endif

#ifndef WIN32
#	include <cstdio> // for Debian/gcc 2.95.4
#endif

#include "ScpStream.h"
#include "UtlException.h"

#include "ExprScriptVariable.h"
#include "DtaScriptVariable.h"
#include "DtaProject.h"
#include "DtaScript.h"
#include "CppParsingTree.h"

namespace CodeWorker {
	CppParsingTree_expr::CppParsingTree_expr(const std::string& sExpression) {
		DtaScript theEmptyScript(NULL);
		GrfBlock* pBlock = NULL;
		GrfBlock& myNullBlock = *pBlock;
		ScpStream myStream(sExpression);
		_pExpression = theEmptyScript.parseExpression(myNullBlock, myStream);
	}

	CppParsingTree_expr::~CppParsingTree_expr() {
		delete _pExpression;
	}

	CppParsingTree_varexpr::CppParsingTree_varexpr(const std::string& sVariableExpression) {
		DtaScript theEmptyScript(NULL);
		GrfBlock* pBlock = NULL;
		GrfBlock& myNullBlock = *pBlock;
		ScpStream myStream(sVariableExpression);
		_pExpression = theEmptyScript.parseVariableExpression(myNullBlock, myStream);
	}

	ExprScriptVariable& CppParsingTree_varexpr::getVariableExpression() const {
		return *dynamic_cast<ExprScriptVariable*>(_pExpression);
	}


	CppParsingTree_var::CppParsingTree_var() : _pInternalNode(NULL) {
	}

	CppParsingTree_var::CppParsingTree_var(const CppParsingTree_var& tree) {
		_pInternalNode = tree._pInternalNode;
	}

	CppParsingTree_var::CppParsingTree_var(DtaScriptVariable* pVariable) : _pInternalNode(pVariable) {
	}

	CppParsingTree_var::CppParsingTree_var(DtaScriptVariable& variable) : _pInternalNode(&variable) {
	}

	bool CppParsingTree_var::isLocal() const {
		if (_pInternalNode == NULL) return false;
		return _pInternalNode->isLocal();
	}

	ExternalValueNode* CppParsingTree_var::getExternalValueNode() const {
		if (_pInternalNode == NULL) return NULL;
		return _pInternalNode->getExternalValueNode();
	}

	const char* CppParsingTree_var::getName() const {
		if (_pInternalNode == NULL) return NULL;
		return _pInternalNode->getName();
	}

	const char* CppParsingTree_var::getValue() const {
		if (_pInternalNode == NULL) return NULL;
		return _pInternalNode->getValue();
	}

	size_t CppParsingTree_var::getValueLength() const {
		if (_pInternalNode == NULL) return 0;
		return _pInternalNode->getValueLength();
	}

	bool CppParsingTree_var::getBooleanValue() const {
		if (_pInternalNode == NULL) return 0;
		const char* tcValue = _pInternalNode->getValue();
		return (tcValue != NULL) && (*tcValue != '\0');
	}

	int CppParsingTree_var::getIntValue() const {
		if (_pInternalNode == NULL) return 0;
		const char* tcValue = _pInternalNode->getValue();
		if (tcValue == NULL) return 0;
		return atoi(tcValue);
	}

	double CppParsingTree_var::getDoubleValue() const {
		if (_pInternalNode == NULL) return 0.0;
		const char* tcValue = _pInternalNode->getValue();
		if (tcValue == NULL) return 0.0;
		return atof(tcValue);
	}

	std::string CppParsingTree_var::getStringValue() const {
		const char* tcValue = getValue();
		return ((tcValue == NULL) ? "" : tcValue);
	}

	DtaScriptVariable* CppParsingTree_var::getReference() const {
		if (_pInternalNode == NULL) return 0;
		return _pInternalNode->getReferencedVariable();
	}

	void CppParsingTree_var::clearValue() const {
		if (_pInternalNode != NULL) _pInternalNode->clearValue();
	}

	void CppParsingTree_var::setValue(const char* tcValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to set value \"" + std::string(tcValue) + "\" to an empty node");
		_pInternalNode->setValue(tcValue);
	}

	void CppParsingTree_var::setValue(const std::string& sValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to set value \"" + sValue + "\" to an empty node");
		_pInternalNode->setValue(sValue.c_str());
	}

	void CppParsingTree_var::setValue(double dValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to set a numeric to an empty node");
		_pInternalNode->setValue(dValue);
	}

	void CppParsingTree_var::setValue(bool bValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to set a boolean to an empty node");
		_pInternalNode->setValue((bValue ? "true" : ""));
	}

	void CppParsingTree_var::setValue(int iValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to set an integer to an empty node");
		_pInternalNode->setValue(iValue);
	}

	void CppParsingTree_var::setValue(const CppParsingTree_var& pValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to set a value to an empty node");
		if (pValue._pInternalNode == NULL) _pInternalNode->setValue((const char*) NULL);
		_pInternalNode->setValue(pValue.getValue());
	}

	void CppParsingTree_var::setValue(ExternalValueNode* pExternal) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to set a value to an empty node");
		_pInternalNode->setValue(pExternal);
	}

	void CppParsingTree_var::setValue(const std::list<std::string>& listOfValues) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to set an array of strings to an empty node");
		_pInternalNode->setValue(listOfValues);
	}

	void CppParsingTree_var::concatenateValue(const std::string& sValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to concatenate value \"" + sValue + "\" to an empty node");
		const char* tcLeft = _pInternalNode->getValue();
		if (tcLeft != NULL) {
			std::string sConcatenation(tcLeft);
			sConcatenation += sValue;
			_pInternalNode->setValue(sConcatenation.c_str());
		} else {
			_pInternalNode->setValue(sValue.c_str());
		}
	}

	void CppParsingTree_var::concatenateValue(const CppParsingTree_var& pValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to concatenate a value to an empty node");
		const char* tcLeft = _pInternalNode->getValue();
		const char* tcValue = pValue.getValue();
		if (tcLeft != NULL) {
			std::string sConcatenation(tcLeft);
			if (tcValue != 0) sConcatenation += tcValue;
			_pInternalNode->setValue(sConcatenation.c_str());
		} else {
			_pInternalNode->setValue(tcValue);
		}
	}

	void CppParsingTree_var::setReference(const CppParsingTree_var& reference) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to assign a reference to an empty node");
		if (reference._pInternalNode == NULL) throw UtlException("internal error: trying to assign an empty node as reference");
		_pInternalNode->setValue(reference._pInternalNode);
	}

	void CppParsingTree_var::setAll(const CppParsingTree_var& tree) const {
		if (tree._pInternalNode == NULL) throw UtlException("internal error: trying to assign an empty subtree to a node");
		_pInternalNode->copyAll(*tree._pInternalNode);
	}

	void CppParsingTree_var::merge(const CppParsingTree_var& tree) const {
		if (tree._pInternalNode == NULL) throw UtlException("internal error: trying to assign an empty subtree to a node");
		_pInternalNode->copyAll(*tree._pInternalNode, true);
	}

	CppParsingTree_var CppParsingTree_var::pushItem() const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to push a value to the array of an empty node");
		return _pInternalNode->pushItem("");
	}

	void CppParsingTree_var::pushItem(const std::string& sValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to push a value to the array of an empty node");
		_pInternalNode->pushItem(sValue);
	}

	void CppParsingTree_var::pushItem(const CppParsingTree_var& theValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to push a value to the array of an empty node");
		_pInternalNode->pushItem(theValue.getStringValue());
	}

	void CppParsingTree_var::addElements(const CppParsingTree_var& theValue) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to add elements to the array of an empty node");
		if (theValue._pInternalNode != NULL) _pInternalNode->addArrayElements(*(theValue._pInternalNode));
	}

	const std::list<DtaScriptVariable*>* CppParsingTree_var::getArray() const {
		if (_pInternalNode == NULL) return NULL;
		return _pInternalNode->getArray();
	}

	const std::map<std::string, DtaScriptVariable*>* CppParsingTree_var::getSortedArray() const {
		if (_pInternalNode == NULL) return NULL;
		return _pInternalNode->getSortedArray();
	}

	std::auto_ptr<std::vector<DtaScriptVariable*> > CppParsingTree_var::getSortedNoCaseArray() const {
		if (_pInternalNode == NULL) return std::auto_ptr<std::vector<DtaScriptVariable*> >(new std::vector<DtaScriptVariable*>());
		return _pInternalNode->getSortedNoCaseArray();
	}

	std::auto_ptr<std::vector<DtaScriptVariable*> > CppParsingTree_var::getSortedArrayOnValue() const {
		if (_pInternalNode == NULL) return std::auto_ptr<std::vector<DtaScriptVariable*> >(new std::vector<DtaScriptVariable*>());
		return _pInternalNode->getSortedArrayOnValue();
	}

	std::auto_ptr<std::vector<DtaScriptVariable*> > CppParsingTree_var::getSortedNoCaseArrayOnValue() const {
		if (_pInternalNode == NULL) return std::auto_ptr<std::vector<DtaScriptVariable*> >(new std::vector<DtaScriptVariable*>());
		return _pInternalNode->getSortedNoCaseArrayOnValue();
	}

	std::list<std::string> CppParsingTree_var::getAttributeNames() const {
		std::list<std::string> attributes;
		if (_pInternalNode != NULL) {
			DtaScriptVariableList* pAttributes = _pInternalNode->getAttributes();
			while (pAttributes != NULL) {
				DtaScriptVariable* pNode = pAttributes->getNode();
				attributes.push_back(pNode->getName());
				pAttributes = pAttributes->getNext();
			}
		}
		return attributes;
	}

	void CppParsingTree_var::clearNode() const {
		if (_pInternalNode != NULL) _pInternalNode->clearContent();
	}

	CppParsingTree_var CppParsingTree_var::getNode(const std::string& sBranch) const {
		if (_pInternalNode == NULL) return CppParsingTree_var();
		DtaScriptVariable* pVariable = _pInternalNode->getNode(sBranch.c_str());
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::getEvaluatedNode(const std::string& sDynamicVariable) const {
		if (_pInternalNode == NULL) return CppParsingTree_var();
		DtaScriptVariable* pVariable = _pInternalNode->getEvaluatedNode(sDynamicVariable);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::getOrCreateLocalEvaluatedNode(const std::string& sDynamicVariable) const {
		if (_pInternalNode == NULL) return CppParsingTree_var();
		DtaScriptVariable* pVariable = _pInternalNode->getOrCreateLocalEvaluatedNode(sDynamicVariable);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::getParentNode() const {
		if (_pInternalNode == NULL) return CppParsingTree_var();
		DtaScriptVariable* pVariable = _pInternalNode->getParent();
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::getRootNode() const {
		if (_pInternalNode == NULL) return CppParsingTree_var();
		DtaScriptVariable* pVariable = _pInternalNode->getRoot();
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::getArrayNodeFromKey(const std::string& sKey) const {
		if (_pInternalNode == NULL) return CppParsingTree_var();
		DtaScriptVariable* pVariable = _pInternalNode->getArrayElement(sKey);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::getArrayNodeFromPosition(int iPosition) const {
		if (_pInternalNode == NULL) return CppParsingTree_var();
		DtaScriptVariable* pVariable = _pInternalNode->getArrayElement(iPosition);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::getFirstArrayNode() const {
		if (_pInternalNode == NULL) return CppParsingTree_var();
		DtaScriptVariable* pVariable = _pInternalNode->getArrayElement((int) 0);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::getLastArrayNode() const {
		if (_pInternalNode == NULL) return CppParsingTree_var();
		DtaScriptVariable* pVariable = _pInternalNode->getArrayElement(_pInternalNode->getArraySize() - 1);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::insertNode(const std::string& sBranch) const {
		if (_pInternalNode == NULL) {
			throw UtlException("internal error: trying to insert the node '" + sBranch + "' into an empty tree");
		}
		DtaScriptVariable* pVariable = _pInternalNode->insertNode(sBranch.c_str());
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::insertEvaluatedNode(const std::string& sDynamicVariable) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to insert an evaluated node '" + sDynamicVariable + "' into an empty tree");
		DtaScriptVariable* pVariable = _pInternalNode->insertEvaluatedNode(sDynamicVariable);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::insertClassicalEvaluatedNode(const std::string& sDynamicVariable) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to insert an evaluated node '" + sDynamicVariable + "' into an empty tree");
		DtaScriptVariable* pVariable = _pInternalNode->insertClassicalEvaluatedNode(sDynamicVariable);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::insertArrayNodeFromKey(const std::string& sKey) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to insert '" + sKey + "' key node into an empty array");
		DtaScriptVariable* pVariable = _pInternalNode->addElement(sKey);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::insertArrayNodeFromKey(int iKey) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to insert key node into an empty array");
		char tcNumber[16];
		sprintf(tcNumber, "%d", iKey);
		DtaScriptVariable* pVariable = _pInternalNode->addElement(tcNumber);
		return CppParsingTree_var(pVariable);
	}

	CppParsingTree_var CppParsingTree_var::insertArrayNodeFromKey(const CppParsingTree_var& pKey) const {
		if (_pInternalNode == NULL) throw UtlException("internal error: trying to insert key node into an empty tree");
		DtaScriptVariable* pVariable = _pInternalNode->addElement(pKey.getValue());
		return CppParsingTree_var(pVariable);
	}


	CppParsingTree_value::CppParsingTree_value() {
		_pInternalNode = new DtaScriptVariable;
	}

	CppParsingTree_value::CppParsingTree_value(const char* tcValue) {
		_pInternalNode = new DtaScriptVariable;
		_pInternalNode->setValue(tcValue);
	}

	CppParsingTree_value::CppParsingTree_value(const CppParsingTree_var& tree) {
		_pInternalNode = new DtaScriptVariable;
		setValue(tree);
	}

	CppParsingTree_value::CppParsingTree_value(const CppParsingTree_value& tree) {
		_pInternalNode = new DtaScriptVariable;
		setValue(tree);
	}

	CppParsingTree_value::CppParsingTree_value(bool bValue) {
		_pInternalNode = new DtaScriptVariable;
		_pInternalNode->setValue(bValue);
	}

	CppParsingTree_value::CppParsingTree_value(int iValue) {
		_pInternalNode = new DtaScriptVariable;
		_pInternalNode->setValue(iValue);
	}

	CppParsingTree_value::CppParsingTree_value(double dValue) {
		_pInternalNode = new DtaScriptVariable;
		_pInternalNode->setValue(dValue);
	}

	CppParsingTree_value::CppParsingTree_value(const std::string& sValue) {
		_pInternalNode = new DtaScriptVariable;
		_pInternalNode->setValue(sValue.c_str());
	}

	CppParsingTree_value::~CppParsingTree_value() {
		delete _pInternalNode;
	}

	void CppParsingTree_global::initialize(const std::string& sName) {
		_pInternalNode = DtaProject::getInstance().getGlobalVariable(sName);
		if (_pInternalNode == NULL) {
			_pInternalNode = DtaProject::getInstance().setGlobalVariable(sName);
		}
	}
}
