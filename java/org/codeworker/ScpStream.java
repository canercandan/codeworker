package org.codeworker;

class SizeAttributes {
	public int _iSize;
	public int _iReadCursor;
	public int _iWriteCursor;
	public char _cEndChar;
	public SizeAttributes _pShiftedStream;

	public SizeAttributes() {
		_iSize = -1;
		_iReadCursor = -1;
		_iWriteCursor = -1;
		_cEndChar = '\0';
	}

	public SizeAttributes(SizeAttributes copy) {
		_iSize = copy._iSize;
		_iReadCursor = copy._iReadCursor;
		_iWriteCursor = copy._iWriteCursor;
		_cEndChar = copy._cEndChar;
		if ((copy._pShiftedStream != null))
			{
				_pShiftedStream = new SizeAttributes(copy._pShiftedStream);
			}
		else
			_pShiftedStream = null;
	}

	protected void finalize() {
	}
	public boolean empty() {
		return (_iSize < 0);
	}

}

public class ScpStream {
	public static int IN = 1;
	public static int OUT = 2;
	public static int INOUT = 3;
	public static int APPEND = 4;
	public static int PATH = 8;
	public static String ENDL = "\n";
	private char[] _tcStream;
	private int _iCacheMemory;
	private int _iSize;
	private int _iReadCursor;
	private int _iWriteCursor;
	private boolean _bInsertText;
	private String _sIndentation;
	private int _iMode;
	private String _sFilename;
	private org.codeworker.ScpStream _pParentStream;
	private org.codeworker.ScpStream _pShiftedStream;
	private int _iShiftedStreamPosition;
	private org.codeworker.ScpStream _pPrecStream;
	private org.codeworker.ScpStream _pNextStream;
	private int _iLineCounter;
	private int _iLinePosition;
	private java.util.Map<String,java.lang.Integer> _mapOfFloatingLocations;
	private java.util.Set<String> _setOfTextsToInsertOnce;
	private static java.util.List<String> _listOfIncludePaths;
	private static java.util.Map<String,String> _listOfVirtualFiles;
	private org.codeworker.END_STREAM_CALLBACK _pfStreamReaderCallback;
	private java.lang.Object _pStreamReaderCBKData;
	private org.codeworker.END_STREAM_CALLBACK _pfStreamWriterCallback;
	private java.lang.Object _pStreamWriterCBKData;

//##protect##"Java ScpStream Methods"
	protected static void memcpy(char[] dest, char[] org) {
		memcpy(dest, org, org.length);
	}

	protected static void memcpy(char[] dest, char[] org, int iLength) {
		for (int i = 0; i < iLength; i++) {
			dest[i] = org[i];
		}
	}

	protected void copy(char[] tcDest, int iDestStart, char[] tcOrg, int iOrgStart, int iLength) {
		for (int i = 0; i < iLength; i++) {
			tcDest[iDestStart + i] = tcOrg[iOrgStart + i];
		}
	}

	public boolean isEqualTo(char ch) {
		int iChar = readChar();
		if ((iChar == ((int) ch)))
			return true;
		if ((iChar >= 0))
			goBack();
		return false;
	}

	public static boolean createVirtualFile(String sHandle, char[] sContent) {
		return createVirtualFile(sHandle, new String(sContent));
	}

	private void WRITECHAR(byte a) {
		WRITECHAR((char) a);
	}

	private void WRITECHAR(char a) {
		_tcStream[_iWriteCursor++] = a;
		if (_iWriteCursor >= _iCacheMemory) {
			int iOldSize = _iCacheMemory;
			_iCacheMemory *= 2;
			char[] tcStream = new char[_iCacheMemory];
			memcpy(tcStream, _tcStream, iOldSize);
			_tcStream = tcStream;
		}
	}

	private void writeText(String tcText) {
		writeBinaryData(tcText.toCharArray(), tcText.length());
	}
//##protect##"Java ScpStream Methods"

	public ScpStream(int iCacheMemory) {
		_iCacheMemory = iCacheMemory;
		_iSize = 0;
		_iReadCursor = 0;
		_iWriteCursor = 0;
		_bInsertText = false;
		_iMode = 0;
		_iShiftedStreamPosition = 0;
		_iLineCounter = 1;
		_iLinePosition = 0;
		_tcStream = new char[iCacheMemory];
	}

	public ScpStream(String sFilename, int iMode, int iCacheMemory, int iLength) {
		_iCacheMemory = iCacheMemory;
		_iSize = 0;
		_iReadCursor = 0;
		_iWriteCursor = 0;
		_bInsertText = false;
		_iMode = iMode;
		_sFilename = sFilename;
		_iShiftedStreamPosition = 0;
		_iLineCounter = 1;
		_iLinePosition = 0;
//##protect##"Java ScpStream::ScpStream.const string&.int.int.int"
		throw new java.lang.RuntimeException("ScpStream::ScpStream.const string&.int.int.int not implemented yet!");
//##protect##"Java ScpStream::ScpStream.const string&.int.int.int"
	}

	public ScpStream(String sFilename, java.io.InputStream f, int iCacheMemory) {
		_sFilename = sFilename;
		_iCacheMemory = iCacheMemory;
		_iSize = 0;
		_iReadCursor = 0;
		_iWriteCursor = 0;
		_bInsertText = false;
		_iMode = IN;
		_iShiftedStreamPosition = 0;
		_iLineCounter = 1;
		_iLinePosition = 0;
//##protect##"Java ScpStream::ScpStream.const string&.FILE*.int"
		int iFileSize = (int) new java.io.File(sFilename).length();
		if (iFileSize >= _iCacheMemory) _iCacheMemory = iFileSize + 1;
		_tcStream = new char[_iCacheMemory];
		if (iFileSize > 0) {
			byte[] tbFileData = new byte[iFileSize];
			do {
				iFileSize = f.read(tbFileData);
				for (int i = 0; i < iFileSize; ++i) {
					_tcStream[_iSize + i] = (char) tbFileData[i];
				}
				if (iFileSize > 0) _iSize += iFileSize; 
			} while (iFileSize >= 0);
		}
		f.close();
//##protect##"Java ScpStream::ScpStream.const string&.FILE*.int"
	}

	public ScpStream(String sText) {
		_iSize = 0;
		_iReadCursor = 0;
		_iWriteCursor = 0;
		_bInsertText = false;
		_iMode = 0;
		_iShiftedStreamPosition = 0;
		_iLineCounter = 1;
		_iLinePosition = 0;
//##protect##"Java ScpStream::ScpStream.const string&"
		_iSize = sText.length();
		_iCacheMemory = _iSize + 1;
		_tcStream = new char[_iCacheMemory];
		memcpy(_tcStream, sText.toCharArray());
//##protect##"Java ScpStream::ScpStream.const string&"
	}

	public ScpStream(org.codeworker.ScpStream shiftedStream, int iShiftedStreamPosition) {
		_iReadCursor = 0;
		_iWriteCursor = 0;
		_bInsertText = false;
		_iMode = 0;
		_pShiftedStream = shiftedStream;
		_iShiftedStreamPosition = iShiftedStreamPosition;
		_iLineCounter = 1;
		_iLinePosition = 0;
//##protect##"Java ScpStream::ScpStream.ScpStream&.int"
		throw new java.lang.RuntimeException("ScpStream::ScpStream.ScpStream&.int not implemented yet!");
//##protect##"Java ScpStream::ScpStream.ScpStream&.int"
	}

	 ScpStream() {
		if ((_pShiftedStream == null))
			_tcStream = null; // delete the object!
	}

	public static org.codeworker.ScpStream createFile(String sFilename) {
		if (!existVirtualFile(sFilename))
			{
				java.io.InputStream f = new java.io.FileInputStream(sFilename);
				if ((f == null))
					{
						if (createDirectoriesForFile(sFilename))
							f = new java.io.FileInputStream(sFilename);
						if ((f == null))
							return null;
					}
				f.close();
			}
		org.codeworker.ScpStream pStream = new org.codeworker.ScpStream();
		pStream.setFilename(sFilename);
		return pStream;
	}

	public static boolean existInputFileFromIncludePath(String tcFileName, org.codeworker.StringRef sCompleteFileName) {
//##protect##"Java ScpStream::existInputFileFromIncludePath.const char*.string&"
		sCompleteFileName.ref_ = tcFileName;
		if (existVirtualFile(tcFileName))
			return true;
		boolean bFile = existInputFile(sCompleteFileName.ref_);
		if ((!bFile && ((sCompleteFileName.ref_.charAt(0) == '/') || (sCompleteFileName.ref_.charAt(1) == ':'))))
			return false;
		for (String i : org.codeworker.ScpStream._listOfIncludePaths) {
			if (bFile) break;
			sCompleteFileName.ref_ = i + tcFileName;
			bFile = existInputFile(sCompleteFileName.ref_);
		}
		return bFile;
//##protect##"Java ScpStream::existInputFileFromIncludePath.const char*.string&"
	}

	public static boolean existInputFile(String sFileName) {
		if (existVirtualFile(sFileName))
			return true;
		java.io.FileInputStream pFile = openSTLInputFile(sFileName);
		if ((pFile == null))
			return false;
		pFile.close();
		pFile = null; // delete the object!
		return true;
	}

	public static org.codeworker.ScpStream openInputFileFromIncludePath(String tcFileName, org.codeworker.StringRef sCompleteFileName) {
//##protect##"Java ScpStream::openInputFileFromIncludePath.const char*.string&"
		org.codeworker.StringRef sContent = new org.codeworker.StringRef();
		if (loadVirtualFile(tcFileName, sContent))
			{
				org.codeworker.ScpStream pStream = new org.codeworker.ScpStream();
				pStream.setFilename(tcFileName);
				pStream.writeBinaryData(sContent.ref_.toCharArray(), sContent.ref_.length());
				pStream.setOutputLocation(0);
				sCompleteFileName.ref_ = tcFileName;
				return pStream;
			}
		sCompleteFileName.ref_ = tcFileName;
		java.io.FileInputStream pFile = openSTLInputFile(sCompleteFileName.ref_);
		if (((pFile == null) && ((sCompleteFileName.ref_.charAt(0) == '/') || (sCompleteFileName.ref_.charAt(1) == ':'))))
			return null;
		for (String i : org.codeworker.ScpStream._listOfIncludePaths) {
			if (pFile == null) break;
			sCompleteFileName.ref_ = i + tcFileName;
			pFile = openSTLInputFile(sCompleteFileName.ref_);
		}
		if ((pFile == null))
			return null;
		pFile.close();
		pFile = null; // delete the object!
		java.io.InputStream f = new java.io.FileInputStream(sCompleteFileName.ref_);
		if ((f == null))
			return null;
		int iFileSize = (int) new java.io.File(sCompleteFileName.ref_).length();
		return new org.codeworker.ScpStream(sCompleteFileName.ref_, f, iFileSize + 1);
//##protect##"Java ScpStream::openInputFileFromIncludePath.const char*.string&"
	}

	public static org.codeworker.ScpStream openInputFile(String sFileName) {
//##protect##"Java ScpStream::openInputFile.const char*"
		org.codeworker.StringRef sContent = new org.codeworker.StringRef();
		if (loadVirtualFile(sFileName, sContent))
			{
				org.codeworker.ScpStream pStream = new org.codeworker.ScpStream();
				pStream.setFilename(sFileName);
				pStream.writeBinaryData(sContent.ref_.toCharArray(), sContent.ref_.length());
				pStream.setOutputLocation(0);
				return pStream;
			}
		java.io.FileInputStream pFile = openSTLInputFile(sFileName);
		if ((pFile == null))
			return null;
		pFile.close();
		pFile = null; // delete the object!
		java.io.InputStream f = new java.io.FileInputStream(sFileName);
		if ((f == null))
			return null;
		int iFileSize = (int) new java.io.File(sFileName).length();
		return new org.codeworker.ScpStream(sFileName, f, iFileSize + 1);
//##protect##"Java ScpStream::openInputFile.const char*"
	}

	public char[] readBuffer() {
		_tcStream[_iSize] = '\0';
		return _tcStream;
	}

	public void setFilename(String sFilename) {
		_sFilename = sFilename;
	}

	public String getFilename() {
		return _sFilename;
	}

	public int size() {
		return _iSize;
	}

	public boolean empty() {
		return (_iSize == 0);
	}

	public boolean insertMode() {
		return _bInsertText;
	}

	public void insertMode(boolean bInsertMode) {
		_bInsertText = bInsertMode;
	}

	public org.codeworker.ScpStream getParentStream() {
		return _pParentStream;
	}

	public void setParentStream(org.codeworker.ScpStream pStream) {
		_pParentStream = pStream;
	}

	public org.codeworker.END_STREAM_CALLBACK getStreamReaderCallback() {
		return _pfStreamReaderCallback;
	}

	public void setStreamReaderCallback(org.codeworker.END_STREAM_CALLBACK pfCBK, java.lang.Object pData) {
		_pfStreamReaderCallback = pfCBK;
		_pStreamReaderCBKData = pData;
	}

	public org.codeworker.END_STREAM_CALLBACK getStreamWriterCallback() {
		return _pfStreamWriterCallback;
	}

	public void setStreamWriterCallback(org.codeworker.END_STREAM_CALLBACK pfCBK, java.lang.Object pData) {
		_pfStreamWriterCallback = pfCBK;
		_pStreamWriterCBKData = pData;
	}

	public String getIndentation() {
		return _sIndentation;
	}

	public void incrementIndentation(int iLevel) {
		while ((iLevel > 0))
			{
				_sIndentation += "\t";
				--iLevel;
			}
	}

	public boolean decrementIndentation(int iLevel) {
//##protect##"Java ScpStream::decrementIndentation.int"
		if ((_sIndentation.length() < iLevel))
			{
				_sIndentation = new String("");
				return false;
			}
		_sIndentation = _sIndentation.substring(iLevel);
		return true;
//##protect##"Java ScpStream::decrementIndentation.int"
	}

	public String getMessagePrefix(boolean bCountCols) {
//##protect##"Java ScpStream::getMessagePrefix.bool.const"
		String sFilename = _sFilename;
		int iIndex = Math.max(sFilename.lastIndexOf("\\"), sFilename.lastIndexOf("\\"));
		if ((iIndex != -1/*npos*/))
			sFilename = sFilename.substring(iIndex + 1);
		String sMessage;
		if (bCountCols)
			sMessage = sFilename + "(" + getLineCount() + "): ";
		else
			sMessage = sFilename + "(" + getLineCount() + "," + getColCount() + "): ";
		return sMessage;
//##protect##"Java ScpStream::getMessagePrefix.bool.const"
	}

	public int readChar() {
//##protect##"Java ScpStream::readChar"
		if ((_iSize <= _iReadCursor))
		{
			if ((_pNextStream != null))
				return _pNextStream.readChar();
			if ((_pfStreamReaderCallback == null))
				return -1;
			_pfStreamReaderCallback.callback(this, _pStreamReaderCBKData);
			if ((_iSize <= _iReadCursor))
				return -1;
		}
	byte c = (byte) _tcStream[_iReadCursor++];
	return (int) c;
//##protect##"Java ScpStream::readChar"
	}

	public int peekChar() {
//##protect##"Java ScpStream::peekChar"
		if ((_iSize <= _iReadCursor))
		{
			if ((_pNextStream != null))
				return _pNextStream.peekChar();
			return -1;
		}
	byte c = (byte) _tcStream[_iReadCursor];
	return (int) c;
//##protect##"Java ScpStream::peekChar"
	}

	public int getInputLocation() {
		return _iReadCursor;
	}

	public void setInputLocation(int iLocation) {
		if ((iLocation <= _iSize))
			_iReadCursor = iLocation;
	}

	public int getOutputLocation() {
		return _iWriteCursor;
	}

	public void setOutputLocation(int iLocation) {
		if ((iLocation <= _iSize))
			_iWriteCursor = iLocation;
	}

	public SizeAttributes resize(int iNewSize) {
		SizeAttributes sizeAttrs = new SizeAttributes();
		if (((iNewSize >= 0) && (iNewSize < _iSize)))
			{
				sizeAttrs._iSize = _iSize;
				sizeAttrs._iWriteCursor = _iWriteCursor;
				sizeAttrs._iReadCursor = _iReadCursor;
				sizeAttrs._cEndChar = _tcStream[iNewSize];
				_iSize = iNewSize;
				if ((_iWriteCursor > _iSize))
					_iWriteCursor = _iSize;
				if ((_iReadCursor > _iSize))
					_iReadCursor = _iSize;
				if ((_pShiftedStream != null))
					{
						sizeAttrs._pShiftedStream = new SizeAttributes(_pShiftedStream.resize((iNewSize + _iShiftedStreamPosition)));
					}
			}
		return sizeAttrs;
	}

	public void restoreSize(SizeAttributes sizeAttrs) {
		if (!sizeAttrs.empty())
			{
				_tcStream[_iSize] = sizeAttrs._cEndChar;
				_iSize = sizeAttrs._iSize;
				_iWriteCursor = sizeAttrs._iWriteCursor;
				_iReadCursor = sizeAttrs._iReadCursor;
				if (((_pShiftedStream != null) && (sizeAttrs._pShiftedStream != null)))
					{
						_pShiftedStream.restoreSize(sizeAttrs._pShiftedStream);
					}
			}
	}

	public void setLineDirective(int iLine) {
		_iLineCounter = iLine;
		_iLinePosition = _iReadCursor;
	}

	public int getLineCount() {
		int iLine = _iLineCounter;
		int iCursor = _iLinePosition;
		while ((iCursor < _iReadCursor))
			{
				if ((_tcStream[iCursor++] == ((int) '\n')))
					++iLine;
			}
		return iLine;
	}

	public int getOutputLineCount() {
		int iLine = 1;
		int iCursor = 0;
		while ((iCursor < _iWriteCursor))
			{
				if ((_tcStream[iCursor++] == ((int) '\n')))
					++iLine;
			}
		return iLine;
	}

	public int getColCount() {
		int iPosition = _iReadCursor;
		int iCurrent = _iReadCursor;
		while ((iCurrent > 0))
			{
				--iCurrent;
				if ((_tcStream[iCurrent] == ((int) '\n')))
					{
						return (iPosition - iCurrent);
					}
			}
		return (1 + iPosition);
	}

	public int getOutputColCount() {
		int iPosition = _iWriteCursor;
		int iCurrent = _iWriteCursor;
		while ((iCurrent > 0))
			{
				--iCurrent;
				if ((_tcStream[iCurrent] == ((int) '\n')))
					{
						return (iPosition - iCurrent);
					}
			}
		return (1 + iPosition);
	}

	public boolean goBack() {
		if ((_iReadCursor <= 0))
			{
				if ((_pPrecStream != null))
					return _pPrecStream.goBack();
				return false;
			}
		--_iReadCursor;
		return true;
	}

	public boolean skipBlanks() {
		int iChar = readChar();
		while (((iChar >= ((int) '\0')) && (iChar <= ((int) ' '))))
			iChar = readChar();
		if ((iChar < 0))
			return false;
		goBack();
		return true;
	}

	public boolean skipLineBlanks() {
		int iChar = readChar();
		while (((((iChar >= ((int) '\0')) && (iChar <= ((int) ' '))) && (iChar != '\r')) && (iChar != '\n')))
			iChar = readChar();
		if ((iChar < 0))
			return false;
		goBack();
		return true;
	}

	public boolean skipCppComments() {
		int iCursor = getInputLocation();
		int iChar = readChar();
		if ((iChar == ((int) '/')))
			{
				iChar = readChar();
				if ((iChar == ((int) '/')))
					do
						iChar = readChar();
					while (((iChar >= 0) && (iChar != ((int) '\n'))));
				else
					if ((iChar == ((int) '*')))
						{
							iChar = readChar();
							do
								{
									if ((iChar == ((int) '*')))
										{
											iChar = readChar();
											if ((iChar == ((int) '/')))
												break;
										}
									else
										{
											iChar = readChar();
										}
								}
							while ((iChar >= 0));
							if ((iChar < 0))
								{
									setInputLocation(iCursor);
									return false;
								}
						}
					else
						{
							setInputLocation(iCursor);
							return false;
						}
			}
		else
			{
				setInputLocation(iCursor);
				return false;
			}
		return true;
	}

	public boolean skipCppExceptDoxygenComments() {
		int iCursor = getInputLocation();
		int iChar = readChar();
		if ((iChar == ((int) '/')))
			{
				iChar = readChar();
				if ((iChar == ((int) '/')))
					{
						iChar = readChar();
						if ((iChar == ((int) '!')))
							{
								setInputLocation(iCursor);
								return false;
							}
						do
							iChar = readChar();
						while (((iChar >= 0) && (iChar != ((int) '\n'))));
					}
				else
					if ((iChar == ((int) '*')))
						{
							iChar = readChar();
							if ((iChar == ((int) '!')))
								{
									setInputLocation(iCursor);
									return false;
								}
							do
								{
									if ((iChar == ((int) '*')))
										{
											iChar = readChar();
											if ((iChar == ((int) '/')))
												break;
										}
									else
										{
											iChar = readChar();
										}
								}
							while ((iChar >= 0));
							if ((iChar < 0))
								{
									setInputLocation(iCursor);
									return false;
								}
						}
					else
						{
							setInputLocation(iCursor);
							return false;
						}
			}
		else
			{
				setInputLocation(iCursor);
				return false;
			}
		return true;
	}

	public boolean skipEmpty() {
		boolean bSuccess;
		do
			bSuccess = skipBlanks();
		while ((bSuccess && skipCppComments()));
		return bSuccess;
	}

	public boolean skipEmptyCppExceptDoxygen() {
		boolean bSuccess;
		do
			bSuccess = skipBlanks();
		while ((bSuccess && skipCppExceptDoxygenComments()));
		return bSuccess;
	}

	public boolean skipEmptyAda() {
		boolean bSuccess = skipBlanks();
		while ((bSuccess && isEqualTo("--")))
			{
				int iChar;
				do
					iChar = readChar();
				while (((iChar >= 0) && (iChar != ((int) '\n'))));
				if ((iChar < 0))
					return false;
				bSuccess = skipBlanks();
			}
		return bSuccess;
	}

	public boolean skipEmptyHTML() {
		int iChar;
		for (;;)
			{
				do
					iChar = readChar();
				while (((iChar >= ((int) '\0')) && (iChar <= ((int) ' '))));
				if (((iChar == ((int) '<')) && isEqualTo("!--")))
					{
						do
							{
								do
									iChar = readChar();
								while (((iChar >= 0) && (iChar != ((int) '-'))));
							}
						while (((iChar >= 0) && !isEqualTo("->")));
					}
				else
					break;
			}
		if ((iChar < 0))
			return false;
		goBack();
		return true;
	}

	public boolean skipEmptyLaTeX() {
		int iChar = peekChar();
		if ((iChar != ((int) '%')))
			return false;
		do
			{
				++_iReadCursor;
				do
					iChar = readChar();
				while (((iChar >= 0) && (iChar != ((int) '\n'))));
			}
		while (((iChar == '\n') && (peekChar() == '%')));
		return true;
	}

	public boolean skipLine() {
		int iChar;
		int i = 0;
		for (;;)
			{
				iChar = readChar();
				if (((iChar == '\n') || (iChar < 0)))
					break;
				++i;
			}
		if ((iChar >= 0))
			isEqualTo('\r');
		return ((iChar >= 0) || (i > 0));
	}

	public boolean readChars(int iLength, org.codeworker.StringRef sText) {
		if ((_iSize < (_iReadCursor + iLength)))
			{
				_tcStream[_iSize] = '\0';
				sText.ref_ += String.copyValueOf(_tcStream, _iReadCursor, _tcStream.length - _iReadCursor);
				iLength += (_iReadCursor - _iSize);
				_iReadCursor = _iSize;
				if ((_pNextStream != null))
					return _pNextStream.readChars(iLength, sText);
				if ((_pfStreamReaderCallback == null))
					return false;
				int iOldSize = _iSize;
				do
					{
						_pfStreamReaderCallback.callback(this, _pStreamReaderCBKData);
						if ((iOldSize == _iSize))
							return false;
						iOldSize = _iSize;
					}
				while ((_iSize < (_iReadCursor + iLength)));
			}
		char c = _tcStream[(_iReadCursor + iLength)];
		_tcStream[(_iReadCursor + iLength)] = '\0';
		sText.ref_ += String.copyValueOf(_tcStream, _iReadCursor, _tcStream.length - _iReadCursor);
		_iReadCursor += iLength;
		_tcStream[_iReadCursor] = c;
		return true;
	}

	public boolean readWord(org.codeworker.StringRef sWord) {
//##protect##"Java ScpStream::readWord.string&"
		int iIndex = 0;
		int iChar;
		int iPos = _iReadCursor;
		do
			{
				iChar = readChar();
				++iIndex;
			}
		while ((((((((((((((((iChar >= ((int) 'a')) && (iChar <= ((int) 'z'))) || ((iChar >= ((int) 'A')) && (iChar <= ((int) 'Z')))) || ((iChar >= ((int) '0')) && (iChar <= ((int) '9')))) || (iChar == ((int) '_'))) || (iChar == 'é')) || (iChar == 'è')) || (iChar == 'ê')) || (iChar == 'à')) || (iChar == 'î')) || (iChar == 'ù')) || (iChar == 'ô')) || (iChar == 'û')) || (iChar == 'â')) || (iChar == 'ç')));
		if ((iChar >= 0))
			goBack();
		--iIndex;
		if ((iIndex == 0))
			sWord.ref_ = new String("");
		else
			sWord.ref_ = String.copyValueOf(_tcStream, iPos, iIndex);
		return (iIndex > 0);
//##protect##"Java ScpStream::readWord.string&"
	}

	public boolean readIdentifier(org.codeworker.StringRef sIdentifier) {
		byte[] tcText = new byte[1000];
		int iIndex = 0;
		int iChar;
		iChar = readChar();
		if (((((iChar >= ((int) 'a')) && (iChar <= ((int) 'z'))) || ((iChar >= ((int) 'A')) && (iChar <= ((int) 'Z')))) || (iChar == ((int) '_'))))
			{
				tcText[iIndex++] = ((byte) iChar);
				do
					{
						iChar = readChar();
						tcText[iIndex++] = ((byte) iChar);
					}
				while ((((((iChar >= ((int) 'a')) && (iChar <= ((int) 'z'))) || ((iChar >= ((int) 'A')) && (iChar <= ((int) 'Z')))) || ((iChar >= ((int) '0')) && (iChar <= ((int) '9')))) || (iChar == ((int) '_'))));
				--iIndex;
				tcText[iIndex] = '\0';
				sIdentifier.ref_ = new String(tcText);
			}
		if ((iChar >= 0))
			goBack();
		return (iIndex >= 1);
	}

	public boolean readInt(int iResult) {
		byte[] tcReadInt = new byte[32];
		int iIndex = 0;
		int iChar = readChar();
		if ((iChar == ((int) '-')))
			{
				tcReadInt[iIndex++] = '-';
				iChar = readChar();
			}
		if (((iChar < ((int) '0')) || (iChar > ((int) '9'))))
			{
				if ((iChar >= 0))
					goBack();
				if ((iIndex != 0))
					goBack();
				return false;
			}
		tcReadInt[iIndex++] = ((byte) iChar);
		iChar = readChar();
		while (((iChar >= ((int) '0')) && (iChar <= ((int) '9'))))
			{
				tcReadInt[iIndex++] = ((byte) iChar);
				iChar = readChar();
			}
		if ((iChar >= 0))
			setInputLocation((getInputLocation() - 1));
		tcReadInt[iIndex] = '\0';
		iResult = java.lang.Integer.parseInt(new String(new String(tcReadInt)));
		return true;
	}

	public boolean readHexadecimal(int iResult) {
		int iChar = readChar();
		if (((((iChar < ((int) '0')) || (iChar > ((int) '9'))) && ((iChar < ((int) 'A')) || (iChar > ((int) 'F')))) && ((iChar < ((int) 'a')) || (iChar > ((int) 'f')))))
			{
				if ((iChar >= 0))
					goBack();
				return false;
			}
		iResult = 0;
		for (;;)
			{
				int iDigit;
				if (((iChar >= ((int) '0')) && (iChar <= ((int) '9'))))
					iDigit = (iChar - ((int) '0'));
				else
					if (((iChar >= ((int) 'A')) && (iChar <= ((int) 'F'))))
						iDigit = ((10 + iChar) - ((int) 'A'));
					else
						if (((iChar >= ((int) 'a')) && (iChar <= ((int) 'f'))))
							iDigit = ((10 + iChar) - ((int) 'a'));
						else
							break;
				iResult = ((16 * iResult) + iDigit);
				iChar = readChar();
			}
		if ((iChar >= 0))
			goBack();
		return true;
	}

	public boolean readDouble(double dValue) {
		byte[] tcReadDouble = new byte[32];
		int iIndex = 0;
		int iPosition = getInputLocation();
		int iChar = readChar();
		if ((iChar == ((int) '-')))
			{
				tcReadDouble[iIndex++] = '-';
				iChar = readChar();
			}
		else
			if ((iChar == ((int) '+')))
				{
					iChar = readChar();
				}
		if (((iChar < ((int) '0')) || (iChar > ((int) '9'))))
			{
				setInputLocation(iPosition);
				return false;
			}
		tcReadDouble[iIndex++] = ((byte) iChar);
		iChar = readChar();
		while (((iChar >= ((int) '0')) && (iChar <= ((int) '9'))))
			{
				tcReadDouble[iIndex++] = ((byte) iChar);
				iChar = readChar();
			}
		if (((iChar == ((int) '.')) && (peekChar() != ((int) '.'))))
			{
				tcReadDouble[iIndex++] = '.';
				iChar = readChar();
				while (((iChar >= ((int) '0')) && (iChar <= ((int) '9'))))
					{
						tcReadDouble[iIndex++] = ((byte) iChar);
						iChar = readChar();
					}
			}
		if (((iChar == ((int) 'e')) || (iChar == ((int) 'E'))))
			{
				int iCursor = getInputLocation();
				int iMarkedIndex = iIndex;
				tcReadDouble[iIndex++] = 'E';
				iChar = readChar();
				if ((iChar == ((int) '-')))
					{
						tcReadDouble[iIndex++] = ((byte) iChar);
						iChar = readChar();
					}
				else
					if ((iChar == ((int) '+')))
						{
							iChar = readChar();
						}
				if (((iChar < ((int) '0')) || (iChar > ((int) '9'))))
					{
						iIndex = iMarkedIndex;
						setInputLocation(iCursor);
					}
				else
					{
						tcReadDouble[iIndex++] = ((byte) iChar);
						iChar = readChar();
						while (((iChar >= ((int) '0')) && (iChar <= ((int) '9'))))
							{
								tcReadDouble[iIndex++] = ((byte) iChar);
								iChar = readChar();
							}
					}
			}
		if ((iChar >= 0))
			goBack();
		tcReadDouble[iIndex] = '\0';
		dValue = java.lang.Double.parseDouble(new String(new String(tcReadDouble)));
		return true;
	}

	public boolean readStringOrCharLiteral(org.codeworker.StringRef sText) {
//##protect##"Java ScpStream::readStringOrCharLiteral.string&"
		if (readString(sText))
			return true;
		int iChar;
		if (!readCharLiteral(iChar))
			return false;
		sText.ref_ = String.valueOf((char) iChar);
		return true;
//##protect##"Java ScpStream::readStringOrCharLiteral.string&"
	}

	public boolean readCharLiteral(int iChar) {
		int iPosition = getInputLocation();
		if (!isEqualTo('\''))
			return false;
		iChar = readChar();
		if ((iChar == '\\'))
			{
				iChar = readChar();
				switch(iChar) {
					case 'a':
						iChar = '\007';
						break;
					case 'b':
						iChar = '\b';
						break;
					case 'f':
						iChar = '\f';
						break;
					case 'n':
						iChar = '\n';
						break;
					case 'r':
						iChar = '\r';
						break;
					case 't':
						iChar = '\t';
						break;
					case 'v':
						iChar = '\013';
						break;
					case 'u':
						readHexadecimal(iChar);
						break;
					case '0':
						{
							iChar = 0;
							int iCurrentChar = readChar();
							if ((iCurrentChar == ((int) 'x')))
								{
									do
										{
											iCurrentChar = readChar();
											if (((iCurrentChar >= '0') && (iCurrentChar <= '9')))
												{
													iChar *= 16;
													iChar += (iCurrentChar - ((int) '0'));
												}
											else
												if (((iCurrentChar >= 'a') && (iCurrentChar <= 'f')))
													{
														iChar *= 16;
														iChar += ((iCurrentChar - ((int) 'a')) + 10);
													}
												else
													if (((iCurrentChar >= 'A') && (iCurrentChar <= 'F')))
														{
															iChar *= 16;
															iChar += ((iCurrentChar - ((int) 'A')) + 10);
														}
													else
														break;
										}
									while (true);
								}
							else
								if (((iCurrentChar >= ((int) '0')) && (iCurrentChar <= ((int) '7'))))
									{
										iChar = (iCurrentChar - ((int) '0'));
										do
											{
												iCurrentChar = readChar();
												if (((iCurrentChar >= '0') && (iCurrentChar <= '7')))
													{
														iChar *= 8;
														iChar += (iCurrentChar - ((int) '0'));
													}
												else
													break;
											}
										while (true);
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
							do
								{
									iCurrentChar = readChar();
									if (((iCurrentChar >= '0') && (iCurrentChar <= '7')))
										{
											iChar *= 8;
											iChar += (iCurrentChar - ((int) '0'));
										}
									else
										break;
								}
							while (true);
						}
						break;
				}
			}
		if (!isEqualTo('\''))
			{
				setInputLocation(iPosition);
				return false;
			}
		return true;
	}

	public boolean readString(org.codeworker.StringRef sText) {
		byte[] tcReadString = new byte[32000];
		int iIndex = 0;
		int iLocation = _iReadCursor;
		int iChar = readChar();
		if ((iChar != ((int) '\"')))
			{
				if ((iChar >= 0))
					goBack();
				return false;
			}
		iChar = readChar();
		while (((iChar >= 0) && (iChar != ((int) '\"'))))
			{
				if ((iChar == '\\'))
					{
						iChar = readChar();
						switch(iChar) {
							case 'a':
								iChar = '\007';
								break;
							case 'b':
								iChar = '\b';
								break;
							case 'f':
								iChar = '\f';
								break;
							case 'n':
								iChar = '\n';
								break;
							case 'r':
								iChar = '\r';
								break;
							case 't':
								iChar = '\t';
								break;
							case 'v':
								iChar = '\013';
								break;
							case 'u':
								if (!readHexadecimal(iChar))
									{
										_iReadCursor = iLocation;
										return false;
									}
								break;
							case '0':
								{
									iChar = 0;
									int iCurrentChar = readChar();
									if ((iCurrentChar == ((int) 'x')))
										{
											do
												{
													iCurrentChar = readChar();
													if (((iCurrentChar >= '0') && (iCurrentChar <= '9')))
														{
															iChar *= 16;
															iChar += (iCurrentChar - ((int) '0'));
														}
													else
														if (((iCurrentChar >= 'a') && (iCurrentChar <= 'f')))
															{
																iChar *= 16;
																iChar += ((iCurrentChar - ((int) 'a')) + 10);
															}
														else
															if (((iCurrentChar >= 'A') && (iCurrentChar <= 'F')))
																{
																	iChar *= 16;
																	iChar += ((iCurrentChar - ((int) 'A')) + 10);
																}
															else
																break;
												}
											while (true);
										}
									else
										if (((iCurrentChar >= ((int) '0')) && (iCurrentChar <= ((int) '7'))))
											{
												iChar = (iCurrentChar - ((int) '0'));
												do
													{
														iCurrentChar = readChar();
														if (((iCurrentChar >= '0') && (iCurrentChar <= '7')))
															{
																iChar *= 8;
																iChar += (iCurrentChar - ((int) '0'));
															}
														else
															break;
													}
												while (true);
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
									do
										{
											iCurrentChar = readChar();
											if (((iCurrentChar >= '0') && (iCurrentChar <= '7')))
												{
													iChar *= 8;
													iChar += (iCurrentChar - ((int) '0'));
												}
											else
												break;
										}
									while (true);
								}
								break;
						}
					}
				else
					if ((iChar == ((int) '\n')))
						{
							break;
						}
				tcReadString[iIndex++] = ((byte) iChar);
				iChar = readChar();
			}
		if ((iChar != ((int) '\"')))
			{
				_iReadCursor = iLocation;
				return false;
			}
		tcReadString[iIndex] = '\0';
		sText.ref_ = new String(tcReadString);
		return true;
	}

	public boolean readAdaString(org.codeworker.StringRef sText) {
		byte[] tcReadAdaString = new byte[32000];
		int iIndex = 0;
		int iLocation = _iReadCursor;
		int iChar = readChar();
		if ((iChar != ((int) '\"')))
			{
				if ((iChar >= 0))
					goBack();
				return false;
			}
		iChar = readChar();
		while ((iChar >= 0))
			{
				if ((iChar == '\"'))
					{
						if ((peekChar() != '\"'))
							break;
						iChar = readChar();
					}
				tcReadAdaString[iIndex++] = ((byte) iChar);
				iChar = readChar();
			}
		if ((iChar != ((int) '\"')))
			{
				_iReadCursor = iLocation;
				return false;
			}
		tcReadAdaString[iIndex] = '\0';
		sText.ref_ = new String(tcReadAdaString);
		return true;
	}

	public boolean readLine(org.codeworker.StringRef sLine) {
		char[] tcReadLine = new char[10000];
		int i = 0;
		int iChar;
		do
			{
				iChar = readChar();
				if (((iChar == '\n') || (iChar < 0)))
					break;
				tcReadLine[i++] = ((char) iChar);
			}
		while (true);
		if ((iChar >= 0))
			isEqualTo('\r');
		if (((i > 0) && (tcReadLine[(i - 1)] == '\r')))
			--i;
		tcReadLine[i] = '\0';
		sLine.ref_ = new String(tcReadLine);
		return ((iChar >= 0) || (i > 0));
	}

	public boolean readLastChars(int iLength, org.codeworker.StringRef sLastChars) {
		if ((iLength < 0))
			return false;
		char a = (((_iSize > _iReadCursor)) ? _tcStream[_iReadCursor] : '\0');
		_tcStream[_iReadCursor] = '\0';
		if (((_iReadCursor - iLength) > 0))
			sLastChars.ref_ = String.copyValueOf(_tcStream, (_iReadCursor - iLength), _tcStream.length - (_iReadCursor - iLength));
		else
			sLastChars.ref_ = new String(_tcStream);
		_tcStream[_iReadCursor] = a;
		return true;
	}

	public boolean readUptoChar(char cEnd, org.codeworker.StringRef sText) {
		int iLocation = getInputLocation();
		String sLocalText = new String();
		int iChar = readChar();
		while (((iChar > 0) && (iChar != ((int) cEnd))))
			{
				sLocalText += ((char) iChar);
				iChar = readChar();
			}
		if ((iChar < 0))
			{
				setInputLocation(iLocation);
				return false;
			}
		sText.ref_ = sLocalText;
		goBack();
		return true;
	}

	public boolean readUptoChar(String sEnd, org.codeworker.StringRef sText) {
//##protect##"Java ScpStream::readUptoChar.const string&.string&"
		int iLocation = getInputLocation();
		String sLocalText = new String();
		int iChar = readChar();
		while ((iChar > 0) && !sEnd.contains(String.valueOf((char) iChar)))
			{
				sLocalText += ((char) iChar);
				iChar = readChar();
			}
		if ((iChar < 0))
			{
				setInputLocation(iLocation);
				return false;
			}
		sText.ref_ = sLocalText;
		goBack();
		return true;
//##protect##"Java ScpStream::readUptoChar.const string&.string&"
	}

	public boolean isEqualTo(byte ch) {
		int iChar = readChar();
		if ((iChar == ((int) ch)))
			return true;
		if ((iChar >= 0))
			goBack();
		return false;
	}

	public boolean isEqualTo(char[] sText) {
		int iChar;
		int iPosition = getInputLocation();
		for (int i = 0;(sText[i] != '\0');++i)
			{
				iChar = readChar();
				if (((iChar < 0) || (((char) iChar) != sText[i])))
					{
						setInputLocation(iPosition);
						return false;
					}
			}
		return true;
	}

	public boolean isEqualTo(String sText) {
		return isEqualTo(sText.toCharArray());
	}

	public boolean isEqualToIgnoreCase(byte ch) {
		int iChar = readChar();
		if (((ch >= 'A') && (ch <= 'Z')))
			ch += ' ';
		if (((iChar >= 'A') && (iChar <= 'Z')))
			iChar += 32;
		if ((iChar == ((int) ch)))
			return true;
		if ((iChar >= 0))
			goBack();
		return false;
	}

	public boolean isEqualToIgnoreCase(char[] sText) {
		int iChar;
		int iPosition = getInputLocation();
		for (int i = 0;(sText[i] != '\0');++i)
			{
				iChar = readChar();
				if (((iChar >= 'A') && (iChar <= 'Z')))
					iChar += 32;
				char a = sText[i];
				if (((a >= 'A') && (a <= 'Z')))
					a += ' ';
				if (((iChar < 0) || (((char) iChar) != a)))
					{
						setInputLocation(iPosition);
						return false;
					}
			}
		return true;
	}

	public boolean isEqualToIgnoreCase(String sText) {
		return isEqualToIgnoreCase(sText.toCharArray());
	}

	public boolean isEqualToIdentifier(String sText) {
		int iPosition = getInputLocation();
		org.codeworker.StringRef sIdentifier = new org.codeworker.StringRef();
		if (!readIdentifier(sIdentifier))
			return false;
		if (!sIdentifier.ref_.equals(sText))
			{
				setInputLocation(iPosition);
				return false;
			}
		return true;
	}

	public boolean findString(char[] sText) {
		int iChar = -1;
		int iPosition = getInputLocation();
		int iCurrent = iPosition;
		int i;
		do
			{
				setInputLocation(iCurrent);
				for ( i = 0;(sText[i] != '\0');++i)
					{
						iChar = readChar();
						if ((iChar < 0))
							break;
						if ((((char) iChar) != sText[i]))
							break;
					}
				++iCurrent;
			}
		while (((iChar >= 0) && (sText[i] != '\0')));
		if ((sText[i] == '\0'))
			return true;
		setInputLocation(iPosition);
		return false;
	}

	public boolean findString(char[] sText, int iBoundary) {
		int iChar = -1;
		int iPosition = getInputLocation();
		int iCurrent = iPosition;
		int i;
		do
			{
				setInputLocation(iCurrent);
				for ( i = 0;(sText[i] != '\0');++i)
					{
						iChar = readChar();
						if ((iChar < 0))
							break;
						if ((((char) iChar) != sText[i]))
							break;
					}
				++iCurrent;
			}
		while ((((iChar >= 0) && (sText[i] != '\0')) && (iCurrent < iBoundary)));
		if ((sText[i] == '\0'))
			return true;
		setInputLocation(iPosition);
		return false;
	}

	public boolean findString(String sText) {
		return findString(sText.toCharArray());
	}

	public boolean findString(String sText, int iBoundary) {
		return findString(sText.toCharArray(), iBoundary);
	}

	public java.util.Map<String,java.lang.Integer> allFloatingLocations() {
		return _mapOfFloatingLocations;
	}

	public boolean newFloatingLocation(String sKey) {
		_mapOfFloatingLocations.put(sKey, getOutputLocation());
		return true;
	}

	public void setFloatingLocation(String sKey, int iLocation) {
		_mapOfFloatingLocations.put(sKey, iLocation);
	}

	public int getFloatingLocation(String sKey, org.codeworker.ScpStream pOwner) {
		java.util.MapIterator<String,java.lang.Integer> cursor = _mapOfFloatingLocations.find(sKey);
		if ((cursor == _mapOfFloatingLocations.end()))
			{
				if ((_pParentStream != null))
					return _pParentStream.getFloatingLocation(sKey, pOwner);
				pOwner = null;
				return -1;
			}
		pOwner = this;
		return cursor.second;
	}

	public int removeFloatingLocation(String sKey, org.codeworker.ScpStream pOwner) {
		java.util.MapIterator<String,java.lang.Integer> cursor = _mapOfFloatingLocations.find(sKey);
		if ((cursor == _mapOfFloatingLocations.end()))
			{
				if ((_pParentStream != null))
					return _pParentStream.removeFloatingLocation(sKey, pOwner);
				pOwner = null;
				return -1;
			}
		pOwner = this;
		int iPosition = cursor.second;
		_mapOfFloatingLocations.erase(cursor);
		return iPosition;
	}

	public org.codeworker.ScpStream writeOperator(char cValue) {
		WRITECHAR(cValue);
		if ((_iWriteCursor > _iSize))
			_iSize = _iWriteCursor;
		return this;
	}

	public org.codeworker.ScpStream writeOperator(byte cValue) {
		WRITECHAR(cValue);
		if ((_iWriteCursor > _iSize))
			_iSize = _iWriteCursor;
		return this;
	}

	public org.codeworker.ScpStream writeOperator(int iValue) {
//##protect##"Java ScpStream::operator<<.double"
		writeText(String.valueOf(iValue));
		return this;
//##protect##"Java ScpStream::operator<<.double"
	}

	public org.codeworker.ScpStream writeOperator(long lValue) {
//##protect##"Java ScpStream::operator<<.double"
		writeText(String.valueOf(lValue));
		return this;
//##protect##"Java ScpStream::operator<<.double"
	}

	public org.codeworker.ScpStream writeOperator(/*unsigned*/ long lValue) {
		char[] tcNumber = new char[32];
		sprintf(tcNumber, "%lu", lValue);
		writeText(tcNumber);
		return this;
	}

	public org.codeworker.ScpStream writeOperator(double dValue) {
//##protect##"Java ScpStream::operator<<.double"
		writeText(String.valueOf(dValue));
		return this;
//##protect##"Java ScpStream::operator<<.double"
	}

	public org.codeworker.ScpStream writeOperator(char[] tcValue) {
		if ((tcValue == null))
			writeText("(null)");
		else
			writeText(tcValue);
		return this;
	}

	public org.codeworker.ScpStream writeOperator(String sValue) {
		writeBinaryData(sValue.toCharArray(), sValue.length());
		return this;
	}

	public org.codeworker.ScpStream writeOperator(org.codeworker.ScpStream theStream) {
//##protect##"Java ScpStream::operator<<.const ScpStream&"
		for (java.util.Map.Entry<String, Integer> i : theStream._mapOfFloatingLocations.entrySet()) {
				i.setValue(new Integer(i.getValue().intValue() + _iWriteCursor));
			}
		theStream._mapOfFloatingLocations.clear();
		theStream._pParentStream = this;
		writeText(theStream.readBuffer());
		return this;
//##protect##"Java ScpStream::operator<<.const ScpStream&"
	}

	public void writeString(String sString) {
		int iRequiredSpace = (sString.length() + 2);
		char[] tcEscape = sString.toCharArray();
		while ((tcEscape != '\0'))
			{
				switch(tcEscape) {
					case '\\':
					case '\"':
					case '\007':
					case '\b':
					case '\f':
					case '\n':
					case '\r':
					case '\t':
					case '\013':
						++iRequiredSpace;
				}
				++tcEscape;
			}
		int iNeededSize = (_iWriteCursor + iRequiredSpace);
		int iTotalSize = ((_bInsertText) ? (_iSize + iRequiredSpace) : iNeededSize);
		if ((_iSize < iTotalSize))
			{
				_iSize = iTotalSize;
				if ((iTotalSize >= _iCacheMemory))
					{
						int iOldSize = _iCacheMemory;
						do
							_iCacheMemory *= 2;
						while ((_iSize >= _iCacheMemory));
						char[] tcStream = new char[_iCacheMemory];
						memcpy(tcStream, _tcStream, iOldSize);
						_tcStream = null; // delete the object!
						_tcStream = new String(tcStream);
					}
			}
		if (_bInsertText)
			{
				memmove(String.copyValueOf(_tcStream, iNeededSize, _tcStream.length - iNeededSize), String.copyValueOf(_tcStream, _iWriteCursor, _tcStream.length - _iWriteCursor), (_iSize - iNeededSize));
				for (java.util.MapIterator<String,java.lang.Integer> i = _mapOfFloatingLocations.begin();(i != _mapOfFloatingLocations.end());++i)
					{
						if ((i.second >= _iWriteCursor))
							_mapOfFloatingLocations.put(i.first, += iRequiredSpace;
					}
			}
		char[] u = sString.toCharArray();
		_tcStream[_iWriteCursor++] = '\"';
		if ((iRequiredSpace == (sString.length() + 2)))
			{
				memcpy(String.copyValueOf(_tcStream, _iWriteCursor, _tcStream.length - _iWriteCursor), sString.toCharArray(), sString.length());
				_iWriteCursor += sString.length();
			}
		else
			{
				while ((u != '\0'))
					{
						switch(u) {
							case '\\':
								_tcStream[_iWriteCursor++] = '\\';
								_tcStream[_iWriteCursor++] = '\\';
								break;
							case '\"':
								_tcStream[_iWriteCursor++] = '\\';
								_tcStream[_iWriteCursor++] = '\"';
								break;
							case '\007':
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
							case '\013':
								_tcStream[_iWriteCursor++] = '\\';
								_tcStream[_iWriteCursor++] = 'v';
								break;
							default:
								_tcStream[_iWriteCursor++] = u;
						}
						++u;
					}
			}
		_tcStream[_iWriteCursor++] = '\"';
	}

	public void writeQuotedChar(char c) {
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
			case '\007':
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
			case '\013':
				WRITECHAR('\\');
				WRITECHAR('v');
				break;
			default:
				WRITECHAR(c);
		}
		WRITECHAR('\'');
		if ((_iWriteCursor > _iSize))
			_iSize = _iWriteCursor;
	}

	public void writeBinaryData(char[] tcBinary, int iLength) {
		int iFinalLength = iLength;
		boolean bStartIndents = false;
		if (!_sIndentation.equals(""))
			{
				int iStartCursor = (_iWriteCursor - 1);
				while ((iStartCursor >= 0))
					{
						if ((_tcStream[iStartCursor] != '\t'))
							break;
						--iStartCursor;
					}
				if (((iStartCursor < 0) || (_tcStream[iStartCursor] == '\n')))
					{
						int iStartIndents = ((_iWriteCursor - iStartCursor) - 1);
						int iExtraStartingTabs = (_sIndentation.length() - iStartIndents);
						if ((iExtraStartingTabs > 0))
							{
								bStartIndents = true;
								_iWriteCursor -= iStartIndents;
								iFinalLength += iExtraStartingTabs;
							}
					}
				char[] u = tcBinary;
				for (int i = (iLength - 1);(i > 0);--i)
					{
						if ((u++ == '\n'))
							iFinalLength += _sIndentation.length();
					}
			}
		int iNeededSize = (_iWriteCursor + iFinalLength);
		int iTotalSize = ((_bInsertText) ? (_iSize + iFinalLength) : iNeededSize);
		if ((_iSize < iTotalSize))
			{
				_iSize = iTotalSize;
				if ((iTotalSize >= _iCacheMemory))
					{
						int iOldSize = _iCacheMemory;
						do
							_iCacheMemory *= 2;
						while ((_iSize >= _iCacheMemory));
						char[] tcStream = new char[_iCacheMemory];
						memcpy(tcStream, _tcStream, iOldSize);
						_tcStream = null; // delete the object!
						_tcStream = new String(tcStream);
					}
			}
		if (_bInsertText)
			{
				memmove(String.copyValueOf(_tcStream, iNeededSize, _tcStream.length - iNeededSize), String.copyValueOf(_tcStream, _iWriteCursor, _tcStream.length - _iWriteCursor), (_iSize - iNeededSize));
				for (java.util.MapIterator<String,java.lang.Integer> i = _mapOfFloatingLocations.begin();(i != _mapOfFloatingLocations.end());++i)
					{
						if ((i.second >= _iWriteCursor))
							_mapOfFloatingLocations.put(i.first, += iFinalLength;
					}
			}
		if (_sIndentation.equals(""))
			{
				memcpy(String.copyValueOf(_tcStream, _iWriteCursor, _tcStream.length - _iWriteCursor), tcBinary, iLength);
			}
		else
			{
				char[] tcIndentStart = String.copyValueOf(_tcStream, _iWriteCursor, _tcStream.length - _iWriteCursor);
				if (bStartIndents)
					{
						char[] v = _sIndentation.toCharArray();
						do
							{
								++tcIndentStart = v++;
							}
						while ((v != '\0'));
					}
				char[] u = tcBinary;
				for (int i = (iLength - 1);(i > 0);--i)
					{
						++tcIndentStart = u;
						if ((u++ == '\n'))
							{
								char[] v = _sIndentation.toCharArray();
								do
									{
										++tcIndentStart = v++;
									}
								while ((v != '\0'));
							}
					}
				tcIndentStart = u;
			}
		_iWriteCursor += iFinalLength;
	}

	public String getLastWrittenChars(int iNbChars) {
//##protect##"Java ScpStream::writeTextOnce.const string&"
		if (iNbChars > _iWriteCursor)
			iNbChars = _iWriteCursor;
		if ((iNbChars == 0))
			return "";
		int iBegin = _iWriteCursor - iNbChars;
		return new String(_tcStream, iBegin, iNbChars);
//##protect##"Java ScpStream::writeTextOnce.const string&"
	}

	public boolean writeTextOnce(String sText) {
//##protect##"Java ScpStream::writeTextOnce.const string&"
		if (_setOfTextsToInsertOnce.contains(sText))
			{
				writeBinaryData(sText.toCharArray(), sText.length());
				_setOfTextsToInsertOnce.add(sText);
				return true;
			}
		return false;
//##protect##"Java ScpStream::writeTextOnce.const string&"
	}

	public org.codeworker.ScpStream flush() {
		return this;
	}

	public org.codeworker.ScpStream endl() {
		writeText(ENDL.toCharArray());
		return this;
	}

	public boolean close() {
		if ((_pfStreamWriterCallback == null))
			return true;
		return _pfStreamWriterCallback.callback(this, _pStreamWriterCBKData);
	}

	public boolean equals(org.codeworker.ScpStream theStream, int iPosition) {
		if ((_iSize != theStream._iSize))
			{
				iPosition = -1;
				return false;
			}
		int i = _iSize;
		char[] u = _tcStream;
		char[] v = theStream._tcStream;
		while ((i > 0))
			{
				if ((u++ != v++))
					{
						iPosition = (_iSize - i);
						return false;
					}
				--i;
			}
		iPosition = _iSize;
		return true;
	}

	public boolean equalsFromInputLocations(org.codeworker.ScpStream theStream, int iPosition) {
		if (((_iSize - _iReadCursor) != (theStream._iSize - theStream._iReadCursor)))
			{
				iPosition = -1;
				return false;
			}
		int i = (_iSize - _iReadCursor);
		char[] u = String.copyValueOf(_tcStream, _iReadCursor, _tcStream.length - _iReadCursor);
		char[] v = String.copyValueOf(theStream._tcStream, theStream._iReadCursor, theStream._tcStream.length - theStream._iReadCursor);
		while ((i > 0))
			{
				if ((u++ != v++))
					{
						iPosition = (_iSize - i);
						return false;
					}
				--i;
			}
		iPosition = _iSize;
		return true;
	}

	public boolean insertText(String sText, int iLocation, int iAreaToRecover) {
		if ((iLocation < 0))
			return false;
		int iEndPortion = (iLocation + iAreaToRecover);
		if ((iEndPortion > _iSize))
			{
				iAreaToRecover = (_iSize - iLocation);
				if ((iAreaToRecover < 0))
					return false;
				iEndPortion = (iLocation + iAreaToRecover);
			}
		int iBytesToAdd = (sText.length() - iAreaToRecover);
		if (((_iSize + iBytesToAdd) >= _iCacheMemory))
			{
				int iOldSize = _iCacheMemory;
				do
					_iCacheMemory *= 2;
				while (((_iSize + iBytesToAdd) >= _iCacheMemory));
				char[] tcStream = new char[_iCacheMemory];
				memcpy(tcStream, _tcStream, iOldSize);
				_tcStream = null; // delete the object!
				_tcStream = new String(tcStream);
			}
		if ((iBytesToAdd != 0))
			{
				memmove(String.copyValueOf(_tcStream, (iLocation + sText.length()), _tcStream.length - (iLocation + sText.length())), String.copyValueOf(_tcStream, iEndPortion, _tcStream.length - iEndPortion), (_iSize - iEndPortion));
				_iSize += iBytesToAdd;
				if ((iEndPortion < _iWriteCursor))
					_iWriteCursor += iBytesToAdd;
				for (java.util.MapIterator<String,java.lang.Integer> i = _mapOfFloatingLocations.begin();(i != _mapOfFloatingLocations.end());++i)
					{
						if ((i.second >= iEndPortion))
							_mapOfFloatingLocations.put(i.first, += iBytesToAdd;
					}
			}
		memcpy(String.copyValueOf(_tcStream, iLocation, _tcStream.length - iLocation), sText.toCharArray(), sText.length());
		return true;
	}

	public boolean insertTextOnce(String sText, int iLocation, int iAreaToRecover) {
//##protect##"Java ScpStream::insertTextOnce.const string&.int.int"
		if (_setOfTextsToInsertOnce.contains(sText))
			return false;
		if (!insertText(sText, iLocation, iAreaToRecover))
			return false;
		_setOfTextsToInsertOnce.add(sText);
		return true;
//##protect##"Java ScpStream::insertTextOnce.const string&.int.int"
	}

	public boolean insertStream(org.codeworker.ScpStream theStream, int iLocation, int iAreaToRecover) {
		if ((iLocation < 0))
			return false;
		int iEndPortion = (iLocation + iAreaToRecover);
		if ((iEndPortion > _iSize))
			{
				iAreaToRecover = (_iSize - iLocation);
				if ((iAreaToRecover < 0))
					return false;
				iEndPortion = (iLocation + iAreaToRecover);
			}
		int iBytesToAdd = (theStream.size() - iAreaToRecover);
		if (((_iSize + iBytesToAdd) >= _iCacheMemory))
			{
				int iOldSize = _iCacheMemory;
				do
					_iCacheMemory *= 2;
				while (((_iSize + iBytesToAdd) >= _iCacheMemory));
				char[] tcStream = new char[_iCacheMemory];
				memcpy(tcStream, _tcStream, iOldSize);
				_tcStream = null; // delete the object!
				_tcStream = new String(tcStream);
			}
		if ((iBytesToAdd != 0))
			{
				memmove(String.copyValueOf(_tcStream, (iLocation + theStream.size()), _tcStream.length - (iLocation + theStream.size())), String.copyValueOf(_tcStream, iEndPortion, _tcStream.length - iEndPortion), (_iSize - iEndPortion));
				_iSize += iBytesToAdd;
				if ((iEndPortion < _iWriteCursor))
					_iWriteCursor += iBytesToAdd;
				for (java.util.MapIterator<String,java.lang.Integer> i = _mapOfFloatingLocations.begin();(i != _mapOfFloatingLocations.end());++i)
					{
						if ((i.second >= iEndPortion))
							_mapOfFloatingLocations.put(i.first, += iBytesToAdd;
					}
			}
		memcpy(String.copyValueOf(_tcStream, iLocation, _tcStream.length - iLocation), theStream.readBuffer(), theStream.size());
		return true;
	}

	public boolean copy(org.codeworker.ScpStream theStream, int iLocation, int iLength) {
//##protect##"Java ScpStream::copy.const ScpStream&.int.int"
		if ((iLength < 0))
			iLength = theStream.size() - iLocation;
		else
			if (iLength + iLocation > theStream.size())
				return false;
		if (_iWriteCursor + iLength >= _iCacheMemory)
			{
				int iOldSize = _iCacheMemory;
				do
					_iCacheMemory *= 2;
				while (_iWriteCursor + iLength >= _iCacheMemory);
				char[] tcStream = new char[_iCacheMemory];
				memcpy(tcStream, _tcStream, iOldSize);
				_tcStream = tcStream;
			}
		copy(_tcStream, _iWriteCursor, theStream._tcStream, iLocation, iLength);
		_iWriteCursor += iLength;
		if ((_iWriteCursor > _iSize))
			_iSize = _iWriteCursor;
		return true;
//##protect##"Java ScpStream::copy.const ScpStream&.int.int"
	}

	public void saveIntoFile(String sFilename, boolean bCreateFileIfUnknown) {
		if (existVirtualFile(sFilename))
			{
				createVirtualFile(sFilename, readBuffer());
			}
		else
			{
				java.io.InputStream f = new java.io.FileInputStream(sFilename);
				if ((f == null))
					{
						if (bCreateFileIfUnknown)
							{
								if (createDirectoriesForFile(sFilename))
									f = new java.io.FileInputStream(sFilename);
							}
						if ((f == null))
							throw new org.codeworker.Exception((("unable to copy a stream into file '" + sFilename) + "'"));
					}
				int iCursor = 0;
				int iBlock = (((_iSize >= 32000)) ? 32000 : _iSize);
				do
					{
						int iWritten = fwrite(String.copyValueOf(_tcStream, iCursor, _tcStream.length - iCursor), 1, iBlock, f);
						if ((iWritten != iBlock))
							{
								char[] tcNumber = new char[32];
								sprintf(tcNumber, "%d", ferror(f));
								throw new org.codeworker.Exception((((("error [" + tcNumber) + "] has occurred while saving bytes into file \"") + sFilename) + "\""));
							}
						iCursor += iBlock;
						iBlock = ((((iCursor + 32000) <= _iSize)) ? 32000 : (_iSize - iCursor));
					}
				while ((iBlock != 0));
				f.close();
			}
	}

	public void appendFile(String sFilename, boolean bCreateFileIfUnknown) {
		if (existVirtualFile(sFilename))
			{
				org.codeworker.StringRef sContent = new org.codeworker.StringRef();
				loadVirtualFile(sFilename, sContent.ref_);
				createVirtualFile(sFilename, (sContent.ref_ + readBuffer()));
			}
		else
			{
				java.io.InputStream f = new java.io.FileInputStream(sFilename);
				if ((f == null))
					{
						if (bCreateFileIfUnknown)
							{
								if (createDirectoriesForFile(sFilename))
									f = new java.io.FileInputStream(sFilename);
							}
						if ((f == null))
							throw new org.codeworker.Exception((("unable to append a stream into file '" + sFilename) + "'"));
					}
				int iCursor = 0;
				int iBlock = (((_iSize >= 32000)) ? 32000 : _iSize);
				do
					{
						int iWritten = fwrite(String.copyValueOf(_tcStream, iCursor, _tcStream.length - iCursor), 1, iBlock, f);
						if ((iWritten != iBlock))
							{
								char[] tcNumber = new char[32];
								sprintf(tcNumber, "%d", ferror(f));
								throw new org.codeworker.Exception((((("error [" + tcNumber) + "] has occurred while appending bytes into file \"") + sFilename) + "\""));
							}
						iCursor += iBlock;
						iBlock = ((((iCursor + 32000) <= _iSize)) ? 32000 : (_iSize - iCursor));
					}
				while ((iBlock != 0));
				f.close();
			}
	}

	public boolean indentAsCpp() {
		org.codeworker.ScpStream currentStream = new org.codeworker.ScpStream();
		String sIndentation = new String();
		String sSequence = new String();
		org.codeworker.StringRef sLastIdentifier = new org.codeworker.StringRef();
		int iChar = readChar();
		while ((iChar >= 0))
			{
				int iStartLocation = (_iReadCursor - 1);
				while (((iChar == '\t') || (iChar == ((int) ' '))))
					iChar = readChar();
				if ((iChar < 0))
					break;
				if ((iChar == ((int) '\r')))
					currentStream.writeOperator('\r');
				else
					if ((iChar == ((int) '\n')))
						currentStream.writeOperator('\n');
					else
						if ((iChar == ((int) '#')))
							{
								do
									{
										currentStream.writeOperator(((byte) iChar));
										iChar = readChar();
									}
								while (((iChar >= 0) && (iChar != '\n')));
								if ((iChar >= 0))
									currentStream.writeOperator('\n');
							}
						else
							if (((iChar == '/') && isEqualTo("/##")))
								{
									_iReadCursor = iStartLocation;
									iChar = readChar();
									while (((iChar >= 0) && (iChar != '\n')))
										{
											currentStream.writeOperator(((byte) iChar));
											iChar = readChar();
										}
									if ((iChar >= 0))
										currentStream.writeOperator('\n');
								}
							else
								{
									String sLocalIndentation = sIndentation;
									String sLine = new String();
									int iPreviousChar = -1;
									int iPreviousNotEmptyChar = -1;
									int iBeginningOfLine = iStartLocation;
									do
										{
											switch(iChar) {
												case ':':
													if ((((iPreviousChar != ((int) ':')) && (peekChar() != ':')) && (iPreviousNotEmptyChar != ((int) ')'))))
														{
															if ((((sLastIdentifier.ref_.equals("public") || sLastIdentifier.ref_.equals("protected")) || sLastIdentifier.ref_.equals("private")) || sLastIdentifier.ref_.equals("case")))
																{
																	if ((sSequence.equals("") || (sSequence.charAt((sSequence.length() - 1)) != ':')))
																		{
																			sSequence += ':';
																			sIndentation += '\t';
																		}
																	else
																		{
																			--_iWriteCursor;
																			if ((sLocalIndentation.length() == sIndentation.length()))
																				sLocalIndentation = sLocalIndentation.substring(0, (sLocalIndentation.length() - 1));
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
													int iOffset;
													if ((!sSequence.equals("") && (sSequence.charAt((sSequence.length() - 1)) == ':')))
														iOffset = 2;
													else
														iOffset = 1;
													if ((sIndentation.length() <= iOffset))
														{
															if ((iPreviousChar < 0))
																sLocalIndentation = new String("");
															sSequence = new String("");
															sIndentation = new String("");
														}
													else
														{
															if ((iPreviousChar < 0))
																sLocalIndentation = sLocalIndentation.substring(0, (sLocalIndentation.length() - iOffset));
															sSequence = sSequence.substring(0, (sSequence.length() - iOffset));
															sIndentation = sIndentation.substring(0, (sIndentation.length() - iOffset));
														}
													sLine += '}';
													iPreviousNotEmptyChar = iChar;
													break;
												case '\\':
													sLine += '\\';
													iPreviousNotEmptyChar = iChar;
													iChar = readChar();
													if ((iChar >= 0))
														sLine += ((byte) iChar);
													break;
												case '\'':
												case '\"':
													{
														int iQuote = iChar;
														do
															{
																if ((iChar == ((int) '\\')))
																	{
																		sLine += '\\';
																		iChar = readChar();
																	}
																sLine += ((byte) iChar);
																iChar = readChar();
															}
														while (((iChar >= 0) && (iChar != iQuote)));
														if ((iChar >= 0))
															sLine += ((byte) iChar);
														iPreviousNotEmptyChar = iChar;
													}
													break;
												case '/':
													if (isEqualTo('/'))
														{
															sLine += "//";
															iChar = readChar();
															while (((iChar >= 0) && (iChar != '\n')))
																{
																	sLine += ((byte) iChar);
																	iChar = readChar();
																}
															if ((iChar >= 0))
																--_iReadCursor;
														}
													else
														if (isEqualTo('*'))
															{
																int iCommentDistance = ((getInputLocation() - iBeginningOfLine) - 2);
																sLine += "/*";
																iChar = readChar();
																while (((iChar >= 0) && ((iChar != '*') || !isEqualTo('/'))))
																	{
																		sLine += ((byte) iChar);
																		if ((iChar == '\n'))
																			{
																				int i;
																				String sBeginning = new String();
																				for ( i = 0;(i < iCommentDistance);++i)
																					{
																						iChar = readChar();
																						if (((iChar < 0) || (iChar == '\n')))
																							break;
																						sBeginning += ((byte) iChar);
																						if ((iChar > ' '))
																							break;
																					}
																				if ((i >= iCommentDistance))
																					{
																						sLine += sLocalIndentation;
																						iChar = readChar();
																					}
																				else
																					if (((iChar != '\n') && (iChar >= 0)))
																						{
																							sLine += sBeginning;
																						}
																			}
																		else
																			{
																				iChar = readChar();
																			}
																	}
																if ((iChar >= 0))
																	sLine += "*/";
															}
														else
															{
																sLine += '/';
																iPreviousNotEmptyChar = iChar;
															}
													break;
												default:
													if ((((iChar == '_') || ((iChar >= 'a') && (iChar <= 'z'))) || ((iChar >= 'A') && (iChar <= 'Z'))))
														{
															goBack();
															readIdentifier(sLastIdentifier.ref_);
															sLine += sLastIdentifier.ref_;
															iPreviousNotEmptyChar = iChar;
														}
													else
														{
															sLine += ((byte) iChar);
															if ((iChar > ' '))
																iPreviousNotEmptyChar = iChar;
														}
											}
											iPreviousChar = iChar;
											iChar = readChar();
										}
									while (((iChar >= 0) && (iChar != '\n')));
									if ((iChar >= 0))
										sLine += '\n';
									currentStream.writeOperator(sLocalIndentation);
									currentStream.writeOperator(sLine);
								}
				iChar = readChar();
			}
		int iPosition;
		if (equals(currentStream, iPosition))
			return false;
		_iWriteCursor = 0;
		_iSize = 0;
		copy(currentStream, 0);
		return true;
	}

	public static java.util.List<String> getListOfIncludePaths() {
		return _listOfIncludePaths;
	}

	public static void setListOfIncludePaths(java.util.List<String> listOfIncludePaths) {
		_listOfIncludePaths = listOfIncludePaths;
	}

	public static boolean createVirtualFile(String sHandle, String sContent) {
		_listOfVirtualFiles.put(sHandle, sContent);
		return true;
	}

	public static String createVirtualTemporaryFile(String sContent) {
//##protect##"Java ScpStream::createVirtualTemporaryFile.const string&"
		String sHandle;
		int iHandle = 0;
		do {
			sHandle = ".~#%d" + iHandle;
		} while (_listOfVirtualFiles.containsKey(sHandle));
		createVirtualFile(sHandle, sContent);
		return sHandle;
//##protect##"Java ScpStream::createVirtualTemporaryFile.const string&"
	}

	public static boolean existVirtualFile(String sHandle) {
//##protect##"Java ScpStream::existVirtualFile.const string&"
		return _listOfVirtualFiles.containsKey(sHandle);
//##protect##"Java ScpStream::existVirtualFile.const string&"
	}

	public static boolean loadVirtualFile(String sHandle, org.codeworker.StringRef sContent) {
//##protect##"Java ScpStream::loadVirtualFile.const string&.string&"
		sContent.ref_ = _listOfVirtualFiles.get(sHandle);
		return sContent.ref_ != null;
//##protect##"Java ScpStream::loadVirtualFile.const string&.string&"
	}

	public static boolean appendVirtualFile(String sHandle, String sContent) {
//##protect##"Java ScpStream::appendVirtualFile.const string&.const string&"
		String sOld = _listOfVirtualFiles.get(sHandle);
		if (sOld == null) return false;
		_listOfVirtualFiles.put(sHandle, sOld + sContent);
		return true;
//##protect##"Java ScpStream::appendVirtualFile.const string&.const string&"
	}

	public static boolean deleteVirtualFile(String sHandle) {
//##protect##"Java ScpStream::deleteVirtualFile.const string&"
		return (_listOfVirtualFiles.remove(sHandle) != null);
//##protect##"Java ScpStream::deleteVirtualFile.const string&"
	}

	public static java.io.FileInputStream openSTLInputFile(String sInputFile) {
//##protect##"Java ScpStream::openSTLInputFile.const char*"
		throw new java.lang.RuntimeException("ScpStream::openSTLInputFile(String sInputFile) not implemented yet!");
//##protect##"Java ScpStream::openSTLInputFile.const char*"
	}

	private void writeText(char[] tcText) {
		writeBinaryData(tcText, tcText.length);
	}

}
