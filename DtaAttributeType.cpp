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

#include "DtaProject.h"
#include "DtaScriptVariable.h"
#include "DtaAttributeType.h"

namespace CodeWorker {
	std::map<std::string, DtaAttributeType*> DtaAttributeType::_mapOfTypes;


	bool DtaFollowingAttributeInfo::isAttribute() const {
		return _pAttributeType->isAttribute();
	}

	void DtaFollowingAttributeInfo::insertAttribute(DtaScriptVariable& myUser) {
		incrementUse();
	}


	DtaAttributeType::DtaAttributeType(const std::string& sName) : _iNumberOfUse(0), _iNumberOfElements(0), _sName(sName) {
		_mapOfTypes[sName] = this;
	}

	DtaAttributeType* DtaAttributeType::getAttributeType(const std::string& sName) {
		std::map<std::string, DtaAttributeType*>::const_iterator i = _mapOfTypes.find(sName);
		if (i == _mapOfTypes.end()) return NULL;
		return i->second;
	}

	bool DtaAttributeType::isAttribute() const {
		return _mapOfAttributes.empty() && _mapOfAttributeElements.empty();
	}

	void DtaAttributeType::insertUser(DtaScriptVariable& myUser) {
		_iNumberOfUse++;
		const std::list<DtaScriptVariable*>* pArray = myUser.getArray();
		if (pArray != NULL) {
			for (std::list<DtaScriptVariable*>::const_iterator i = pArray->begin(); i != pArray->end(); i++) {
				insertElement(*(*i));
			}
		}
		DtaScriptVariableList* pAttributes = myUser.getAttributes();
		while (pAttributes != NULL) {
			insertAttribute(*(pAttributes->getNode()));
			pAttributes = pAttributes->getNext();
		}
	}

	void DtaAttributeType::insertElement(DtaScriptVariable& myUser) {
		_iNumberOfElements++;
		DtaScriptVariableList* pAttributes = myUser.getAttributes();
		while (pAttributes != NULL) {
			DtaScriptVariable* pNode = pAttributes->getNode();
			std::string sKey = pNode->getName();
			DtaAttributeType* pType = getAttributeType(sKey);
			if (pType == NULL) pType = new DtaAttributeType(sKey);
			std::map<std::string, DtaFollowingAttributeInfo*>::const_iterator j = _mapOfAttributeElements.find(sKey);
			DtaFollowingAttributeInfo* pInfo;
			if (j == _mapOfAttributeElements.end()) {
				pInfo = new DtaFollowingAttributeInfo(pType);
				_mapOfAttributeElements[sKey] = pInfo;
			} else pInfo = j->second;
			pInfo->insertAttribute(*pNode);
			pType->insertUser(*pNode);
			pAttributes = pAttributes->getNext();
		}
	}

	void DtaAttributeType::insertAttribute(DtaScriptVariable& myUser) {
		std::string sKey = myUser.getName();
		DtaAttributeType* pType = getAttributeType(sKey);
		if (pType == NULL) pType = new DtaAttributeType(sKey);
		std::map<std::string, DtaFollowingAttributeInfo*>::const_iterator i = _mapOfAttributes.find(sKey);
		DtaFollowingAttributeInfo* pInfo;
		if (i == _mapOfAttributes.end()) {
			pInfo = new DtaFollowingAttributeInfo(pType);
			_mapOfAttributes[sKey] = pInfo;
		} else pInfo = i->second;
		pInfo->insertAttribute(myUser);
		pType->insertUser(myUser);
	}

	DtaAttributeType& DtaAttributeType::extractProjectTypes(DtaScriptVariable& visibility) {
		removeProjectTypes();
		DtaAttributeType* pProjectType = new DtaAttributeType("project");
		pProjectType->insertUser(DtaProject::getInstance());
		return *pProjectType;
	}

	void DtaAttributeType::removeProjectTypes() {
		for (std::map<std::string, DtaAttributeType*>::iterator i = _mapOfTypes.begin(); i != _mapOfTypes.end(); i++) {
			delete i->second;
		}
		_mapOfTypes = std::map<std::string, DtaAttributeType*>();
	}
}
