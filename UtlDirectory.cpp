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

#ifdef WIN32
#	include <windows.h>
#	include <direct.h>
#else
#	include <sys/stat.h>
#	include <unistd.h>
#	include <glob.h>
#endif

#include "UtlException.h"
#include "ScpStream.h"
#include "UtlDirectory.h"


namespace CodeWorker {
	UtlFile::UtlFile(UtlDirectory& myParent, const std::string& sFileName) : _pParent(&myParent), _sFileName(sFileName) {
	}

	UtlFile::UtlFile(const std::string& sFileName) : _pParent(NULL), _sFileName(sFileName) {
	}

	UtlFile::~UtlFile() {
	}

	std::string UtlFile::getFileNameBody() const {
		std::string::size_type iIndex = _sFileName.find(".");
		if (iIndex == std::string::npos) return _sFileName;
		if (iIndex == 0) return "";
		return _sFileName.substr(0, iIndex);
	}

	std::string UtlFile::getFileNameExtension() const {
		std::string::size_type iIndex = _sFileName.find(".");
		if (iIndex == std::string::npos) return "";
		iIndex++;
		if (_sFileName.size() == iIndex) return "";
		return _sFileName.substr(iIndex);
	}

	bool UtlFile::remove() {
		std::string sFilename = _pParent->getFullPath() + _sFileName;
		return (unlink(sFilename.c_str()) == 0);
	}


	UtlDirectory::UtlDirectory(UtlDirectory& myParent, const std::string& sDirectoryName) : _bScanned(false), _pParent(&myParent), _sDirectoryName(sDirectoryName) {
	}

	UtlDirectory::UtlDirectory() : _bScanned(false), _pParent(NULL), _sDirectoryName("") {
	}


	UtlDirectory::UtlDirectory(const std::string& sDirectoryName) : _bScanned(false), _pParent(NULL), _sDirectoryName(sDirectoryName) {
	}

	UtlDirectory::~UtlDirectory() {
		for (std::list<UtlFile*>::const_iterator iterateFiles = _listOfFiles.begin(); iterateFiles != _listOfFiles.end(); iterateFiles++) {
			delete (*iterateFiles);
		}
		for (std::list<UtlDirectory*>::const_iterator iterateDirectories = _listOfDirectories.begin(); iterateDirectories != _listOfDirectories.end(); iterateDirectories++) {
			delete (*iterateDirectories);
		}
	}

	UtlDirectory* UtlDirectory::getSubdirectory(const std::string& sDirectoryName) const {
		if (!_bScanned) throw UtlException("can't call 'UtlDirectory(\"" + getFullPath() + "\").getSubdirectory(\"" + sDirectoryName + "\")': its files and directories must be scanned before");
		for (std::list<UtlDirectory*>::const_iterator iterateDirectories = _listOfDirectories.begin(); iterateDirectories != _listOfDirectories.end(); iterateDirectories++) {
			if ((*iterateDirectories)->getDirectoryName() == sDirectoryName) return (*iterateDirectories);
		}
		return NULL;
	}

	std::string UtlDirectory::getFullPath() const {
		std::string sFullPath;
		std::string sRelativePath = getRelativePath();
#ifdef WIN32
		char tcFullPath[2048];
		char* pFileName;
		std::string sCrazyFile = sRelativePath;
		if (GetFullPathName(sCrazyFile.c_str(), 2048, tcFullPath, &pFileName) != 0) {
			sFullPath = tcFullPath;
			if (!sFullPath.empty()) {
				if ((sFullPath[sFullPath.size() - 1] != '\\') && (sFullPath[sFullPath.size() - 1] != '/')) sFullPath += "/";
			}
		}
#else
		if (sRelativePath[0]=='/') {
			sFullPath= sRelativePath;
		} else {
			char pcFullPath[CW_PATH_MAX];
			pcFullPath[0] = '\0';
			getcwd(pcFullPath, CW_PATH_MAX - 1);
			sFullPath= pcFullPath;
			if (!sFullPath.empty()) {
				if (sFullPath[sFullPath.size() - 1] != '/') sFullPath += "/";
				sFullPath += sRelativePath;
			} else {
				sFullPath = "/" + sRelativePath;
			}
		}
#endif
		return sFullPath;
	}

	std::string UtlDirectory::getRelativePath() const {
		std::string sRelativePath;
		if (_pParent != NULL) sRelativePath = _pParent->getRelativePath();
		sRelativePath += _sDirectoryName;
		if (!sRelativePath.empty()) {
			char a = sRelativePath[sRelativePath.size() - 1];
			if ((a != '/') && (a != '\\')) sRelativePath += "/";
		}
		return sRelativePath;
	}

	bool UtlDirectory::createDirectory(const std::string& sDirectory) {
		std::string sPath = getFullPath() + sDirectory + "/";
		bool bCreated = CodeWorker::createDirectoriesForFile(sPath);
		if (bCreated) {
			UtlDirectory* pNewDirectory = new UtlDirectory(*this, sDirectory);
			_listOfDirectories.push_back(pNewDirectory);
		}
		return bCreated;
	}

	bool UtlDirectory::scan(const std::string& sExtendedPattern) {
		bool bSuccess = true;
		if (_bScanned) {
			_bScanned = false;
			for (std::list<UtlDirectory*>::const_iterator iterateDirectories = _listOfDirectories.begin(); iterateDirectories != _listOfDirectories.end(); iterateDirectories++) {
				delete (*iterateDirectories);
			}
			_listOfDirectories = std::list<UtlDirectory*>();
			for (std::list<UtlFile*>::const_iterator iterateFiles = _listOfFiles.begin(); iterateFiles != _listOfFiles.end(); iterateFiles++) {
				delete (*iterateFiles);
			}
			_listOfFiles = std::list<UtlFile*>();
		}
		std::string sPattern = sExtendedPattern;
		std::string sPatternDirectory;
		{
			std::string::size_type iIndex = sPattern.find_last_of("/\\");
			if (iIndex != std::string::npos) {
				sPatternDirectory = sPattern.substr(0, iIndex);
				sPattern = sPattern.substr(iIndex + 1);
			}
		}
		std::string sPath = getFullPath();
		if (sPattern.empty()) sPath += "*";
		else sPath += sPattern;
#ifdef WIN32
		WIN32_FIND_DATA myFindData;
		HANDLE hFind = FindFirstFile(sPath.c_str(), &myFindData);
		bool bMore = (hFind != (HANDLE) -1);
		// bug under WIN 32: "*.bar" accepts also "*.bar~" !
		std::string::size_type iIndex = sPattern.find_last_of('*');
		std::string::size_type iTempIndex = sPattern.find_last_of('?');
		std::string sSuffix;
		if (iIndex != iTempIndex) {
			if ((iIndex == std::string::npos) || ((iIndex < iTempIndex) && (iTempIndex != std::string::npos))) iIndex = iTempIndex;
			iIndex++;
			if (iIndex < sPattern.size()) sSuffix = sPattern.substr(iIndex);
		}
		while (bMore) {
			if ((myFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				char* tcName = (char*) myFindData.cFileName;
				std::string::size_type iLength = strlen(tcName);
				if (!sSuffix.empty() && (iLength > sSuffix.size()) && (strcmp(tcName + (iLength - sSuffix.size()), sSuffix.c_str()) != 0)) {
					// nothing, it shouldn't have been filtered
				} else if (matchPatternDirectory(sPatternDirectory)) {
					UtlFile* pNewFile = new UtlFile(*this, tcName);
					_listOfFiles.push_back(pNewFile);
				}
			}
			bMore = (FindNextFile(hFind, &myFindData) != 0);
		}
		FindClose(hFind);
		std::string sDirectoriesPath = getFullPath() + "*";
		WIN32_FIND_DATA myDirectoriesFindData;
		hFind = FindFirstFile(sDirectoriesPath.c_str(), &myDirectoriesFindData);
		bMore = (hFind != (HANDLE) -1);
		bSuccess = bMore;
		while (bMore) {
			if ((myDirectoriesFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				char* tcName = (char*) myDirectoriesFindData.cFileName;
				if (tcName[0] != '.') {
					UtlDirectory* pNewDirectory = new UtlDirectory(*this, tcName);
					_listOfDirectories.push_back(pNewDirectory);
				}
			}
			bMore = (FindNextFile(hFind, &myDirectoriesFindData) != 0);
		}
		FindClose(hFind);
#else
		glob_t globbuf;
		memset(&globbuf, '\0', sizeof(globbuf));
		if (glob (sPath.c_str(), GLOB_DOOFFS|GLOB_MARK, NULL, &globbuf) == 0) {
			for (int i=0; i< globbuf.gl_pathc; ++i) {
				std::string sFileName = globbuf.gl_pathv[i];
				std::string::size_type iIndex = sFileName.find_last_of('/');
				if (iIndex != std::string::npos) {
					iIndex++;
					if (sFileName.size() == iIndex) continue;
					sFileName = sFileName.substr(iIndex);
				}
				if (matchPatternDirectory(sPatternDirectory)) {
					UtlFile* pNewFile = new UtlFile(*this, sFileName);
					_listOfFiles.push_back(pNewFile);
				}
			}
		}
		globfree (&globbuf);
		std::string sDirectoryPath = getFullPath() + "*";
		if (glob (sDirectoryPath.c_str(), GLOB_DOOFFS|GLOB_MARK, NULL, &globbuf) == 0) {
			for (int i=0; i< globbuf.gl_pathc; ++i) {
				std::string sDirectoryName = globbuf.gl_pathv[i];
				std::string::size_type iIndex = sDirectoryName.size() - 1;
				if (sDirectoryName[iIndex] == '/') {
					iIndex = sDirectoryName.find_last_of('/', iIndex - 1);
					if (iIndex != std::string::npos) sDirectoryName = sDirectoryName.substr(iIndex + 1);
					sDirectoryName = sDirectoryName.substr(0, sDirectoryName.size() - 1);
					if ((sDirectoryName == ".") || (sDirectoryName == "..")) continue;
					UtlDirectory* pNewDir = new UtlDirectory(*this, sDirectoryName);
					_listOfDirectories.push_back(pNewDir);
				}
			}
		}
		globfree (&globbuf);
#endif
		_bScanned = bSuccess;
		return bSuccess;
	}

	bool UtlDirectory::matchPatternDirectory(const std::string& sPatternDirectory) {
		if (sPatternDirectory.empty()) return true;
		std::string::size_type iIndex = sPatternDirectory.find_last_of("/\\");
		if (iIndex == std::string::npos) {
			return (_sDirectoryName == sPatternDirectory);
		}
		if (_sDirectoryName != sPatternDirectory.substr(iIndex + 1)) return false;
		return matchPatternDirectory(sPatternDirectory.substr(0, iIndex));
	}

	bool UtlDirectory::scanRecursively(const std::string& sPattern) {
		bool bSuccess = scan(sPattern);
		for (std::list<UtlDirectory*>::const_iterator iterateDirectories = _listOfDirectories.begin(); iterateDirectories != _listOfDirectories.end(); iterateDirectories++) {
			bool bRecSuccess = (*iterateDirectories)->scanRecursively(sPattern);
			bSuccess = bSuccess || bRecSuccess;
		}
		return bSuccess;
	}

	bool UtlDirectory::remove() {
		bool bSuccess = scan();
		_bScanned = false;
		for (std::list<UtlDirectory*>::const_iterator iterateDirectories = _listOfDirectories.begin(); iterateDirectories != _listOfDirectories.end(); iterateDirectories++) {
			if (!(*iterateDirectories)->remove()) bSuccess = false;
			delete (*iterateDirectories);
		}
		_listOfDirectories = std::list<UtlDirectory*>();
		for (std::list<UtlFile*>::const_iterator iterateFiles = _listOfFiles.begin(); iterateFiles != _listOfFiles.end(); iterateFiles++) {
			if (!(*iterateFiles)->remove()) bSuccess = false;
			delete (*iterateFiles);
		}
		_listOfFiles = std::list<UtlFile*>();
		if (bSuccess) {
			std::string sDirName = getFullPath();
			sDirName = sDirName.substr(0, sDirName.size() - 1);
			bSuccess = (rmdir(sDirName.c_str()) == 0);
		}
		return bSuccess;
	}

	std::string UtlDirectory::getTmpDirectory() {
		char *pRepTmp1=getenv("TMP");
		char *pRepTmp2=getenv("TEMP");

		std::string sRepTmp;

		if (((pRepTmp1 == NULL) || (pRepTmp1[0] == '\0'))  && ((pRepTmp2 == NULL) || (pRepTmp2[0]=='\0'))) {
			THROW_UTLEXCEPTION("Environnement variable TMP or TEMP must be defined for method UtlDirectory::getTmpDirectory");
		}

		if ((pRepTmp1 != NULL) && (pRepTmp1[0]!='\0')) {
			sRepTmp = pRepTmp1;
			int iSize = sRepTmp.size() - 1;
			if (pRepTmp1[iSize] == '\\') sRepTmp = sRepTmp.substr(0, iSize) + "/";
			else if (pRepTmp1[iSize] != '/') sRepTmp += "/";
		} else if ((pRepTmp1 != NULL) && (pRepTmp2[0]!='\0')) {
			sRepTmp = pRepTmp2;
			int iSize = sRepTmp.size() - 1;
			if (pRepTmp2[iSize] == '\\') sRepTmp = sRepTmp.substr(0, iSize) + "/";
			else if (pRepTmp1[iSize] != '/') sRepTmp += "/";
		}

		return sRepTmp;
	}
}
