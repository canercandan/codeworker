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

#ifndef _DtaClass_h_
#define _DtaClass_h_

#include <vector>
#include <string>

#include "ExprScriptFunction.h"

namespace CodeWorker {
	class GrfBlock;

	class DtaClass {
	private:
		GrfBlock* _pBlock;
		std::string _sName;
		EXPRESSION_TYPE _iId;
		DtaClass* _pExtendedClass;
		std::map<std::string, EXPRESSION_TYPE> _mapOfAttributes;

		static std::vector<DtaClass*> _idToClass;

	public:
		DtaClass(GrfBlock* pBlock, const std::string& sName);
		virtual ~DtaClass();

		inline const std::string& getName() const { return _sName; }
		inline EXPRESSION_TYPE getId() const { return _iId; }
		inline GrfBlock* getBlock() const { return _pBlock; }
		inline void setBlock(GrfBlock* pBlock) { _pBlock = pBlock; }
		inline DtaClass* getExtendedClass() const { return _pExtendedClass; }
		inline void setExtendedClass(DtaClass* pExtendedClass) { _pExtendedClass = pExtendedClass; }
		EXPRESSION_TYPE getAttributeType(const std::string& sAttribute) const;
		bool addAttribute(const std::string& sAttribute, EXPRESSION_TYPE parameterType, DtaClass* pClassAttribute = NULL);
		virtual std::string getCppTypeSpecifier() const;

		inline bool isForwardDeclaration() const { return (_pExtendedClass == NULL) && _mapOfAttributes.empty(); }

		EXPRESSION_TYPE composeExprType(EXPRESSION_TYPE exprType) const;
		static DtaClass* getClass(EXPRESSION_TYPE iId);
	};

	class DtaTargetLanguageTypeSpecifier : public DtaClass {
	public:
		DtaTargetLanguageTypeSpecifier(GrfBlock* pBlock, const std::string& sTypeSpecifier) : DtaClass(pBlock, sTypeSpecifier) {}
		virtual ~DtaTargetLanguageTypeSpecifier();

		inline const std::string& getTypeSpecifier() const { return getName(); }
		virtual std::string getCppTypeSpecifier() const;
	};
}

#endif
