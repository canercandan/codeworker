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

#ifndef _ScpStream_h_
#define _ScpStream_h_

#ifndef WIN32
#	include <features.h>
#	if defined(__cplusplus) && __GNUC_PREREQ (4, 3)
#		include <cstring>
#		include <cstdlib>
#	endif
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

#ifndef WIN32
#	include <cstdio> // for Debian/gcc 2.95.4
#endif

#include <list>
#include <map>
#include <set>

namespace CodeWorker {

#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

	class ScpStream;

	typedef bool (*END_STREAM_CALLBACK)(ScpStream& theStream, void* data);

	class ScpStream {
	public:
		static int IN;
		static int OUT;
		static int INOUT;
		static int APPEND;
		static int PATH;

		static std::string ENDL;

		struct SizeAttributes {
			int _iSize;
			int _iReadCursor;
			int _iWriteCursor;
			char _cEndChar;
			SizeAttributes* _pShiftedStream;

			inline SizeAttributes() : _iSize(-1), _iReadCursor(-1), _iWriteCursor(-1), _pShiftedStream(NULL), _cEndChar('\0') {}
			inline SizeAttributes(const SizeAttributes& copy) : _iSize(copy._iSize), _iReadCursor(copy._iReadCursor), _iWriteCursor(copy._iWriteCursor), _cEndChar(copy._cEndChar) {
				if (copy._pShiftedStream != NULL) {
					_pShiftedStream = new SizeAttributes(*(copy._pShiftedStream));
				} else _pShiftedStream = NULL;
			}
			inline ~SizeAttributes() { delete _pShiftedStream; }
			inline bool empty() const { return (_iSize < 0); }
		};

	private:
		char* _tcStream;
		int _iCacheMemory;
		int _iSize;
		int _iReadCursor;
		int _iWriteCursor;
		bool _bInsertText;
		std::string _sIndentation;

		int _iMode;
		std::string _sFilename;
		mutable ScpStream* _pParentStream;
		ScpStream* _pShiftedStream;
		int _iShiftedStreamPosition;
		ScpStream* _pPrecStream;
		ScpStream* _pNextStream;

		// for the line directive
		int _iLineCounter;
		int _iLinePosition;

		mutable std::map<std::string, int> _mapOfFloatingLocations;
		std::set<std::string> _setOfTextsToInsertOnce;

		static std::list<std::string> _listOfIncludePaths;
		static std::map<std::string, std::string> _listOfVirtualFiles;

		END_STREAM_CALLBACK _pfStreamReaderCallback;
		void*	_pStreamReaderCBKData;
		END_STREAM_CALLBACK _pfStreamWriterCallback;
		void*	_pStreamWriterCBKData;

	public:
		ScpStream(int iCacheMemory = 2000000);
		ScpStream(const std::string& sFilename, int iMode, int iCacheMemory = 2000000, int iLength = -1);
		ScpStream(const std::string& sFilename, FILE* f, int iCacheMemory = 2000000);
		ScpStream(const std::string& sText);
		ScpStream(ScpStream& shiftedStream, int iShiftedStreamPosition);
		~ScpStream();

		static ScpStream* createFile(const std::string& sFilename);
		static bool existInputFileFromIncludePath(const char* tcFileName, std::string& sCompleteFileName);
		static bool existInputFile(const char* sFileName);
		static ScpStream* openInputFileFromIncludePath(const char* tcFileName, std::string& sCompleteFileName);
		static ScpStream* openInputFile(const char* sFileName);

		inline const char* readBuffer() const { _tcStream[_iSize] = '\0';return _tcStream; }
		inline void setFilename(const std::string& sFilename) { _sFilename = sFilename; }
		inline const std::string& getFilename() const { return _sFilename; }
		inline int size() const { return _iSize; }
		inline bool empty() const { return (_iSize == 0); }
		inline bool insertMode() const { return _bInsertText; }
		inline void insertMode(bool bInsertMode) { _bInsertText = bInsertMode; }
		inline ScpStream* getParentStream() const { return _pParentStream; }
		inline void setParentStream(ScpStream* pStream) { _pParentStream = pStream; }
		inline END_STREAM_CALLBACK getStreamReaderCallback() const { return _pfStreamReaderCallback; }
		void setStreamReaderCallback(END_STREAM_CALLBACK pfCBK, void* pData = NULL);
		inline END_STREAM_CALLBACK getStreamWriterCallback() const { return _pfStreamWriterCallback; }
		void setStreamWriterCallback(END_STREAM_CALLBACK pfCBK, void* pData = NULL);

		inline const std::string& getIndentation() const { return _sIndentation; }
		void incrementIndentation(int iLevel = 1);
		bool decrementIndentation(int iLevel = 1);

		std::string getMessagePrefix(bool bCountCols = false) const;

		int readChar();
		int peekChar();
		inline int getInputLocation() const { return _iReadCursor; }
		inline void setInputLocation(int iLocation) { if (iLocation <= _iSize) _iReadCursor = iLocation; }
		inline int getOutputLocation() const { return _iWriteCursor; }
		inline void setOutputLocation(int iLocation) { if (iLocation <= _iSize) _iWriteCursor = iLocation; }
		SizeAttributes resize(int iNewSize);
		void restoreSize(const SizeAttributes& sizeAttrs);
		void setLineDirective(int iLine);
		int getLineCount() const;
		int getOutputLineCount() const;
		int getColCount() const;
		int getOutputColCount() const;
		bool goBack();
		bool skipBlanks();
		bool skipSpaces();
		bool skipLineBlanks();
		bool skipCppComments();
		bool skipCppExceptDoxygenComments();
		bool skipEmpty();
		bool skipEmptyCppExceptDoxygen();
		bool skipEmptyAda();
		bool skipEmptyHTML();
		bool skipEmptyLaTeX();
		bool skipLine();
		bool readChars(int iLength, std::string& sText);
		bool readWord(std::string& sWord);
		bool readIdentifier(std::string& sIdentifier);
		bool readInt(int& iResult);
		bool readHexadecimal(int& iResult);
		bool readDouble(double& dValue);
		bool readStringOrCharLiteral(std::string& sText);
		bool readCharLiteral(int& iChar);
		bool readString(std::string& sText);
		bool readAdaString(std::string& sText);
		bool readPythonString(std::string& sText);
		bool readLine(std::string& sLine);
		bool readLastChars(int iLength, std::string& sLastChars);
		bool readUptoChar(char cEnd, std::string& sText);
		bool readUptoChar(const std::string& sEnd, std::string& sText);

		bool isEqualTo(unsigned char ch);
		bool isEqualTo(const char* sText);
		bool isEqualToIgnoreCase(unsigned char ch);
		bool isEqualToIgnoreCase(const char* sText);
		bool isEqualToIdentifier(const char* sText);
		inline bool isEqualTo(const std::string& sText) { return isEqualTo(sText.c_str()); }
		inline bool isEqualToIgnoreCase(const std::string& sText) { return isEqualToIgnoreCase(sText.c_str()); }
		bool findString(const char* sText);
		bool findString(const char* sText, int iBoundary);
		inline bool findString(const std::string& sText) { return findString(sText.c_str()); }
		inline bool findString(const std::string& sText, int iBoundary) { return findString(sText.c_str(), iBoundary); }

		inline const std::map<std::string, int>& allFloatingLocations() const { return _mapOfFloatingLocations; }
		bool newFloatingLocation(const std::string& sKey);
		void setFloatingLocation(const std::string& sKey, int iLocation);
		int getFloatingLocation(const std::string& sKey, ScpStream*& pOwner) const;
		int removeFloatingLocation(const std::string& sKey, ScpStream*& pOwner);

		ScpStream& operator <<(char cValue);
		ScpStream& operator <<(unsigned char cValue);
		ScpStream& operator <<(int iValue);
		ScpStream& operator <<(long lValue);
		ScpStream& operator <<(unsigned long lValue);
		ScpStream& operator <<(double dValue);
		ScpStream& operator <<(const char* tcValue);
		ScpStream& operator <<(const std::string& sValue);
		ScpStream& operator <<(const ScpStream& theStream);
		void writeString(const std::string& sString);
		void writeQuotedChar(char c);
		void writeBinaryData(const char* tcBinary, int iLength);
		std::string getLastWrittenChars(int iNbChars) const;
		bool writeTextOnce(const std::string& sText);
		ScpStream& flush();
		ScpStream& endl();
		bool close();

		bool equals(const ScpStream& theStream, int& iPosition) const;
		bool equalsFromInputLocations(const ScpStream& theStream, int& iPosition) const;
		bool insertText(const std::string& sText, int iLocation, int iAreaToRecover = 0);
		bool insertTextOnce(const std::string& sText, int iLocation, int iAreaToRecover = 0);
		bool insertStream(const ScpStream& theStream, int iLocation, int iAreaToRecover = 0);
		bool copy(const ScpStream& theStream, int iLocation, int iLength = -1);
		void saveIntoFile(const std::string& sFilename, bool bCreateFileIfUnknown) const;
		void appendFile(const std::string& sFilename, bool bCreateFileIfUnknown) const;

		bool indentAsCpp();

		static const std::list<std::string>& getListOfIncludePaths() { return _listOfIncludePaths; }
		static void setListOfIncludePaths(const std::list<std::string>& listOfIncludePaths) { _listOfIncludePaths = listOfIncludePaths; }

		static bool createVirtualFile(const std::string& sHandle, const std::string& sContent);
		static std::string createVirtualTemporaryFile(const std::string& sContent);
		static bool existVirtualFile(const std::string& sHandle);
		static bool loadVirtualFile(const std::string& sHandle, std::string& sContent);
		static bool appendVirtualFile(const std::string& sHandle, const std::string& sContent);
		static bool deleteVirtualFile(const std::string& sHandle);

		static std::ifstream* openSTLInputFile(const char* sInputFile);
	private:
		void writeText(const char* tcText);
	};


	bool createDirectoriesForFile(const std::string& sFileName);

	std::fstream* openIOFile(const char* sInputFile, bool bCreateDirectories = false);


	std::ifstream* openInputFileFromIncludePath(const char* sFileName, std::string& sCompleteFileName);
	int getLocation(std::istream& stream);
	void setLocation(std::istream& stream, int iCursor);
	void setLocationAtXLines(std::istream& stream, int iCursor);
	int getLineCount(std::istream& stream);
	int getColCount(std::istream& stream);
	bool previousLine(std::istream& stream);
	int getColCount(std::istream& stream);
	bool getColEmptyHeader(std::istream& stream, std::string& sHeader);
	void goBack(std::istream& stream);
	int readChar(std::istream& stream);
	int peekChar(std::istream& stream);
	bool skipBlanks(std::istream& stream);
	bool skipLineBlanks(std::istream& stream);
	bool skipCppComments(std::istream& stream);
	bool skipEmpty(std::istream& stream);
	bool skipEmptyHTML(std::istream& stream);
	bool skipEmptyLaTeX(std::istream& stream);
	bool readWord(std::istream& stream, std::string& sWord);
	bool readIdentifier(std::istream& stream, std::string& sIdentifier);
	bool readLine(std::istream& stream, std::string& sLine);
	bool readInt(std::istream& stream, int& iResult);
	bool readLong(std::istream& stream, long& iResult);
	bool readBoolean(std::istream& stream, bool& bResult);
	bool readDouble(std::istream& stream, double& dResult);
	bool readCharLiteral(std::istream& stream, int& iChar);
	bool readStringOrCharLiteral(std::istream& stream, std::string& sText);
	bool readString(std::istream& stream, std::string& sResult);
	bool readDate(std::istream& stream, tm& myDate);
	bool readFormattedText(std::istream& stream, const std::string& sFormat, std::string& sResult);
	bool readPointer(std::istream& stream, unsigned long& iPointer);
	bool readUptoChar(std::istream& myStream, char cEnd, std::string& sText);
	bool readUptoChar(std::istream& myStream, const std::string& sEnd, std::string& sText);
	bool isEqualTo(std::istream& stream, unsigned char ch);
	bool isEqualTo(std::istream& stream, const char* sText);
	bool isEqualToIdentifier(std::istream& stream, const char* sText);
	bool findString(std::istream& stream, const char* sText);
	std::string getLastReadChars(std::istream& stream, int iNbChars);


	std::ofstream* openOutputFile(const char* sOutputFile, bool bCreateDirectories = false);
	std::ofstream* openAppendFile(const char* sAppendFile);
	void writeChar(std::ostream& stream, char c);
	void writeText(std::ostream& stream, const char* sText);
	void writeComments(std::ostream& stream, const char* sComment);
	void writeBorderedComments(std::ostream& stream, const char* sComment);
	void carriageReturn(std::ostream& stream);
	void writeInt(std::ostream& stream, int iValue);
	void writeBoolean(std::ostream& stream, bool bValue);
	void writeDouble(std::ostream& stream, double dValue);
	void writeString(std::ostream& stream, const char* sText);
	void writeDate(std::ostream& stream, const tm& myDate);
	void writePointer(std::ostream& stream, unsigned long iPointer);

	void copyToStream(std::istream& inputStream, std::ostream& outputStream, int iStart, int iLength = -1);

	bool isPathTerminator_(char c);
	void toSlashes_(char *str);
	std::string canonize_(const char* tcPathname, char* dest1);

}

#endif
