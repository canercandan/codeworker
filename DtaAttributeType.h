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

#ifndef _DtaAttributeType_h_
#define _DtaAttributeType_h_

#include <map>
#include <string>

namespace CodeWorker {
	class DtaAttributeType;
	class DtaScriptVariable;

	class DtaFollowingAttributeInfo {
	private:
		DtaAttributeType* _pAttributeType;
		int _iNumberOfUse;

	public:
		DtaFollowingAttributeInfo(DtaAttributeType* pType) : _iNumberOfUse(0), _pAttributeType(pType) {}

		inline DtaAttributeType& getAttributeType() const { return *_pAttributeType; }
		inline int getNumberOfUse() const { return _iNumberOfUse; }
		inline void incrementUse() { _iNumberOfUse++; }

		bool isAttribute() const;
		void insertAttribute(DtaScriptVariable& myUser);
	};

	class DtaAttributeType {
	private:
		static std::map<std::string, DtaAttributeType*> _mapOfTypes;

		std::string _sName;
		int _iNumberOfUse;
		int _iNumberOfElements;
		std::map<std::string, DtaFollowingAttributeInfo*> _mapOfAttributes;
		std::map<std::string, DtaFollowingAttributeInfo*> _mapOfAttributeElements;

	protected:
		DtaAttributeType(const std::string& sName);

		void insertUser(DtaScriptVariable& myUser);
		void insertElement(DtaScriptVariable& myUser);
		void insertAttribute(DtaScriptVariable& myUser);

	public:
		static inline const std::map<std::string, DtaAttributeType*>& getTypes() { return _mapOfTypes; }
		static DtaAttributeType* getAttributeType(const std::string& sName);

		inline const std::string& getName() const { return _sName; }
		inline const std::map<std::string, DtaFollowingAttributeInfo*>& getAttributes() const { return _mapOfAttributes; }
		inline const std::map<std::string, DtaFollowingAttributeInfo*>& getAttributeElements() const { return _mapOfAttributeElements; }
		inline int getNumberOfUse() const { return _iNumberOfUse; }
		inline int getNumberOfElementUse() const { return _iNumberOfElements; }
		bool isAttribute() const;

		static DtaAttributeType& extractProjectTypes(DtaScriptVariable& visibility);
		static void removeProjectTypes();
	};
}

#endif
