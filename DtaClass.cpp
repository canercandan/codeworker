/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2006 Cédric Lemaire

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

#include "DtaClass.h"

namespace CodeWorker {
	std::vector<DtaClass*> DtaClass::_idToClass;


	DtaClass::DtaClass(GrfBlock* pBlock, const std::string& sName) : _pBlock(pBlock), _sName(sName), _pExtendedClass(NULL) {
		_idToClass.push_back(this);
		_iId = (EXPRESSION_TYPE) (_idToClass.size() << 8);
	}

	DtaClass::~DtaClass() {
		unsigned int iIndex = ((unsigned int) _iId) >> 8;
		_idToClass[iIndex - 1] = NULL;
	}

	EXPRESSION_TYPE DtaClass::getAttributeType(const std::string& sAttribute) const {
		std::map<std::string, EXPRESSION_TYPE>::const_iterator cursor = _mapOfAttributes.find(sAttribute);
		if (cursor == _mapOfAttributes.end()) return UNKNOWN_EXPRTYPE;
		return cursor->second;
	}

	bool DtaClass::addAttribute(const std::string& sAttribute, EXPRESSION_TYPE parameterType, DtaClass* pClassAttribute) {
		std::map<std::string, EXPRESSION_TYPE>::const_iterator cursor = _mapOfAttributes.find(sAttribute);
		if (cursor == _mapOfAttributes.end()) return false;
		if (pClassAttribute == NULL) {
			_mapOfAttributes[sAttribute] = parameterType;
		} else {
			int iAttributeType = ((int) parameterType) & pClassAttribute->getId();
			_mapOfAttributes[sAttribute] = (EXPRESSION_TYPE) iAttributeType;
		}
		return true;
	}

	std::string DtaClass::getCppTypeSpecifier() const {
		return getName();
	}

	EXPRESSION_TYPE DtaClass::composeExprType(EXPRESSION_TYPE exprType) const {
		int iExprType = ((int) exprType) | ((int) _iId);
		return (EXPRESSION_TYPE) iExprType;
	}

	DtaClass* DtaClass::getClass(EXPRESSION_TYPE iId) {
		int iIndex = ((int) iId) >> 8;
		if  ((iIndex <= 0) || ((size_t) iIndex > _idToClass.size())) return NULL;
		return _idToClass[iIndex - 1];
	}


	DtaTargetLanguageTypeSpecifier::~DtaTargetLanguageTypeSpecifier() {}

	std::string DtaTargetLanguageTypeSpecifier::getCppTypeSpecifier() const {
		return "TARGET_LANGUAGE_TYPE_SPECIFIER(" + getName() + ")";
	}

}
