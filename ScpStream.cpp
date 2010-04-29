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
#	pragma warning(disable: 4786)
#	include <direct.h>
#	include <windows.h>
#else
	// function 'mkdir()'
#	include <sys/stat.h>
	// functions 'ntohl()' and 'htonl()'
#	include <netinet/in.h>
#endif

#include "UtlException.h"
#include "ScpStream.h"

namespace CodeWorker {
	bool createDirectoriesForFile(const std::string& sFileName) {
		bool bCreated;
		std::string::size_type iEndPos = sFileName.find_first_of("/\\");
		while (iEndPos != std::string::npos) {
			std::string sDirectory = sFileName.substr(0, iEndPos);
#ifdef WIN32
			bCreated = (_mkdir(sDirectory.c_str()) == 0);
#else
			bCreated = (mkdir(sDirectory.c_str(), S_IRWXU) == 0);
#endif
			iEndPos = sFileName.find_first_of("/\\", iEndPos + 1);
		}
		return bCreated;
	}


#define WRITECHAR(a) \
	_tcStream[_iWriteCursor++] = a;\
	if (_iWriteCursor >= _iCacheMemory) {\
		int iOldSize = _iCacheMemory;\
		_iCacheMemory *= 2;\
		char* tcStream = new char[_iCacheMemory];\
		memcpy(tcStream, _tcStream, iOldSize);\
		delete [] _tcStream;\
		_tcStream = tcStream;\
	}


	int ScpStream::IN = 1;
	int ScpStream::OUT = 2;
	int ScpStream::INOUT = 3;
	int ScpStream::APPEND = 4;
	int ScpStream::PATH = 8;
#ifdef WIN32
	std::string ScpStream::ENDL = "\r\n";
#else
	std::string ScpStream::ENDL = "\n";
#endif

	std::list<std::string> ScpStream::_listOfIncludePaths;
	std::map<std::string, std::string> ScpStream::_listOfVirtualFiles;


	ScpStream::ScpStream(int iCacheMemory) : _iCacheMemory(iCacheMemory), _iSize(0), _iReadCursor(0), _iWriteCursor(0), _bInsertText(false), _iMode(0), _pParentStream(NULL), _pShiftedStream(NULL), _iShiftedStreamPosition(0), _pPrecStream(NULL), _pNextStream(NULL), _pfStreamReaderCallback(NULL), _pfStreamWriterCallback(NULL), _iLineCounter(1), _iLinePosition(0) {
		_tcStream = new char[iCacheMemory];
	}

	ScpStream::ScpStream(const std::string& sFilename, int iMode, int iCacheMemory, int iLength) : _iCacheMemory(iCacheMemory), _iSize(0), _iReadCursor(0), _iWriteCursor(0), _bInsertText(false), _iMode(iMode), _sFilename(sFilename), _pParentStream(NULL), _pShiftedStream(NULL), _iShiftedStreamPosition(0), _pPrecStream(NULL), _pNextStream(NULL), _pfStreamReaderCallback(NULL), _pfStreamWriterCallback(NULL), _iLineCounter(1), _iLinePosition(0) {
		std::string sContent;
		if (loadVirtualFile(sFilename, sContent)) {
			_iSize = ((iLength < 0) ? sContent.length() : iLength);
			if (iCacheMemory <= _iSize) _tcStream = new char[_iSize + 1];
			else _tcStream = new char[iCacheMemory];
			memcpy(_tcStream, sContent.c_str(), _iSize);
		} else {
			_tcStream = new char[iCacheMemory];
			if (iMode & IN) {
				if (iMode & PATH) {
					std::ifstream* pFile = openSTLInputFile(_sFilename.c_str());
					std::list<std::string>::const_iterator i = ScpStream::getListOfIncludePaths().begin();
					if ((sFilename[0] != '/') && ((sFilename.length() < 2) || (sFilename[1] != ':'))) {
						while ((pFile == NULL) && (i != ScpStream::getListOfIncludePaths().end())) {
							_sFilename = *i + sFilename;
							pFile = openSTLInputFile(_sFilename.c_str());
							++i;
						}
					}
					if (pFile == NULL) THROW_UTLEXCEPTION("impossible to find file \"" + sFilename + "\" into include paths");
					pFile->close();
					delete pFile;
				}
				FILE* f = fopen(_sFilename.c_str(), "rb");
				if (f == NULL) {
					if ((iMode & OUT) == 0) THROW_UTLEXCEPTION("impossible to open file \"" + _sFilename + "\" for reading");
				} else {
					unsigned int iLoaded;
					do {
						if (_iSize + 32000 >= _iCacheMemory) {
							if ((iLength < 0) || (iLength > _iCacheMemory)) {
								int iOldSize = _iCacheMemory;
								if (_iCacheMemory < 32000) _iCacheMemory = 32000;
								_iCacheMemory *= 2;
								char* tcStream = new char[_iCacheMemory];
								memcpy(tcStream, _tcStream, iOldSize);
								delete [] _tcStream;
								_tcStream = tcStream;
							}
						}
						if ((iLength > 0) && (iLength < _iSize + 32000)) {
							unsigned int iBlockSize = iLength - _iSize;
							if (iBlockSize == 0) {
								iLoaded = 0;
							} else {
								iLoaded = fread(_tcStream + _iSize, 1, iBlockSize, f);
							}
						} else {
							iLoaded = fread(_tcStream + _iSize, 1, 32000, f);
						}
						_iSize += iLoaded;
					} while (iLoaded == 32000);
					if ((iLoaded < 32000) && ferror(f)) THROW_UTLEXCEPTION("an error has occurred while reading file \"" + _sFilename + "\"");
					fclose(f);
				}
			}
		}
	}

	ScpStream::ScpStream(const std::string& sFilename, FILE* f, int iCacheMemory) : _sFilename(sFilename), _iCacheMemory(iCacheMemory), _iSize(0), _iReadCursor(0), _iWriteCursor(0), _bInsertText(false), _iMode(ScpStream::IN), _pParentStream(NULL), _pShiftedStream(NULL), _iShiftedStreamPosition(0), _pPrecStream(NULL), _pNextStream(NULL), _pfStreamReaderCallback(NULL), _pfStreamWriterCallback(NULL), _iLineCounter(1), _iLinePosition(0) {
		_tcStream = new char[iCacheMemory];
		unsigned int iLoaded;
		do {
			if (_iSize + 32000 >= _iCacheMemory) {
				int iOldSize = _iCacheMemory;
				if (_iCacheMemory < 32000) _iCacheMemory = 32000;
				_iCacheMemory *= 2;
				char* tcStream = new char[_iCacheMemory];
				memcpy(tcStream, _tcStream, iOldSize);
				delete [] _tcStream;
				_tcStream = tcStream;
			}
			iLoaded = fread(_tcStream + _iSize, 1, 32000, f);
			_iSize += iLoaded;
			if ((iLoaded < 32000) && ferror(f)) {
				char tcNumber[32];
				sprintf(tcNumber, "%d", ferror(f));
				std::string sMessage = "error [";
				sMessage += tcNumber;
				sMessage += "] has occurred while reading bytes of file \"" + _sFilename + "\"";
				THROW_UTLEXCEPTION(sMessage);
			}
		} while (iLoaded == 32000);
		fclose(f);
	}

	ScpStream::ScpStream(const std::string& sText) : _iSize(0), _iReadCursor(0), _iWriteCursor(0), _bInsertText(false), _iMode(0), _pParentStream(NULL), _pShiftedStream(NULL), _iShiftedStreamPosition(0), _pPrecStream(NULL), _pNextStream(NULL), _pfStreamReaderCallback(NULL), _pfStreamWriterCallback(NULL), _iLineCounter(1), _iLinePosition(0) {
		_iSize = sText.length();
		_iCacheMemory = _iSize + 1;
		_tcStream = new char[_iCacheMemory];
		memcpy(_tcStream, sText.c_str(), _iSize);
	}

	ScpStream::ScpStream(ScpStream& shiftedStream, int iShiftedStreamPosition) : _iReadCursor(0), _iWriteCursor(0), _bInsertText(false), _iMode(0), _pParentStream(NULL), _pShiftedStream(&shiftedStream), _iShiftedStreamPosition(iShiftedStreamPosition), _pPrecStream(NULL), _pNextStream(NULL), _pfStreamReaderCallback(NULL), _pfStreamWriterCallback(NULL), _iLineCounter(1), _iLinePosition(0) {
		_tcStream = _pShiftedStream->_tcStream + _iShiftedStreamPosition;
		_iCacheMemory = _pShiftedStream->_iCacheMemory - _iShiftedStreamPosition;
		_iSize = _pShiftedStream->_iSize - _iShiftedStreamPosition;
	}

	ScpStream::~ScpStream() {
		if (_pShiftedStream == NULL) delete [] _tcStream;
	}

	void ScpStream::setStreamReaderCallback(END_STREAM_CALLBACK pfCBK, void* pData) {
		_pfStreamReaderCallback = pfCBK;
		_pStreamReaderCBKData = pData;
	}

	void ScpStream::setStreamWriterCallback(END_STREAM_CALLBACK pfCBK, void* pData) {
		_pfStreamWriterCallback = pfCBK;
		_pStreamWriterCBKData = pData;
	}

	ScpStream* ScpStream::createFile(const std::string& sFilename) {
		if (!existVirtualFile(sFilename)) {
			FILE* f = fopen(sFilename.c_str(), "w+b");
			if (f == NULL) {
				if (createDirectoriesForFile(sFilename)) {
					f = fopen(sFilename.c_str(), "w+b");
				}
				if (f == NULL) return NULL;
			}
			fclose(f);
		}
		ScpStream* pStream = new ScpStream;
		pStream->setFilename(sFilename);
		return pStream;
	}


	bool ScpStream::existInputFile(const char* /*java:"const std::string&"*/ sFileName) {
		if (existVirtualFile(sFileName)) return true;
#ifdef WIN32
		HANDLE h = CreateFile(sFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		bool bSuccess = (h != INVALID_HANDLE_VALUE);
		if (bSuccess) {
			CloseHandle(h);
		}
		return bSuccess;
#else
		std::ifstream* pFile = openSTLInputFile(sFileName);
		if (pFile == NULL) return false;
		pFile->close();
		delete pFile;
		return true;
#endif
	}

	bool ScpStream::existInputFileFromIncludePath(const char* /*java:"const std::string&"*/ tcFileName, std::string& sCompleteFileName) {
		sCompleteFileName = tcFileName;
		if (existVirtualFile(tcFileName)) return true;
		bool bFile = existInputFile(sCompleteFileName.c_str());
		if (!bFile && ((sCompleteFileName[0] == '/') || (sCompleteFileName[1] == ':'))) return false;
		std::list<std::string>::const_iterator i = ScpStream::getListOfIncludePaths().begin();
		while (!bFile && (i != ScpStream::getListOfIncludePaths().end())) {
			sCompleteFileName = *i + tcFileName;
			bFile = existInputFile(sCompleteFileName.c_str());
			++i;
		}
		return bFile;
	}

	ScpStream* ScpStream::openInputFileFromIncludePath(const char* /*java:"const std::string&"*/ tcFileName, std::string& sCompleteFileName) {
		std::string/*java:"std::string&"*/ sContent;
		if (loadVirtualFile(tcFileName, sContent)) {
			ScpStream* pStream = new ScpStream;
			pStream->setFilename(tcFileName);
			pStream->writeBinaryData(sContent.c_str(), sContent.length());
			pStream->setOutputLocation(0);
			sCompleteFileName = tcFileName;
			return pStream;
		}
		sCompleteFileName = tcFileName;
		std::ifstream* pFile = openSTLInputFile(sCompleteFileName.c_str());
		if ((pFile == NULL) && ((sCompleteFileName[0] == '/') || (sCompleteFileName[1] == ':'))) return NULL;
		std::list<std::string>::const_iterator i = ScpStream::getListOfIncludePaths().begin();
		while ((pFile == NULL) && (i != ScpStream::getListOfIncludePaths().end())) {
			sCompleteFileName = *i + tcFileName;
			pFile = openSTLInputFile(sCompleteFileName.c_str());
			++i;
		}
		if (pFile == NULL) return NULL;
		pFile->close();
		delete pFile;
		FILE* f= fopen(sCompleteFileName.c_str(), "rb");
		if (f == NULL) return NULL;
		return new ScpStream(sCompleteFileName, f);
	}

	ScpStream* ScpStream::openInputFile(const char* /*java:"const std::string&"*/ sFileName) {
		std::string/*java:"std::string&"*/ sContent;
		if (loadVirtualFile(sFileName, sContent)) {
			ScpStream* pStream = new ScpStream;
			pStream->setFilename(sFileName);
			pStream->writeBinaryData(sContent.c_str(), sContent.length());
			pStream->setOutputLocation(0);
			return pStream;
		}
		std::ifstream* pFile = openSTLInputFile(sFileName);
		if (pFile == NULL) return NULL;
		pFile->close();
		delete pFile;
		FILE* f= fopen(sFileName, "rb");
		if (f == NULL) return NULL;
		return new ScpStream(sFileName, f);
	}

	bool ScpStream::createVirtualFile(const std::string& sHandle, const std::string& sContent) {
		_listOfVirtualFiles[sHandle] = sContent;
		return true;
	}

	std::string ScpStream::createVirtualTemporaryFile(const std::string& sContent) {
		int iHandle = 0;
		char tcHandle[32];
		do {
			sprintf(tcHandle, ".~#%d", iHandle);
			++iHandle;
		} while (_listOfVirtualFiles.find(tcHandle) != _listOfVirtualFiles.end());
		std::string sHandle = tcHandle;
		createVirtualFile(sHandle, sContent);
		return sHandle;
	}

	bool ScpStream::existVirtualFile(const std::string& sHandle) {
		std::map<std::string, std::string>::const_iterator cursor = _listOfVirtualFiles.find(sHandle);
		return (cursor != _listOfVirtualFiles.end());
	}

	bool ScpStream::loadVirtualFile(const std::string& sHandle, std::string& sContent) {
		std::map<std::string, std::string>::iterator cursor = _listOfVirtualFiles.find(sHandle);
		if (cursor == _listOfVirtualFiles.end()) return false;
		sContent = cursor->second;
		return true;
	}

	bool ScpStream::appendVirtualFile(const std::string& sHandle, const std::string& sContent) {
		std::map<std::string, std::string>::iterator cursor = _listOfVirtualFiles.find(sHandle);
		if (cursor == _listOfVirtualFiles.end()) return false;
		cursor->second += sContent;
		return true;
	}

	bool ScpStream::deleteVirtualFile(const std::string& sHandle) {
		std::map<std::string, std::string>::iterator cursor = _listOfVirtualFiles.find(sHandle);
		if (cursor == _listOfVirtualFiles.end()) return false;
		_listOfVirtualFiles.erase(cursor);
		return true;
	}

	ScpStream::SizeAttributes ScpStream::resize(int iNewSize) {
		ScpStream::SizeAttributes sizeAttrs;
		if ((iNewSize >= 0) && (iNewSize < _iSize)) {
			sizeAttrs._iSize = _iSize;
			sizeAttrs._iWriteCursor = _iWriteCursor;
			sizeAttrs._iReadCursor = _iReadCursor;
			sizeAttrs._cEndChar = _tcStream[iNewSize];
			_iSize = iNewSize;
			if (_iWriteCursor > _iSize) _iWriteCursor = _iSize;
			if (_iReadCursor > _iSize) _iReadCursor = _iSize;
			if (_pShiftedStream != NULL) {
				sizeAttrs._pShiftedStream = new SizeAttributes(_pShiftedStream->resize(iNewSize + _iShiftedStreamPosition));
			}
		}
		return sizeAttrs;
	}

	void ScpStream::restoreSize(const ScpStream::SizeAttributes& sizeAttrs) {
		if (!sizeAttrs.empty()) {
			_tcStream[_iSize] = sizeAttrs._cEndChar;
			_iSize = sizeAttrs._iSize;
			_iWriteCursor = sizeAttrs._iWriteCursor;
			_iReadCursor = sizeAttrs._iReadCursor;
			if ((_pShiftedStream != NULL) && (sizeAttrs._pShiftedStream != NULL)) {
				_pShiftedStream->restoreSize(*(sizeAttrs._pShiftedStream));
			}
		}
	}

	void ScpStream::incrementIndentation(int iLevel) {
		while (iLevel > 0) {
			_sIndentation += "\t";
			--iLevel;
		}
	}

	bool ScpStream::decrementIndentation(int iLevel) {
		if (_sIndentation.length() < (unsigned int) iLevel) {
			_sIndentation = "";
			return false;
		}
		_sIndentation.resize(_sIndentation.length() - iLevel);
		return true;
	}

	std::string ScpStream::getMessagePrefix(bool bCountCols) const {
		std::string sFilename = _sFilename;
		std::string::size_type iIndex = sFilename.find_last_of("\\/");
		if (iIndex != std::string::npos) sFilename = sFilename.substr(iIndex + 1);
		char tcNumber[64];
		if (bCountCols) sprintf(tcNumber, "(%d): ", getLineCount());
		else sprintf(tcNumber, "(%d,%d): ", getLineCount(), getColCount());
		return sFilename + tcNumber;
	}

	void ScpStream::setLineDirective(int iLine) {
		_iLineCounter = iLine;
		_iLinePosition = _iReadCursor;
	}

	int ScpStream::getLineCount() const {
	int iLine = _iLineCounter;
	int iCursor = _iLinePosition;
	while (iCursor < _iReadCursor) {
		if (_tcStream[iCursor++] == (int) '\n') ++iLine;
	}
	return iLine;
	}

	int ScpStream::getColCount() const {
		int iPosition = _iReadCursor;
		int iCurrent = _iReadCursor;
		while (iCurrent > 0) {
			--iCurrent;
			if (_tcStream[iCurrent] == (int) '\n') {
				return iPosition - iCurrent;
			}
		}
		return 1 + iPosition;
	}

	int ScpStream::getOutputLineCount() const {
	int iLine = 1;
	int iCursor = 0;
	while (iCursor < _iWriteCursor) {
		if (_tcStream[iCursor++] == (int) '\n') ++iLine;
	}
	return iLine;
	}

	int ScpStream::getOutputColCount() const {
		int iPosition = _iWriteCursor;
		int iCurrent = _iWriteCursor;
		while (iCurrent > 0) {
			--iCurrent;
			if (_tcStream[iCurrent] == (int) '\n') {
				return iPosition - iCurrent;
			}
		}
		return 1 + iPosition;
	}

	bool ScpStream::goBack() {
		if (_iReadCursor <= 0) {
			if (_pPrecStream != NULL) return _pPrecStream->goBack();
			return false;
		}
		--_iReadCursor;
		return true;
	}

	int ScpStream::readChar() {
		if (_iSize <= _iReadCursor) {
			if (_pNextStream != NULL) return _pNextStream->readChar();
			if (_pfStreamReaderCallback == NULL) return -1;
			(*_pfStreamReaderCallback)(*this, _pStreamReaderCBKData);
			if (_iSize <= _iReadCursor) return -1;
		}
		unsigned char c = (unsigned char) _tcStream[_iReadCursor++];
		return c;
	}

	int ScpStream::peekChar() {
		if (_iSize <= _iReadCursor) {
			if (_pNextStream != NULL) return _pNextStream->peekChar();
			return -1;
		}
		unsigned char c = (unsigned char) _tcStream[_iReadCursor];
		return c;
	}

	bool ScpStream::skipBlanks() {
		int iChar = readChar();
		while ((iChar >= (int) '\0') && (iChar <= (int) ' ')) iChar = readChar();
		if (iChar < 0) return false;
		goBack();
		return true;
	}

	bool ScpStream::skipSpaces() {
		int iChar = readChar();
		while ((iChar >= (int) '\0') && (iChar == (int) ' ' || iChar == (int) '\t')) iChar = readChar();
		if (iChar < 0) return false;
		goBack();
		return true;
	}

	bool ScpStream::skipLineBlanks() {
		int iChar = readChar();
		while ((iChar >= (int) '\0') && (iChar <= (int) ' ') && (iChar != '\r') && (iChar != '\n')) iChar = readChar();
		if (iChar < 0) return false;
		goBack();
		return true;
	}

	bool ScpStream::skipCppComments() {
		int iCursor = getInputLocation();
		int iChar = readChar();
		if (iChar == (int) '/') {
			iChar = readChar();
			if (iChar == (int) '/') do iChar = readChar(); while ((iChar >= 0) && (iChar != (int) '\n'));
			else if (iChar == (int) '*') {
				iChar = readChar();
				do {
					if (iChar == (int) '*') {
						iChar = readChar();
						if (iChar == (int) '/') break;
					} else {
						iChar = readChar();
					}
				} while (iChar >= 0);
				if (iChar < 0) {
					setInputLocation(iCursor);
					return false;
				}
			} else {
				setInputLocation(iCursor);
				return false;
			}
		} else {
			setInputLocation(iCursor);
			return false;
		}
		return true;
	}

	bool ScpStream::skipCppExceptDoxygenComments() {
		int iCursor = getInputLocation();
		int iChar = readChar();
		if (iChar == (int) '/') {
			iChar = readChar();
			if (iChar == (int) '/') {
				iChar = readChar();
				if (iChar == (int) '!') {
					setInputLocation(iCursor);
					return false;
				}
				do iChar = readChar(); while ((iChar >= 0) && (iChar != (int) '\n'));
			} else if (iChar == (int) '*') {
				iChar = readChar();
				if (iChar == (int) '!') {
					setInputLocation(iCursor);
					return false;
				}
				do {
					if (iChar == (int) '*') {
						iChar = readChar();
						if (iChar == (int) '/') break;
					} else {
						iChar = readChar();
					}
				} while (iChar >= 0);
				if (iChar < 0) {
					setInputLocation(iCursor);
					return false;
				}
			} else {
				setInputLocation(iCursor);
				return false;
			}
		} else {
			setInputLocation(iCursor);
			return false;
		}
		return true;
	}

	bool ScpStream::skipEmpty() {
		bool bSuccess;
		do bSuccess = skipBlanks(); while (bSuccess && skipCppComments());
		return bSuccess;
	}

	bool ScpStream::skipEmptyCppExceptDoxygen() {
		bool bSuccess;
		do bSuccess = skipBlanks(); while (bSuccess && skipCppExceptDoxygenComments());
		return bSuccess;
	}

	bool ScpStream::skipEmptyHTML() {
		int iChar;
		for(;;) {
			do iChar = readChar(); while ((iChar >= (int) '\0') && (iChar <= (int) ' '));
			if ((iChar == (int) '<') && isEqualTo("!--")) {
				do {
					do iChar = readChar(); while ((iChar >= 0) && (iChar != (int) '-'));
				} while ((iChar >= 0) && !isEqualTo("->"));
			} else break;
		} 
		if (iChar < 0) return false;
		goBack();
		return true;
	}

	bool ScpStream::skipEmptyAda() {
		bool bSuccess = skipBlanks();
		while (bSuccess && isEqualTo("--")) {
			int iChar;
			do iChar = readChar(); while ((iChar >= 0) && (iChar != (int) '\n'));
			if (iChar < 0) return false;
			bSuccess = skipBlanks();
		}
		return bSuccess;
	}

	bool ScpStream::skipEmptyLaTeX() {
		int iChar = peekChar();
		if (iChar != (int) '%') return false;
		do {
			++_iReadCursor;
			do iChar = readChar(); while ((iChar >= 0) && (iChar != (int) '\n'));
		} while ((iChar == '\n') && (peekChar() == '%'));
		return true;
	}

	bool ScpStream::skipLine() {
		int iChar;
		int i = 0;
		for(;;) {
			iChar = readChar();
			if ((iChar == '\n') || (iChar < 0)) break;
			++i;
		}
		if (iChar >= 0) isEqualTo('\r');
		return ((iChar >= 0) || (i > 0));
	}

	bool ScpStream::readChars(int iLength, std::string& sText) {
		if (_iSize < _iReadCursor + iLength) {
			_tcStream[_iSize] = '\0';
			sText += _tcStream + _iReadCursor;
			iLength += _iReadCursor - _iSize;
			_iReadCursor = _iSize;
			if (_pNextStream != NULL) return _pNextStream->readChars(iLength, sText);
			if (_pfStreamReaderCallback == NULL) return false;
			int iOldSize = _iSize;
			do {
				(*_pfStreamReaderCallback)(*this, _pStreamReaderCBKData);
				if (iOldSize == _iSize) return false;
				iOldSize = _iSize;
			} while (_iSize < _iReadCursor + iLength);
		}
		char c = _tcStream[_iReadCursor + iLength];
		_tcStream[_iReadCursor + iLength] = '\0';
		sText += _tcStream + _iReadCursor;
		_iReadCursor += iLength;
		_tcStream[_iReadCursor] = c;
		return true;
	}

	bool ScpStream::readWord(std::string& sWord) {
		int iIndex = 0;
		int iChar;
		int iPos = _iReadCursor;
		do {
			iChar = readChar();
			++iIndex;
		} while (((iChar >= (int) 'a') && (iChar <= (int) 'z')) || ((iChar >= (int) 'A') && (iChar <= (int) 'Z')) || ((iChar >= (int) '0') && (iChar <= (int) '9')) || (iChar == (int) '_') || (iChar == (unsigned char) 'é') || (iChar == (unsigned char) 'è') || (iChar == (unsigned char) 'ê') || (iChar == (unsigned char) 'à') || (iChar == (unsigned char) 'î') || (iChar == (unsigned char) 'ù') || (iChar == (unsigned char) 'ô') || (iChar == (unsigned char) 'û') || (iChar == (unsigned char) 'â') || (iChar == (unsigned char) 'ç'));
		if (iChar >= 0) goBack();
		--iIndex;
		if (iIndex == 0) sWord = "";
		else sWord = std::string(_tcStream + iPos, iIndex);
		return (iIndex > 0);
	}

	bool ScpStream::readIdentifier(std::string& sIdentifier) {
		unsigned char tcText[1000];
		int iIndex = 0;
		int iChar;
		iChar = readChar();
		if (((iChar >= (int) 'a') && (iChar <= (int) 'z')) || ((iChar >= (int) 'A') && (iChar <= (int) 'Z')) || (iChar == (int) '_')) {
			tcText[iIndex++] = (unsigned char) iChar;
			do {
				iChar = readChar();
				tcText[iIndex++] = (unsigned char) iChar;
			} while (((iChar >= (int) 'a') && (iChar <= (int) 'z')) || ((iChar >= (int) 'A') && (iChar <= (int) 'Z')) || ((iChar >= (int) '0') && (iChar <= (int) '9')) || (iChar == (int) '_'));
			--iIndex;
			tcText[iIndex] = '\0';
			sIdentifier = (const char*) tcText;
		}
		if (iChar >= 0) goBack();
		return (iIndex >= 1);
	}

	bool ScpStream::readInt(int& iResult) {
		unsigned char tcReadInt[32];
		int iIndex = 0;
		int iChar = readChar();
		if (iChar == (int) '-') {
			tcReadInt[iIndex++] = '-';
			iChar = readChar();
		}
		if ((iChar < (int) '0') || (iChar > (int) '9')) {
			if (iChar >= 0) goBack();
			if (iIndex != 0) goBack();
			return false;
		}
		tcReadInt[iIndex++] = (unsigned char) iChar;
		iChar = readChar();
		while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
			tcReadInt[iIndex++] = (unsigned char) iChar;
			iChar = readChar();
		}
		if (iChar >= 0) setInputLocation(getInputLocation() - 1);
		tcReadInt[iIndex] = '\0';
		iResult = atoi((const char*) tcReadInt);
		return true;
	}

	bool ScpStream::readHexadecimal(int& iResult) {
		int iChar = readChar();
		if (((iChar < (int) '0') || (iChar > (int) '9')) && ((iChar < (int) 'A') || (iChar > (int) 'F')) && ((iChar < (int) 'a') || (iChar > (int) 'f'))) {
			if (iChar >= 0) goBack();
			return false;
		}
		iResult = 0;
		for (;;) {
			int iDigit;
			if ((iChar >= (int) '0') && (iChar <= (int) '9')) iDigit = iChar - (int) '0';
			else if ((iChar >= (int) 'A') && (iChar <= (int) 'F')) iDigit = 10 + iChar - (int) 'A';
			else if ((iChar >= (int) 'a') && (iChar <= (int) 'f')) iDigit = 10 + iChar - (int) 'a';
			else break;
			iResult = 16*iResult + iDigit;
			iChar = readChar();
		}
		if (iChar >= 0) goBack();
		return true;
	}

	bool ScpStream::readDouble(double& dValue) {
		unsigned char tcReadDouble[32];
		int iIndex = 0;
		int iPosition = getInputLocation();
		int iChar = readChar();
		if (iChar == (int) '-') {
			tcReadDouble[iIndex++] = '-';
			iChar = readChar();
		} else if (iChar == (int) '+') {
			iChar = readChar();
		}
		if ((iChar < (int) '0') || (iChar > (int) '9')) {
			setInputLocation(iPosition);
			return false;
		}
		tcReadDouble[iIndex++] = (unsigned char) iChar;
		iChar = readChar();
		while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
			tcReadDouble[iIndex++] = (unsigned char) iChar;
			iChar = readChar();
		}
		if ((iChar == (int) '.') && (peekChar() != (int) '.')) {
			tcReadDouble[iIndex++] = (unsigned char) '.';
			iChar = readChar();
			while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
				tcReadDouble[iIndex++] = (unsigned char) iChar;
				iChar = readChar();
			}
		}
		if ((iChar == (int) 'e') || (iChar == (int) 'E')) {
			int iCursor = getInputLocation();
			int iMarkedIndex = iIndex;
			tcReadDouble[iIndex++] = (unsigned char) 'E';
			iChar = readChar();
			if (iChar == (int) '-') {
				tcReadDouble[iIndex++] = (unsigned char) iChar;
				iChar = readChar();
			} else if (iChar == (int) '+') {
				iChar = readChar();
			}
			if ((iChar < (int) '0') || (iChar > (int) '9')) {
				iIndex = iMarkedIndex;
				setInputLocation(iCursor);
			} else {
				tcReadDouble[iIndex++] = (unsigned char) iChar;
				iChar = readChar();
				while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
					tcReadDouble[iIndex++] = (unsigned char) iChar;
					iChar = readChar();
				}
			}
		}
		if (iChar >= 0) goBack();
		tcReadDouble[iIndex] = '\0';
		dValue = atof((const char*) tcReadDouble);
		return true;
	}

	bool ScpStream::readStringOrCharLiteral(std::string& sText) {
		if (readString(sText)) return true;
		int iChar;
		if (!readCharLiteral(iChar)) return false;
		sText.assign(1, (char) iChar);
		return true;
	}

	bool ScpStream::readCharLiteral(int& iChar) {
		int iPosition = getInputLocation();
		if (!isEqualTo('\'')) return false;
		iChar = readChar();
		if (iChar == '\\') {
			iChar = readChar();
			switch(iChar) {
				case 'a': iChar = '\a';break;
				case 'b': iChar = '\b';break;
				case 'f': iChar = '\f';break;
				case 'n': iChar = '\n';break;
				case 'r': iChar = '\r';break;
				case 't': iChar = '\t';break;
				case 'v': iChar = '\v';break;
				case 'u': readHexadecimal(iChar);break;
				case '0':
					{
						iChar = 0;
						int iCurrentChar = readChar();
						if (iCurrentChar == (int) 'x') {
							do {
								iCurrentChar = readChar();
								if ((iCurrentChar >= '0') && (iCurrentChar <= '9')) {
									iChar *= 16;
									iChar += (iCurrentChar - (int) '0');
								} else if ((iCurrentChar >= 'a') && (iCurrentChar <= 'f')) {
									iChar *= 16;
									iChar += (iCurrentChar - (int) 'a') + 10;
								} else if ((iCurrentChar >= 'A') && (iCurrentChar <= 'F')) {
									iChar *= 16;
									iChar += (iCurrentChar - (int) 'A') + 10;
								} else break;
							} while (true);
						} else if ((iCurrentChar >= (int) '0') && (iCurrentChar <= (int) '7')) {
							iChar = iCurrentChar - (int) '0';
							do {
								iCurrentChar = readChar();
								if ((iCurrentChar >= '0') && (iCurrentChar <= '7')) {
									iChar *= 8;
									iChar += (iCurrentChar - (int) '0');
								} else break;
							} while (true);
						}
						goBack();
					}
					break;
				case '1':
				case '2':
				case '3':
					{
						iChar -= '0';
						int iCurrentChar;
						do {
							iCurrentChar = readChar();
							if ((iCurrentChar >= '0') && (iCurrentChar <= '7')) {
								iChar *= 8;
								iChar += (iCurrentChar - (int) '0');
							} else break;
						} while (true);
					}
					break;
			}
		}
		if (!isEqualTo('\'')) {
			setInputLocation(iPosition);
			return false;
		}
		return true;
	}

	bool ScpStream::readString(std::string& sText) {
		unsigned char tcReadString[32766];
		int iIndex = 0;
		int iLocation = _iReadCursor;
		int iChar = readChar();
		if (iChar != (int) '\"') {
			if (iChar >= 0) goBack();
			return false;
		}
		iChar = readChar();
		while ((iChar >= 0) && (iChar != (int) '\"')) {
			if (iIndex > 32760) {
				THROW_UTLEXCEPTION(*this, "cannot define a string of more than 32760 characters");
			}
			if (iChar == '\\') {
				iChar = readChar();
				switch(iChar) {
					case 'a': iChar = '\a';break;
					case 'b': iChar = '\b';break;
					case 'f': iChar = '\f';break;
					case 'n': iChar = '\n';break;
					case 'r': iChar = '\r';break;
					case 't': iChar = '\t';break;
					case 'v': iChar = '\v';break;
					case 'u':
						if (!readHexadecimal(iChar)) {
							_iReadCursor = iLocation;
							return false;
						}
						break;
					case '0':
						{
							iChar = 0;
							int iCurrentChar = readChar();
							if (iCurrentChar == (int) 'x') {
								do {
									iCurrentChar = readChar();
									if ((iCurrentChar >= '0') && (iCurrentChar <= '9')) {
										iChar *= 16;
										iChar += (iCurrentChar - (int) '0');
									} else if ((iCurrentChar >= 'a') && (iCurrentChar <= 'f')) {
										iChar *= 16;
										iChar += (iCurrentChar - (int) 'a') + 10;
									} else if ((iCurrentChar >= 'A') && (iCurrentChar <= 'F')) {
										iChar *= 16;
										iChar += (iCurrentChar - (int) 'A') + 10;
									} else break;
								} while (true);
							} else if ((iCurrentChar >= (int) '0') && (iCurrentChar <= (int) '7')) {
								iChar = iCurrentChar - (int) '0';
								do {
									iCurrentChar = readChar();
									if ((iCurrentChar >= '0') && (iCurrentChar <= '7')) {
										iChar *= 8;
										iChar += (iCurrentChar - (int) '0');
									} else break;
								} while (true);
							}
							goBack();
						}
						break;
					case '1':
					case '2':
					case '3':
						{
							iChar -= '0';
							int iCurrentChar;
							do {
								iCurrentChar = readChar();
								if ((iCurrentChar >= '0') && (iCurrentChar <= '7')) {
									iChar *= 8;
									iChar += (iCurrentChar - (int) '0');
								} else break;
							} while (true);
						}
						break;
				}
			} else if (iChar == (int) '\n') {
				break;
			}
			tcReadString[iIndex++] = (unsigned char) iChar;
			iChar = readChar();
		}
		if (iChar != (int) '\"') {
			_iReadCursor = iLocation;
			return false;
		}
		tcReadString[iIndex] = '\0';
		sText = (char*) tcReadString;
		return true;
	}

	bool ScpStream::readPythonString(std::string& sText) {
		unsigned char tcReadString[32766];
		int iIndex = 0;
		int iLocation = _iReadCursor;
		int iChar = readChar();
		if (iChar != (int) '\"' && iChar != (int) '\'') {
			if (iChar >= 0) goBack();
			return false;
		}
		int iQuote = iChar;
		bool b3Quoted = false;
		iChar = readChar();
		if (iChar == iQuote) {
			iChar = readChar();
			if (iChar != iQuote) {
				if (iChar >= 0) goBack();
				sText = "";
				return true;
			}
			iChar = readChar();
			b3Quoted = true;
		}
		do {
			while ((iChar >= 0) && (iChar != iQuote)) {
				if (iIndex > 32760) {
					THROW_UTLEXCEPTION(*this, "cannot define a string of more than 32760 characters");
				}
				if (iChar == '\\') {
					iChar = readChar();
					switch(iChar) {
						case 'a': iChar = '\a';break;
						case 'b': iChar = '\b';break;
						case 'f': iChar = '\f';break;
						case 'n': iChar = '\n';break;
						case 'r': iChar = '\r';break;
						case 't': iChar = '\t';break;
						case 'v': iChar = '\v';break;
						case 'u':
							if (!readHexadecimal(iChar)) {
								_iReadCursor = iLocation;
								return false;
							}
							break;
						case '0':
							{
								iChar = 0;
								int iCurrentChar = readChar();
								if (iCurrentChar == (int) 'x') {
									do {
										iCurrentChar = readChar();
										if ((iCurrentChar >= '0') && (iCurrentChar <= '9')) {
											iChar *= 16;
											iChar += (iCurrentChar - (int) '0');
										} else if ((iCurrentChar >= 'a') && (iCurrentChar <= 'f')) {
											iChar *= 16;
											iChar += (iCurrentChar - (int) 'a') + 10;
										} else if ((iCurrentChar >= 'A') && (iCurrentChar <= 'F')) {
											iChar *= 16;
											iChar += (iCurrentChar - (int) 'A') + 10;
										} else break;
									} while (true);
								} else if ((iCurrentChar >= (int) '0') && (iCurrentChar <= (int) '7')) {
									iChar = iCurrentChar - (int) '0';
									do {
										iCurrentChar = readChar();
										if ((iCurrentChar >= '0') && (iCurrentChar <= '7')) {
											iChar *= 8;
											iChar += (iCurrentChar - (int) '0');
										} else break;
									} while (true);
								}
								goBack();
							}
							break;
						case '1':
						case '2':
						case '3':
							{
								iChar -= '0';
								int iCurrentChar;
								do {
									iCurrentChar = readChar();
									if ((iCurrentChar >= '0') && (iCurrentChar <= '7')) {
										iChar *= 8;
										iChar += (iCurrentChar - (int) '0');
									} else break;
								} while (true);
							}
							break;
					}
				} else if (iChar == (int) '\n' && !b3Quoted) {
					break;
				}
				tcReadString[iIndex++] = (unsigned char) iChar;
				iChar = readChar();
			}
			if (b3Quoted) {
				if (iChar < 0) break;
				iChar = readChar();
				if (iChar == iQuote) {
					iChar = readChar();
					if (iChar == iQuote) break;
					tcReadString[iIndex++] = (unsigned char) iQuote;
				}
				tcReadString[iIndex++] = (unsigned char) iQuote;
			}
		} while (b3Quoted);
		if (iChar != iQuote) {
			_iReadCursor = iLocation;
			return false;
		}
		tcReadString[iIndex] = '\0';
		sText = (char*) tcReadString;
		return true;
	}

	bool ScpStream::readAdaString(std::string& sText) {
		unsigned char tcReadAdaString[32000];
		int iIndex = 0;
		int iLocation = _iReadCursor;
		int iChar = readChar();
		if (iChar != (int) '\"') {
			if (iChar >= 0) goBack();
			return false;
		}
		iChar = readChar();
		while ((iChar >= 0)) {
			if (iChar == '"') {
				if (peekChar() != '"') break;
				iChar = readChar();
			}
			tcReadAdaString[iIndex++] = (unsigned char) iChar;
			iChar = readChar();
		}
		if (iChar != (int) '\"') {
			_iReadCursor = iLocation;
			return false;
		}
		tcReadAdaString[iIndex] = '\0';
		sText = (char*) tcReadAdaString;
		return true;
	}

	bool ScpStream::readLine(std::string& sLine) {
		char tcReadLine[10000];
		int i = 0;
		int iChar;
		do {
			iChar = readChar();
			if ((iChar == '\n') || (iChar < 0)) break;
			tcReadLine[i++] = (char) iChar;
		} while (true);
		if (iChar >= 0) isEqualTo('\r');
		if ((i > 0) && (tcReadLine[i - 1] == '\r')) --i;
		tcReadLine[i] = '\0';
		sLine = (const char*) tcReadLine;
		return ((iChar >= 0) || (i > 0));
	}

	bool ScpStream::readLastChars(int iLength, std::string& sLastChars) {
		if (iLength < 0) return false;
		char a = (_iSize > _iReadCursor) ? _tcStream[_iReadCursor] : '\0';
		_tcStream[_iReadCursor] = '\0';
		if (_iReadCursor - iLength > 0) sLastChars = _tcStream + (_iReadCursor - iLength);
		else sLastChars = _tcStream;
		_tcStream[_iReadCursor] = a;
		return true;
	}

	bool ScpStream::readUptoChar(char cEnd, std::string& sText) {
		int iLocation = getInputLocation();
		std::string sLocalText;
		int iChar = readChar();
		while ((iChar > 0) && (iChar != (int) cEnd)) {
			sLocalText += (char) iChar;
			iChar = readChar();
		}
		if (iChar < 0) {
			setInputLocation(iLocation);
			return false;
		}
		sText = sLocalText;
		goBack();
		return true;
	}

	bool ScpStream::readUptoChar(const std::string& sEnd, std::string& sText) {
		int iLocation = getInputLocation();
		std::string sLocalText;
		int iChar = readChar();
		while ((iChar > 0) && (sEnd.find((char) iChar) == std::string::npos)) {
			sLocalText += (char) iChar;
			iChar = readChar();
		}
		if (iChar < 0) {
			setInputLocation(iLocation);
			return false;
		}
		sText = sLocalText;
		goBack();
		return true;
	}

	bool ScpStream::isEqualTo(unsigned char ch) {
		int iChar = readChar();
		if (iChar == (int) ch) return true;
		if (iChar >= 0) goBack();
		return false;
	}

	bool ScpStream::isEqualTo(const char* sText) {
		int iChar;
		int iPosition = getInputLocation();
		for (int i = 0; sText[i] != '\0'; ++i) {
			iChar = readChar();
			if ((iChar < 0) || ((char) iChar != sText[i])) {
        		setInputLocation(iPosition);
				return false;
			}
		}
		return true;
	}

	bool ScpStream::isEqualToIgnoreCase(unsigned char ch) {
		int iChar = readChar();
		if ((ch >= 'A') && (ch <= 'Z')) ch += ' ';
		if ((iChar >= 'A') && (iChar <= 'Z')) iChar += 32;
		if (iChar == (int) ch) return true;
		if (iChar >= 0) goBack();
		return false;
	}

	bool ScpStream::isEqualToIgnoreCase(const char* sText) {
		int iChar;
		int iPosition = getInputLocation();
		for (int i = 0; sText[i] != '\0'; ++i) {
			iChar = readChar();
			if ((iChar >= 'A') && (iChar <= 'Z')) iChar += 32;
			char a = sText[i];
			if ((a >= 'A') && (a <= 'Z')) a += ' ';
			if ((iChar < 0) || ((char) iChar != a)) {
        		setInputLocation(iPosition);
				return false;
			}
		}
		return true;
	}

	bool ScpStream::isEqualToIdentifier(const char* /*java:"const std::string&"*/ sText) {
		int iPosition = getInputLocation();
		std::string/*java:"std::string&"*/ sIdentifier;
		if (!readIdentifier(sIdentifier)) return false;
		if (sIdentifier != sText) {
			setInputLocation(iPosition);
			return false;
		}
		return true;
	}

	bool ScpStream::findString(const char* sText) {
		int iChar = -1;
		int iPosition = getInputLocation();
		int iCurrent = iPosition;
		int i;
		do {
			setInputLocation(iCurrent);
			for (i = 0; sText[i] != '\0'; ++i) {
				iChar = readChar();
				if (iChar < 0) break;
				if ((char) iChar != sText[i]) break;
			}
			++iCurrent;
		} while ((iChar >= 0) && (sText[i] != '\0'));
		if (sText[i] == '\0') return true;
		setInputLocation(iPosition);
		return false;
	}

	bool ScpStream::findString(const char* sText, int iBoundary) {
		int iChar = -1;
		int iPosition = getInputLocation();
		int iCurrent = iPosition;
		int i;
		do {
			setInputLocation(iCurrent);
			for (i = 0; sText[i] != '\0'; ++i) {
				iChar = readChar();
				if (iChar < 0) break;
				if ((char) iChar != sText[i]) break;
			}
			++iCurrent;
		} while ((iChar >= 0) && (sText[i] != '\0') && (iCurrent < iBoundary));
		if (sText[i] == '\0') return true;
		setInputLocation(iPosition);
		return false;
	}

	bool ScpStream::newFloatingLocation(const std::string& sKey) {
		_mapOfFloatingLocations[sKey] = getOutputLocation();
		return true;
	}

	void ScpStream::setFloatingLocation(const std::string& sKey, int iLocation) {
		_mapOfFloatingLocations[sKey] = iLocation;
	}

	int ScpStream::getFloatingLocation(const std::string& sKey, ScpStream*& pOwner) const {
		std::map<std::string, int>::const_iterator cursor = _mapOfFloatingLocations.find(sKey);
		if (cursor == _mapOfFloatingLocations.end()) {
			if (_pParentStream != NULL) return _pParentStream->getFloatingLocation(sKey, pOwner);
			pOwner = NULL;
			return -1;
		}
		pOwner = const_cast<ScpStream*>(this);
		return cursor->second;
	}

	int ScpStream::removeFloatingLocation(const std::string& sKey, ScpStream*& pOwner) {
		std::map<std::string, int>::iterator cursor = _mapOfFloatingLocations.find(sKey);
		if (cursor == _mapOfFloatingLocations.end()) {
			if (_pParentStream != NULL) return _pParentStream->removeFloatingLocation(sKey, pOwner);
			pOwner = NULL;
			return -1;
		}
		pOwner = const_cast<ScpStream*>(this);
		int iPosition = cursor->second;
		_mapOfFloatingLocations.erase(cursor);
		return iPosition;
	}

	ScpStream& ScpStream::operator <<(char cValue) {
		WRITECHAR(cValue);
		if (_iWriteCursor > _iSize) _iSize = _iWriteCursor;
		return *this;
	}

	ScpStream& ScpStream::operator <<(unsigned char cValue) {
		WRITECHAR(cValue);
		if (_iWriteCursor > _iSize) _iSize = _iWriteCursor;
		return *this;
	}

	ScpStream& ScpStream::operator <<(int iValue) {
		char tcNumber[32];
		sprintf(tcNumber, "%d", iValue);
		writeText(tcNumber);
		return *this;
	}

	ScpStream& ScpStream::operator <<(long lValue) {
		char tcNumber[32];
		sprintf(tcNumber, "%ld", lValue);
		writeText(tcNumber);
		return *this;
	}

	ScpStream& ScpStream::operator <<(unsigned long lValue) {
		char tcNumber[32];
		sprintf(tcNumber, "%lu", lValue);
		writeText(tcNumber);
		return *this;
	}

	ScpStream& ScpStream::operator <<(double dValue) {
		char tcNumber[320];
		sprintf(tcNumber, "%f", dValue);
		writeText(tcNumber);
		return *this;
	}

	ScpStream& ScpStream::operator <<(const char* tcValue) {
		if (tcValue == NULL) writeText("(null)");
		else writeText(tcValue);
		return *this;
	}

	ScpStream& ScpStream::operator <<(const std::string& sValue) {
		writeBinaryData(sValue.c_str(), sValue.length());
		return *this;
	}

	ScpStream& ScpStream::operator <<(const ScpStream& theStream) {
		for (std::map<std::string, int>::iterator i = theStream._mapOfFloatingLocations.begin(); i != theStream._mapOfFloatingLocations.end(); ++i) {
			_mapOfFloatingLocations[i->first] = i->second + _iWriteCursor;
		}
		theStream._mapOfFloatingLocations.clear();
		theStream._pParentStream = this;
		writeText(theStream.readBuffer());
		return *this;
	}

	std::string ScpStream::getLastWrittenChars(int iNbChars) const {
		if (iNbChars > _iWriteCursor) iNbChars = _iWriteCursor;
		if (iNbChars == 0) return "";
		int iBegin = _iWriteCursor - iNbChars;
		char* tcText = new char[iNbChars + 1];
		memcpy(tcText, _tcStream + iBegin, iNbChars);
		tcText[iNbChars] = '\0';
		std::string sLastChars(tcText);
		delete [] tcText;
		return 	sLastChars;
	}

	ScpStream& ScpStream::flush() {
		return *this;
	}

	ScpStream& ScpStream::endl() {
		writeText(ENDL.c_str());
		return *this;
	}

	bool ScpStream::close() {
		if (_pfStreamWriterCallback == NULL) return true;
		return (*_pfStreamWriterCallback)(*this, _pStreamWriterCBKData);
	}

	bool ScpStream::equals(const ScpStream& theStream, int& iPosition) const {
		if (_iSize != theStream._iSize) {
			iPosition = -1;
			return false;
		}
		int i = _iSize;
		char* u = _tcStream;
		char* v = theStream._tcStream;
		while (i > 0) {
			if (*u++ != *v++) {
				iPosition = _iSize - i;
				return false;
			}
			--i;
		}
		iPosition = _iSize;
		return true;
	}

	bool ScpStream::equalsFromInputLocations(const ScpStream& theStream, int& iPosition) const {
		if (_iSize - _iReadCursor != theStream._iSize - theStream._iReadCursor) {
			iPosition = -1;
			return false;
		}
		int i = _iSize - _iReadCursor;
		char* u = _tcStream + _iReadCursor;
		char* v = theStream._tcStream + theStream._iReadCursor;
		while (i > 0) {
			if (*u++ != *v++) {
				iPosition = _iSize - i;
				return false;
			}
			--i;
		}
		iPosition = _iSize;
		return true;
	}

	bool ScpStream::insertTextOnce(const std::string& sText, int iLocation, int iAreaToRecover) {
		if (_setOfTextsToInsertOnce.find(sText) != _setOfTextsToInsertOnce.end()) return false;
		if (!insertText(sText, iLocation, iAreaToRecover)) return false;
		_setOfTextsToInsertOnce.insert(sText);
		return true;
	}
	bool ScpStream::insertText(const std::string& sText, int iLocation, int iAreaToRecover) {
		if (iLocation < 0)
			return false;
		int iEndPortion = iLocation + iAreaToRecover;
		if (iEndPortion > _iSize) {
			iAreaToRecover = _iSize - iLocation;
			if (iAreaToRecover < 0) return false;
			iEndPortion = iLocation + iAreaToRecover;
		}
		int iBytesToAdd = sText.length() - iAreaToRecover;
		if (_iSize + iBytesToAdd >= _iCacheMemory) {
			int iOldSize = _iCacheMemory;
			do _iCacheMemory *= 2; while (_iSize + iBytesToAdd >= _iCacheMemory);
			char* tcStream = new char[_iCacheMemory];
			memcpy(tcStream, _tcStream, iOldSize);
			delete [] _tcStream;
			_tcStream = tcStream;
		}
		if (iBytesToAdd != 0) {
			memmove(_tcStream + (iLocation + sText.length()), _tcStream + iEndPortion, _iSize - iEndPortion);
			_iSize += iBytesToAdd;
			if (iEndPortion < _iWriteCursor) _iWriteCursor += iBytesToAdd;
//==        if (iEndPortion < _iWriteCursor || _iSize > _iWriteCursor) _iWriteCursor += iBytesToAdd;
			for (std::map<std::string, int>::iterator i = _mapOfFloatingLocations.begin(); i != _mapOfFloatingLocations.end(); ++i) {
				if (i->second >= iEndPortion) _mapOfFloatingLocations[i->first] += iBytesToAdd;
			}
		}
		memcpy(_tcStream + iLocation, sText.c_str(), sText.length());
		return true;
	}

	bool ScpStream::insertStream(const ScpStream& theStream, int iLocation, int iAreaToRecover) {
		if (iLocation < 0)
			return false;
		int iEndPortion = iLocation + iAreaToRecover;
		if (iEndPortion > _iSize) {
			iAreaToRecover = _iSize - iLocation;
			if (iAreaToRecover < 0) return false;
			iEndPortion = iLocation + iAreaToRecover;
		}
		int iBytesToAdd = theStream.size() - iAreaToRecover;
		if (_iSize + iBytesToAdd >= _iCacheMemory) {
			int iOldSize = _iCacheMemory;
			do _iCacheMemory *= 2; while (_iSize + iBytesToAdd >= _iCacheMemory);
			char* tcStream = new char[_iCacheMemory];
			memcpy(tcStream, _tcStream, iOldSize);
			delete [] _tcStream;
			_tcStream = tcStream;
		}
		if (iBytesToAdd != 0) {
			memmove(_tcStream + (iLocation + theStream.size()), _tcStream + iEndPortion, _iSize - iEndPortion);
			_iSize += iBytesToAdd;
			if (iEndPortion < _iWriteCursor) _iWriteCursor += iBytesToAdd;
//==        if (iEndPortion < _iWriteCursor || _iSize > _iWriteCursor) _iWriteCursor += iBytesToAdd;
			for (std::map<std::string, int>::iterator i = _mapOfFloatingLocations.begin(); i != _mapOfFloatingLocations.end(); ++i) {
				if (i->second >= iEndPortion) _mapOfFloatingLocations[i->first] += iBytesToAdd;
			}
		}
		memcpy(_tcStream + iLocation, theStream.readBuffer(), theStream.size());
		return true;
	}

	bool ScpStream::copy(const ScpStream& theStream, int iLocation, int iLength) {
		if (iLength < 0) iLength = theStream.size() - iLocation;
		else if (iLength + iLocation > theStream.size()) return false;
		if (_iWriteCursor + iLength >= _iCacheMemory) {
			int iOldSize = _iCacheMemory;
			do _iCacheMemory *= 2; while (_iWriteCursor + iLength >= _iCacheMemory);
			char* tcStream = new char[_iCacheMemory];
			memcpy(tcStream, _tcStream, iOldSize);
			delete [] _tcStream;
			_tcStream = tcStream;
		}
		memcpy(_tcStream + _iWriteCursor, theStream.readBuffer() + iLocation, iLength);
		_iWriteCursor += iLength;
		if (_iWriteCursor > _iSize) _iSize = _iWriteCursor;
		return true;
	}

	void ScpStream::saveIntoFile(const std::string& sFilename, bool bCreateFileIfUnknown) const {
		if (existVirtualFile(sFilename)) {
			createVirtualFile(sFilename, readBuffer());
		} else {
			FILE* f = fopen(sFilename.c_str(), "wb");
			if (f == NULL) {
				if (bCreateFileIfUnknown) {
					if (createDirectoriesForFile(sFilename)) f = fopen(sFilename.c_str(), "wb");
				}
				if (f == NULL) THROW_UTLEXCEPTION("unable to copy a stream into file '" + sFilename + "'");
			}
			int iCursor = 0;
			int iBlock = (_iSize >= 32000) ? 32000 : _iSize;
			do {
				int iWritten = fwrite(_tcStream + iCursor, 1, iBlock, f);
				if (iWritten != iBlock) {
					char tcNumber[32];
					sprintf(tcNumber, "%d", ferror(f));
					THROW_UTLEXCEPTION(std::string("error [") + tcNumber + "] has occurred while saving bytes into file \"" + sFilename + "\"");
				}
				iCursor += iBlock;
				iBlock = (iCursor + 32000 <= _iSize) ? 32000 : _iSize - iCursor;
			} while (iBlock != 0);
			fclose(f);
		}
	}

	void ScpStream::appendFile(const std::string& sFilename, bool bCreateFileIfUnknown) const {
		if (existVirtualFile(sFilename)) {
			std::string/*java:"std::string&"*/ sContent;
			loadVirtualFile(sFilename, sContent);
			createVirtualFile(sFilename, sContent + readBuffer());
		} else {
			FILE* f = fopen(sFilename.c_str(), "ab");
			if (f == NULL) {
				if (bCreateFileIfUnknown) {
					if (createDirectoriesForFile(sFilename)) f = fopen(sFilename.c_str(), "ab");
				}
				if (f == NULL) THROW_UTLEXCEPTION("unable to append a stream into file '" + sFilename + "'");
			}
			int iCursor = 0;
			int iBlock = (_iSize >= 32000) ? 32000 : _iSize;
			do {
				int iWritten = fwrite(_tcStream + iCursor, 1, iBlock, f);
				if (iWritten != iBlock) {
					char tcNumber[32];
					sprintf(tcNumber, "%d", ferror(f));
					THROW_UTLEXCEPTION(std::string("error [") + tcNumber + "] has occurred while appending bytes into file \"" + sFilename + "\"");
				}
				iCursor += iBlock;
				iBlock = (iCursor + 32000 <= _iSize) ? 32000 : _iSize - iCursor;
			} while (iBlock != 0);
			fclose(f);
		}
	}

	void ScpStream::writeBinaryData(const char* tcBinary, int iLength) {
		int iFinalLength = iLength;
		bool bStartIndents = false; // if indent-mode active, do we have to start the writing by an indentation?
		if (!_sIndentation.empty()) {
			// indent-mode active
			int iStartCursor = _iWriteCursor - 1;
			while (iStartCursor >= 0) {
				if (_tcStream[iStartCursor] != '\t') break;
				--iStartCursor;
			}
			// do we have reached a carriage return or the beginning of file,
			// after purging the eventual tabulations?
			if ((iStartCursor < 0) || (_tcStream[iStartCursor] == '\n')) {
				// yes, we'll have to start the writing by an indentation.
				// If indent-mode active, number of tabs for reaching the beginning of line
				int iStartIndents = _iWriteCursor - iStartCursor - 1;
				int iExtraStartingTabs = _sIndentation.length() - iStartIndents; // how many tabs to add?
				if (iExtraStartingTabs > 0) {
					bStartIndents = true;
					_iWriteCursor -= iStartIndents; // we'll have to write at the beginning of the line
					iFinalLength += iExtraStartingTabs; // forecast of extra starting tabulation
				}
			}
			// in the total number of bytes to reserve to the data, add as many indentation as
			// carriage return, ignoring the last one, if at the end of the data
			char* u = (char*) tcBinary;
			for (int i = iLength - 1; i > 0; --i) {
				if (*u++ == '\n') iFinalLength += _sIndentation.length();
			}
		}
		int iNeededSize = _iWriteCursor + iFinalLength;
		int iTotalSize = ((_bInsertText) ? _iSize + iFinalLength : iNeededSize);
		if (_iSize < iTotalSize) {
			_iSize = iTotalSize;
			if (iTotalSize >= _iCacheMemory) {
				int iOldSize = _iCacheMemory;
				do _iCacheMemory *= 2; while (_iSize >= _iCacheMemory);
				char* tcStream = new char[_iCacheMemory];
				memcpy(tcStream, _tcStream, iOldSize);
				delete [] _tcStream;
				_tcStream = tcStream;
			}
		}
		if (_bInsertText) {
			memmove(_tcStream + iNeededSize, _tcStream + _iWriteCursor, _iSize - iNeededSize);
			for (std::map<std::string, int>::iterator i = _mapOfFloatingLocations.begin(); i != _mapOfFloatingLocations.end(); ++i) {
				if (i->second >= _iWriteCursor) _mapOfFloatingLocations[i->first] += iFinalLength;
			}
		}
		if (_sIndentation.empty()) {
			// indent-mode disabled: just have to copy the data to the appropriate area
			// of the buffer
			memcpy(_tcStream + _iWriteCursor, tcBinary, iLength);
		} else {
			// indent-mode active
			char* tcIndentStart = (char*) (_tcStream + _iWriteCursor); // where to copy the data
			if (bStartIndents) {
				// start with an indentation: copy it
				char* v = (char*) _sIndentation.c_str();
				do {
					*tcIndentStart++ = *v++;
				} while (*v != '\0');
			}
			char* u = (char*) tcBinary; // the data to copy
			// copy all bytes, but not the last (handled apart)
			for (int i = iLength - 1; i > 0; --i) {
				*tcIndentStart++ = *u;
				if (*u++ == '\n') {
					// a carriage return: copy an indentation
					char* v = (char*) _sIndentation.c_str();
					do {
						*tcIndentStart++ = *v++;
					} while (*v != '\0');
				}
			}
			*tcIndentStart = *u; // independent copy of the last byte, in case of encountering an end of line
		}
		// update the writing position: at the end of the copied data
		_iWriteCursor += iFinalLength;
	}

	void ScpStream::writeText(const char* tcText) {
		writeBinaryData(tcText, strlen(tcText));
	}

	bool ScpStream::writeTextOnce(const std::string& sText) {
		if (_setOfTextsToInsertOnce.find(sText) == _setOfTextsToInsertOnce.end()) {
			writeBinaryData(sText.c_str(), sText.length());
			_setOfTextsToInsertOnce.insert(sText);
			return true;
		}
		return false;
	}

	void ScpStream::writeString(const std::string& sString) {
		int iRequiredSpace = sString.length() + 2;
		char* tcEscape = (char*) sString.c_str();
		while (*tcEscape != '\0') {
			switch(*tcEscape) {
				case '\\':
				case '\"':
				case '\a':
				case '\b':
				case '\f':
				case '\n':
				case '\r':
				case '\t':
				case '\v':
					++iRequiredSpace;
			}
			++tcEscape;
		}
		int iNeededSize = _iWriteCursor + iRequiredSpace;
		int iTotalSize = ((_bInsertText) ? _iSize + iRequiredSpace : iNeededSize);
		if (_iSize < iTotalSize) {
			_iSize = iTotalSize;
			if (iTotalSize >= _iCacheMemory) {
				int iOldSize = _iCacheMemory;
				do _iCacheMemory *= 2; while (_iSize >= _iCacheMemory);
				char* tcStream = new char[_iCacheMemory];
				memcpy(tcStream, _tcStream, iOldSize);
				delete [] _tcStream;
				_tcStream = tcStream;
			}
		}
		if (_bInsertText) {
			memmove(_tcStream + iNeededSize, _tcStream + _iWriteCursor, _iSize - iNeededSize);
			for (std::map<std::string, int>::iterator i = _mapOfFloatingLocations.begin(); i != _mapOfFloatingLocations.end(); ++i) {
				if (i->second >= _iWriteCursor) _mapOfFloatingLocations[i->first] += iRequiredSpace;
			}
		}
		char* u = (char*) sString.c_str();
		_tcStream[_iWriteCursor++] = '"';
		if ((unsigned int) iRequiredSpace == sString.length() + 2) {
			memcpy(_tcStream + _iWriteCursor, sString.c_str(), sString.length());
			_iWriteCursor += sString.length();
		} else {
			while (*u != '\0') {
				switch(*u) {
					case '\\':
						_tcStream[_iWriteCursor++] = '\\';
						_tcStream[_iWriteCursor++] = '\\';
						break;
					case '"':
						_tcStream[_iWriteCursor++] = '\\';
						_tcStream[_iWriteCursor++] = '"';
						break;
					case '\a':
						_tcStream[_iWriteCursor++] = '\\';
						_tcStream[_iWriteCursor++] = 'a';
						break;
					case '\b':
						_tcStream[_iWriteCursor++] = '\\';
						_tcStream[_iWriteCursor++] = 'b';
						break;
					case '\f':
						_tcStream[_iWriteCursor++] = '\\';
						_tcStream[_iWriteCursor++] = 'f';
						break;
					case '\n':
						_tcStream[_iWriteCursor++] = '\\';
						_tcStream[_iWriteCursor++] = 'n';
						break;
					case '\r':
						_tcStream[_iWriteCursor++] = '\\';
						_tcStream[_iWriteCursor++] = 'r';
						break;
					case '\t':
						_tcStream[_iWriteCursor++] = '\\';
						_tcStream[_iWriteCursor++] = 't';
						break;
					case '\v':
						_tcStream[_iWriteCursor++] = '\\';
						_tcStream[_iWriteCursor++] = 'v';
						break;
					default:
						_tcStream[_iWriteCursor++] = *u;
				}
				++u;
			}
		}
		_tcStream[_iWriteCursor++] = '"';
	}

	void ScpStream::writeQuotedChar(char c) {
		WRITECHAR('\'');
		switch(c) {
			case '\\':
				WRITECHAR('\\');
				WRITECHAR('\\');
				break;
			case '\"':
				WRITECHAR('\\');
				WRITECHAR('\"');
				break;
			case '\'':
				WRITECHAR('\\');
				WRITECHAR('\'');
				break;
			case '\a':
				WRITECHAR('\\');
				WRITECHAR('a');
				break;
			case '\b':
				WRITECHAR('\\');
				WRITECHAR('b');
				break;
			case '\f':
				WRITECHAR('\\');
				WRITECHAR('f');
				break;
			case '\n':
				WRITECHAR('\\');
				WRITECHAR('n');
				break;
			case '\r':
				WRITECHAR('\\');
				WRITECHAR('r');
				break;
			case '\t':
				WRITECHAR('\\');
				WRITECHAR('t');
				break;
			case '\v':
				WRITECHAR('\\');
				WRITECHAR('v');
				break;
			default:
				WRITECHAR(c);
		}
		WRITECHAR('\'');
		if (_iWriteCursor > _iSize) _iSize = _iWriteCursor;
	}

	bool ScpStream::indentAsCpp() {
		ScpStream currentStream;
		std::string sIndentation;
		std::string sSequence;
		std::string/*java:"std::string&"*/ sLastIdentifier;
		int iChar = readChar();
		while (iChar >= 0) {
			int iStartLocation = _iReadCursor - 1;
			while ((iChar == '\t') || (iChar == (int) ' ')) iChar = readChar();
			if (iChar < 0) break;
			if (iChar == (int) '\r') currentStream << '\r';
			else if (iChar == (int) '\n') currentStream << '\n';
			else if (iChar == (int) '#') {
				do {
					currentStream << (unsigned char) iChar;
					iChar = readChar();
				} while ((iChar >= 0) && (iChar != '\n'));
				if (iChar >= 0) currentStream << '\n';
			} else if ((iChar == '/') && isEqualTo("/##")) {
				_iReadCursor = iStartLocation;
				iChar = readChar();
				while ((iChar >= 0) && (iChar != '\n')) {
					currentStream << (unsigned char) iChar;
					iChar = readChar();
				}
				if (iChar >= 0) currentStream << '\n';
			} else {
				std::string sLocalIndentation = sIndentation;
				std::string sLine;
				int iPreviousChar = -1;
				int iPreviousNotEmptyChar = -1;
				int iBeginningOfLine = iStartLocation;
				do {
					switch(iChar) {
						case ':':
							if ((iPreviousChar != (int) ':') && (peekChar() != ':') && (iPreviousNotEmptyChar != (int) ')')) {
								if ((sLastIdentifier == "public") || (sLastIdentifier == "protected") || (sLastIdentifier == "private") || (sLastIdentifier == "case")) {
									if (sSequence.empty() || (sSequence[sSequence.length() - 1] != ':')) {
										sSequence += ':';
										sIndentation += '\t';
									} else {
										--_iWriteCursor;
										if (sLocalIndentation.length() == sIndentation.length()) sLocalIndentation = sLocalIndentation.substr(0, sLocalIndentation.length() - 1);
									}
								}
							}
							sLine += ':';
							iPreviousNotEmptyChar = iChar;
							break;
						case '{':
							sSequence += '{';
							sIndentation += '\t';
							sLine += '{';
							iPreviousNotEmptyChar = iChar;
							break;
						case '}':
							std::string::size_type iOffset;
							if (!sSequence.empty() && (sSequence[sSequence.length() - 1] == ':')) iOffset = 2;
							else iOffset = 1;
							if (sIndentation.length() <= iOffset) {
								if (iPreviousChar < 0) sLocalIndentation = "";
								sSequence = "";
								sIndentation = "";
							} else {
								if (iPreviousChar < 0) sLocalIndentation = sLocalIndentation.substr(0, sLocalIndentation.length() - iOffset);
								sSequence = sSequence.substr(0, sSequence.length() - iOffset);
								sIndentation = sIndentation.substr(0, sIndentation.length() - iOffset);
							}
							sLine += '}';
							iPreviousNotEmptyChar = iChar;
							break;
						case '\\':
							sLine += '\\';
							iPreviousNotEmptyChar = iChar;
							iChar = readChar();
							if (iChar >= 0) sLine += (unsigned char) iChar;
							break;
						case '\'':
						case '\"':
							{
								int iQuote = iChar;
								do {
									if (iChar == (int) '\\') {
										sLine += '\\';
										iChar = readChar();
									}
									sLine += (unsigned char) iChar;
									iChar = readChar();
								} while ((iChar >= 0) && (iChar != iQuote));
								if (iChar >= 0) sLine += (unsigned char) iChar;
								iPreviousNotEmptyChar = iChar;
							}
							break;
						case '/':
							if (isEqualTo('/')) {
								sLine += "//";
								iChar = readChar();
								while ((iChar >= 0) && (iChar != '\n')) {
									sLine += (unsigned char) iChar;
									iChar = readChar();
								}
								if (iChar >= 0) --_iReadCursor;
							} else if (isEqualTo('*')) {
								int iCommentDistance = getInputLocation() - iBeginningOfLine - 2;
								sLine += "/*";
								iChar = readChar();
								while ((iChar >= 0) && ((iChar != '*') || !isEqualTo('/'))) {
									sLine += (unsigned char) iChar;
									if (iChar == '\n') {
										// multiline comment
										int i;
										std::string sBeginning;
										for (i = 0; i < iCommentDistance; ++i) {
											iChar = readChar();
											if ((iChar < 0) || (iChar == '\n')) break;
											sBeginning += (unsigned char) iChar;
											if (iChar > ' ') break;
										}
										if (i >= iCommentDistance) {
											sLine += sLocalIndentation;
											iChar = readChar();
										} else if ((iChar != '\n') && (iChar >= 0)) {
											sLine += sBeginning;
										}
									} else {
										iChar = readChar();
									}
								}
								if (iChar >= 0) sLine += "*/";
							} else {
								sLine += '/';
								iPreviousNotEmptyChar = iChar;
							}
							break;
						default:
							if ((iChar == '_') || ((iChar >= 'a') && (iChar <= 'z')) || ((iChar >= 'A') && (iChar <= 'Z'))) {
								goBack();
								readIdentifier(sLastIdentifier);
								sLine += sLastIdentifier;
								iPreviousNotEmptyChar = iChar;
							} else {
								sLine += (unsigned char) iChar;
								if (iChar > ' ') iPreviousNotEmptyChar = iChar;
							}
					}
					iPreviousChar = iChar;
					iChar = readChar();
				} while ((iChar >= 0) && (iChar != '\n'));
				if (iChar >= 0) sLine += '\n';
				currentStream << sLocalIndentation << sLine;
			}
			iChar = readChar();
		}
		int iPosition;
		if (equals(currentStream, iPosition)) return false;
		_iWriteCursor = 0;
		_iSize = 0;
		copy(currentStream, 0);
		return true;
	}


	std::fstream* openIOFile(const char* sFile, bool bCreateDirectories) {
		std::fstream* pStream = new std::fstream(sFile, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
		if (pStream->fail()) {
			delete pStream;
			pStream = NULL;
			if (bCreateDirectories) {
				createDirectoriesForFile(sFile);
				pStream = new std::fstream(sFile, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
				if (pStream->fail()) {
					delete pStream;
					pStream = NULL;
				}
			}
		}
		return pStream;
	}

	std::ifstream* ScpStream::openSTLInputFile(const char* /*java:"const std::string&"*/ sInputFile) {
		if ((sInputFile == NULL) || (sInputFile[0] == '\0')) return NULL;
		std::ifstream* pStream = new std::ifstream(sInputFile, std::ios::in | std::ios::binary);
		if (pStream->fail()) {
			delete pStream;
			pStream = NULL;
		}
		return pStream;
	}

	std::ifstream* openInputFileFromIncludePath(const char* tcFileName, std::string& sCompleteFileName) {
		sCompleteFileName = tcFileName;
		std::ifstream* pFile = ScpStream::openSTLInputFile(sCompleteFileName.c_str());
		if ((pFile == NULL) && ((sCompleteFileName[0] == '/') || (sCompleteFileName[1] == ':'))) return NULL;
		std::list<std::string>::const_iterator i = ScpStream::getListOfIncludePaths().begin();
		while ((pFile == NULL) && (i != ScpStream::getListOfIncludePaths().end())) {
			sCompleteFileName = *i + tcFileName;
			pFile = ScpStream::openSTLInputFile(sCompleteFileName.c_str());
			i++;
		}
		return pFile;
	}

	int getLocation(std::istream& stream) {
		int iPos = stream.tellg();
		if ((iPos < 0) && !stream.fail()) return 0;
		return iPos;
	}

	void setLocation(std::istream& stream, int iCursor) {
		stream.seekg(iCursor);
	}

	void setLocationAtXLines(std::istream& stream, int iLines) {
		if (iLines <= 0) {
			int iLocation = stream.tellg();
			do {
				int iChar = -1;
				while (iLocation > 0) {
					stream.seekg(--iLocation);
					iChar = readChar(stream);
					if (iChar == '\n') break;
					if (iChar < 0) return;
				}
				if (iLocation == 0) {
					if ((iChar != (int) '\n') || (iLines++ >= 0)) stream.seekg(0);
				}
			} while ((iLocation > 0) && (iLines++ < 0));
			isEqualTo(stream, '\r');
		} else {
			int iChar;
			do {
				do {
					iChar = readChar(stream);
				} while ((iChar != '\n') && (iChar > 0));
				if (iChar >= 0) isEqualTo(stream, '\r');
			} while ((--iLines > 0) && (iChar > 0));
		}
	}

	bool getColEmptyHeader(std::istream& stream, std::string& sHeader) {
		static char tcGetColEmptyHeader[10000];
		int iNbCols = getColCount(stream);
		int iPos = getLocation(stream);
		int iLocation = iPos;
		tcGetColEmptyHeader[--iNbCols] = '\0';
		while (iNbCols > 0) {
			stream.seekg(--iLocation);
			int iChar = readChar(stream);
			if (iChar == (int) '\t') tcGetColEmptyHeader[--iNbCols] = '\t';
			else tcGetColEmptyHeader[--iNbCols] = ' ';
		}
		setLocation(stream, iPos);
		sHeader = (const char*) tcGetColEmptyHeader;
		return true;
	}

	int getLineCount(std::istream& stream) {
	int iLine = 1;
	int iPos = getLocation(stream);
	setLocation(stream, 0);
	int iCursor = 0;
	while (iCursor < iPos) {
		if (readChar(stream) == (int) '\n') iLine++;
		iCursor++;
	}
	return iLine;
	}

	int getColCount(std::istream& stream) {
		int iPosition = CodeWorker::getLocation(stream);
		int iCurrent = iPosition;
		while (iCurrent > 0) {
			iCurrent--;
			CodeWorker::setLocation(stream, iCurrent);
			if (readChar(stream) == (int) '\n') {
				setLocation(stream, iPosition);
				return iPosition - iCurrent;
			}
		}
		setLocation(stream, iPosition);
		return 1 + iPosition;
	}

	bool previousLine(std::istream& stream) {
		int iPosition = getLocation(stream);
		int iCurrent = iPosition;
		bool bEOL = false;
		while ((!bEOL) && (iCurrent > 0)) {
			iCurrent--;
			setLocation(stream, iCurrent);
			if (readChar(stream) == (int) '\n') bEOL = true;
		}
		if (!bEOL) {
			setLocation(stream, iPosition);
			return false;
		}
		bEOL = false;
		while ((!bEOL) && (iCurrent > 0)) {
			iCurrent--;
			setLocation(stream, iCurrent);
			if (readChar(stream) == (int) '\n') bEOL = true;
		}
		return true;
	}

	void goBack(std::istream& stream) {
		int iPos = stream.tellg();
		if (iPos > 0) iPos--;
		stream.seekg(iPos);
	}

	int readChar(std::istream& stream) {
		int iChar = stream.get();
		if (stream.eof()) {
			iChar = -1;
			stream.clear();
		}
		return iChar;
	}

	int peekChar(std::istream& stream) {
		int iChar = stream.get();
		if (stream.eof()) {
			iChar = -1;
			stream.clear();
		}
		stream.seekg(-1, std::ios::cur);
		return iChar;
	}

	bool skipBlanks(std::istream& stream) {
		int iChar = readChar(stream);
		while ((iChar >= (int) '\0') && (iChar <= (int) ' ')) iChar = readChar(stream);
		if (iChar < 0) return false;
		goBack(stream);
		return true;
	}

	bool skipLineBlanks(std::istream& stream) {
		int iChar = readChar(stream);
		while ((iChar >= (int) '\0') && (iChar <= (int) ' ') && (iChar != '\r') && (iChar != '\n')) iChar = readChar(stream);
		if (iChar < 0) return false;
		goBack(stream);
		return true;
	}

	bool skipCppComments(std::istream& stream) {
		int iCursor = getLocation(stream);
		int iChar = readChar(stream);
		if (iChar == (int) '/') {
			iChar = readChar(stream);
			if (iChar == (int) '/') do iChar = readChar(stream); while ((iChar >= 0) && (iChar != (int) '\n'));
			else if (iChar == (int) '*') {
				iChar = readChar(stream);
				do {
					if (iChar == (int) '*') {
						iChar = readChar(stream);
						if (iChar == (int) '/') break;
					} else {
						iChar = readChar(stream);
					}
				} while (iChar >= 0);
				if (iChar < 0) {
					setLocation(stream, iCursor);
					return false;
				}
			} else {
				setLocation(stream, iCursor);
				return false;
			}
		} else {
			setLocation(stream, iCursor);
			return false;
		}
		return true;
	}

	bool skipEmpty(std::istream& stream) {
		bool bSuccess;
		do bSuccess = skipBlanks(stream); while (bSuccess && skipCppComments(stream));
		return bSuccess;
	}

	bool skipEmptyHTML(std::istream& stream) {
		int iChar;
		for(;;) {
			do iChar = readChar(stream); while ((iChar >= (int) '\0') && (iChar <= (int) ' '));
			if ((iChar == (int) '<') && isEqualTo(stream, "!--")) {
				do {
					do iChar = readChar(stream); while ((iChar >= 0) && (iChar != (int) '-'));
				} while ((iChar >= 0) && !isEqualTo(stream, "->"));
			} else break;
		} 
		if (iChar < 0) return false;
		goBack(stream);
		return true;
	}

	bool skipEmptyLaTeX(std::istream& stream) {
		int iChar = readChar(stream);
		if (iChar == (int) '%') {
			do iChar = readChar(stream); while ((iChar >= 0) && (iChar != (int) '\n'));
		} else {
			if (iChar >= 0) goBack(stream);
			return false;
		}
		return true;
	}

	bool readWord(std::istream& stream, std::string& sWord) {
		unsigned char tcReadWord[1000];
		int iIndex = 0;
		int iChar;
		do {
			iChar = readChar(stream);
			tcReadWord[iIndex++] = (unsigned char) iChar;
		} while (((iChar >= (int) 'a') && (iChar <= (int) 'z')) || ((iChar >= (int) 'A') && (iChar <= (int) 'Z')) || ((iChar >= (int) '0') && (iChar <= (int) '9')) || (iChar == (int) '_') || (iChar == (unsigned char) '°') || (iChar == (int) '\'') || (iChar == (int) '^') || (iChar == (unsigned char) 'é') || (iChar == (unsigned char) 'è') || (iChar == (unsigned char) 'ê') || (iChar == (unsigned char) 'à') || (iChar == (unsigned char) 'î') || (iChar == (unsigned char) 'ù') || (iChar == (unsigned char) 'ô') || (iChar == (unsigned char) 'û') || (iChar == (unsigned char) 'â') || (iChar == (unsigned char) 'ç'));
		if (iChar >= 0) goBack(stream);
		iIndex--;
		tcReadWord[iIndex] = '\0';
		sWord = (char*) tcReadWord;
		return (iIndex >= 1);
	}

	bool readIdentifier(std::istream& stream, std::string& sWord) {
		unsigned char tcText[1000];
		int iIndex = 0;
		int iChar;
		iChar = readChar(stream);
		if (((iChar >= (int) 'a') && (iChar <= (int) 'z')) || ((iChar >= (int) 'A') && (iChar <= (int) 'Z')) || (iChar == (int) '_')) {
			tcText[iIndex++] = (unsigned char) iChar;
			do {
				iChar = readChar(stream);
				tcText[iIndex++] = (unsigned char) iChar;
			} while (((iChar >= (int) 'a') && (iChar <= (int) 'z')) || ((iChar >= (int) 'A') && (iChar <= (int) 'Z')) || ((iChar >= (int) '0') && (iChar <= (int) '9')) || (iChar == (int) '_'));
			iIndex--;
			tcText[iIndex] = '\0';
			sWord = (const char*) tcText;
		}
		if (iChar >= 0) goBack(stream);
		return (iIndex >= 1);
	}

	bool readLine(std::istream& stream, std::string& sLine) {
		static char tcText[10000];
		int i = 0;
		int iChar;
		do {
			iChar = readChar(stream);
			if ((iChar == '\n') || (iChar < 0)) break;
			tcText[i++] = (char) iChar;
		} while (true);
		if (iChar >= 0) isEqualTo(stream, '\r');
		if ((i > 0) && (tcText[i - 1] == '\r')) i--;
		tcText[i] = '\0';
		sLine = (const char*) tcText;
		return ((iChar >= 0) || (i > 0));
	}

	bool readInt(std::istream& stream, int& iResult) {
		unsigned char tcText[32];
		int iIndex = 0;
		int iChar = readChar(stream);
		if (iChar == (int) '-') {
			tcText[iIndex++] = '-';
			iChar = readChar(stream);
		}
		if ((iChar < (int) '0') || (iChar > (int) '9')) {
			goBack(stream);
			if (iIndex != 0) goBack(stream);
			return false;
		}
		tcText[iIndex++] = (unsigned char) iChar;
		iChar = readChar(stream);
		while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
			tcText[iIndex++] = (unsigned char) iChar;
			iChar = readChar(stream);
		}
		if (iChar >= 0) setLocation(stream, getLocation(stream) - 1);
		tcText[iIndex] = '\0';
		iResult = atoi((const char*) tcText);
		return true;
	}

	bool readLong(std::istream& stream, long& lResult) {
		unsigned char tcText[32];
		int iIndex = 0;
		int iChar = readChar(stream);
		if (iChar == (int) '-') {
			tcText[iIndex++] = '-';
			iChar = readChar(stream);
		}
		if ((iChar < (int) '0') || (iChar > (int) '9')) {
			goBack(stream);
			if (iIndex != 0) goBack(stream);
			return false;
		}
		tcText[iIndex++] = (unsigned char) iChar;
		iChar = readChar(stream);
		while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
			tcText[iIndex++] = (unsigned char) iChar;
			iChar = readChar(stream);
		}
		if (iChar >= 0) setLocation(stream, getLocation(stream) - 1);
		tcText[iIndex] = '\0';
		lResult = atol((const char*) tcText);
		return true;
	}


	bool readBoolean(std::istream& stream, bool& bResult) {
		int iChar = readChar(stream);
		if ((iChar == (int) 'f') || (iChar == (int) 'F')) {
			iChar = readChar(stream);
			if ((iChar != (int) 'a') && (iChar != (int) 'A')) return false;
			iChar = readChar(stream);
			if ((iChar != (int) 'l') && (iChar != (int) 'L')) return false;
			iChar = readChar(stream);
			if ((iChar != (int) 's') && (iChar != (int) 'S')) return false;
			iChar = readChar(stream);
			if ((iChar != (int) 'e') && (iChar != (int) 'E')) return false;
			bResult = false;
		} else if ((iChar == (int) 't') || (iChar == (int) 'T')) {
			iChar = readChar(stream);
			if ((iChar != (int) 'r') && (iChar != (int) 'R')) return false;
			iChar = readChar(stream);
			if ((iChar != (int) 'u') && (iChar != (int) 'U')) return false;
			iChar = readChar(stream);
			if ((iChar != (int) 'e') && (iChar != (int) 'E')) return false;
			bResult = true;
		}
		return true;
	}

	bool readStringOrCharLiteral(std::istream& stream, std::string& sText) {
		if (readString(stream, sText)) return true;
		int iChar;
		if (!readCharLiteral(stream, iChar)) return false;
		sText.assign(1, (char) iChar);
		return true;
	}

	bool readCharLiteral(std::istream& stream, int& iChar) {
		int iPosition = getLocation(stream);
		if (!isEqualTo(stream, '\'')) return false;
		iChar = readChar(stream);
		if (iChar == '\\') {
			iChar = readChar(stream);
			switch(iChar) {
				case 'a': iChar = '\a';break;
				case 'b': iChar = '\b';break;
				case 'f': iChar = '\f';break;
				case 'n': iChar = '\n';break;
				case 'r': iChar = '\r';break;
				case 't': iChar = '\t';break;
				case 'v': iChar = '\v';break;
				case '0':
					{
						iChar = 0;
						int iCurrentChar = readChar(stream);
						if (iCurrentChar == (int) 'x') {
							do {
								iCurrentChar = readChar(stream);
								if ((iCurrentChar >= '0') && (iCurrentChar <= '9')) {
									iChar *= 16;
									iChar += (iCurrentChar - (int) '0');
								} else if ((iCurrentChar >= 'a') && (iCurrentChar <= 'f')) {
									iChar *= 16;
									iChar += (iCurrentChar - (int) 'a') + 10;
								} else if ((iCurrentChar >= 'A') && (iCurrentChar <= 'F')) {
									iChar *= 16;
									iChar += (iCurrentChar - (int) 'A') + 10;
								} else break;
							} while (true);
						} else if ((iCurrentChar >= (int) '0') && (iCurrentChar <= (int) '7')) {
							iChar = iCurrentChar - (int) '0';
							do {
								iCurrentChar = readChar(stream);
								if ((iCurrentChar >= '0') && (iCurrentChar <= '7')) {
									iChar *= 8;
									iChar += (iCurrentChar - (int) '0');
								} else break;
							} while (true);
						}
						goBack(stream);
					}
					break;
				case '1':
				case '2':
				case '3':
					{
						iChar -= '0';
						int iCurrentChar;
						do {
							iCurrentChar = readChar(stream);
							if ((iCurrentChar >= '0') && (iCurrentChar <= '7')) {
								iChar *= 8;
								iChar += (iCurrentChar - (int) '0');
							} else break;
						} while (true);
					}
					break;
			}
		}
		if (!isEqualTo(stream, '\'')) {
			setLocation(stream, iPosition);
			return false;
		}
		return true;
	}

	bool readString(std::istream& stream, std::string& sResult) {
		unsigned char tcText[32000];
		int iIndex = 0;
		int iLocation = getLocation(stream);
		int iChar = readChar(stream);
		if (iChar != (int) '\"') {
			setLocation(stream, iLocation);
			return false;
		}
		iChar = readChar(stream);
		while ((iChar >= 0) && (iChar != (int) '\"')) {
			if (iChar == '\\') {
				iChar = readChar(stream);
				switch(iChar) {
					case 'a': iChar = '\a';break;
					case 'b': iChar = '\b';break;
					case 'n': iChar = '\n';break;
					case 'r': iChar = '\r';break;
					case 't': iChar = '\t';break;
					case 'v': iChar = '\v';break;
				}
			} else if (iChar == (int) '\n') break;
			tcText[iIndex++] = (unsigned char) iChar;
			iChar = readChar(stream);
		}
		if (iChar != (int) '\"') {
			setLocation(stream, iLocation);
			return false;
		}
		tcText[iIndex] = '\0';
		sResult = (char*) tcText;
		return true;
	}

	bool readDouble(std::istream& stream, double& dResult) {
		unsigned char tcText[32];
		int iIndex = 0;
		int iChar = readChar(stream);
		if (iChar == (int) '-') {
			tcText[iIndex++] = '-';
			iChar = readChar(stream);
		}
		if ((iChar < (int) '0') || (iChar > (int) '9')) {
			setLocation(stream, getLocation(stream) - 1 - iIndex);
			return false;
		}
		tcText[iIndex++] = (unsigned char) iChar;
		iChar = readChar(stream);
		while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
			tcText[iIndex++] = (unsigned char) iChar;
			iChar = readChar(stream);
		}
		if (iChar == (int) '.') {
			tcText[iIndex++] = (unsigned char) '.';
			iChar = readChar(stream);
			while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
				tcText[iIndex++] = (unsigned char) iChar;
				iChar = readChar(stream);
			}
		}
		if ((iChar == (int) 'e') || (iChar == (int) 'E')) {
			int iCursor = getLocation(stream);
			int iMarkedIndex = iIndex;
			tcText[iIndex++] = (unsigned char) 'E';
			iChar = readChar(stream);
			if (iChar == (int) '-') {
				tcText[iIndex++] = (unsigned char) iChar;
				iChar = readChar(stream);
			} else if (iChar == (int) '+') {
				iChar = readChar(stream);
			}
			if ((iChar < (int) '0') || (iChar > (int) '9')) {
				iIndex = iMarkedIndex;
				setLocation(stream, iCursor);
			} else {
				tcText[iIndex++] = (unsigned char) iChar;
				iChar = readChar(stream);
				while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
					tcText[iIndex++] = (unsigned char) iChar;
					iChar = readChar(stream);
				}
			}
		}
		if (iChar >= 0) setLocation(stream, getLocation(stream) - 1);
		tcText[iIndex] = '\0';
		dResult = atof((const char*) tcText);
		return true;
	}

	bool readDate(std::istream& stream, tm& myDate) {
		int iChar = readChar(stream);
		if ((iChar < (int) '0') || (iChar > (int) '9')) {
			std::string sWord;
			if ((iChar == (int) 't') && !readWord(stream, sWord)) return false;
			if (sWord != "oday") return false;
			myDate.tm_mday = 0;
			myDate.tm_mon  = 0;
			myDate.tm_year = 0;
			return true;
		}
		int iDay = iChar - ((int) '0');
		iChar = readChar(stream);
		if ((iChar >= (int) '0') && (iChar <= (int) '9')) {
			iDay *= 10;
			iDay += iChar - ((int) '0');
			iChar = readChar(stream);
		}
		while ((iChar >= 0) && (iChar <= (int) ' ')) iChar = readChar(stream);
		if (iChar != (int) '/') return false;
		iChar = readChar(stream);
		while ((iChar >= 0) && (iChar <= (int) ' ')) iChar = readChar(stream);
		if ((iChar < (int) '0') || (iChar > (int) '9')) return false;
		int iMonth = iChar - ((int) '0');
		iChar = readChar(stream);
		if ((iChar >= (int) '0') && (iChar <= (int) '9')) {
			iMonth *= 10;
			iMonth += iChar - ((int) '0');
			iChar = readChar(stream);
		}
		while ((iChar >= 0) && (iChar <= (int) ' ')) iChar = readChar(stream);
		if (iChar != (int) '/') return false;
		iChar = readChar(stream);
		while ((iChar >= 0) && (iChar <= (int) ' ')) iChar = readChar(stream);
		if ((iChar < (int) '0') || (iChar > (int) '9')) return false;
		int iYear = iChar - ((int) '0');
		iChar = readChar(stream);
		while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
			iYear *= 10;
			iYear += iChar - ((int) '0');
			iChar = readChar(stream);
		}
		goBack(stream);
		myDate.tm_mday = iDay;
		myDate.tm_mon  = iMonth - 1;
		myDate.tm_year = iYear;
		return true;
	}

	bool readFormattedText(std::istream& stream, const std::string& sFormat, std::string& sResult) {
		int iPosition = getLocation(stream);
		std::string::size_type iIndex = 0;
		sResult = "";
		while (iIndex < sFormat.length()) {
			char cFormat = sFormat[iIndex++];
			if (cFormat == '%') {
				switch(sFormat[iIndex++]) {
					case 'd':
						{
							int iChar = readChar(stream);
							if (iChar == (int) '-') {
								sResult += "-";
								iChar = readChar(stream);
							}
							if ((iChar < (int) '0') || (iChar > (int) '9')) {
		        				setLocation(stream, iPosition);
								return false;
							}
							sResult += (char) iChar;
							iChar = readChar(stream);
							while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
								sResult += (char) iChar;
								iChar = readChar(stream);
							}
							goBack(stream);

						}
						break;
					default:
		        		setLocation(stream, iPosition);
						return false;
				}
			} else {
				int iChar = readChar(stream);
				if (iChar == (unsigned char) cFormat) {
					sResult += cFormat;
				} else {
        			setLocation(stream, iPosition);
					return false;
				}
			}
		}
		return true;
	}

	bool readPointer(std::istream& theStream, unsigned long& iPointer) {
		unsigned char tcPointer[4];
		int iChar = theStream.get();
		if (!theStream.eof()) {
			tcPointer[0] = (unsigned char) iChar;
			iChar = theStream.get();
			if (!theStream.eof()) {
				tcPointer[1] = (unsigned char) iChar;
				iChar = theStream.get();
				if (!theStream.eof()) {
					tcPointer[2] = (unsigned char) iChar;
					iChar = theStream.get();
					if (!theStream.eof()) {
						tcPointer[3] = (unsigned char) iChar;
#ifdef WIN32
						unsigned char* tcBuffer = (unsigned char*) &iPointer;
						tcBuffer[0] = tcPointer[3];
						tcBuffer[1] = tcPointer[2];
						tcBuffer[2] = tcPointer[1];
						tcBuffer[3] = tcPointer[0];
#else
						iPointer = ntohl(*((unsigned long*) tcPointer));
#endif
						return true;
					}
				}
			}
		}
		theStream.clear();
		return false;
	}

	bool readUptoChar(std::istream& myStream, char cEnd, std::string& sText) {
		int iLocation = getLocation(myStream);
		std::string sLocalText;
		int iChar = readChar(myStream);
		while ((iChar > 0) && (iChar != (int) cEnd)) {
			sLocalText += (char) iChar;
			iChar = readChar(myStream);
		}
		if (iChar < 0) {
			setLocation(myStream, iLocation);
			return false;
		}
		sText = sLocalText;
		goBack(myStream);
		return true;
	}

	bool readUptoChar(std::istream& myStream, const std::string& sEnd, std::string& sText) {
		int iLocation = getLocation(myStream);
		std::string sLocalText;
		int iChar = readChar(myStream);
		while ((iChar > 0) && (sEnd.find((char) iChar) == std::string::npos)) {
			sLocalText += (char) iChar;
			iChar = readChar(myStream);
		}
		if (iChar < 0) {
			setLocation(myStream, iLocation);
			return false;
		}
		sText = sLocalText;
		goBack(myStream);
		return true;
	}

	bool isEqualTo(std::istream& stream, unsigned char ch) {
		int iChar = readChar(stream);
		if (iChar == (int) ch) return true;
		if (iChar >= 0) setLocation(stream, getLocation(stream) - 1);
		return false;
	}

	bool isEqualTo(std::istream& stream, const char* sText) {
		int iChar;
		int iPosition = getLocation(stream);
		for (int i = 0; sText[i] != '\0'; ++i) {
			iChar = readChar(stream);
			if ((iChar < 0) || ((char) iChar != sText[i])) {
        		setLocation(stream, iPosition);
				return false;
			}
		}
		return true;
	}

	bool isEqualToIdentifier(std::istream& stream, const char* sText) {
		int iPosition = getLocation(stream);
		std::string sIdentifier;
		if (!readIdentifier(stream, sIdentifier)) return false;
		if (sIdentifier != sText) {
			setLocation(stream, iPosition);
			return false;
		}
		return true;
	}

	bool findString(std::istream& stream, const char* sText) {
		int iChar = -1;
		int iPosition = getLocation(stream);
		int iCurrent = iPosition;
		int i;
		do {
			setLocation(stream, iCurrent);
			for (i = 0; sText[i] != '\0'; ++i) {
				iChar = readChar(stream);
				if (iChar < 0) break;
				if ((char) iChar != sText[i]) break;
			}
			++iCurrent;
		} while ((iChar >= 0) && (sText[i] != '\0'));
		if (sText[i] == '\0') return true;
		setLocation(stream, iPosition);
		return false;
	}

	std::string getLastReadChars(std::istream& stream, int iNbChars) {
		int iPosition = getLocation(stream);
		if (iPosition < iNbChars) iNbChars = iPosition;
		setLocation(stream, iPosition - iNbChars);
		std::string sText;
		while (iNbChars > 0) {
			int iChar = readChar(stream);
			if (iChar < 0) break;
			sText += (char) iChar;
			iNbChars--;
		}
		return sText;
	}


	std::ofstream* openOutputFile(const char* sOutputFile, bool bCreateDirectories) {
		std::ofstream* pStream = new std::ofstream(sOutputFile, std::ios::out | std::ios::binary);
		if (pStream->fail()) {
			delete pStream;
			pStream = NULL;
			if (bCreateDirectories) {
				createDirectoriesForFile(sOutputFile);
				pStream = new std::ofstream(sOutputFile, std::ios::out | std::ios::binary);
				if (pStream->fail()) {
					delete pStream;
					pStream = NULL;
				}
			}
		}
		return pStream;
	}

	std::ofstream* openAppendFile(const char* sAppendFile) {
		std::ofstream* pStream = new std::ofstream(sAppendFile, std::ios::app | std::ios::binary);
		if (pStream->fail()) {
			delete pStream;
			pStream = NULL;
		}
		return pStream;
	}

	void writeChar(std::ostream& stream, char c) {
		stream.put(c);
	}

	void writeText(std::ostream& stream, const char* sText) {
		if (*sText != '\0') stream.write(sText, strlen(sText));
	}

	void writeComments(std::ostream& stream, const char* sComment) {
#ifdef WIN32
		char* tLines = ::_strdup(sComment);
#else
		char* tLines = ::strdup(sComment);
#endif
		char* sCursor = tLines;
		int iNbLines = 1;
		while ((sCursor = strchr(sCursor, '\n')) != NULL) {
			*sCursor++ = '\0';
			iNbLines++;
		}
		sCursor = tLines;
		for (int iLine = 0; iLine < iNbLines; iLine++) {
			writeText(stream, "// ");
			writeText(stream, sCursor);
			carriageReturn(stream);
			int iLength = strlen(sCursor);
			sCursor += (iLength + 1);
		}
		free(tLines);
	}

	void writeBorderedComments(std::ostream& stream, const char* sComment) {
#ifdef WIN32
		char* tLines = ::_strdup(sComment);
#else
		char* tLines = ::strdup(sComment);
#endif
		char* sCursor = tLines;
		int iNbLines = 1;
		while ((sCursor = strchr(sCursor, '\n')) != NULL) {
			*sCursor++ = '\0';
			iNbLines++;
		}
		int iIndex;
		int iMaxLength = 0;
		sCursor = tLines;
		for (iIndex = 0; iIndex < iNbLines; iIndex++) {
			int iLength = strlen(sCursor);
			if (iLength > iMaxLength) iMaxLength = iLength;
			sCursor += (iLength + 1);
		}
		writeText(stream, "///");
		for (iIndex = 0; iIndex < iMaxLength; iIndex++) writeChar(stream, '/');
		writeText(stream, "///\n");
		sCursor = tLines;
		for (int iLine = 0; iLine < iNbLines; iLine++) {
			int iLength = strlen(sCursor);
			writeText(stream, "// ");
			writeText(stream, sCursor);
			for (iIndex = iLength; iIndex < iMaxLength; iIndex++) writeChar(stream, ' ');
			writeText(stream, " //\n");
			sCursor += (iLength + 1);
		}
		writeText(stream, "///");
		for (iIndex = 0; iIndex < iMaxLength; iIndex++) writeChar(stream, '/');
		writeText(stream, "///\n");
		free(tLines);
	}

	void carriageReturn(std::ostream& stream) {
		writeChar(stream, '\n');
	}

	void writeInt(std::ostream& stream, int iValue) {
		char sNumber[32];
		sprintf(sNumber, "%d", iValue);
		writeText(stream, sNumber);
	}

	void writeBoolean(std::ostream& stream, bool bValue) {
		if (bValue) writeText(stream, "True");
		else writeText(stream, "False");
	}

	void writeDouble(std::ostream& stream, double dValue) {
		char sNumber[320];
		sprintf(sNumber, "%f", dValue);
		writeText(stream, sNumber);
	}

	void writeString(std::ostream& stream, const char* sText) {
		writeChar(stream, '\"');
		const char* u = sText;
		while (*u != '\0') {
			switch(*u) {
				case '\"':
					stream.put('\\');
					stream.put('"');
					break;
				case '\\':
					stream.put('\\');
					stream.put('\\');
					break;
				case '\a':
					stream.put('\\');
					stream.put('a');
					break;
				case '\b':
					stream.put('\\');
					stream.put('b');
					break;
				case '\f':
					stream.put('\\');
					stream.put('f');
					break;
				case '\n':
					stream.put('\\');
					stream.put('n');
					break;
				case '\r':
					stream.put('\\');
					stream.put('r');
					break;
				case '\t':
					stream.put('\\');
					stream.put('t');
					break;
				case '\v':
					stream.put('\\');
					stream.put('v');
					break;
				default:
					stream.put(*u);
			}
			u++;
		}
		writeChar(stream, '\"');
	}

	void writeDate(std::ostream& stream, const tm& myDate) {
		if ((myDate.tm_mday == 0) && (myDate.tm_mon == 0) && (myDate.tm_year == 0)) writeText(stream, "today");
		else {
			if (myDate.tm_mday < 10) writeChar(stream, '0');
			writeInt(stream, myDate.tm_mday);
			writeChar(stream, '/');
			if (myDate.tm_mon < 9) writeChar(stream, '0');
			writeInt(stream, myDate.tm_mon + 1);
			writeChar(stream, '/');
			writeInt(stream, myDate.tm_year);
		}
	}

	void writePointer(std::ostream& theStream, unsigned long iPointer) {
#ifdef WIN32
		unsigned char* tcPointer = (unsigned char*) &iPointer;
		theStream.put(tcPointer[3]);
		theStream.put(tcPointer[2]);
		theStream.put(tcPointer[1]);
		theStream.put(tcPointer[0]);
#else
		iPointer = htonl(iPointer);
		unsigned char* tcPointer = (unsigned char*) &iPointer;
		theStream.put(tcPointer[0]);
		theStream.put(tcPointer[1]);
		theStream.put(tcPointer[2]);
		theStream.put(tcPointer[3]);
#endif
	}


	void copyToStream(std::istream& inputStream, std::ostream& outputStream, int iStart, int iLength) {
		int iPosition = getLocation(inputStream);
		setLocation(inputStream, iStart);
		while (iLength-- != 0) {
			int iChar = readChar(inputStream);
			if (iChar == -1) break;
			outputStream.put((char) iChar);
		}
		setLocation(inputStream, iPosition);
	}

	bool isPathTerminator_(char c) {
		return ((c=='/') || (c=='\0'));
	}

	void toSlashes_(char *str) {
		for(char *p = str; *p != '\0'; ++p) if (*p == '\\') *p='/';
	}

	// bug on canonizeUNC_ ; once fixed, the canonize_ below will disappear
	// and canonizeUNC_ will be renamed to canonize_
	std::string canonizeUNC_(const char* tcPathname, char* dest1) {
		char dest2[4096];

		{ // === First make an absolute path in dest1
			const char *src = tcPathname;
			char       *dst = dest1;

#ifdef WIN32
			// Handle the Windows drive designation 'X:'
			if (*(src+1)==':') {
				char drive=*src;
				src+=2;
				// Cedric, 12/04/2007 - code moved here
				*dst=tolower(drive);  dst++;
				*dst=':';             dst++;
				*dst='/';             dst++;
			} else if ((*src == '/' || *src == '\\') && (*(src+1) == '/' || *(src+1) == '\\'))  {
				// Cedric, 10/04/2007
				strcpy(dest2, src);
				return dest2;
			}
#endif

			// Add working dir if necessary
			if (*src!='/'
					&&
				*src!='\\'
#ifdef WIN32
					&&
				*src!='\0'
#endif
				) {
				unsigned int iLength = strlen(dst);
				if (iLength == 0 || (dst[iLength-1] != '\\' && dst[iLength-1] != '/')) {
					// Cedric 14/05/2007
					dst+=iLength;
					*dst='/'; dst++;
					*dst = '\0';
				} else {
					dst+=iLength;
					*dst = '\0';
				}
			}
			else
#ifdef WIN32
			if (*src!='\0')
#endif
			{
				src++;
			}

			// Copy end of path, with '/' as directory separator
			strcpy(dst,src);
			toSlashes_(dest1);
		}

		// === Remove trailing slash
		size_t length=strlen(dest1);
		if (dest1[length-1]=='/') {
			dest1[length-1]=0;
		}

		{ // == Fix path (remove ./ and ../) to dest2
			char *src=dest1;
			char *dst=dest2;
#ifdef WIN32
			*dst=*src; dst++; src++;  // Copy drive letter [or first '\' Cedric 12/04/2007]
			if (*src != '\\' && *src != '/') {
				*dst=*src; dst++; src++;  // Copy ':' [but not  second '\' Cedric 14/05/2007]
			}
			*dst='\0';
#endif
			char *dstMin=dst+1;
			// Step through the input path
			while(*src) {
				// Skip input slashes
				if (*src=='/') {
					src++;
					continue;
				}
				// Skip "." and output nothing
				if ((*src=='.') && isPathTerminator_(*(src+1))) {
					src++;
					continue;
				}
				// Skip ".." and remove previous output directory
				if ((*src=='.') && (*(src + 1)=='.') && isPathTerminator_(*(src+2))) {
					src+=2;
					// Don't back up over slash directory
					if (dst > dstMin) {
						// This requires "/" to follow drive spec
						while ((*--dst)!='/');
						*dst=0;
					}
					continue;
				}
				// Copy path component from in to out
				*dst++='/';
				while(!isPathTerminator_(*src)) *dst++=*src++;
				// Null terminate the output
				*dst = '\0';
			}
		}
		return dest2;
	}

	std::string canonize_(const char* tcPathname, char* dest1) {
		char dest2[4096];

		{ // === First make an absolute path in dest1
			const char *src = tcPathname;
			char       *dst = dest1;

#ifdef WIN32
			if ((*src == '/' || *src == '\\') && (*(src+1) == '/' || *(src+1) == '\\'))  {
				// Cedric - 03/08/2007
				return canonizeUNC_(tcPathname, dest1);
			}
			// Handle the Windows drive designation 'X:'
			char drive;
			if (*(src+1)==':') {
				drive=*src;
				src+=2;
			} else {
				drive=*dest1;
			}
			*dst=tolower(drive);  dst++;
			*dst=':';             dst++;
			*dst='/';             dst++;
#endif

			// Add working dir if necessary
			if (*src!='/'
					&&
				*src!='\\'
#ifdef WIN32
					&&
				*src!='\0'
#endif
				) {
				dst+=strlen(dst);
				*dst='/'; dst++;
			}
			else
#ifdef WIN32
			if (*src!='\0')
#endif
			{
				src++;
			}

			// Copy end of path, with '/' as directory separator
			strcpy(dst,src);
			toSlashes_(dest1);
		}

		// === Remove trailing slash
		size_t length=strlen(dest1);
		if (dest1[length-1]=='/') {
			dest1[length-1]=0;
		}

		{ // == Fix path (remove ./ and ../) to dest2
			char *src=dest1;
			char *dst=dest2;
#ifdef WIN32
			*dst=*src; dst++; src++;  // Copy drive letter
			*dst=*src; dst++; src++;  // Copy ':'
			*dst='\0';
#endif
			char *dstMin=dst+1;
			// Step through the input path
			while(*src) {
				// Skip input slashes
				if (*src=='/') {
					src++;
					continue;
				}
				// Skip "." and output nothing
				if ((*src=='.') && isPathTerminator_(*(src+1))) {
					src++;
					continue;
				}
				// Skip ".." and remove previous output directory
				if ((*src=='.') && (*(src + 1)=='.') && isPathTerminator_(*(src+2))) {
					src+=2;
					// Don't back up over slash directory
					if (dst > dstMin) {
						// This requires "/" to follow drive spec
						while ((*--dst)!='/');
						*dst=0;
					}
					continue;
				}
				// Copy path component from in to out
				*dst++='/';
				while(!isPathTerminator_(*src)) *dst++=*src++;
				// Null terminate the output
				*dst = '\0';
			}
		}
		return dest2;
	}
}
