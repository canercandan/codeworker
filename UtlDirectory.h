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

#ifndef _UtlDirectory_h_
#define _UtlDirectory_h_

#include <string>
#include <list>

#pragma warning(disable : 4251)

namespace CodeWorker {
#	define CW_PATH_MAX	4096

	class UtlDirectory;

	class UtlFile {
		private:
			UtlDirectory* _pParent;
			std::string _sFileName;

		public:
			UtlFile(UtlDirectory& myParent, const std::string& sFileName);
			UtlFile(const std::string& sFileName);
			~UtlFile();
			
			inline const std::string& getFileName() const {return _sFileName;}
			std::string getFileNameBody() const;
			std::string getFileNameExtension() const;
			inline UtlDirectory* getDirectory() const { return _pParent; }

			bool remove();
	};


	class UtlDirectory {
		private:
			bool _bScanned;
			UtlDirectory* _pParent;
			std::string _sDirectoryName;
			std::list<UtlFile*> _listOfFiles;
			std::list<UtlDirectory*> _listOfDirectories;

		public:
			UtlDirectory();
			UtlDirectory(UtlDirectory& myParent, const std::string& sDirectoryName);
			UtlDirectory(const std::string& sDirectoryName);
			~UtlDirectory();

			inline const std::list<UtlDirectory*>& getDirectories() const {return _listOfDirectories;}
			inline const std::list<UtlFile*>& getFiles() const {return _listOfFiles;}
			inline const std::string& getDirectoryName() const {return _sDirectoryName;}
			inline void setDirectoryName(const std::string& sDirectoryName) {_sDirectoryName = sDirectoryName;}
       
			UtlDirectory* getSubdirectory(const std::string& sDirectoryName) const;

			std::string getFullPath() const;
			std::string getRelativePath() const;

			bool createDirectory(const std::string& sDirectory);
			
			bool scan(const std::string& sExtendedPattern = "");
			bool scanRecursively(const std::string& sPattern = "");

			bool remove();
			
			static std::string getTmpDirectory() ;

		private:
			bool matchPatternDirectory(const std::string& sPatternDirectory);
	};
}

#endif
