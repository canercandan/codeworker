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

#ifndef _DtaProtectedAreasBag_h_
#define _DtaProtectedAreasBag_h_

#include <map>
#include <list>
#include <string>


namespace CodeWorker {
	class ScpStream;
	class DtaScriptVariable;
	class DtaSharpTagsHandler;

	class DtaProtectedArea {
	private:
		std::string _sDefine;
		std::string _sText;
		bool _bIsAlreadyGenerated;

	public:
		inline DtaProtectedArea(const std::string& sText, const std::string& sDefine = "") : _sText(sText), _sDefine(sDefine), _bIsAlreadyGenerated(false) {}

		inline void setDefine(const std::string& sDefine) { _sDefine = sDefine; }
		inline const std::string& getDefine() const { return _sDefine; }
		inline void setText(const std::string& sText) { _sText = sText; }
		inline const std::string& getText() const { return _sText; }
		inline void isAlreadyGenerated(bool bIsAlreadyGenerated) { _bIsAlreadyGenerated = bIsAlreadyGenerated; }
		inline bool isAlreadyGenerated() const { return _bIsAlreadyGenerated; }
	};


	class DtaProtectedAreasBag {
	private:
		std::map<std::string, DtaProtectedArea*> _codes;

	public:
		inline DtaProtectedAreasBag() {}
		virtual ~DtaProtectedAreasBag();

		inline const std::map<std::string, DtaProtectedArea*>& getProtectedAreas() const { return _codes; }

		void loadProtectedCodes(const char* sFile);
		void recoverProtectedCodes(ScpStream& inputFile);
		void recoverMarker(DtaSharpTagsHandler& tagsHandler);
		void clearAll();

		std::string getProtection(const char* sProtection) const;
		void setProtection(const char* sProtection, const char* sContent);
		bool removeProtection(const char* sProtection);
		std::list<std::string> getProtectionKeys() const;
		std::list<std::string> remainingProtectionKeys() const;
		const DtaProtectedArea& registerNewProtection(const char* sProtection);

	private:
		bool recoverProtectedCode(DtaSharpTagsHandler& tagsHandler, int iEnd = -1);
	};
}

#endif
