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

#include <stdarg.h>
#include "ScpStream.h"
#include "UtlTrace.h"

namespace CodeWorker {
	int UtlTrace::_iLowerLevel = 0;
	int UtlTrace::_iUpperLevel = 100;
	int UtlTrace::_iCountLine = 1;
	int UtlTrace::_iIndentation = 0;
	char* UtlTrace::_sIndentation = (char*) memset(new char[81], '\0', 81);

	std::string UtlTrace::_sTraceFile;
	bool  UtlTrace::_bPersistent = true;
	std::auto_ptr<std::ofstream> UtlTrace::_pFile;

	int UtlTrace::_iTraceSize = 0;
	int UtlTrace::_iMaxTraceSize = 128;
	const char** UtlTrace::_tsTraceStack = new const char*[128];
	const char** UtlTrace::_tsTraceFileStack = new const char*[128];
	int* UtlTrace::_tiTraceLocationStack = new int[128];


	UtlTrace::UtlTrace(int iLevel, const char* sClass, const char* sMethod) {
		if (isValidLevel(iLevel)) {
			_iLevel = iLevel;
			_sClass = sClass;
			_sMethod = sMethod;

			char sBuffer[1024];
			sprintf(sBuffer, "%sIN %s::%s (%d)", _sIndentation, _sClass.c_str(), _sMethod.c_str(), _iCountLine);
			writeText(sBuffer);

			if (_iIndentation < 40) {
				char* pCursor = _sIndentation + (_iIndentation * 2);
				*pCursor++ = ' ';
				*pCursor++ = ' ';
				*pCursor++ = '\0';
			}
			_iIndentation++;
		} else {
			_iLevel = -1;
		}
	}

	UtlTrace::~UtlTrace() {
		if (isValidLevel(_iLevel)) {
			_iIndentation--;
			if (_iIndentation < 40) {
				char* pCursor = _sIndentation + (_iIndentation * 2);
				*pCursor = '\0';
			}

			char sBuffer[1024];
			sprintf(sBuffer, "%sOUT %s::%s (%d)", _sIndentation, _sClass.c_str(), _sMethod.c_str(), _iCountLine);
			writeText(sBuffer);
		}
	}


	void UtlTrace::traceFormat(int iLevel, const char* sFormat, ...) {
		if (UtlTrace::isValidLevel(iLevel)) {
			char sOrgDestination[1024];
			char* sDestination = (char*) sOrgDestination;
			char* sCursor = (char*) sFormat;
			va_list marker;

			va_start(marker, sFormat);     /* Initialize variable arguments. */
			strcpy(sDestination, _sIndentation);
			sDestination += strlen(sDestination);
			do {
				if (*sCursor == '%') {
					sCursor++;
					switch(*sCursor) {
#ifdef WIN32
						case 'c' : *sDestination++ = va_arg(marker, char);break;
#else
						case 'c' : *sDestination++ = va_arg(marker, int);break;
#endif
						case 'd' :
							sprintf(sDestination, "%d", va_arg(marker, int));
							sDestination += strlen(sDestination);
							break;
						case 'f' :
							sprintf(sDestination, "%f", va_arg(marker, double));
							sDestination += strlen(sDestination);
							break;
						case 's' :
							strcpy(sDestination, va_arg(marker, const char*));
							sDestination += strlen(sDestination);
							break;
						default:
							*sDestination++ = *sCursor;
							break;
					}
					sCursor++;
				} else {
					*sDestination++ = *sCursor++;
				}
			} while (*sCursor != '\0');
			*sDestination = '\0';
			va_end( marker );              /* Reset variable arguments.      */

			writeText(sOrgDestination);
		}
	}

	void UtlTrace::traceText(const char* sSourceFile, int iLineCode, const char* sText) {
		char sBuffer[1024];
		sprintf(sBuffer, "%s%s(%d):\t%s (%d)", _sIndentation, sSourceFile, iLineCode, sText, _iCountLine);
		writeText(sBuffer);
	}

	void UtlTrace::writeText(const char* sText) {
		if (!_sTraceFile.empty()) {
			if (_bPersistent) {
				std::auto_ptr<std::ofstream> f(CodeWorker::openAppendFile(_sTraceFile.c_str()));
				if (f.get() != NULL) {
					(*f) << sText << std::endl;
					f->close();
				}
			} else {
				if (_pFile.get() != NULL) (*_pFile) << sText << std::endl;
			}
		}
		_iCountLine++;
	}


	void UtlTrace::initialize(const char* sTraceFile, bool bPersistent, int iLowerLevel, int iUpperLevel) {
		setConfiguration(sTraceFile, bPersistent, iLowerLevel, iUpperLevel);
	}

	void UtlTrace::setConfiguration(const char* sTraceFile, bool bPersistent, int iLowerLevel, int iUpperLevel) {
		_bPersistent = bPersistent;
		_iLowerLevel = iLowerLevel;
		_iUpperLevel = iUpperLevel;

		if (_pFile.get() != NULL) {
			_pFile->close();
			std::auto_ptr<std::ofstream> pNull;
			_pFile = pNull;
		}

		if ((sTraceFile == NULL) || (sTraceFile[0] == '\0')) _sTraceFile = "";
		else {
			_sTraceFile = sTraceFile;
			std::auto_ptr<std::ofstream> pTraceFile(CodeWorker::openOutputFile(_sTraceFile.c_str()));
			_pFile = pTraceFile;
			if (_bPersistent && (_pFile.get() != NULL)) {
				_pFile->close();
				std::auto_ptr<std::ofstream> pNull;
				_pFile = pNull;
			}
		}
	}

	void UtlTrace::finalize() {
		if (_pFile.get() != NULL) {
			_pFile->close();
			std::auto_ptr<std::ofstream> pNull;
			_pFile = pNull;
		}
		delete [] _tsTraceStack;
		delete [] _tsTraceFileStack;
		delete [] _tiTraceLocationStack;
		delete _sIndentation;
	}


	void UtlTrace::pushTraceMethod(const char* sTraceMethod) {
		if (!_sTraceFile.empty()) {
			char sBuffer[1024];
			sprintf(sBuffer, "%sIN %s (%d)", _sIndentation, sTraceMethod, _iCountLine);
			writeTraceText(sBuffer);

			if (_iIndentation < 40) {
				char* pCursor = _sIndentation + (_iIndentation * 2);
				*pCursor++ = ' ';
				*pCursor++ = ' ';
				*pCursor++ = '\0';
			}
			_iIndentation++;
		}
		traceStackFunction(NULL, sTraceMethod, 0);
	}

	void UtlTrace::popTraceMethod() {
		_iTraceSize--;
		if (!_sTraceFile.empty()) {
			_iIndentation--;
			if (_iIndentation < 40) {
				char* pCursor = _sIndentation + (_iIndentation * 2);
				*pCursor = '\0';
			}

			char sBuffer[1024];
			sprintf(sBuffer, "%sOUT %s (%d)", _sIndentation, _tsTraceStack[_iTraceSize], _iCountLine);
			writeTraceText(sBuffer);
		}
	}

	void UtlTrace::traceStackInstruction(const char* sFileName, int iLine) {
		if (_iTraceSize == 0) traceStackFunction(sFileName, "main scope",iLine);
		_tsTraceFileStack[_iTraceSize - 1] = sFileName;
		_tiTraceLocationStack[_iTraceSize - 1] = iLine;
	}

	void UtlTrace::traceStackFunction(const char* sFileName, const char* sFunctionName, int iLine) {
		if (_iTraceSize >= _iMaxTraceSize) {
			const char** tsTraceStack = _tsTraceStack;
			int iNextMaxTraceSize = _iMaxTraceSize * 2;
			_tsTraceStack = new const char*[iNextMaxTraceSize];
			memcpy(_tsTraceStack, tsTraceStack, sizeof(const char*)*_iMaxTraceSize);
			delete [] tsTraceStack;
			const char** tsTraceFileStack = _tsTraceFileStack;
			_tsTraceFileStack = new const char*[iNextMaxTraceSize];
			memcpy(_tsTraceFileStack, tsTraceFileStack, sizeof(const char*)*_iMaxTraceSize);
			delete [] tsTraceFileStack;
			int* tiTraceLocationStack = _tiTraceLocationStack;
			_tiTraceLocationStack = new int[iNextMaxTraceSize];
			memcpy(_tiTraceLocationStack, tiTraceLocationStack, sizeof(int*)*_iMaxTraceSize);
			delete [] tiTraceLocationStack;
			_iMaxTraceSize = iNextMaxTraceSize;
		}
		_tsTraceStack[_iTraceSize] = sFunctionName;
		_tiTraceLocationStack[_iTraceSize] = iLine;
		_tsTraceFileStack[_iTraceSize] = sFileName;
		_iTraceSize++;
	}

	void UtlTrace::popTraceStack() {
		_iTraceSize--;
	}

	std::string UtlTrace::getTraceStack() {
		std::string sTraceStack;
		for (int i = 0; i < _iTraceSize; i++) {
			if (_tsTraceFileStack[i] != NULL) {
				char tcNumber[1024];
				std::string sCompleteFileName;
				std::auto_ptr<std::ifstream> pFile(openInputFileFromIncludePath(_tsTraceFileStack[i], sCompleteFileName));
				if (pFile.get() == NULL) {
					sprintf(tcNumber, "\"%s\" offset[%d]: ", _tsTraceFileStack[i], _tiTraceLocationStack[i]);
				} else {
					CodeWorker::setLocation(*pFile, _tiTraceLocationStack[i]);
					int iLine = CodeWorker::getLineCount(*pFile);
					std::string sShortFilename = _tsTraceFileStack[i];
					std::string::size_type iIndex = sShortFilename.find_last_of("\\/");
					if (iIndex != std::string::npos) sShortFilename = sShortFilename.substr(iIndex + 1);
					sprintf(tcNumber, "%s(%d): ", sShortFilename.c_str(), iLine);
					pFile->close();
				}
				sTraceStack += tcNumber;
			}
			sTraceStack += _tsTraceStack[i];
			sTraceStack += "\n";
		}
		return sTraceStack;
	}

	const char** UtlTrace::copyTraceStack() {
		const char** tsTraceStack = new const char*[_iTraceSize];
		memcpy(tsTraceStack, _tsTraceStack, _iTraceSize * sizeof(const char*));
		return tsTraceStack;
	}

	const char** UtlTrace::copyTraceFileStack() {
		const char** tsTraceStack = new const char*[_iTraceSize];
		memcpy(tsTraceStack, _tsTraceStack, _iTraceSize * sizeof(const char*));
		return tsTraceStack;
	}

	int* UtlTrace::copyTraceLocationStack() {
		int* tiTraceLocationStack = new int[_iTraceSize];
		memcpy(tiTraceLocationStack, _tiTraceLocationStack, _iTraceSize * sizeof(int));
		return tiTraceLocationStack;
	}

	void UtlTrace::writeTraceText(const char* sText) {
		if (!_sTraceFile.empty()) {
			std::auto_ptr<std::ofstream> f(CodeWorker::openAppendFile(_sTraceFile.c_str()));
			if (f.get() != NULL) {
				(*f) << sText << std::endl;
				f->close();
			}
		}
		_iCountLine++;
	}



	UtlTraceStackFunction::UtlTraceStackFunction(const char* sFileName, const char* sFunctionName, int iLine) {
		UtlTrace::traceStackFunction(sFileName, sFunctionName, iLine);
	}

	UtlTraceStackFunction::~UtlTraceStackFunction() {
		UtlTrace::popTraceStack();
	}


	UtlTraceSession::UtlTraceSession() {
		UtlTrace::initialize(NULL, false, 0, 0);
	}

	UtlTraceSession::~UtlTraceSession() {
		UtlTrace::finalize();
	}
}
