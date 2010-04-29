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
#pragma warning (disable : 4503)
#endif

#ifndef WIN32
#	include <cstdio> // for Debian/gcc 2.95.4
#endif

#include <string.h>

#include "ScpStream.h"

#include "UtlException.h"
#include "UtlTrace.h"
#include "UtlString.h"

#include "CGRuntime.h"
#include "DtaArrayIterator.h"
#include "ExprScriptVariable.h"
#include "Workspace.h"
#include "DtaProject.h"
#include "ExternalValueNode.h"
#include "DynPackage.h"
#include "CGExternalHandling.h" // for CGQuietOutput
#include "DtaScriptVariable.h"

namespace CodeWorker {
	DtaNodeListener::~DtaNodeListener() {}

	class DtaNodeListeners {
	private:
		DtaScriptVariable* _pNode;
		std::list<DtaNodeListener*> _listeners;

	public:
		inline DtaNodeListeners(DtaScriptVariable* pNode) : _pNode(pNode) {}
		~DtaNodeListeners() {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onDelete();
				delete *i;
			}
		}

		void addListener(DtaNodeListener* pListener) {
			pListener->_pNode = _pNode;
			_listeners.push_back(pListener);
		}

		bool removeListener(DtaNodeListener* pListener) {
			if (pListener->_pNode != _pNode) return false;
			_listeners.remove(pListener);
			delete pListener;
			return true;
		}

		void onSetValue(const char* tcValue) {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onSetValue(tcValue);
			}
		}

		void onConcatValue(const char* tcValue) {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onConcatValue(tcValue);
			}
		}

		void onSetReference(DtaScriptVariable* pReference) {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onSetReference(pReference);
			}
		}

		void onSetExternal(ExternalValueNode* pExternalValue) {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onSetExternal(pExternalValue);
			}
		}

		void onSetIterator(DtaArrayIterator** pIteratorData) {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onSetIterator(pIteratorData);
			}
		}

		void onAddAttribute(DtaScriptVariable* pAttribute) {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onAddAttribute(pAttribute);
			}
		}

		void onRemoveAttribute(DtaScriptVariable* pAttribute) {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onRemoveAttribute(pAttribute);
			}
		}

		void onArrayHasChanged() {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onArrayHasChanged();
			}
		}

		void onClearContent() {
			for (std::list<DtaNodeListener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
				(*i)->onClearContent();
			}
		}
	};

	void DtaScriptVariableList::pushBack(DtaScriptVariable* pVariable) {
		register DtaScriptVariableList* pList = this;
		while (pList->getNext() != NULL) pList = pList->getNext();
		pList->setNext(new DtaScriptVariableList(pVariable));
	}


	DtaScriptVariable::NodeArrayType::~NodeArrayType() {
		for (std::list<DtaScriptVariable*>::iterator i = items.begin(); i != items.end(); i++) {
			delete (*i);
		}
		delete cache;
	}

	DtaScriptVariable::NodeArrayType::NodeArrayType(DtaScriptVariable* pElt) : cache(NULL) {
		items.push_back(pElt);
	}


	DtaScriptVariable::~DtaScriptVariable() {
		delete _pListeners;
		_pListeners = NULL;
		clearContent();
		if (_pReferencedByVariables != NULL) {
			while (!_pReferencedByVariables->empty()) {
				DtaScriptVariable* pReferencedBy = *(_pReferencedByVariables->begin());
				pReferencedBy->removeReference();
	/*
				std::string sErrorMessage = "warning: variable '";
				sErrorMessage += pReferencedBy->getCompleteName();
				sErrorMessage += "' refers to '";
				sErrorMessage += getCompleteName();
				sErrorMessage += "' that is being removed from the stack" + CGRuntime::endl();
				sErrorMessage += UtlTrace::getTraceStack();
				if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
	*/
			}
			delete _pReferencedByVariables;
			_pReferencedByVariables = NULL;
		}
		free((char*) _tcName);
	}

	void DtaScriptVariable::addListener(DtaNodeListener* pListener) {
		if (_pListeners == NULL) _pListeners = new DtaNodeListeners(this);
		_pListeners->addListener(pListener);
	}

	bool DtaScriptVariable::removeListener(DtaNodeListener* pListener) {
		if (_pListeners == NULL) return false;
		return _pListeners->removeListener(pListener);
	}

	DtaScriptVariable* DtaScriptVariable::getReferencedVariable() const {
		if (_eValueNode == REFERENCE_VALUE_NODE) return _value.var;
		if (_eValueNode == INDEX_VALUE_NODE) {
			DtaScriptVariable* pVariable = (*_value.ind.var)->itemNoRef();
			return pVariable;
		}
		return NULL;
	}

	DtaScriptVariable* DtaScriptVariable::getRoot() const {
		DtaScriptVariable* pVariable = const_cast<DtaScriptVariable*>(this);
		register DtaScriptVariable* pParent = _pParent;
		while ((pParent != NULL) && (strncmp(pParent->_tcName, "##stack##", 9) != 0)) {
			pVariable = pParent;
			pParent = pVariable->getParent();
		}
		return pVariable;
	}

	void DtaScriptVariable::changeName(const char* tcName) {
		if ((_pParent != NULL) && (_pParent->_pArray != NULL)) {
			if (_pParent->getNode(_tcName) != this) {
				// this node belongs to the array of its parent:
				// purge the cache: the key will change!
				delete _pParent->_pArray->cache;
				_pParent->_pArray->cache = NULL;
			}
		}
		delete _tcName;
#ifdef WIN32
		_tcName = ::_strdup(tcName);
#else
		_tcName = ::strdup(tcName);
#endif
	}

	bool DtaScriptVariable::isLocal() const {
		const DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		if (pVariable->_pParent == NULL || pVariable->_pParent->getName() == NULL) return false;
		return (strncmp(pVariable->_pParent->getName(), "##", 2) == 0);
	}

	DtaScriptVariable* DtaScriptVariable::detachAttribute(const char* tcName) {
		DtaScriptVariable* pNode = NULL;
		DtaScriptVariableList* pAttributes = _pAttributes;
		DtaScriptVariableList* pLastAttribute = NULL;
		while (pAttributes != NULL) {
			if (strcmp(pAttributes->getNode()->_tcName, tcName) == 0) {
				pNode = pAttributes->getNode();
				if (pLastAttribute == NULL) _pAttributes = pAttributes->getNext();
				else pLastAttribute->setNext(pAttributes->getNext());
				pAttributes->setNext(NULL);
				delete pAttributes;
				break;
			}
			pLastAttribute = pAttributes;
			pAttributes = pAttributes->getNext();
		}
		return pNode;
	}

	const std::list<DtaScriptVariable*>* DtaScriptVariable::getArray() const {
		const DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		return &(pVariable->_pArray->items);
	}

	int DtaScriptVariable::getArraySize() const {
		const DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		if (pVariable->_pArray == NULL) return 0;
		return pVariable->_pArray->items.size();
	}

	const std::map<std::string, DtaScriptVariable*>* DtaScriptVariable::getSortedArray() const {
		const DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		NodeArrayType* pANT = pVariable->_pArray;
		if (pANT == NULL) return NULL;
		if (pANT->cache == NULL) {
			pANT->cache = new std::map<std::string, DtaScriptVariable*>();
			for (std::list<DtaScriptVariable*>::const_iterator i = pANT->items.begin(); i != pANT->items.end(); i++) {
				(*(pANT->cache))[(*i)->_tcName] = (*i);
			}
		}
		return pVariable->_pArray->cache;
	}

	std::auto_ptr<std::vector<DtaScriptVariable*> > DtaScriptVariable::getSortedNoCaseArray() const {
		const DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		std::vector<DtaScriptVariable*>* pListOfNodes = NULL;
		NodeArrayType* pANT = pVariable->_pArray;
		if (pANT != NULL) {
			std::map<std::string, std::map<std::string, DtaScriptVariable*> > mapOfNodes;
			for (std::list<DtaScriptVariable*>::const_iterator i = pANT->items.begin(); i != pANT->items.end(); i++) {
				std::string sKey = toLowercase((*i)->_tcName);
				mapOfNodes[sKey][(*i)->_tcName] = *i;
			}
			pListOfNodes = new std::vector<DtaScriptVariable*>(pANT->items.size());
			int iPosition = 0;
			for (std::map<std::string, std::map<std::string, DtaScriptVariable*> >::const_iterator j = mapOfNodes.begin(); j != mapOfNodes.end(); ++j) {
				for (std::map<std::string, DtaScriptVariable*>::const_iterator k = j->second.begin(); k != j->second.end(); ++k) {
					(*pListOfNodes)[iPosition++] = k->second;
				}
			}
		}
		return std::auto_ptr<std::vector<DtaScriptVariable*> >(pListOfNodes);
	}

	std::auto_ptr<std::vector<DtaScriptVariable*> > DtaScriptVariable::getSortedArrayOnValue() const {
		const DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		std::vector<DtaScriptVariable*>* pListOfNodes = NULL;
		NodeArrayType* pANT = pVariable->_pArray;
		if (pANT != NULL) {
			std::map<std::string, std::map<std::string, DtaScriptVariable*> > mapOfNodes;
			for (std::list<DtaScriptVariable*>::const_iterator i = pANT->items.begin(); i != pANT->items.end(); i++) {
				std::string sKey = (*i)->getValue();
				mapOfNodes[sKey][(*i)->_tcName] = *i;
			}
			pListOfNodes = new std::vector<DtaScriptVariable*>(pANT->items.size());
			int iPosition = 0;
			for (std::map<std::string, std::map<std::string, DtaScriptVariable*> >::const_iterator j = mapOfNodes.begin(); j != mapOfNodes.end(); ++j) {
				for (std::map<std::string, DtaScriptVariable*>::const_iterator k = j->second.begin(); k != j->second.end(); ++k) {
					(*pListOfNodes)[iPosition++] = k->second;
				}
			}
		}
		return std::auto_ptr<std::vector<DtaScriptVariable*> >(pListOfNodes);
	}

	std::auto_ptr<std::vector<DtaScriptVariable*> > DtaScriptVariable::getSortedNoCaseArrayOnValue() const {
		const DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		std::vector<DtaScriptVariable*>* pListOfNodes = NULL;
		NodeArrayType* pANT = pVariable->_pArray;
		if (pANT != NULL) {
			std::map<std::string, std::map<std::string, DtaScriptVariable*> > mapOfNodes;
			for (std::list<DtaScriptVariable*>::const_iterator i = pANT->items.begin(); i != pANT->items.end(); i++) {
				std::string sKey = toLowercase((*i)->getValue());
				mapOfNodes[sKey][(*i)->_tcName] = *i;
			}
			pListOfNodes = new std::vector<DtaScriptVariable*>(pANT->items.size());
			int iPosition = 0;
			for (std::map<std::string, std::map<std::string, DtaScriptVariable*> >::const_iterator j = mapOfNodes.begin(); j != mapOfNodes.end(); ++j) {
				for (std::map<std::string, DtaScriptVariable*>::const_iterator k = j->second.begin(); k != j->second.end(); ++k) {
					(*pListOfNodes)[iPosition++] = k->second;
				}
			}
		}
		return std::auto_ptr<std::vector<DtaScriptVariable*> >(pListOfNodes);
	}

	DtaScriptVariable* DtaScriptVariable::getArrayElement(const std::string& sPosition) const {
		if (getReferencedVariable() != NULL) return getReferencedVariable()->getArrayElement(sPosition);
		if (_pArray == NULL) return NULL;
		if (_pArray->cache != NULL) {
			std::map<std::string, DtaScriptVariable*>::const_iterator i = _pArray->cache->find(sPosition);
			if (i != _pArray->cache->end()) return i->second;
		} else if (_pArray->items.size() >= 12) {
			DtaScriptVariable* pVariable = NULL;
			_pArray->cache = new std::map<std::string, DtaScriptVariable*>();
			for (std::list<DtaScriptVariable*>::const_iterator i = _pArray->items.begin(); i != _pArray->items.end(); i++) {
				if (strcmp((*i)->_tcName, sPosition.c_str()) == 0) pVariable = (*i);
				(*_pArray->cache)[(*i)->_tcName] = (*i);
			}
			return pVariable;
		} else {
			for (std::list<DtaScriptVariable*>::const_iterator i = _pArray->items.begin(); i != _pArray->items.end(); i++) {
				if (strcmp((*i)->_tcName, sPosition.c_str()) == 0) return (*i);
			}
		}
		return NULL;
	}

	DtaScriptVariable* DtaScriptVariable::getArrayElement(int iPosition) const {
		if (getReferencedVariable() != NULL) return getReferencedVariable()->getArrayElement(iPosition);
		if (_pArray == NULL) return NULL;
		if (iPosition >= 0) {
			for (std::list<DtaScriptVariable*>::const_iterator i = _pArray->items.begin(); i != _pArray->items.end(); i++) {
				if (iPosition == 0) return (*i);
				iPosition--;
			}
		}
		return NULL;
	}

	void DtaScriptVariable::invertArray() {
		if (getReferencedVariable() != NULL) {
			getReferencedVariable()->invertArray();
		} else if (_pArray != NULL) {
			std::list<DtaScriptVariable*> array;
			for (std::list<DtaScriptVariable*>::iterator i = _pArray->items.begin(); i != _pArray->items.end(); i++) {
				array.push_front(*i);
			}
			_pArray->items = array;
			if (_pListeners != NULL) _pListeners->onArrayHasChanged();
		}
	}

	DtaScriptVariable* DtaScriptVariable::insertElementAt(const std::string& sProposedKey, int iPosition) {
		if (getReferencedVariable() != NULL) {
			return getReferencedVariable()->insertElementAt(sProposedKey, iPosition);
		}
		DtaScriptVariable* pVariable = NULL;
		std::string sKey;
		if (sProposedKey.empty()) {
			char tcNumber[32];
			int iKey = ((_pArray != NULL) ? _pArray->items.size() : 0);
			do {
				sprintf(tcNumber, "%d", iKey);
				sKey = tcNumber;
				++iKey;
			} while (getArrayElement(sKey) != NULL);
		} else {
			sKey = sProposedKey;
			if (getArrayElement(sKey) != NULL) return NULL;
		}
		if (_pArray != NULL) {
			if (iPosition >= 0) {
				for (std::list<DtaScriptVariable*>::iterator i = _pArray->items.begin(); i != _pArray->items.end(); i++) {
					if (iPosition == 0) {
						pVariable = new DtaScriptVariable(this, sKey);
						_pArray->items.insert(i, pVariable);
						if (_pArray->cache != NULL) (*_pArray->cache)[sKey] = pVariable;
						if (_pListeners != NULL) _pListeners->onArrayHasChanged();
						return pVariable;
					}
					iPosition--;
				}
				if (iPosition == 0) {
					pVariable = addInternalElement(sKey);
				}
			}
		} else if (iPosition == 0) {
			pVariable = addInternalElement(sKey);
		}
		return pVariable;
	}

	void DtaScriptVariable::removeArrayElement() {
		if (getReferencedVariable() != NULL) {
			getReferencedVariable()->removeArrayElement();
		} else {
			delete _pArray;
			_pArray = NULL;
			if (_pListeners != NULL) _pListeners->onArrayHasChanged();
		}
	}

	void DtaScriptVariable::removeArrayElement(const char* sPosition) {
		if (getReferencedVariable() != NULL) getReferencedVariable()->removeArrayElement(sPosition);
		else if (_pArray != NULL) {
			for (std::list<DtaScriptVariable*>::iterator i = _pArray->items.begin(); i != _pArray->items.end(); i++) {
				if (strcmp((*i)->_tcName, sPosition) == 0) {
					delete (*i);
					_pArray->items.erase(i);
					if (_pArray->cache != NULL) _pArray->cache->erase(sPosition);
					if (_pListeners != NULL) _pListeners->onArrayHasChanged();
					break;
				}
			}
		}
	}

	void DtaScriptVariable::removeArrayElement(int iPosition) {
		if (getReferencedVariable() != NULL) getReferencedVariable()->removeArrayElement(iPosition);
		else if (_pArray != NULL) {
			if (iPosition >= 0) {
				for (std::list<DtaScriptVariable*>::iterator i = _pArray->items.begin(); i != _pArray->items.end(); i++) {
					if (iPosition == 0) {
						if (_pArray->cache != NULL) _pArray->cache->erase((*i)->_tcName);
						delete *i;
						_pArray->items.erase(i);
						if (_pListeners != NULL) _pListeners->onArrayHasChanged();
						break;
					}
					iPosition--;
				}
			}
		}
	}

	void DtaScriptVariable::removeFirstElement() {
		if (getReferencedVariable() != NULL) getReferencedVariable()->removeFirstElement();
		else if (_pArray != NULL) {
			std::list<DtaScriptVariable*>::iterator cursor = _pArray->items.begin();
			if (cursor != _pArray->items.end()) {
				if (_pArray->cache != NULL) _pArray->cache->erase((*cursor)->_tcName);
				delete *cursor;
				_pArray->items.erase(cursor);
				if (_pListeners != NULL) _pListeners->onArrayHasChanged();
			}
		}
	}

	void DtaScriptVariable::removeLastElement() {
		if (getReferencedVariable() != NULL) getReferencedVariable()->removeLastElement();
		else if ((_pArray != NULL) && !_pArray->items.empty()) {
			DtaScriptVariable* pElt = _pArray->items.back();
			if (_pArray->cache != NULL) _pArray->cache->erase(pElt->_tcName);
			_pArray->items.remove(pElt);
			delete pElt;
			if (_pListeners != NULL) _pListeners->onArrayHasChanged();
		}
	}

	std::string DtaScriptVariable::getCompleteName() const {
		std::string sName;
		if (_pParent != NULL) {
			sName = _pParent->getCompleteName();
			DtaScriptVariableList* pAttributes = _pParent->_pAttributes;
			while (pAttributes != NULL) {
				if (strcmp(pAttributes->getNode()->_tcName, _tcName) == 0) {
					if (!sName.empty()) sName += ".";
					sName += _tcName;
					return sName;
				}
				pAttributes = pAttributes->getNext();
			}
			if (_pParent->_pArray != NULL) {
				const std::list<DtaScriptVariable*>& pList = _pParent->_pArray->items;
				for (std::list<DtaScriptVariable*>::const_iterator i = pList.begin(); i != pList.end(); i++) {
					if (((*i) != NULL) && (strcmp((*i)->_tcName, _tcName) == 0)) {
						sName += "[\"" + std::string(_tcName) + "\"]";
						return sName;
					}
				}
			}
			sName = "";
		} else if (_tcName != NULL) {
			sName = _tcName;
		} else {
			sName = "<unnamed>";
		}
		return sName;
	}

	void DtaScriptVariable::clearArray() {
		delete _pArray;
		_pArray = NULL;
		if (_pListeners != NULL) _pListeners->onArrayHasChanged();
	}

	void DtaScriptVariable::sortArray() {
		DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		NodeArrayType* pANT = pVariable->_pArray;
		if (pANT == NULL) return;
		bool bPurgeCacheAfter = (pANT->cache == NULL);
		if (bPurgeCacheAfter) {
			pANT->cache = new std::map<std::string, DtaScriptVariable*>();
			for (std::list<DtaScriptVariable*>::const_iterator i = pANT->items.begin(); i != pANT->items.end(); i++) {
				(*(pANT->cache))[(*i)->_tcName] = (*i);
			}
		}
		pANT->items = std::list<DtaScriptVariable*>();
		std::map<std::string, DtaScriptVariable*>* mapOfElts = pANT->cache;
		for (std::map<std::string, DtaScriptVariable*>::const_iterator j = mapOfElts->begin(); j != mapOfElts->end(); ++j) {
			pANT->items.push_back(j->second);
		}
		if (bPurgeCacheAfter) {
			delete pANT->cache;
			pANT->cache = NULL;
		}
		if (_pListeners != NULL) _pListeners->onArrayHasChanged();
	}

	void DtaScriptVariable::removeReference() {
		_value.var->_pReferencedByVariables->erase(this);
		_eValueNode = NO_VALUE_NODE;
		if (_pListeners != NULL) _pListeners->onSetValue(NULL);
	}

	void DtaScriptVariable::addReferencedBy(DtaScriptVariable* pVariable) {
		if (pVariable != NULL) {
			if (_pReferencedByVariables == NULL) _pReferencedByVariables = new std::set<DtaScriptVariable*>();
			_pReferencedByVariables->insert(pVariable);
		}
	}

	DtaScriptVariable* DtaScriptVariable::getInternalVariableExtraction(DtaScriptVariable& visibility, const ExprScriptVariable& exprVariable, bool bRecursiveSearch, bool bCreateIfUnknown, bool bReferencedVariable) {
		DtaScriptVariable* pVariable = NULL;
		if (exprVariable.isEvaluation()) {
			std::string sCommand = exprVariable.getExpression()->getValue(visibility);
			ScpStream theCommand(sCommand);
			DtaScript script;
			CGQuietOutput quiet;
			std::auto_ptr<ExprScriptVariable> pExprVariable(script.parseVariableExpression(script.getBlock(), theCommand));
			pVariable = getInternalVariableExtraction(visibility, *pExprVariable, bRecursiveSearch, bCreateIfUnknown, bReferencedVariable);
			return pVariable;
		}
		if (exprVariable.getPackage() != NULL) {
			// the variable comes from a package
			pVariable = exprVariable.getPackage()->getVariable(exprVariable.getName());
			if (pVariable == NULL) return NULL;
			if (bReferencedVariable || exprVariable.isNext()) {
				DtaScriptVariable* pRefVar = pVariable->getReferencedVariable();
				if (pRefVar != NULL) pVariable = pRefVar;
			}
		} else {
			if (bReferencedVariable || exprVariable.isNext()) pVariable = getReferencedVariable();
			if (pVariable == NULL) {
				bool bExtractVariableWithReference = bReferencedVariable || !exprVariable.isAttributeOnly();
				std::string sName = exprVariable.getName();
				pVariable = getVariable(sName.c_str(), bRecursiveSearch, bCreateIfUnknown, bExtractVariableWithReference);
				if (pVariable == NULL) return NULL;
			}
		}
		if (exprVariable.isArrayKey()) {
			std::string sPosition = exprVariable.getExpression()->getValue(visibility);
			DtaScriptVariable* pArrayElement = pVariable->getArrayElement(sPosition);
			if (pArrayElement == NULL) {
				if (bCreateIfUnknown) {
					pArrayElement = pVariable->addInternalElement(sPosition);
				} else {
					return NULL;
				}
				pVariable = pArrayElement;
			} else {
				if (bReferencedVariable || exprVariable.isNext()) {
					pVariable = pArrayElement->getReferencedVariable();
					if (pVariable == NULL) pVariable = pArrayElement;
				} else {
					pVariable = pArrayElement;
				}
			}
		} else if (exprVariable.isArrayPosition()) {
			std::string sPosition = exprVariable.getExpression()->getValue(visibility);
			if (sPosition == "parent") {
				register DtaScriptVariable* pParent = pVariable->getParent();
				if ((pParent != NULL) && (strncmp(pParent->_tcName, "##stack##", 9) != 0)) pVariable = pParent;
			} else if (sPosition == "root") {
				register DtaScriptVariable* pParent = pVariable->getParent();
				while ((pParent != NULL) && (strncmp(pParent->_tcName, "##stack##", 9) != 0)) {
					pVariable = pParent;
					pParent = pVariable->getParent();
				}
			} else {
				int iPosition;
				if (sPosition == "back") iPosition = pVariable->getArraySize() - 1;
				else iPosition = atoi(sPosition.c_str());
				DtaScriptVariable* pArrayElement = pVariable->getArrayElement(iPosition);
				if (pArrayElement == NULL) throw UtlException(std::string("Array index is out of bounds on variable '" + exprVariable.toString() + "': \"") + sPosition + "\"");
				pVariable = pArrayElement;
				if (bReferencedVariable || exprVariable.isNext()) {
					pVariable = pVariable->getReferencedVariable();
					if (pVariable == NULL) pVariable = pArrayElement;
				}
			}
		}
		if (exprVariable.isNext()) {
			pVariable = pVariable->getInternalVariableExtraction(visibility, *(exprVariable.getNext()), false, bCreateIfUnknown, bReferencedVariable);
		}
		return pVariable;
	}

	DtaScriptVariable* DtaScriptVariable::getVariable(const ExprScriptVariable& exprVariable) {
		return getInternalVariableExtraction(*this, exprVariable, true, true, true);
	}

	DtaScriptVariable* DtaScriptVariable::getNonRecursiveVariable(DtaScriptVariable& visibility, const ExprScriptVariable& exprVariable) {
		return getInternalVariableExtraction(visibility, exprVariable, false, true, true);
	}

	DtaScriptVariable* DtaScriptVariable::getNonRecursiveNonReferencedVariable(DtaScriptVariable& visibility, const ExprScriptVariable& exprVariable) {
		return getInternalVariableExtraction(visibility, exprVariable, false, true, false);
	}

	DtaScriptVariable* DtaScriptVariable::getVariableForReferenceAssignment(const ExprScriptVariable& exprVariable) {
		return getInternalVariableExtraction(*this, exprVariable, true, true, false);
	}

	DtaScriptVariable* DtaScriptVariable::getExistingVariable(const ExprScriptVariable& exprVariable) {
		DtaScriptVariable* pVariable = getInternalVariableExtraction(*this, exprVariable, true, false, true);
		if ((pVariable == NULL) && Workspace::warningOnUnknownVariables()) {
			std::string sErrorMessage = "warning: variable '" + exprVariable.toString() + "' doesn't exist yet" + CGRuntime::endl();
			sErrorMessage += UtlTrace::getTraceStack();
			if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		}
		return pVariable;
	}

	DtaScriptVariable* DtaScriptVariable::getNoWarningExistingVariable(const ExprScriptVariable& exprVariable) {
		return getInternalVariableExtraction(*this, exprVariable, true, false, true);
	}


	DtaScriptVariable* DtaScriptVariable::getNonRecursiveVariable(const char* sName) {
		return getVariable(sName, false, true, true);
	}

	DtaScriptVariable* DtaScriptVariable::getVariable(const char* sName, bool bRecursiveSearch, bool bCreateIfUnknown, bool bReferencedVariable) {
		DtaScriptVariable* pVariable = NULL;
		if (*sName == '\0') return this;
		if (bRecursiveSearch) {
			if (strcmp(sName, "project") == 0) return &(DtaProject::getInstance());
			if (strcmp(sName, "this") == 0) {
				DtaScriptVariable* pThis = &CGRuntime::getThisInternalNode();
				if (pThis == NULL) throw UtlException("INTERNAL ERROR: NULL 'this'!");
				return pThis;
			}
			if (strcmp(sName, "null") == 0) return NULL;
		}
		DtaScriptVariableList* pAttributes = _pAttributes;
		DtaScriptVariableList* pLastAttribute = _pAttributes;
		while (pAttributes != NULL) {
			if (strcmp(pAttributes->getNode()->_tcName, sName) == 0) {
				pVariable = pAttributes->getNode();
				if (bReferencedVariable && (pVariable->getReferencedVariable() != NULL)) {
					do pVariable = pVariable->getReferencedVariable(); while (pVariable->getReferencedVariable() != NULL);
				}
				break;
			}
			if (pAttributes->getNext() == NULL) pLastAttribute = pAttributes;
			pAttributes = pAttributes->getNext();
		}
		if (pAttributes == NULL) {
			if (bRecursiveSearch) {
	//			pVariable = _pParent->getVariable(sName, true, false, bReferencedVariable);
				// we are looking for the variable into local variables, above the function call
				DtaScriptVariable* pLocalScope = NULL;
				bool bClause = false;
				if ((strncmp(_tcName, "##stack## ", 10) == 0) && (strncmp(_tcName + 10, "function", 8) != 0) && (strncmp(_tcName + 10, "clause", 6) != 0)) {
					pLocalScope = getParent();
					while ((pVariable == NULL) && (pLocalScope != NULL) && (pLocalScope->_tcName != NULL) && (strncmp(pLocalScope->_tcName, "##stack##", 9) == 0)) {
						pVariable = pLocalScope->getVariable(sName, false, false, bReferencedVariable);
						const char* tcFunction = pLocalScope->_tcName + 10;
						if (strncmp(tcFunction, "function", 8) == 0) break;
						if (strncmp(tcFunction, "clause", 6) == 0) {
							bClause = true;
							break;
						}
						pLocalScope = pLocalScope->getParent();
					}
				}
				if ((pVariable == NULL) && !bClause) {
					// the variable doesn't belong to the stack, perhaps it is a node of the current subtree
					DtaScriptVariable* pThis = &CGRuntime::getThisInternalNode();
					if (pThis == 0) throw UtlException("INTERNAL ERROR: NULL 'this'!");
					pVariable = pThis->getVariable(sName, false, false, bReferencedVariable);
					int tiVersion[] = {1, 13, -1};
					if ((pVariable == NULL) && (pLocalScope != NULL) && DtaProject::getInstance().isScriptVersionOlderThan(tiVersion)) {
						// this is an old version of scripts, so variable may be stored
						// into the stack, above the function call
						while ((pVariable == NULL) && (pLocalScope->_tcName != NULL) && (strncmp(pLocalScope->_tcName, "##stack##", 9) == 0)) {
							pVariable = pLocalScope->getVariable(sName, false, false, bReferencedVariable);
							pLocalScope = pLocalScope->getParent();
						}
						if (pVariable != NULL) {
							std::string sErrorMessage = "since version 1.13, the scope of a local variable is restricted to the function it is declared into, so a change of behaviour may occur when looking at local variable '";
							sErrorMessage += sName;
							sErrorMessage += "'" + UtlTrace::getTraceStack();
							if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
							pVariable = NULL;
						}
					}
				}
			}
			if (pVariable == NULL) {
				if (bRecursiveSearch) pVariable = DtaProject::getInstance().getGlobalVariable(sName);
				if ((pVariable == NULL) && bCreateIfUnknown) {
					pVariable = new DtaScriptVariable(this, sName);
					if (_pAttributes == NULL) _pAttributes = new DtaScriptVariableList(pVariable);
					else pLastAttribute->pushBack(pVariable);
					if (_pListeners != NULL) _pListeners->onAddAttribute(pVariable);
				}
			}
		}
		return pVariable;
	}

	DtaScriptVariable* DtaScriptVariable::getNode(const char* sName) {
		DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		return pVariable->getVariable(sName, false, false, true);
	}

	DtaScriptVariable* DtaScriptVariable::insertNode(const char* sName) {
		DtaScriptVariable* pVariable = this;
		while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
		return pVariable->getVariable(sName, false, true, true);
	}

	DtaScriptVariable* DtaScriptVariable::getEvaluatedNode(const std::string& sDynamicVariable) {
		ScpStream theCommand(sDynamicVariable);
		DtaScript script;
		std::auto_ptr<ExprScriptVariable> pExprVariable(script.parseVariableExpression(script.getBlock(), theCommand));
		DtaScriptVariable* pVariable = getInternalVariableExtraction(*this, *pExprVariable, true/*bRecursiveSearch*/, false/*bCreateIfUnknown*/, true/*bReferencedVariable*/);
		return pVariable;
	}

	DtaScriptVariable* DtaScriptVariable::getOrCreateLocalEvaluatedNode(const std::string& sDynamicVariable) {
		ScpStream theCommand(sDynamicVariable);
		DtaScript script;
		std::auto_ptr<ExprScriptVariable> pExprVariable(script.parseVariableExpression(script.getBlock(), theCommand));
		DtaScriptVariable* pVariable = getInternalVariableExtraction(*this, *pExprVariable, true/*bRecursiveSearch*/, true/*bCreateIfUnknown*/, true/*bReferencedVariable*/);
		return pVariable;
	}

	DtaScriptVariable* DtaScriptVariable::insertEvaluatedNode(const std::string& sDynamicVariable) {
		ScpStream theCommand(sDynamicVariable);
		DtaScript script;
		std::auto_ptr<ExprScriptVariable> pExprVariable(script.parseVariableExpression(script.getBlock(), theCommand));
		DtaScriptVariable* pVariable = getInternalVariableExtraction(*this, *pExprVariable, true/*bRecursiveSearch*/, true/*bCreateIfUnknown*/, true/*bReferencedVariable*/);
		return pVariable;
	}

	DtaScriptVariable* DtaScriptVariable::insertClassicalEvaluatedNode(const std::string& sDynamicVariable) {
		ScpStream theCommand(sDynamicVariable);
		DtaScript script;
		std::auto_ptr<ExprScriptVariable> pExprVariable(script.parseVariableExpression(script.getBlock(), theCommand));
		DtaScriptVariable* pVariable = getInternalVariableExtraction(*this, *pExprVariable, true/*bRecursiveSearch*/, true/*bCreateIfUnknown*/, true/*bReferencedVariable*/);
		return pVariable;
	}

	void DtaScriptVariable::moveToNode(DtaScriptVariable& node) {
		if (node._pAttributes == NULL) {
			node._pAttributes = _pAttributes;
		} else {
			DtaScriptVariableList* pAttributes = node._pAttributes;
			while (pAttributes->getNext() != NULL) pAttributes = pAttributes->getNext();
			pAttributes->setNext(_pAttributes);
		}
		_pAttributes = NULL;
		if (node._pArray == NULL) {
			node._pArray = _pArray;
		} else {
			while (!_pArray->items.empty()) {
				node._pArray->items.push_back(_pArray->items.back());
				_pArray->items.pop_back();
			}
			delete node._pArray->cache;
			node._pArray->cache = NULL;
			delete _pArray;
		}
		_pArray = NULL;
		node._eValueNode = _eValueNode;
		node._value = _value;
		_eValueNode = NO_VALUE_NODE;
	}

	DtaScriptVariable* DtaScriptVariable::createNodeArgument(const char* sName, DtaScriptVariable* pParameter) {
		DtaScriptVariable* pIndex = NULL;
		DtaScriptVariableList* pAttributes = _pAttributes;
		DtaScriptVariableList* pLastAttribute = _pAttributes;
		while (pAttributes != NULL) {
			if (strcmp(pAttributes->getNode()->_tcName, sName) == 0) {
				pIndex = pAttributes->getNode();
				break;
			}
			if (pAttributes->getNext() == NULL) pLastAttribute = pAttributes;
			pAttributes = pAttributes->getNext();
		}
		if (pAttributes == NULL) {
			pIndex = new DtaScriptVariable(this, sName);
			if (_pAttributes == NULL) _pAttributes = new DtaScriptVariableList(pIndex);
			else pLastAttribute->pushBack(pIndex);
			if (_pListeners != NULL) _pListeners->onAddAttribute(pIndex);
		}
		pIndex->setValue(pParameter);
		return pIndex;
	}

	DtaScriptVariable* DtaScriptVariable::getNodeArgument(const char* sName) const {
		DtaScriptVariable* pIndex = NULL;
		DtaScriptVariableList* pAttributes = _pAttributes;
		DtaScriptVariableList* pLastAttribute = _pAttributes;
		while (pAttributes != NULL) {
			if (strcmp(pAttributes->getNode()->_tcName, sName) == 0) {
				pIndex = pAttributes->getNode();
				break;
			}
			if (pAttributes->getNext() == NULL) pLastAttribute = pAttributes;
			pAttributes = pAttributes->getNext();
		}
		if (pAttributes == NULL) {
			if (_pParent != NULL) pIndex = _pParent->getNodeArgument(sName);
			if (pIndex == NULL) {
				throw UtlException(std::string("unable to find argument called '") + sName + "'");
			}
		}
		return pIndex;
	}

	DtaScriptVariable* DtaScriptVariable::createIterator(const char* sName, DtaArrayIterator** pIteratorData) {
		DtaScriptVariable* pIndex = NULL;
		DtaScriptVariableList* pAttributes = _pAttributes;
		DtaScriptVariableList* pLastAttribute = _pAttributes;
		while (pAttributes != NULL) {
			if (strcmp(pAttributes->getNode()->_tcName, sName) == 0) {
				pIndex = pAttributes->getNode();
				break;
			}
			if (pAttributes->getNext() == NULL) pLastAttribute = pAttributes;
			pAttributes = pAttributes->getNext();
		}
		if (pAttributes == NULL) {
			pIndex = new DtaScriptVariable(this, sName);
			if (_pAttributes == NULL) _pAttributes = new DtaScriptVariableList(pIndex);
			else pLastAttribute->pushBack(pIndex);
			if (_pListeners != NULL) _pListeners->onAddAttribute(pIndex);
		}
		pIndex->setIteratorData(pIteratorData);
		return pIndex;
	}

	DtaScriptVariable* DtaScriptVariable::getIterator(const char* sName) const {
		DtaScriptVariable* pIndex = NULL;
		DtaScriptVariableList* pAttributes = _pAttributes;
		DtaScriptVariableList* pLastAttribute = _pAttributes;
		while (pAttributes != NULL) {
			if (strcmp(pAttributes->getNode()->_tcName, sName) == 0) {
				pIndex = pAttributes->getNode();
				break;
			}
			if (pAttributes->getNext() == NULL) pLastAttribute = pAttributes;
			pAttributes = pAttributes->getNext();
		}
		if (pAttributes == NULL) {
			if (_pParent != NULL) pIndex = _pParent->getIterator(sName);
		}
		if (pIndex != NULL) {
			while (pIndex->_eValueNode == REFERENCE_VALUE_NODE) pIndex = pIndex->_value.var;
			if (pIndex->_eValueNode != INDEX_VALUE_NODE) return NULL;
		}
		return pIndex;
	}

	DtaArrayIterator* DtaScriptVariable::getIteratorData() const {
		if (_eValueNode != INDEX_VALUE_NODE) return NULL;
		return *_value.ind.var;
	}

	DtaScriptVariable* DtaScriptVariable::addElement(int iKey) {
		char tcKey[32];
		sprintf(tcKey, "%d", iKey);
		return addElement(tcKey);
	}

	DtaScriptVariable* DtaScriptVariable::addElement(const std::string& sName) {
		DtaScriptVariable* pVariable = getArrayElement(sName);
		if (pVariable == NULL) pVariable = addInternalElement(sName);
		return pVariable;
	}

	DtaScriptVariable* DtaScriptVariable::addInternalElement(const std::string& sName) {
		DtaScriptVariable* pVariable = new DtaScriptVariable(this, sName);
		if (_pArray == NULL) _pArray = new NodeArrayType(pVariable);
		else {
			_pArray->items.push_back(pVariable);
			if (_pArray->cache != NULL) (*_pArray->cache)[sName] = pVariable;
		}
		if (_pListeners != NULL) _pListeners->onArrayHasChanged();
		return pVariable;
	}

	void DtaScriptVariable::setValueAtVariable(const char* sVariable, const char* sValue, bool bRecursiveSearch, bool bCreateIfUnknown) {
		getVariable(sVariable, bRecursiveSearch, bCreateIfUnknown)->setValue(sValue);
	}

	const char* DtaScriptVariable::getValue() const {
		if (_eValueNode == STRING_VALUE_NODE) return _value.str.text;
		if (_eValueNode == REFERENCE_VALUE_NODE) return _value.var->getValue();
		if (_eValueNode == EXTERNAL_VALUE_NODE) return _value.ext->getValue();
		if (_eValueNode == INDEX_VALUE_NODE) return (*_value.ind.var)->item()->getValue();
		return NULL;
	}

	size_t DtaScriptVariable::getValueLength() const {
		if (_eValueNode == STRING_VALUE_NODE) return _value.str.length;
		if (_eValueNode == REFERENCE_VALUE_NODE) return _value.var->getValueLength();
		if (_eValueNode == EXTERNAL_VALUE_NODE) return _value.ext->getValueLength();
		if (_eValueNode == INDEX_VALUE_NODE) return (*_value.ind.var)->item()->getValueLength();
		return 0;
	}

	double DtaScriptVariable::getDoubleValue() const {
		if (_eValueNode == STRING_VALUE_NODE) return atof(_value.str.text);
		if (_eValueNode == REFERENCE_VALUE_NODE) return _value.var->getDoubleValue();
		if (_eValueNode == EXTERNAL_VALUE_NODE) return _value.ext->getDoubleValue();
		if (_eValueNode == INDEX_VALUE_NODE) return (*_value.ind.var)->item()->getDoubleValue();
		return 0.0;
	}

	bool DtaScriptVariable::getBooleanValue() const {
		if (_eValueNode == STRING_VALUE_NODE) return (_value.str.length != 0);
		if (_eValueNode == REFERENCE_VALUE_NODE) return _value.var->getBooleanValue();
		if (_eValueNode == EXTERNAL_VALUE_NODE) return _value.ext->getBooleanValue();
		if (_eValueNode == INDEX_VALUE_NODE) return (*_value.ind.var)->item()->getBooleanValue();
		return false;
	}

	int DtaScriptVariable::getIntValue() const {
		if (_eValueNode == STRING_VALUE_NODE) return atoi(_value.str.text);
		if (_eValueNode == REFERENCE_VALUE_NODE) return _value.var->getIntValue();
		if (_eValueNode == EXTERNAL_VALUE_NODE) return _value.ext->getIntValue();
		if (_eValueNode == INDEX_VALUE_NODE) return (*_value.ind.var)->item()->getIntValue();
		return 0;
	}

	void DtaScriptVariable::clearValue() {
		if (_eValueNode == REFERENCE_VALUE_NODE) {
			removeReference();
		} else if (_eValueNode == INDEX_VALUE_NODE) {
			if (_value.ind.own) {
				delete *_value.ind.var;
				delete _value.ind.var;
			}
		} else if (_eValueNode == STRING_VALUE_NODE) {
			delete [] _value.str.text;
		} else if (_eValueNode == EXTERNAL_VALUE_NODE) {
			EXTERNAL_VALUE_NODE_DESTRUCTOR f = _value.ext->getDestructor();
			if (f != NULL) f(_value.ext);
		}
		_eValueNode = NO_VALUE_NODE;
	}

	void DtaScriptVariable::setValue(const char* tcValue) {
		if (_pListeners != NULL) _pListeners->onSetValue(tcValue);
		if ((tcValue == NULL) || (*tcValue == '\0')) {
			clearValue();
		} else {
			size_t iLength = strlen(tcValue);
			if (_eValueNode == STRING_VALUE_NODE) {
				if (iLength > _value.str.length) {
					delete [] _value.str.text;
					_value.str.text = new char[iLength + 1];
				}
			} else {
				if (_eValueNode == REFERENCE_VALUE_NODE) {
					removeReference();
				} else if (_eValueNode == INDEX_VALUE_NODE) {
					if (_value.ind.own) {
						delete *_value.ind.var;
						delete _value.ind.var;
					}
				} else if (_eValueNode == EXTERNAL_VALUE_NODE) {
					EXTERNAL_VALUE_NODE_DESTRUCTOR f = _value.ext->getDestructor();
					if (f != NULL) f(_value.ext);
				}
				_value.str.text = new char[iLength + 1];
			}
			strcpy(_value.str.text, tcValue);
			_value.str.length = iLength;
			_eValueNode = STRING_VALUE_NODE;
		}
	}

	void DtaScriptVariable::setValue(bool bValue) {
		setValue(((bValue) ? "true" : NULL));
	}

	void DtaScriptVariable::setValue(double dValue) {
		std::string sDouble = CGRuntime::toString(dValue);
		setValue(sDouble.c_str());
	}

	void DtaScriptVariable::setValue(int iValue) {
		char tcNumber[16];
		sprintf(tcNumber, "%d", iValue);
		setValue(tcNumber);
	}

	void DtaScriptVariable::setValue(const std::list<std::string>& listOfValues) {
		int iIndex = 0;
		char sNumber[16];
		clearArray();
		for (std::list<std::string>::const_iterator i = listOfValues.begin(); i != listOfValues.end(); i++) {
			sprintf(sNumber, "%d", iIndex);
			addElement(sNumber)->setValue(i->c_str());
			iIndex++;
		}
		if (_pListeners != NULL) _pListeners->onArrayHasChanged();
	}

	void DtaScriptVariable::setValue(DtaScriptVariable* pVariable) {
		if (pVariable != NULL) {
			if (pVariable == this) throw UtlException("DtaScriptVariable::setValue() cyclic reference");
			while (pVariable->_eValueNode == REFERENCE_VALUE_NODE) {
				pVariable = pVariable->getReferencedVariable();
				if (pVariable == this) throw UtlException("DtaScriptVariable::setValue() cyclic reference");
			}
		}
		clearValue();
		if (pVariable != NULL) {
			_eValueNode = REFERENCE_VALUE_NODE;
			_value.var = pVariable;
			_value.var->addReferencedBy(this);
			if (_pListeners != NULL) _pListeners->onSetReference(pVariable);
		}
	}

	void DtaScriptVariable::setValue(ExternalValueNode* pExternalValue) {
		if ((_eValueNode == EXTERNAL_VALUE_NODE) && (pExternalValue == _value.ext)) return;
		clearValue();
		_eValueNode = EXTERNAL_VALUE_NODE;
		_value.ext = pExternalValue;
		if (_pListeners != NULL) _pListeners->onSetExternal(pExternalValue);
	}

	void DtaScriptVariable::setValue(DtaArrayIterator* pIteratorData) {
		DtaArrayIterator** ppIteratorData = new DtaArrayIterator*;
		*ppIteratorData = pIteratorData;
		setIteratorData(ppIteratorData);
		_value.ind.own = true;
	}

	void DtaScriptVariable::setIteratorData(DtaArrayIterator** pIteratorData) {
		clearValue();
		_eValueNode = INDEX_VALUE_NODE;
		_value.ind.var = pIteratorData;
		_value.ind.own = false;
		if (_pListeners != NULL) _pListeners->onSetIterator(pIteratorData);
	}

	void DtaScriptVariable::concatValue(const char* tcValue) {
		if (_pListeners != NULL) _pListeners->onConcatValue(tcValue);
		if ((tcValue != NULL) && (*tcValue != '\0')) {
			size_t iLength = strlen(tcValue);
			if (_eValueNode == STRING_VALUE_NODE) {
				char* tcOld = _value.str.text;
				_value.str.text = new char[_value.str.length + iLength + 1];
				memcpy(_value.str.text, tcOld, _value.str.length);
				strcpy(_value.str.text + _value.str.length, tcValue);
				_value.str.length += iLength;
				delete [] tcOld;
			} else {
				if (_eValueNode == REFERENCE_VALUE_NODE) {
					removeReference();
				} else if (_eValueNode == INDEX_VALUE_NODE) {
					if (_value.ind.own) {
						delete *_value.ind.var;
						delete _value.ind.var;
					}
				} else if (_eValueNode == EXTERNAL_VALUE_NODE) {
					EXTERNAL_VALUE_NODE_DESTRUCTOR f = _value.ext->getDestructor();
					if (f != NULL) f(_value.ext);
				}
				_value.str.text = new char[iLength + 1];
				strcpy(_value.str.text, tcValue);
				_value.str.length = iLength;
				_eValueNode = STRING_VALUE_NODE;
			}
		}
	}

	void DtaScriptVariable::remove(DtaScriptVariable* pVariable) {
		if (getReferencedVariable() != NULL) getReferencedVariable()->remove(pVariable);
		else {
			if (_pAttributes != NULL) {
				if (_pAttributes->getNode() == pVariable) {
					DtaScriptVariableList* pNext = _pAttributes->getNext();
					if (_pListeners != NULL) _pListeners->onRemoveAttribute(pVariable);
					delete pVariable;
					_pAttributes->setNext(NULL);
					delete _pAttributes;
					_pAttributes = pNext;
					return;
				} else {
					DtaScriptVariableList* pAttributes = _pAttributes;
					DtaScriptVariableList* pNext = pAttributes->getNext();
					while (pNext != NULL) {
						if (pNext->getNode() == pVariable) {
							if (_pListeners != NULL) _pListeners->onRemoveAttribute(pVariable);
							pAttributes->setNext(pNext->getNext());
							delete pVariable;
							pNext->setNext(NULL);
							delete pNext;
							return;
						}
						pAttributes = pNext;
						pNext = pNext->getNext();
					}
				}
			}
			if (_pArray != NULL) {
				for (std::list<DtaScriptVariable*>::iterator i = _pArray->items.begin(); i != _pArray->items.end(); i++) {
					if ((*i) == pVariable) {
						std::string sPosition = pVariable->getName();
						delete (*i);
						_pArray->items.erase(i);
						if (_pArray->cache != NULL) _pArray->cache->erase(sPosition);
						if (_pListeners != NULL) _pListeners->onArrayHasChanged();
						return;
					}
				}
			}
		}
	}

	void DtaScriptVariable::remove(const char* sAttribute) {
		if (_pAttributes != NULL) {
			if (strcmp(_pAttributes->getNode()->_tcName, sAttribute) == 0) {
				if (_pListeners != NULL) _pListeners->onRemoveAttribute(_pAttributes->getNode());
				DtaScriptVariableList* pNext = _pAttributes->getNext();
				delete _pAttributes->getNode();
				_pAttributes->setNext(NULL);
				delete _pAttributes;
				_pAttributes = pNext;
			} else {
				DtaScriptVariableList* pAttributes = _pAttributes;
				DtaScriptVariableList* pNext = pAttributes->getNext();
				while (pNext != NULL) {
					if (strcmp(pNext->getNode()->_tcName, sAttribute) == 0) {
						if (_pListeners != NULL) _pListeners->onRemoveAttribute(pNext->getNode());
						pAttributes->setNext(pNext->getNext());
						delete pNext->getNode();
						pNext->setNext(NULL);
						delete pNext;
						break;
					}
					pAttributes = pNext;
					pNext = pNext->getNext();
				}
			}
		}
	}

	void DtaScriptVariable::removeRecursive(const char* sAttribute) {
		remove(sAttribute);
		if (_pAttributes != NULL) {
			DtaScriptVariableList* pAttributes = _pAttributes;
			while (pAttributes != NULL) {
				pAttributes->getNode()->removeRecursive(sAttribute);
				pAttributes = pAttributes->getNext();
			}
		}
		if (_pArray != NULL) {
			for (std::list<DtaScriptVariable*>::iterator j = _pArray->items.begin(); j != _pArray->items.end(); j++) {
				if ((*j)->getParent() == this) (*j)->removeRecursive(sAttribute);
			}
		}
	}

	void DtaScriptVariable::clearContent() {
		clearValue();
		while (_pAttributes != NULL) {
			DtaScriptVariableList* pCurrent = _pAttributes;
			_pAttributes = pCurrent->getNext();
			delete pCurrent->getNode();
			pCurrent->setNext(NULL);
			delete pCurrent;
		}
		clearArray();
		if (_pListeners != NULL) _pListeners->onClearContent();
	}

	void DtaScriptVariable::addArrayElements(const DtaScriptVariable& variable) {
		NodeArrayType* pANT = variable._pArray;
		if ((pANT != NULL) && !pANT->items.empty()) {
			if (_pArray == NULL) _pArray = new NodeArrayType;
			for (std::list<DtaScriptVariable*>::const_iterator j = pANT->items.begin(); j != pANT->items.end(); j++) {
				DtaScriptVariable* pVariable = new DtaScriptVariable(this, (*j)->_tcName);
				pVariable->copyAll(*(*j), false);
				pVariable->setValue((*j)->getValue());
				_pArray->items.push_back(pVariable);
				if (_pArray->cache != NULL) (*_pArray->cache)[pVariable->_tcName] = pVariable;
			}
			if (_pListeners != NULL) _pListeners->onArrayHasChanged();
		}
	}

	int DtaScriptVariable::getNumberOfAttributes() const {
		int iSize = 0;
		DtaScriptVariableList* pVariableAttributes = _pAttributes;
		while (pVariableAttributes != NULL) {
			iSize++;
			pVariableAttributes = pVariableAttributes->getNext();
		}
		return iSize;
	}

	bool DtaScriptVariable::equal(const DtaScriptVariable& variable) {
		if (_pArray == NULL) {
			if (variable._pArray != NULL) {
				if (!variable._pArray->items.empty()) return false;
			}
		} else if (variable._pArray == NULL) {
			if (!_pArray->items.empty()) return false;
			delete _pArray;
			_pArray = NULL;
		} else if (_pArray->items.size() != variable._pArray->items.size()) return false;
		if (_eValueNode != variable._eValueNode) return false;
		if (_eValueNode == REFERENCE_VALUE_NODE) {
			if (_value.var != variable._value.var) return false;
		} else if (_eValueNode == INDEX_VALUE_NODE) {
			if ((*_value.ind.var)->item() != (*variable._value.ind.var)->item()) return false;
		} else if (_eValueNode == STRING_VALUE_NODE) {
			if (strcmp(_value.str.text, variable._value.str.text) != 0) return false;
		} else if (_eValueNode == EXTERNAL_VALUE_NODE) {
			if (_value.ext != variable._value.ext) return false;
		}
		DtaScriptVariableList* pAttributes = _pAttributes;
		if (getNumberOfAttributes() != variable.getNumberOfAttributes()) return false;
		while (pAttributes != NULL) {
			DtaScriptVariableList* pVariableAttributes = variable._pAttributes;
			DtaScriptVariable* pVariable = pAttributes->getNode();
			while (pVariableAttributes != NULL) {
				if (strcmp(pVariable->_tcName, pVariableAttributes->getNode()->_tcName) == 0) {
					if (!pVariable->equal(*(pVariableAttributes->getNode()))) return false;
					break;
				}
				pVariableAttributes = pVariableAttributes->getNext();
			}
			if (pVariableAttributes == NULL) return false;
			pAttributes = pAttributes->getNext();
		}
		std::list<DtaScriptVariable*>::iterator i;
		std::list<DtaScriptVariable*>::const_iterator j;
		if (_pArray != NULL) {
			if (getArraySize() != variable.getArraySize()) return false;
			for (i = _pArray->items.begin(); i != _pArray->items.end(); i++) {
				for (j = variable._pArray->items.begin(); j != variable._pArray->items.end(); ++j) {
					if (strcmp((*i)->_tcName, (*j)->_tcName) == 0) {
						if (!(*i)->equal(*(*j))) return false;
						break;
					}
				}
				if (j == variable._pArray->items.end()) return false;
			}
		}
		return true;
	}

	void DtaScriptVariable::copyAll(const DtaScriptVariable& variable, bool bMerge) {
		if (!bMerge) clearContent();
		const char* tcValue = variable.getValue();
		if ((tcValue != NULL) && (*tcValue != '\0')) setValue(tcValue);
		DtaScriptVariableList* pVariableAttributes = variable._pAttributes;
		while (pVariableAttributes != NULL) {
			const char* tcName = pVariableAttributes->getNode()->_tcName;
			DtaScriptVariable* pVariable = NULL;
			DtaScriptVariableList* pAttributes = _pAttributes;
			DtaScriptVariableList* pLastAttribute = NULL;
			while (pAttributes != NULL) {
				if (strcmp(pAttributes->getNode()->_tcName, tcName) == 0) {
					pVariable = pAttributes->getNode();
					break;
				}
				if (pAttributes->getNext() == NULL) pLastAttribute = pAttributes;
				pAttributes = pAttributes->getNext();
			}
			if (pVariable == NULL) {
				pVariable = new DtaScriptVariable(this, tcName);
				if (pLastAttribute == NULL) _pAttributes = new DtaScriptVariableList(pVariable);
				else pLastAttribute->pushBack(pVariable);
				if (_pListeners != NULL) _pListeners->onAddAttribute(pVariable);
			}
			if (pVariableAttributes->getNode()->_eValueNode == REFERENCE_VALUE_NODE) {
				pVariable->_eValueNode = REFERENCE_VALUE_NODE;
				pVariable->_value.var = pVariableAttributes->getNode()->_value.var;
				pVariable->_value.var->addReferencedBy(pVariable);
			} else if (pVariableAttributes->getNode()->_eValueNode == INDEX_VALUE_NODE) {
				pVariable->_eValueNode = REFERENCE_VALUE_NODE;
				pVariable->_value.var = (*pVariableAttributes->getNode()->_value.ind.var)->item();
				pVariable->_value.var->addReferencedBy(pVariable);
			} else {
				pVariable->copyAll(*(pVariableAttributes->getNode()), bMerge);
			}
			pVariableAttributes = pVariableAttributes->getNext();
		}
		NodeArrayType* pANT = variable._pArray;
		if ((pANT != NULL) && !pANT->items.empty()) {
			if (_pArray == NULL) {
				_pArray = new NodeArrayType;
				_pArray->cache = new std::map<std::string, DtaScriptVariable*>();
			} else if (_pArray->cache == NULL) {
				_pArray->cache = new std::map<std::string, DtaScriptVariable*>();
				for (std::list<DtaScriptVariable*>::const_iterator i = _pArray->items.begin(); i != _pArray->items.end(); i++) (*_pArray->cache)[(*i)->_tcName] = (*i);
			}
			for (std::list<DtaScriptVariable*>::const_iterator i = pANT->items.begin(); i != pANT->items.end(); i++) {
				const char* tcName = (*i)->_tcName;
				std::map<std::string, DtaScriptVariable*>::const_iterator cursor = _pArray->cache->find(tcName);
				DtaScriptVariable* pVariable = NULL;
				if (cursor != _pArray->cache->end()) {
					pVariable = cursor->second;
				} else {
					pVariable = new DtaScriptVariable(this, tcName);
					_pArray->items.push_back(pVariable);
					(*_pArray->cache)[tcName] = pVariable;
				}
				if ((*i)->_eValueNode == REFERENCE_VALUE_NODE) {
					pVariable->_eValueNode = REFERENCE_VALUE_NODE;
					pVariable->_value.var = (*i)->_value.var;
					pVariable->_value.var->addReferencedBy(pVariable);
				} else if ((*i)->_eValueNode == INDEX_VALUE_NODE) {
					pVariable->_eValueNode = REFERENCE_VALUE_NODE;
					pVariable->_value.var = (*(*i)->_value.ind.var)->item();
					pVariable->_value.var->addReferencedBy(pVariable);
				} else {
					pVariable->copyAll(*(*i), bMerge);
				}
			}
		}
	}

	DtaScriptVariable* DtaScriptVariable::pushItem(const std::string& sValue) {
		char tcNumber[32];
		if (_pArray == NULL) _pArray = new NodeArrayType;
		sprintf(tcNumber, "%d", _pArray->items.size());
		std::string sNumber = tcNumber;
		DtaScriptVariable* pVariable = new DtaScriptVariable(this, sNumber);
		_pArray->items.push_back(pVariable);
		if (_pArray->cache != NULL) {
			(*_pArray->cache)[sNumber] = pVariable;
		}
		if (!sValue.empty()) pVariable->setValue(sValue.c_str());
		if (_pListeners != NULL) _pListeners->onArrayHasChanged();
		return pVariable;
	}

	void DtaScriptVariable::slideNodeContent(DtaScriptVariable& visibility, const ExprScriptVariable& varExpr) {
		DtaScriptVariable* pTemporary = new DtaScriptVariable;
		pTemporary->copyAll(*this);
		clearContent();
		DtaScriptVariable* pDestVariable = getNonRecursiveVariable(visibility, varExpr);
		DtaScriptVariable* pParent = pDestVariable->getParent();
		DtaScriptVariableList* pParentAttributes = pParent->_pAttributes;
		while (pParentAttributes != NULL) {
			if (pParentAttributes->getNode() == pDestVariable) {
				pParentAttributes->setNode(pTemporary);
				free((char*) pTemporary->_tcName);
#ifdef WIN32
				pTemporary->_tcName = ::_strdup(pDestVariable->_tcName);
#else
				pTemporary->_tcName = ::strdup(pDestVariable->_tcName);
#endif
				pTemporary->_pParent = pParent;
				break;
			}
			pParentAttributes = pParentAttributes->getNext();
		}
		if (pParentAttributes == NULL) {
			NodeArrayType* pANT = pParent->_pArray;
			if (pANT != NULL) {
				std::list<DtaScriptVariable*>::iterator i;
				for (i = pANT->items.begin(); i != pANT->items.end(); i++) {
					if ((*i) == pDestVariable) {
						(*i) = pTemporary;
						free((char*) pTemporary->_tcName);
#ifdef WIN32
						pTemporary->_tcName = ::_strdup(pDestVariable->_tcName);
#else
						pTemporary->_tcName = ::strdup(pDestVariable->_tcName);
#endif
						pTemporary->_pParent = pParent;
						break;
					}
				}
				if (i == pANT->items.end()) {
					delete pTemporary;
					throw UtlException("internal error: unable to slide a node content because destination node isn't attached to its parent properly");
				}
			} else {
				delete pTemporary;
				throw UtlException("internal error: unable to slide a node content because destination node isn't attached to its parent properly");
			}
		}
		delete pDestVariable;
	}

	bool DtaScriptVariable::traceValue(int iDepth, const std::string& sIndent, const std::string& sPrefix) {
		if (_eValueNode == STRING_VALUE_NODE) {
			if (_value.str.length != 0) CGRuntime::traceLine(sPrefix + "\"" + std::string(_value.str.text) + "\"");
			else return false;
		} else if ((_eValueNode & 0x0FF) == INDEX_VALUE_NODE) {
			DtaScriptVariable* pNode = (*_value.ind.var)->item();
			const char* tcValue = pNode->getValue();
			if (tcValue != NULL) {
				CGRuntime::traceLine(sPrefix + "<index> '" + pNode->getCompleteName() + "' = \"" + std::string(tcValue) + "\"");
			} else {
				CGRuntime::traceLine(sPrefix + "<index> '" + pNode->getCompleteName() + "'");
			}
			pNode->traceObject(iDepth, sIndent);
		} else if ((_eValueNode & 0x0FF) == REFERENCE_VALUE_NODE) {
			const char* tcValue = _value.var->getValue();
			if (tcValue != NULL) {
				CGRuntime::traceLine(sPrefix + "-> '" + _value.var->getCompleteName() + "' = \"" + std::string(tcValue) + "\"");
			} else {
				CGRuntime::traceLine(sPrefix + "-> '" + _value.var->getCompleteName() + "'");
			}
			_value.var->traceObject(iDepth, sIndent);
		} else if (_eValueNode == EXTERNAL_VALUE_NODE) {
			const char* tcValue = _value.ext->getValue();
			if (tcValue != NULL) {
				CGRuntime::traceLine(sPrefix + ">> external value node = \"" + std::string(tcValue) + "\"");
			} else {
				CGRuntime::traceLine(sPrefix + ">> external value node");
			}
		} else {
			return false;
		}
		return true;
	}

	void DtaScriptVariable::traceObject(int iDepth, const std::string& sIndent) {
		if (iDepth < 0) return;
		std::string sCurrentIndent = sIndent + "    ";
		std::string sNextIndent = sCurrentIndent;
		if (sIndent.empty()) {
			char tcMessage[64];
			if (iDepth > 0) sprintf(tcMessage, "' to depth %d:", iDepth);
			else strcpy(tcMessage, "':");
			CGRuntime::traceLine("Tracing variable '" + getCompleteName() + tcMessage);
			if (_eValueNode == INDEX_VALUE_NODE) traceValue(iDepth, sNextIndent, "\t");
			else traceValue(iDepth - 1, sNextIndent, "\t");
		} else if (_pAttributes != NULL) {
			CGRuntime::traceLine(sNextIndent + "|--+");
			sNextIndent += "    ";
		}
		DtaScriptVariableList* pAttributes = _pAttributes;
		while (pAttributes != NULL) {
			DtaScriptVariable* pNode = pAttributes->getNode();
			const char* tcValue = pNode->getValue();
			std::string sAttribute = sNextIndent + pNode->_tcName;
			if (!pNode->traceValue(iDepth - 1, sNextIndent, sAttribute + " = ")) {
				CGRuntime::traceLine(sAttribute);
			}
			if (iDepth > 0) {
				pNode->traceObject(iDepth - 1, sNextIndent);
			} else {
				NodeArrayType* pANT = pNode->_pArray;
				if ((pANT != NULL) && !pANT->items.empty()) {
					CGRuntime::traceText(sCurrentIndent + std::string(pNode->_tcName) + "[");
					for (std::list<DtaScriptVariable*>::iterator j = pANT->items.begin(); j != pANT->items.end(); j++) {
						if (j != pANT->items.begin()) CGRuntime::traceText(", ");
						CGRuntime::traceText("\"" + std::string((*j)->_tcName) + "\"");
					}
					CGRuntime::traceLine("]");
				}
			}
			pAttributes = pAttributes->getNext();
		}
		if ((_pArray != NULL) && !_pArray->items.empty()) {
			if (iDepth <= 0) {
				CGRuntime::traceText(sCurrentIndent + "[");
				for (std::list<DtaScriptVariable*>::iterator k = _pArray->items.begin(); k != _pArray->items.end(); k++) {
					if (k != _pArray->items.begin()) CGRuntime::traceText(", ");
					CGRuntime::traceText("\"" + std::string((*k)->_tcName) + "\"");
					const char* tcValue = (*k)->getValue();
					if ((tcValue != NULL) && (*tcValue != '\0')) CGRuntime::traceText(" -> \"" + std::string(tcValue) + "\"");
				}
				CGRuntime::traceLine("]");
			} else {
				for (std::list<DtaScriptVariable*>::iterator k = _pArray->items.begin(); k != _pArray->items.end(); k++) {
					CGRuntime::traceText(sCurrentIndent + "[\"" + std::string((*k)->_tcName) + "\"]");
					if (!(*k)->traceValue(iDepth - 1, sCurrentIndent, " = ")) CGRuntime::traceLine("");
					(*k)->traceObject(iDepth - 1, sCurrentIndent);
				}
			}
		}
		if (sIndent.empty()) {
			CGRuntime::traceLine("End of variable's trace '" + getCompleteName() + "'.");
		}
	}

	void DtaScriptVariable::traceStack() {
		if (strcmp(_tcName, "project") == 0) return;
		if (_pParent != 0) _pParent->traceStack();
		CGRuntime::traceLine(std::string(_tcName) + ":");
		DtaScriptVariableList* pAttributes = _pAttributes;
		while (pAttributes != NULL) {
			DtaScriptVariable* pNode = pAttributes->getNode();
			if (pNode->getReferencedVariable() != NULL) {
				std::string sReference = pNode->getReferencedVariable()->getCompleteName();
				CGRuntime::traceLine(std::string(pNode->_tcName) + " = " + sReference);
			} else {
				const char* tcValue = pNode->getValue();
				if (tcValue != NULL) {
					CGRuntime::traceLine(std::string(pNode->_tcName) + " = \"" + std::string(tcValue) + "\"");
				} else {
					CGRuntime::traceLine(pNode->_tcName);
				}
			}
			NodeArrayType* pANT = pNode->_pArray;
			if ((pANT != NULL) && !pANT->items.empty()) {
				CGRuntime::traceText("\t" + std::string(pNode->_tcName) + "[");
				for (std::list<DtaScriptVariable*>::iterator j = pANT->items.begin(); j != pANT->items.end(); j++) {
					if (j != pANT->items.begin()) CGRuntime::traceText(", ");
					CGRuntime::traceText("\"" + std::string((*j)->_tcName) + "\"");
				}
				CGRuntime::traceLine("]");
			}
			pAttributes = pAttributes->getNext();
		}
		if ((_pArray != NULL) && !_pArray->items.empty()) {
			CGRuntime::traceText("\t" + std::string(_tcName) + "[");
			for (std::list<DtaScriptVariable*>::iterator k = _pArray->items.begin(); k != _pArray->items.end(); k++) {
				if (k != _pArray->items.begin()) CGRuntime::traceText(", ");
				CGRuntime::traceText("\"" + std::string((*k)->_tcName) + "\"");
			}
			CGRuntime::traceLine("]");
		}
	}
}
