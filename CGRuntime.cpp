/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2008 Cédric Lemaire

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

//##markup##"EXECUTABLE_VERSION"
//##begin##"EXECUTABLE_VERSION"
//##protect##"EXECUTABLE_VERSION"
#define EXECUTABLE_VERSION	"4.5.3"
//##protect##"EXECUTABLE_VERSION"
//##end##"EXECUTABLE_VERSION"
#define EXECUTABLE_NAME		"CodeWorker"

//for 'open' constants
#include <fcntl.h>

//for 'fstat' and 'stat'
#include <sys/types.h>
#include <sys/stat.h>
//for 'mktime'
#include <time.h>

#include <errno.h>

#ifdef CODEWORKER_GNU_READLINE
#	include <stdio.h> // fix for Red Hat 9 - thanks to Justin Cinkelj
#	include <readline/readline.h>
#	include <readline/history.h>
#endif

//for 'chdir'
#ifdef WIN32
#	include <direct.h>
// for 'environ'
#	include <stdlib.h>
//for 'utime'
#	include <sys/utime.h>
//for 'chmod', 'access', 'open' and 'close'
#	include <io.h>
//for 'kbhit()'
#	include <conio.h>
//for 'Sleep'
#	include <windows.h>
#else
#	include <features.h>
#	if defined(__cplusplus) && __GNUC_PREREQ (4, 3)
#		include <cstdlib>
#	endif
#	include <unistd.h>
#	include <utime.h>
#	ifndef _O_RDONLY
#		define _O_RDONLY O_RDONLY  // for Debian/gcc 2.95.4
#	endif
#	include "UtlString.h" // for Debian/gcc 2.95.4
#endif

// for MD5 routines
#include "md5.h"

// Import the process's environement if it has not been done in one
// of the system headers already.
// Fixed by: Eric Nicolas
#ifndef environ
extern "C" char **environ;
#endif

#ifndef WIN32
	// Reading of the keyboard under Linux
	// -----------------------------------
	// source code: "http://linux-sxs.org/programming/kbhit.html"
#	include <termios.h>
#	include <unistd.h>   // for read()
#	include <signal.h>   // for signal()

	static termios initial_settings;
	static termios new_settings;
	static int peek_character = -1;

	// this function is used to catch
	// SIGINT, SIGHUP and SIGTERM signal
	// and restore tty settings for STDIN
	// before quit.
	void	catch_sig(int foo)
	{
#ifndef CODEWORKER_GNU_READLINE
		tcsetattr(0, TCSANOW, &initial_settings);
#endif
		_exit(EXIT_FAILURE);
	}

	void initKeyboard() {
		tcgetattr(0,&initial_settings);
		new_settings = initial_settings;
		new_settings.c_lflag &= ~(ICANON | ECHO);
		new_settings.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
		new_settings.c_cc[VMIN] = 0;
		new_settings.c_cc[VTIME] = 0;
#ifndef CODEWORKER_GNU_READLINE
		tcsetattr(0, TCSANOW, &new_settings);
#endif
		signal(SIGINT, catch_sig);
		signal(SIGHUP, catch_sig);
		signal(SIGTERM, catch_sig);
	}

	void closeKeyboard() {
#ifndef CODEWORKER_GNU_READLINE
		tcsetattr(0, TCSANOW, &initial_settings);
#endif
	}

	int kbhit() {
		unsigned char ch;
		int nread;

		if (peek_character != -1) return 1;
		new_settings.c_cc[VMIN]=0;
		new_settings.c_cc[VTIME] = 1;
#ifndef CODEWORKER_GNU_READLINE
		tcsetattr(0, TCSANOW, &new_settings);
#endif
		nread = read(0,&ch,1);
		new_settings.c_cc[VMIN]=0;
		new_settings.c_cc[VTIME] = 0;
#ifndef CODEWORKER_GNU_READLINE
		tcsetattr(0, TCSANOW, &new_settings);
#endif
		if (nread == 1) {
			peek_character = ch;
			return 1;
		}
		return 0;
	}

	int readch() {
		char ch;

		if(peek_character != -1) {
			ch = peek_character;
			peek_character = -1;
			return ch;
		}
		read(0,&ch,1);
		return ch;
	}
#endif

#include <math.h>
#include <time.h>
#include <fstream>

#include "UtlTrace.h"
#include "UtlDate.h"
#include "UtlDirectory.h"
#include "ScpStream.h"
#include "UtlXMLStream.h"

#include "GrfLoadProject.h"
#include "GrfSaveProject.h"
#include "DtaAttributeType.h"
#include "CGExternalHandling.h"
#include "DtaArrayIterator.h"
#include "GrfSaveProjectTypes.h"
#include "GrfReadonlyHook.h"
#include "GrfWritefileHook.h"
#include "CppCompilerEnvironment.h"
#include "DtaProtectedAreasBag.h"

#include "HTTPRequest.h"
#include "NetSocket.h"

#include "CGRuntime.h"
#include "DtaProject.h"
#include "CppParsingTree.h"
#include "DtaPatternScript.h"
#include "DtaTranslateScript.h"
#include "Workspace.h"

namespace CodeWorker {
#ifndef MAX_PATH
#  define MAX_PATH 1024
#endif

	EXECUTE_FUNCTION::~EXECUTE_FUNCTION() {}


	char CGRuntime::_tcHexa[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	std::string CGRuntime::_sFrozenTime;
	std::string CGRuntime::_sLogFile;
	ScpStream* CGRuntime::_pOutputStream = NULL;
	ScpStream* CGRuntime::_pInputStream = NULL;
	GrfJointPoint* CGRuntime::_pJointPoint = NULL;
	CppParsingTree_var CGRuntime::_pThisTree = CppParsingTree_var(DtaProject::createRootInstance());
	std::list<DtaPatternScript*> CGRuntime::_listOfPatternScripts;
	CGExternalHandling* CGRuntime::_pExternalHandling = NULL;


	CppParsingTree_var CGRuntime::getRootTree() {
		return DtaProject::getInstance();
	}

	std::string CGRuntime::toString(double dValue) {
		char tcNumber[300];
		double dFloor = floor(dValue);
		if (dValue == dFloor) {
			int iValue = (int) dFloor;
			sprintf(tcNumber, "%d", iValue);
		} else {
			double dAbs = fabs(dValue);
			if ((dAbs <= 1.0e-12) || (dAbs >= 1.0e12)) {
				sprintf(tcNumber, "%.12e", dValue);
			} else {
				sprintf(tcNumber, "%.24f", dValue);
				char* u = tcNumber + strlen(tcNumber);
				u--;
				while ((*u == '0') && (u != tcNumber)) {
					*u = '\0';
					u--;
				}
				if ((u != tcNumber) && (u != tcNumber + 1)) {
					if ((u[-1] == '0') && (*u >= '0') && (*u <= '9')) {
						// superfluous '0' series ended by a non-zero digit?
						char* v = u - 2;
						int iZero = 1;
						while ((*v == '0') && (v != tcNumber)) {
							iZero++;
							v--;
						}
						if (iZero >= 6) {
							// yes! a superfluous series of at least '000000'
							char* w = v;
							int iPoint = 0;
							while ((*v != '.') && (v != tcNumber)) {
								iPoint++;
								v--;
							}
							if ((iPoint > 0) && (iZero + iPoint >= 12)) {
								// truncate if at least 12 digits after the comma,
								// and if we weren't just on the dot (this case is
								// handled further)
								w[1] = '\0';
								u = w;
							}
						}
					} else if ((u[-1] == '9') && (*u >= '0') && (*u <= '9')) {
						// superfluous '9' series ended by any digit?
						char* v = u - 2;
						int iNine = 1;
						while ((*v == '9') && (v != tcNumber)) {
							iNine++;
							v--;
						}
						if (iNine >= 6) {
							// yes! a superfluous series of at least '999999'
							char* w = v;
							int iPoint = 0;
							while ((*v != '.') && (v != tcNumber)) {
								iPoint++;
								v--;
							}
							if ((*v == '.') && (iNine + iPoint >= 12)) {
								// truncate if at least 12 digits after the comma
								u = w + 1;
								*u = '\0';
								do {
									u--;
									char a = *u;
									if (a == '9') *u = '0';
									else if (a != '.') {
										*u = a + 1;
										break;
									}
								} while (u != tcNumber);
								if (*u == '0') {
									// if '0', we are necessary at the beginning
									// of the number, and we have to insert '1'
									// just before!
									memmove(u + 1, tcNumber, strlen(tcNumber));
									*u = '1';
								}
							}
						}
					}
				}
				if (*u == '.') {
					if (u == tcNumber) *u = '0';
					else *u = '\0';
				}
			}
		}
		return tcNumber;
	}

	std::string CGRuntime::toString(int iValue) {
		char sNumber[32];
		sprintf(sNumber, "%d", iValue);
		return sNumber;
	}

	int CGRuntime::toInteger(const std::string& sText) {
		return atoi(sText.c_str());
	}

	int CGRuntime::toInteger(double dValue) {
		return (int) dValue;
	}

	double CGRuntime::toDouble(int iValue) {
		return (double) iValue;
	}

	double CGRuntime::toDouble(const std::string& sText) {
		return atof(sText.c_str());
	}


	const char* CGRuntime::getApplicationName() { return EXECUTABLE_NAME; }
	const char* CGRuntime::getVersionNumber() { return EXECUTABLE_VERSION; }

	void CGRuntime::registerExternalFunction(const std::string& sKey, EXTERNAL_FUNCTION externalFunction) {
		getExternalFunctionsRegister()[sKey] = externalFunction;
	}

	void CGRuntime::registerExternalTemplateDispatcherFunction(const std::string& sKey, EXTERNAL_TEMPLATE_DISPATCHER_FUNCTION externalFunction) {
		getExternalTemplateDispatcherFunctionsRegister()[sKey] = externalFunction;
	}

	EXTERNAL_FUNCTION CGRuntime::getExternalFunction(const std::string& sKey) {
		EXTERNAL_FUNCTION externalFunction;
		std::map<std::string, EXTERNAL_FUNCTION>::const_iterator cursor = getExternalFunctionsRegister().find(sKey);
		if (cursor == getExternalFunctionsRegister().end()) externalFunction = NULL;
		else externalFunction = cursor->second;
		return externalFunction;
	}

	void CGRuntime::throwBNFExecutionError(const std::string& sBNFToken, const char* tcComment) throw(UtlException) {
		std::string sText;
		if (!_pInputStream->readIdentifier(sText)) {
			int iChar = _pInputStream->readChar();
			if (iChar < 0) {
				std::string sMessage = "BNF token '" + composeCLikeString(sBNFToken) + "' can't match the end of file";
				if (tcComment != NULL) sMessage += std::string(": ") + tcComment;
				throw UtlException(sMessage);
			}
			sText.assign(1, (char) iChar);
			sText = composeCLikeString(sText);
		} else {
			int iOldLocation = _pInputStream->getInputLocation();
			int iLocation = iOldLocation - sText.size();
			for(;;) {
				iLocation--;
				_pInputStream->setInputLocation(iLocation);
				char a = _pInputStream->peekChar();
				if (((a < 'A') || (a > 'Z')) && ((a < 'a') || (a > 'z')) && (a != '_')) break;
				sText = a + sText;
			}
			_pInputStream->setInputLocation(iOldLocation);
		}
		std::string sMessage = "the BNF token '" + sBNFToken + "' doesn't match '" + sText + "' and the following characters";
		if (tcComment != NULL) sMessage += std::string(": ") + tcComment;
		throw UtlException(sMessage);
	}

	int CGRuntime::entryPoint(int iNargs, char** tsArgs, EXECUTE_FUNCTION* executeFunction) {
		UtlTraceSession traceSession;
		Workspace workspace;
		try {
			if (workspace.checkArguments(iNargs, tsArgs, executeFunction)) {
				if (!workspace.noLogo() && !workspace.quietMode()) {
					if (executeFunction == NULL) {
						traceLine(EXECUTABLE_NAME " v" EXECUTABLE_VERSION " (LGPL), parses and generates source code easily;");
					} else {
						traceLine("This application has been written in C++ entirely by " EXECUTABLE_NAME " v" EXECUTABLE_VERSION);
					}
					traceLine("Copyright (C) 1996-2008 Cedric Lemaire; see 'http://www.codeworker.org'.");
				}
				if (!workspace.execute(executeFunction)) return workspace.executionHasFailed();
			}
		} catch (UtlException& exception) {
			traceLine(exception.getMessage());
			traceText(exception.getTraceStack());
			return workspace.executionHasFailed();
		} catch (UtlExitException& exit) {
			return exit.getCode();
		} catch (std::exception& except) {
			traceLine("Fatal error: " + std::string(except.what()));
			return workspace.executionHasFailed();
		} catch (...) {
			traceLine("Fatal error: ellipsis exception");
			return workspace.executionHasFailed();
		}
		return 0;
	}

	bool CGRuntime::executeScript(int iNargs, char** tsArgs, EXECUTE_FUNCTION* executeFunction) throw(UtlException) {
		UtlTraceSession traceSession;
		Workspace workspace;
		if (workspace.checkArguments(iNargs, tsArgs, executeFunction)) {
			if (!workspace.noLogo() && !workspace.quietMode()) {
				if (getExternalHandling() == NULL) {
					traceLine(EXECUTABLE_NAME " for parsing and generating source code easily;");
					traceLine("Copyright (C) 1996-2008 Cedric Lemaire; see 'http://www.codeworker.org'.");
				} else {
					getExternalHandling()->traceLine(EXECUTABLE_NAME " for parsing and generating source code easily;");
					getExternalHandling()->traceLine("Copyright (C) 1996-2008 Cedric Lemaire; see 'http://www.codeworker.org'.");
				}
			}
			workspace.execute(executeFunction);
			return true;
		}
		return false;
	}

	void CGRuntime::registerScript(const char* sRegistration, EXECUTE_FUNCTION* executeFunction) {
		DtaScript::registerScript(sRegistration, executeFunction);
	}

	void CGRuntime::registerReadonlyHook(READONLYHOOK_FUNCTION readonlyHook) {
		DtaProject::getInstance().setReadonlyHook(new GrfReadonlyHook(readonlyHook));
	}

	void CGRuntime::registerWritefileHook(WRITEFILEHOOK_FUNCTION writefileHook) {
		DtaProject::getInstance().setWritefileHook(new GrfWritefileHook(writefileHook));
	}

	void CGRuntime::expand(EXECUTE_FUNCTION* executeFunction, CppParsingTree_var pThisTree, const std::string& sFilename) {
		std::auto_ptr<DtaPatternScript> pPatternScript(new DtaPatternScript(executeFunction));
		pPatternScript->expand(sFilename.c_str(), *(pThisTree._pInternalNode));
	}

	void CGRuntime::expand(const std::string& sScriptFile, CppParsingTree_var pThisTree, const std::string& sFilename) {
		std::auto_ptr<DtaPatternScript> pPatternScript(new DtaPatternScript/*(_pThisTree._pInternalNode, NULL)*/);
		pPatternScript->parseFile(sScriptFile.c_str());
		pPatternScript->expand(sFilename.c_str(), *(pThisTree._pInternalNode));
	}

	void CGRuntime::autoexpand(const std::string& sFileName, CppParsingTree_var pThisTree) {
		DtaPatternScript patternScript;
		patternScript.autoexpand(sFileName.c_str(), *(pThisTree._pInternalNode));
	}

	void CGRuntime::generate(EXECUTE_FUNCTION* executeFunction, CppParsingTree_var pThisTree, const std::string& sFilename) {
		std::auto_ptr<DtaPatternScript> pPatternScript(new DtaPatternScript(executeFunction));
		pPatternScript->generate(sFilename.c_str(), *(pThisTree._pInternalNode));
	}

	void CGRuntime::generate(const std::string& sScriptFile, CppParsingTree_var pThisTree, const std::string& sFilename) {
		std::auto_ptr<DtaPatternScript> pPatternScript(new DtaPatternScript/*(_pThisTree._pInternalNode, NULL)*/);
		pPatternScript->parseFile(sScriptFile.c_str());
		pPatternScript->generate(sFilename.c_str(), *(pThisTree._pInternalNode));
	}

	void CGRuntime::generateString(EXECUTE_FUNCTION* executeFunction, CppParsingTree_var pThisTree, CppParsingTree_var pOutput) {
		std::auto_ptr<DtaPatternScript> pPatternScript(new DtaPatternScript(executeFunction));
		std::string sOutput;
		if (pPatternScript->generateString(sOutput, *(pThisTree._pInternalNode)) == NO_INTERRUPTION) pOutput.setValue(sOutput);
	}

	void CGRuntime::parseFree(EXECUTE_FUNCTION* executeFunction, CppParsingTree_var pThisTree, const std::string& sFilename) {
		CGThisModifier pThis(pThisTree);
		ScpStream* pStream = new ScpStream(sFilename, ScpStream::IN | ScpStream::PATH);
		ScpStream* pOldInputStream = _pInputStream;
		_pInputStream = pStream;
		try {
			executeFunction->run();
		} catch(UtlException& e) {
			int iLine = _pInputStream->getLineCount();
			_pInputStream->close();
			delete _pInputStream;
			_pInputStream = pOldInputStream;
			std::string sException = e.getMessage();
			std::string sMessage;
			sMessage += sFilename;
			char tcNumber[32];
			sprintf(tcNumber, "(%d):", iLine);
			sMessage += tcNumber;
			sMessage += endl() + sException;
			throw UtlException(e.getTraceStack(), sMessage);
		} catch (std::exception&) {
			_pInputStream->close();
			delete _pInputStream;
			_pInputStream = pOldInputStream;
			throw;
		}
		_pInputStream->close();
		delete _pInputStream;
		_pInputStream = pOldInputStream;
	}

	void CGRuntime::parseAsBNF(EXECUTE_FUNCTION* executeFunction, CppParsingTree_var pThisTree, const std::string& sFilename) {
		CGRuntimeOutputStream noOutput(NULL);
		CGThisModifier pThis(pThisTree);
		ScpStream* pStream = new ScpStream(sFilename, ScpStream::IN | ScpStream::PATH);
		ScpStream* pOldInputStream = _pInputStream;
		_pInputStream = pStream;
		try {
			executeFunction->run();
		} catch(UtlException& exception) {
			int iLine = _pInputStream->getLineCount();
			int iCol = _pInputStream->getColCount();
			_pInputStream->close();
			delete _pInputStream;
			_pInputStream = pOldInputStream;
			std::string sException = exception.getMessage();
			std::string sMessage = sFilename;
			char tcNumber[64];
			sprintf(tcNumber, "(%d,%d):", iLine, iCol);
			sMessage += tcNumber + endl() + sException;
			throw UtlException(exception.getTraceStack(), sMessage);
		} catch (std::exception&) {
			_pInputStream->close();
			delete _pInputStream;
			_pInputStream = pOldInputStream;
			throw;
		}
		_pInputStream->close();
		delete _pInputStream;
		_pInputStream = pOldInputStream;
	}

	void CGRuntime::parseAsBNF(const std::string& sGrammarFile, CppParsingTree_var pThisTree, const std::string& sFilename) {
		std::auto_ptr<DtaBNFScript> pBNFScript(new DtaBNFScript/*(pThisTree._pInternalNode, NULL)*/);
		pBNFScript->parseFile(sGrammarFile.c_str());
		pBNFScript->generate(sFilename.c_str(), *(pThisTree._pInternalNode));
	}

	void CGRuntime::parseStringAsBNF(EXECUTE_FUNCTION* executeFunction, CppParsingTree_var pThisTree, const std::string& sContent) {
		CGRuntimeOutputStream noOutput(NULL);
		CGThisModifier pThis(pThisTree);
		ScpStream* pStream = new ScpStream;
		(*pStream) << sContent;
		ScpStream* pOldInputStream = _pInputStream;
		_pInputStream = pStream;
		try {
			executeFunction->run();
		} catch(UtlException& exception) {
			int iLine = _pInputStream->getLineCount();
			int iCol = _pInputStream->getColCount();
			_pInputStream->close();
			delete _pInputStream;
			_pInputStream = pOldInputStream;
			std::string sException = exception.getMessage();
			std::string sMessage;
			if (!sContent.empty()) {
				sMessage +=	endl() + "----------------- content -----------------" +
							endl() + sContent + endl() + "-------------------------------------------" + endl();
			}
			char tcNumber[64];
			sprintf(tcNumber, "line %d, col %d:", iLine, iCol);
			sMessage += tcNumber + endl() + sException;
			throw UtlException(exception.getTraceStack(), sMessage);
		} catch (std::exception&) {
			_pInputStream->close();
			delete _pInputStream;
			_pInputStream = pOldInputStream;
			throw;
		}
		_pInputStream->close();
		delete _pInputStream;
		_pInputStream = pOldInputStream;
	}

	void CGRuntime::translate(EXECUTE_FUNCTION* executeFunction, CppParsingTree_var pThisTree, const std::string& sInputFilename, const std::string& sOutputFilename) {
		std::auto_ptr<DtaTranslateScript> pTranslateScript(new DtaTranslateScript(executeFunction));
		pTranslateScript->translate(sInputFilename, sOutputFilename, *(pThisTree._pInternalNode));
	}

	SEQUENCE_INTERRUPTION_LIST CGRuntime::writeText(const char* sText) {
		_pOutputStream->writeBinaryData(sText, strlen(sText));
		return NO_INTERRUPTION;
	}

	SEQUENCE_INTERRUPTION_LIST CGRuntime::writeText(int iValue) {
		(*_pOutputStream) << iValue;
		return NO_INTERRUPTION;
	}

	SEQUENCE_INTERRUPTION_LIST CGRuntime::writeText(double dValue) {
		(*_pOutputStream) << dValue;
		return NO_INTERRUPTION;
	}

	SEQUENCE_INTERRUPTION_LIST CGRuntime::writeText(const CppParsingTree_var& pValue) {
		const char* tcText = pValue.getValue();
		if (tcText != NULL) {
			_pOutputStream->writeBinaryData(tcText, pValue.getValueLength());
		}
		return NO_INTERRUPTION;
	}

	SEQUENCE_INTERRUPTION_LIST CGRuntime::writeBinaryData(const char* sText, int iLength) {
		_pOutputStream->writeBinaryData(sText, iLength);
		return NO_INTERRUPTION;
	}

	SEQUENCE_INTERRUPTION_LIST CGRuntime::writeBinaryData(unsigned char cChar) {
		_pOutputStream->writeBinaryData((const char*) &cChar, 1);
		return NO_INTERRUPTION;
	}

	std::string CGRuntime::readNumeric() {
		std::string sNumeric;
		if (_pInputStream->isEqualTo('+')) sNumeric = "+";
		sNumeric += readInteger();
		if (_pInputStream->isEqualTo('.')) {
			sNumeric += "." + readPositiveInteger();
			if (sNumeric.size() == 1 || (sNumeric == "+.")) {
				_pInputStream->setInputLocation(_pInputStream->getInputLocation() - sNumeric.size());
				return "";
			}
		} else if (sNumeric == "+") {
			_pInputStream->goBack();
			return "";
		}
		if (!sNumeric.empty()) {
			int iChar = _pInputStream->readChar();
			if ((iChar == (int) 'e') || (iChar == (int) 'E')) {
				std::string sPrefix(1, (char) iChar);
				iChar = _pInputStream->peekChar();
				if ((iChar == (int) '+') || (iChar == (int) '-')) {
					sPrefix += (char) iChar;
					iChar = _pInputStream->readChar();
				}
				std::string sExponent = readPositiveInteger();
				if (sExponent.empty()) _pInputStream->setInputLocation(_pInputStream->getInputLocation() - sPrefix.size());
				else sNumeric += sPrefix + sExponent;
			} else if (iChar >= 0) _pInputStream->goBack();
		}
		return sNumeric;
	}

	std::string CGRuntime::readInteger() {
		std::string sInteger;
		if (_pInputStream->isEqualTo('-')) {
			sInteger = "-" + readPositiveInteger();
			if (sInteger.size() == 1) {
				_pInputStream->goBack();
				return "";
			}
		} else {
			sInteger = readPositiveInteger();
		}
		return sInteger;
	}

	std::string CGRuntime::readEndOfLine() {
		if (_pInputStream->isEqualTo('\r')) {
			if (_pInputStream->isEqualTo('\n')) {
				return "\r\n";
			}
			_pInputStream->goBack();
		} else if (_pInputStream->isEqualTo('\n')) {
			return "\n";
		} else {
			_pInputStream->goBack();
		}
		return "";
	}

	std::string CGRuntime::readPositiveInteger() {
		std::string sInteger;
		int iChar = _pInputStream->readChar();
		while ((iChar >= (int) '0') && (iChar <= (int) '9')) {
			sInteger += (char) iChar;
			iChar = _pInputStream->readChar();
		}
		if (iChar >= 0) _pInputStream->goBack();
		return sInteger;
	}

	std::string CGRuntime::readCompleteIdentifier() {
		int iLocation = _pInputStream->getInputLocation();
		while (_pInputStream->goBack()) {
			int iChar = _pInputStream->peekChar();
			if ((iChar < (int) '0') || (iChar > '9')) {
				if (((iChar >= (int) 'A') && (iChar <= (int) 'Z')) ||
					((iChar >= (int) 'a') && (iChar <= (int) 'z')) ||
					(iChar == '_')) {
					_pInputStream->setInputLocation(iLocation);
					return "";
				}
				break;
			}
		}
		std::string sIdentifier;
		_pInputStream->setInputLocation(iLocation);
		_pInputStream->readIdentifier(sIdentifier);
		return sIdentifier;
	}

	bool CGRuntime::readIfEqualTo(char cChar) {
		return _pInputStream->isEqualTo(cChar);
	}

	std::list<DtaScriptVariable*>::const_iterator CGRuntime::nextIteration(std::list<DtaScriptVariable*>::const_iterator i) {
		i++;
		return i;
	}

	std::map<std::string, DtaScriptVariable*>::const_iterator CGRuntime::nextIteration(std::map<std::string, DtaScriptVariable*>::const_iterator i) {
		i++;
		return i;
	}

//##markup##"functions and procedures"
//##begin##"functions and procedures"
SEQUENCE_INTERRUPTION_LIST CGRuntime::appendFile(const std::string& sFilename, const std::string& sContent) {
//##protect##"appendFile"
	ScpStream theStream(sContent.size() + 1);
	theStream.writeBinaryData(sContent.c_str(), sContent.size());
	theStream.appendFile(sFilename, true);
	return NO_INTERRUPTION;
//##protect##"appendFile"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::clearVariable(DtaScriptVariable* pNode) {
//##protect##"clearVariable"
	if (pNode != NULL) pNode->clearContent();
	return NO_INTERRUPTION;
//##protect##"clearVariable"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::clearVariable(CppParsingTree_var pNode) {
	SEQUENCE_INTERRUPTION_LIST result = clearVariable(pNode._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::compileToCpp(const std::string& sScriptFileName, const std::string& sProjectDirectory, const std::string& sCodeWorkerDirectory) {
//##protect##"compileToCpp"
	CppCompilerEnvironment theCompilerEnvironment(sProjectDirectory);
	DtaScript script(NULL);
	script.parseFile(sScriptFileName.c_str());
	script.compileCpp(theCompilerEnvironment, sScriptFileName);
	return NO_INTERRUPTION;
//##protect##"compileToCpp"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::copyFile(const std::string& sSourceFileName, const std::string& sDestinationFileName) {
//##protect##"copyFile"
	std::auto_ptr<ScpStream> pInput(new ScpStream(sSourceFileName, ScpStream::IN | ScpStream::PATH));
	pInput->saveIntoFile(sDestinationFileName, true);
	return NO_INTERRUPTION;
//##protect##"copyFile"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::copyGenerableFile(const std::string& sSourceFileName, const std::string& sDestinationFileName) {
//##protect##"copyGenerableFile"
	CppParsingTree_value isDone;
	std::string sScriptHandle = createVirtualTemporaryFile("@set this = true;");
	std::string sFileHandle1 = createVirtualTemporaryFile(loadFile(sSourceFileName, -1));
	expand(sScriptHandle, isDone, sFileHandle1);
	if (!isDone.getBooleanValue()) generate(sScriptHandle, isDone, sFileHandle1);
	bool bDifferent = !existFile(sDestinationFileName);
	if (!bDifferent) {
		std::string sFileHandle2 = createVirtualTemporaryFile(loadFile(sDestinationFileName, -1));
		isDone.setValue("");
		expand(sScriptHandle, isDone, sFileHandle2);
		if (!isDone.getBooleanValue()) generate(sScriptHandle, isDone, sFileHandle2);
		bDifferent = (loadFile(sFileHandle1, -1) != loadFile(sFileHandle2, -1));
		deleteVirtualFile(sFileHandle2);
	}
	deleteVirtualFile(sScriptHandle);
	if (bDifferent) copyFile(sFileHandle1, sDestinationFileName);
	deleteVirtualFile(sFileHandle1);
	return NO_INTERRUPTION;
//##protect##"copyGenerableFile"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::copySmartDirectory(const std::string& sSourceDirectory, const std::string& sDestinationPath) {
//##protect##"copySmartDirectory"
	UtlDirectory sourceDir(sSourceDirectory);
	if (!sourceDir.scanRecursively()) throw UtlException("unable to open directory '" + sSourceDirectory + "'");
	if (sDestinationPath.empty() || (sDestinationPath[sDestinationPath.size() - 1] == '/') || (sDestinationPath[sDestinationPath.size() - 1] == '\\'))
		copySmartDirectory(sourceDir, sDestinationPath);
	else
		copySmartDirectory(sourceDir, sDestinationPath + "/");
	return NO_INTERRUPTION;
//##protect##"copySmartDirectory"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::cutString(const std::string& sText, const std::string& sSeparator, std::list<std::string>& slList) {
//##protect##"cutString"
	slList.clear();
	int iPos = sText.find(sSeparator);
	int iPrec = 0;
	while (iPos >= 0) {
		std::string sValue = sText.substr(iPrec, iPos - iPrec);
		slList.push_back(sValue);
		iPrec = iPos + sSeparator.size();
		iPos = sText.find(sSeparator, iPrec);
	}
	std::string sLastValue = sText.substr(iPrec);
	slList.push_back(sLastValue);
	return NO_INTERRUPTION;
//##protect##"cutString"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::cutString(const std::string& sText, const std::string& sSeparator, CppParsingTree_var pList) {
	std::list<std::string> slList;
	SEQUENCE_INTERRUPTION_LIST result = cutString(sText, sSeparator, slList);
	pList.setValue(slList);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::environTable(DtaScriptVariable* pTable) {
//##protect##"environTable"
	pTable->clearContent();
	int i = 0;
	// If a syntax error occurs on the next line with your compiler,
	// try -D __USE_GNU as a preprocessor directive.
	// If the error doesn't disappear, please find the declaration
	// 'extern char **environ;' amongst your system includes and send me an email
	// (codeworker@free.fr).
	while (environ[i] != NULL) {
		pTable->pushItem(environ[i])->setValue(environ[i]);
		i++;
	}
	return NO_INTERRUPTION;
//##protect##"environTable"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::environTable(CppParsingTree_var pTable) {
	SEQUENCE_INTERRUPTION_LIST result = environTable(pTable._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::extendExecutedScript(const std::string& sScriptContent) {
//##protect##"extendExecutedScript"
	ScpStream stream(sScriptContent);
	DtaProject::getInstance().getScript()->parseStream(stream, false, "");
	return NO_INTERRUPTION;
//##protect##"extendExecutedScript"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::insertElementAt(DtaScriptVariable* pList, const std::string& sKey, int iPosition) {
//##protect##"insertElementAt"
	if (pList != NULL) {
		pList->insertElementAt(sKey, iPosition);
	}
	return NO_INTERRUPTION;
//##protect##"insertElementAt"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::insertElementAt(CppParsingTree_var pList, const std::string& sKey, int iPosition) {
	SEQUENCE_INTERRUPTION_LIST result = insertElementAt(pList._pInternalNode, sKey, iPosition);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::invertArray(DtaScriptVariable* pArray) {
//##protect##"invertArray"
	pArray->invertArray();
	return NO_INTERRUPTION;
//##protect##"invertArray"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::invertArray(CppParsingTree_var pArray) {
	SEQUENCE_INTERRUPTION_LIST result = invertArray(pArray._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::listAllGeneratedFiles(DtaScriptVariable* pFiles) {
//##protect##"listAllGeneratedFiles"
	if (pFiles == NULL) {
		throw UtlException("the tree parameter passed to the procedure 'listAllGeneratedFiles' doesn't exist");
	}
	pFiles->clearContent();
	const std::map<std::string, std::set<std::string> >& listOfGeneratedFiles = DtaProject::getInstance().getCapturedOutputFiles();
	for (std::map<std::string, std::set<std::string> >::const_iterator i = listOfGeneratedFiles.begin(); i != listOfGeneratedFiles.end(); ++i) {
		DtaScriptVariable* pOutputFile = pFiles->addElement(i->first);
		pOutputFile->setValue(i->first.c_str());
		DtaScriptVariable* pScripts = pOutputFile->insertNode("scripts");
		for (std::set<std::string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			pScripts->addElement(*j)->setValue(j->c_str());
		}
	}
	return NO_INTERRUPTION;
//##protect##"listAllGeneratedFiles"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::listAllGeneratedFiles(CppParsingTree_var pFiles) {
	SEQUENCE_INTERRUPTION_LIST result = listAllGeneratedFiles(pFiles._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::loadProject(const std::string& sXMLorTXTFileName, DtaScriptVariable* pNodeToLoad) {
//##protect##"loadProject"
	std::string sCompleteFileName;
	std::ifstream* pFile = CodeWorker::openInputFileFromIncludePath(sXMLorTXTFileName.c_str(), sCompleteFileName);
	if (pFile == NULL) throw UtlException("unable to open file \"" + sXMLorTXTFileName + "\" for reading");
	if (sXMLorTXTFileName.rfind(".xml") == sXMLorTXTFileName.size() - 4) {
		throw UtlException("loadProject() isn't able to load yet a parse tree previously saved to XML");
	} else {
		GrfLoadProject::parseTextFile(*pFile, pNodeToLoad);
	}
	pFile->close();
	return NO_INTERRUPTION;
//##protect##"loadProject"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::loadProject(const std::string& sXMLorTXTFileName, CppParsingTree_var pNodeToLoad) {
	SEQUENCE_INTERRUPTION_LIST result = loadProject(sXMLorTXTFileName, pNodeToLoad._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::openLogFile(const std::string& sFilename) {
//##protect##"openLogFile"
	_sLogFile = sFilename;
	if (!sFilename.empty()) {
		saveToFile(sFilename, EXECUTABLE_NAME " v" EXECUTABLE_VERSION " -- " + getNow() + endl());
	}
	return NO_INTERRUPTION;
//##protect##"openLogFile"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::produceHTML(const std::string& sScriptFileName, const std::string& sHTMLFileName) {
//##protect##"produceHTML"
/*	std::ifstream* pScriptFile = openInputFileFromIncludePath(sScriptFileName.c_str());
	if (pScriptFile == NULL) throw UtlException(std::string("unable to open file \"") + sScriptFileName + "\" for reading");
	std::fstream* pHTMLFile = DtaScript::openOutputFile(sHTMLFileName.c_str(), false);
	if (pHTMLFile == NULL) {
		pScriptFile->close();
		throw UtlException("unable to open file \"" + sHTMLFileName + "\" for writing");
	}

	(*pHTMLFile) << "<HTML>" << std::endl;
	(*pHTMLFile) << "<BODY>" << std::endl;
	bool bExecMode = false;
	int iChar = CodeWorker::readChar((*pScriptFile));
	bool bFirstOfLine = true;
	std::string sColor = "#000000";
	while (iChar > 0) {
		bool bRead = true;
		if (bFirstOfLine) {
			(*pHTMLFile) << "<FONT SIZE=3><FONT COLOR=\"" << sColor << "\">";
			if (!bExecMode) (*pHTMLFile) << "<B>";
			bFirstOfLine = false;
		}
		if (iChar == (int) '@') {
			if (bExecMode) {
				bExecMode = false;
				sColor = "#000000";
			} else {
				(*pHTMLFile) << "</B>";
				bExecMode = true;
				sColor = "#0000ff";
			}
			(*pHTMLFile) << "</FONT><FONT SIZE=1 COLOR=\"";
			if (bExecMode) (*pHTMLFile) << "#00ff00";
			else (*pHTMLFile) << "#ff0000";
			(*pHTMLFile) << "\">@</FONT><FONT SIZE=3 COLOR=\"" << sColor << "\">";
			if (!bExecMode) (*pHTMLFile) << "<B>";
		} else if (iChar == (int) '\\') {
			(*pHTMLFile) << "\\";
			iChar = CodeWorker::readChar((*pScriptFile));
			if (iChar == (int) '@') {
				(*pHTMLFile) << "@";
			} else if (iChar > 0) {
				(*pHTMLFile) << (char) iChar;
			}
		} else if (iChar == ' ') {
			(*pHTMLFile) << "&nbsp;";
		} else if (iChar == '\t') {
			(*pHTMLFile) << "&nbsp;&nbsp;&nbsp;&nbsp;";
		} else if (iChar == '\r') {
			if (!bExecMode) (*pHTMLFile) << "</B>";
			(*pHTMLFile) << "</FONT></FONT><BR>" << std::endl;
			iChar = CodeWorker::readChar((*pScriptFile));
			if (iChar != '\n') bRead = false;
			bFirstOfLine = true;
		} else if (iChar == '\n') {
			if (!bExecMode) (*pHTMLFile) << "</B>";
			(*pHTMLFile) << "</FONT></FONT><BR>" << std::endl;
			iChar = CodeWorker::readChar((*pScriptFile));
			if (iChar != '\r') bRead = false;
			bFirstOfLine = true;
		} else if (iChar == '<') {
			(*pHTMLFile) << "&#139;";
		} else if (iChar == '>') {
			(*pHTMLFile) << "&#155;";
		} else {
			(*pHTMLFile) << (char) iChar;
		}
		if (bRead) iChar = CodeWorker::readChar((*pScriptFile));
	}
	if (!bFirstOfLine) {
		if (!bExecMode) (*pHTMLFile) << "</B>";
		(*pHTMLFile) << "</FONT></FONT><BR>" << std::endl;
	}
	(*pHTMLFile) << "</BODY>" << std::endl;
	(*pHTMLFile) << "</HTML>" << std::endl;

	pScriptFile->close();
	pHTMLFile->close();
*/	return NO_INTERRUPTION;
//##protect##"produceHTML"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::putEnv(const std::string& sName, const std::string& sValue) {
//##protect##"putEnv"
	std::string sLine = sName + "=" + sValue;
#ifdef WIN32
	if (_putenv(sLine.c_str()) == -1)
#else
//!!! under Linux and Mac OS X
	//	if (setenv(sName.c_str(), sValue.c_str(), 1) == -1)
//!!! under Unix, we have to use putenv()
	static std::map<std::string, char*> keepPointers;
	std::map<std::string, char*>::const_iterator cursor = keepPointers.find(sName);
	if (cursor != keepPointers.end()) free(cursor->second);
	char* tcLine = strdup(sLine.c_str());
	keepPointers[sName] = tcLine;
	if (putenv(tcLine) != 0)
#endif
		throw UtlException("error while trying to put environment entry '" + sLine + "'");
	return NO_INTERRUPTION;
//##protect##"putEnv"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::randomSeed(int iSeed) {
//##protect##"randomSeed"
	if (iSeed >= 0) srand((unsigned) iSeed);
	else srand((unsigned) time(NULL));
	return NO_INTERRUPTION;
//##protect##"randomSeed"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeAllElements(DtaScriptVariable* pVariable) {
//##protect##"removeAllElements"
	if (pVariable != NULL) pVariable->removeArrayElement();
	return NO_INTERRUPTION;
//##protect##"removeAllElements"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeAllElements(CppParsingTree_var pVariable) {
	SEQUENCE_INTERRUPTION_LIST result = removeAllElements(pVariable._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeElement(DtaScriptVariable* pVariable, const std::string& sKey) {
//##protect##"removeElement"
	if (pVariable != NULL) pVariable->removeArrayElement(sKey.c_str());
	return NO_INTERRUPTION;
//##protect##"removeElement"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeElement(CppParsingTree_var pVariable, const std::string& sKey) {
	SEQUENCE_INTERRUPTION_LIST result = removeElement(pVariable._pInternalNode, sKey);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeFirstElement(DtaScriptVariable* pList) {
//##protect##"removeFirstElement"
	if (pList != NULL) pList->removeFirstElement();
	return NO_INTERRUPTION;
//##protect##"removeFirstElement"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeFirstElement(CppParsingTree_var pList) {
	SEQUENCE_INTERRUPTION_LIST result = removeFirstElement(pList._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeLastElement(DtaScriptVariable* pList) {
//##protect##"removeLastElement"
	if (pList != NULL) pList->removeLastElement();
	return NO_INTERRUPTION;
//##protect##"removeLastElement"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeLastElement(CppParsingTree_var pList) {
	SEQUENCE_INTERRUPTION_LIST result = removeLastElement(pList._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeRecursive(DtaScriptVariable* pVariable, const std::string& sAttribute) {
//##protect##"removeRecursive"
	pVariable->removeRecursive(sAttribute.c_str());
	return NO_INTERRUPTION;
//##protect##"removeRecursive"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeRecursive(CppParsingTree_var pVariable, const std::string& sAttribute) {
	SEQUENCE_INTERRUPTION_LIST result = removeRecursive(pVariable._pInternalNode, sAttribute);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeVariable(DtaScriptVariable* pNode) {
//##protect##"removeVariable"
	DtaScriptVariable* pVariable = pNode;
//	while (pVariable->getReferencedVariable() != NULL) pVariable = pVariable->getReferencedVariable();
	DtaScriptVariable* pParent = pVariable->getParent();
	if (pParent == NULL) throw UtlException("It is forbidden to remove the main tree or a local variable");
	if (pVariable->isLocal()) {
		std::string strError = "It is forbidden to remove a local variable: '";
		strError += pVariable->getName();
		strError += "'";
		throw UtlException(strError);
	}
	if (strncmp(pVariable->getName(), "##", 2) == 0) throw UtlException("It is forbidden to remove a scope");
	pParent->remove(pVariable);
	return NO_INTERRUPTION;
//##protect##"removeVariable"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::removeVariable(CppParsingTree_var pNode) {
	SEQUENCE_INTERRUPTION_LIST result = removeVariable(pNode._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::saveBinaryToFile(const std::string& sFilename, const std::string& sContent) {
//##protect##"saveBinaryToFile"
	int iLength = sContent.size();
	if ((iLength % 2) != 0) throw UtlException("saveBinaryToFile(\"" + sFilename + "\"): a binary content cannot have an odd length");
	iLength >>= 1;
	unsigned char* tcBinary = new unsigned char[iLength];
	if (!convertBytesToChars(sContent, tcBinary, iLength)) {
		delete [] tcBinary;
		throw UtlException("saveBinaryToFile(\"" + sFilename + "\"): about the binary content, invalid hexadecimal digit encountered");
	}
	ScpStream theStream;
	theStream.writeBinaryData((char*) tcBinary, iLength);
	delete [] tcBinary;
	theStream.saveIntoFile(sFilename, true);
	return NO_INTERRUPTION;
//##protect##"saveBinaryToFile"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::saveProject(const std::string& sXMLorTXTFileName, DtaScriptVariable* pNodeToSave) {
//##protect##"saveProject"
	if (pNodeToSave == NULL) throw UtlException("cannot save an empty node to a file");
	std::ofstream* pFile = CodeWorker::openOutputFile(sXMLorTXTFileName.c_str());
	if (pFile == NULL) throw UtlException("unable to open file \"" + sXMLorTXTFileName + "\" for writing");
	if (sXMLorTXTFileName.rfind(".xml") == sXMLorTXTFileName.size() - 4) {
		UtlXMLStream myXMLStream(*pFile);
		GrfSaveProject::generateXMLFile(myXMLStream, *pNodeToSave);
	} else {
		GrfSaveProject::generateTextFile(*pFile, *pNodeToSave);
	}
	pFile->close();
	return NO_INTERRUPTION;
//##protect##"saveProject"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::saveProject(const std::string& sXMLorTXTFileName, CppParsingTree_var pNodeToSave) {
	SEQUENCE_INTERRUPTION_LIST result = saveProject(sXMLorTXTFileName, pNodeToSave._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::saveProjectTypes(const std::string& sXMLFileName) {
//##protect##"saveProjectTypes"
	std::ofstream* pXMLFile = CodeWorker::openOutputFile(sXMLFileName.c_str());
	if (pXMLFile == NULL) throw UtlException("unable to open file \"" + sXMLFileName + "\" for writing");
	UtlXMLStream myXMLStream(*pXMLFile);
	DtaAttributeType& myType = DtaAttributeType::extractProjectTypes(DtaProject::getInstance());
	std::set<std::string> listForAvoidingCycles;
	GrfSaveProjectTypes::generateXMLFile(myXMLStream, myType, listForAvoidingCycles);
	pXMLFile->close();
	return NO_INTERRUPTION;
//##protect##"saveProjectTypes"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::saveToFile(const std::string& sFilename, const std::string& sContent) {
//##protect##"saveToFile"
	ScpStream theStream(sContent.size() + 1);
	theStream.writeBinaryData(sContent.c_str(), sContent.size());
	theStream.saveIntoFile(sFilename, true);
	return NO_INTERRUPTION;
//##protect##"saveToFile"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setCommentBegin(const std::string& sCommentBegin) {
//##protect##"setCommentBegin"
	DtaProject::getInstance().setCommentBegin(sCommentBegin);
	return NO_INTERRUPTION;
//##protect##"setCommentBegin"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setCommentEnd(const std::string& sCommentEnd) {
//##protect##"setCommentEnd"
	DtaProject::getInstance().setCommentEnd(sCommentEnd);
	return NO_INTERRUPTION;
//##protect##"setCommentEnd"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setGenerationHeader(const std::string& sComment) {
//##protect##"setGenerationHeader"
	DtaProject::getInstance().setGenerationHeader(sComment);
	return NO_INTERRUPTION;
//##protect##"setGenerationHeader"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setIncludePath(const std::string& sPath) {
//##protect##"setIncludePath"
	std::list<std::string> includePath;
	std::string::size_type iStart = 0;
	std::string::size_type iEnd = sPath.find(';');
	while (iEnd != std::string::npos) {
		std::string sOnePath = sPath.substr(iStart, iEnd - iStart);
		if (!sOnePath.empty()) {
			char c = sOnePath[sOnePath.size() - 1];
			if ((c != '/') && (c != '\\')) sOnePath += "/";
			includePath.push_back(sOnePath);
		}
		iStart = iEnd + 1;
		iEnd = sPath.find(';', iStart);
	}
	std::string sOnePath = sPath.substr(iStart);
	if (!sOnePath.empty()) {
		char c = sOnePath[sOnePath.size() - 1];
		if ((c != '/') && (c != '\\')) sOnePath += "/";
		includePath.push_back(sOnePath);
	}
	ScpStream::setListOfIncludePaths(includePath);
	return NO_INTERRUPTION;
//##protect##"setIncludePath"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setNow(const std::string& sConstantDateTime) {
//##protect##"setNow"
	if (sConstantDateTime.empty()) {
		_sFrozenTime = "";
	} else {
		UtlDate theDate = UtlDate::getDateFromFormat(sConstantDateTime, "%d%b%Y%| %H:%M:%S%|.%L");
		_sFrozenTime = sConstantDateTime;
	}
	return NO_INTERRUPTION;
//##protect##"setNow"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setProperty(const std::string& sDefine, const std::string& sValue) {
//##protect##"setProperty"
	DtaProject::getInstance().setDefineTarget(sDefine, sValue);
	return NO_INTERRUPTION;
//##protect##"setProperty"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setTextMode(const std::string& sTextMode) {
//##protect##"setTextMode"
	if (stricmp(sTextMode.c_str(), "dos") == 0) DtaProject::getInstance().setTextMode(DtaProject::DOS_MODE);
	else if (stricmp(sTextMode.c_str(), "unix") == 0) DtaProject::getInstance().setTextMode(DtaProject::UNIX_MODE);
	else if (stricmp(sTextMode.c_str(), "binary") == 0) DtaProject::getInstance().setTextMode(DtaProject::BINARY_MODE);
	else throw UtlException("setTextMode(<text-mode>) doesn't recognize \"" + sTextMode + "\" (\"DOS\" or \"UNIX\" or \"BINARY\" expected)");
	return NO_INTERRUPTION;
//##protect##"setTextMode"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setVersion(const std::string& sVersion) {
//##protect##"setVersion"
	DtaProject::getInstance().setVersion(sVersion);
	return NO_INTERRUPTION;
//##protect##"setVersion"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setWriteMode(const std::string& sMode) {
//##protect##"setWriteMode"
	if (_pOutputStream == NULL) throw UtlException("no output stream on which to apply the function 'setWriteMode(\"" + sMode + "\")'!");
	if (stricmp(sMode.c_str(), "insert") == 0) _pOutputStream->insertMode(true);
	else if (stricmp(sMode.c_str(), "overwrite") == 0) _pOutputStream->insertMode(false);
	else throw UtlException("function 'setWriteMode()': \"insert\" or \"overwrite\" mode expected, instead of \"" + sMode + "\"");
	return NO_INTERRUPTION;
//##protect##"setWriteMode"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setWorkingPath(const std::string& sPath) {
//##protect##"setWorkingPath"
	DtaProject::getInstance().setWorkingPath(sPath);
	return NO_INTERRUPTION;
//##protect##"setWorkingPath"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::sleep(int iMillis) {
//##protect##"sleep"
#ifdef WIN32
	Sleep(iMillis);
#else
	usleep(iMillis*1000);
#endif
	return NO_INTERRUPTION;
//##protect##"sleep"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::slideNodeContent(DtaScriptVariable* pOrgNode, ExprScriptVariable& xDestNode) {
//##protect##"slideNodeContent"
	if (pOrgNode != NULL) {
		pOrgNode->slideNodeContent(*pOrgNode, xDestNode);
	}
	return NO_INTERRUPTION;
//##protect##"slideNodeContent"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::slideNodeContent(CppParsingTree_var pOrgNode, CppParsingTree_varexpr& xDestNode) {
	SEQUENCE_INTERRUPTION_LIST result = slideNodeContent(pOrgNode._pInternalNode, xDestNode.getVariableExpression());
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::sortArray(DtaScriptVariable* pArray) {
//##protect##"sortArray"
	if (pArray != NULL) {
		pArray->sortArray();
	}
	return NO_INTERRUPTION;
//##protect##"sortArray"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::sortArray(CppParsingTree_var pArray) {
	SEQUENCE_INTERRUPTION_LIST result = sortArray(pArray._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::traceEngine() {
//##protect##"traceEngine"
	DtaProject::getInstance().traceEngine();
	return NO_INTERRUPTION;
//##protect##"traceEngine"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::traceLine(const std::string& sLine) {
//##protect##"traceLine"
	if (getExternalHandling() == NULL) std::cout << sLine << std::endl;
	else getExternalHandling()->traceLine(sLine);
	if (!_sLogFile.empty()) {
		if (existVirtualFile(_sLogFile)) {
			ScpStream::appendVirtualFile(_sLogFile, sLine + CGRuntime::endl());
		} else {
			std::auto_ptr<std::ofstream> f(CodeWorker::openAppendFile(_sLogFile.c_str()));
			if (f.get() != NULL) {
				(*f) << sLine << CGRuntime::endl();
				f->close();
			}
		}
	}
	return NO_INTERRUPTION;
//##protect##"traceLine"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::traceObject(DtaScriptVariable* pObject, int iDepth) {
//##protect##"traceObject"
	if (pObject == NULL) {
		traceLine("null");
	} else {
		pObject->traceObject(iDepth);
	}
	return NO_INTERRUPTION;
//##protect##"traceObject"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::traceObject(CppParsingTree_var pObject, int iDepth) {
	SEQUENCE_INTERRUPTION_LIST result = traceObject(pObject._pInternalNode, iDepth);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::traceStack(DtaScriptVariable& visibility) {
//##protect##"traceStack"
	visibility.traceStack();
	return NO_INTERRUPTION;
//##protect##"traceStack"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::traceText(const std::string& sText) {
//##protect##"traceText"
	if (getExternalHandling() == NULL) std::cout << sText << std::flush;
	else getExternalHandling()->traceText(sText);
	if (!_sLogFile.empty()) {
		if (existVirtualFile(_sLogFile)) {
			ScpStream::appendVirtualFile(_sLogFile, sText);
		} else {
			std::auto_ptr<std::ofstream> f(CodeWorker::openAppendFile(_sLogFile.c_str()));
			if (f.get() != NULL) {
				(*f) << sText << std::flush;
				f->close();
			}
		}
	}
	return NO_INTERRUPTION;
//##protect##"traceText"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::attachInputToSocket(int iSocket) {
//##protect##"attachInputToSocket"
	NetSocket::attachInputToSocket(iSocket);
	return NO_INTERRUPTION;
//##protect##"attachInputToSocket"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::detachInputFromSocket(int iSocket) {
//##protect##"detachInputFromSocket"
	NetSocket::detachInputFromSocket(iSocket);
	return NO_INTERRUPTION;
//##protect##"detachInputFromSocket"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::goBack() {
//##protect##"goBack"
	_pInputStream->goBack();
	return NO_INTERRUPTION;
//##protect##"goBack"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setInputLocation(int iLocation) {
//##protect##"setInputLocation"
	_pInputStream->setInputLocation(iLocation);
	return NO_INTERRUPTION;
//##protect##"setInputLocation"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::allFloatingLocations(DtaScriptVariable* pList) {
//##protect##"allFloatingLocations"
	const std::map<std::string, int>& flMap = _pInputStream->allFloatingLocations();
	for (std::map<std::string, int>::const_iterator i = flMap.begin(); i != flMap.end(); ++i) {
		pList->addElement(i->first)->setValue(i->second);
	}
	return NO_INTERRUPTION;
//##protect##"allFloatingLocations"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::allFloatingLocations(CppParsingTree_var pList) {
	SEQUENCE_INTERRUPTION_LIST result = allFloatingLocations(pList._pInternalNode);
	return result;
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::attachOutputToSocket(int iSocket) {
//##protect##"attachOutputToSocket"
	NetSocket::attachOutputToSocket(iSocket);
	return NO_INTERRUPTION;
//##protect##"attachOutputToSocket"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::detachOutputFromSocket(int iSocket) {
//##protect##"detachOutputFromSocket"
	NetSocket::detachOutputFromSocket(iSocket);
	return NO_INTERRUPTION;
//##protect##"detachOutputFromSocket"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::incrementIndentLevel(int iLevel) {
//##protect##"incrementIndentLevel"
	_pOutputStream->incrementIndentation(iLevel);
	return NO_INTERRUPTION;
//##protect##"incrementIndentLevel"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::insertText(int iLocation, const std::string& sText) {
//##protect##"insertText"
	if (!_pOutputStream->insertText(sText, iLocation, 0)) {
		std::string sMessage = "unable to insert \"" + composeCLikeString(sText) + "\" at position ";
		char tcNumber[32];
		sprintf(tcNumber, "%d/%ld", iLocation, _pOutputStream->size());
		sMessage += tcNumber;
		throw UtlException(sMessage);
	}
	return NO_INTERRUPTION;
//##protect##"insertText"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::insertTextOnce(int iLocation, const std::string& sText) {
//##protect##"insertTextOnce"
	if (!_pOutputStream->insertTextOnce(sText, iLocation, 0)) {
		if ((iLocation < 0) || (iLocation > _pOutputStream->size())) {
			std::string sMessage = "unable to insert \"" + composeCLikeString(sText) + "\" at position ";
			char tcNumber[32];
			sprintf(tcNumber, "%d/%ld", iLocation, _pOutputStream->size());
			sMessage += tcNumber;
			throw UtlException(sMessage);
		}
	}
	return NO_INTERRUPTION;
//##protect##"insertTextOnce"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::insertTextToFloatingLocation(const std::string& sLocation, const std::string& sText) {
//##protect##"insertTextToFloatingLocation"
	ScpStream* pOwner;
	int iLocation = _pOutputStream->getFloatingLocation(sLocation, pOwner);
	if (pOwner == NULL) throw UtlException("the floating location '" + sLocation + "' doesn't exist");
	if (!pOwner->insertText(sText, iLocation, 0)) {
		std::string sMessage = "unable to insert \"" + composeCLikeString(sText) + "\" at position ";
		char tcNumber[32];
		sprintf(tcNumber, "%d/%ld", iLocation, pOwner->size());
		sMessage += tcNumber;
		throw UtlException(sMessage);
	}
	return NO_INTERRUPTION;
//##protect##"insertTextToFloatingLocation"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::insertTextOnceToFloatingLocation(const std::string& sLocation, const std::string& sText) {
//##protect##"insertTextOnceToFloatingLocation"
	ScpStream* pOwner;
	int iLocation = _pOutputStream->getFloatingLocation(sLocation, pOwner);
	if (pOwner == NULL) throw UtlException("the floating location '" + sLocation + "' doesn't exist");
	if (!pOwner->insertTextOnce(sText, iLocation, 0)) {
		if ((iLocation < 0) || (iLocation > pOwner->size())) {
			std::string sMessage = "unable to insert \"" + composeCLikeString(sText) + "\" at position ";
			char tcNumber[32];
			sprintf(tcNumber, "%d/%ld", iLocation, pOwner->size());
			sMessage += tcNumber;
			throw UtlException(sMessage);
		}
	}
	return NO_INTERRUPTION;
//##protect##"insertTextOnceToFloatingLocation"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::overwritePortion(int iLocation, const std::string& sText, int iSize) {
//##protect##"overwritePortion"
	if (!_pOutputStream->insertText(sText, iLocation, iSize)) {
		std::string sMessage = "unable to overwrite \"" + composeCLikeString(sText) + "\" at position ";
		char tcNumber[32];
		sprintf(tcNumber, "%d/%ld", iLocation, _pOutputStream->size());
		sMessage += tcNumber;
		throw UtlException(sMessage);
	}
	return NO_INTERRUPTION;
//##protect##"overwritePortion"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::populateProtectedArea(const std::string& sProtectedAreaName, const std::string& sContent) {
//##protect##"populateProtectedArea"
	_listOfPatternScripts.front()->populateProtectedArea(sProtectedAreaName, sContent);
	return NO_INTERRUPTION;
//##protect##"populateProtectedArea"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::resizeOutputStream(int iNewSize) {
//##protect##"resizeOutputStream"
	_pOutputStream->resize(iNewSize);
	return NO_INTERRUPTION;
//##protect##"resizeOutputStream"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setFloatingLocation(const std::string& sKey, int iLocation) {
//##protect##"setFloatingLocation"
	_pOutputStream->setFloatingLocation(sKey, iLocation);
	return NO_INTERRUPTION;
//##protect##"setFloatingLocation"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setOutputLocation(int iLocation) {
//##protect##"setOutputLocation"
	if (iLocation == -1) _pOutputStream->setOutputLocation(_pOutputStream->size());
	else _pOutputStream->setOutputLocation(iLocation);
	return NO_INTERRUPTION;
//##protect##"setOutputLocation"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::setProtectedArea(const std::string& sProtectedAreaName) {
//##protect##"setProtectedArea"
	_listOfPatternScripts.front()->setProtectedArea(sProtectedAreaName);
	return NO_INTERRUPTION;
//##protect##"setProtectedArea"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::writeBytes(const std::string& sBytes) {
//##protect##"writeBytes"
	int iLength = sBytes.size();
	if ((iLength % 2) != 0) throw UtlException("writeBytes(\"" + sBytes + "\"): a binary content cannot have an odd length");
	iLength >>= 1;
	unsigned char* tcBinary = new unsigned char[iLength];
	if (!convertBytesToChars(sBytes, tcBinary, iLength)) {
		delete [] tcBinary;
		throw UtlException("writeBytes(...): about the binary content, invalid hexadecimal digit encountered");
	}
	_pOutputStream->writeBinaryData((const char*) tcBinary, iLength);
	delete [] tcBinary;
	return NO_INTERRUPTION;
//##protect##"writeBytes"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::writeText(const std::string& sText) {
//##protect##"writeText"
	_pOutputStream->writeBinaryData(sText.c_str(), sText.size());
	return NO_INTERRUPTION;
//##protect##"writeText"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::writeTextOnce(const std::string& sText) {
//##protect##"writeTextOnce"
	_pOutputStream->writeTextOnce(sText.c_str());
	return NO_INTERRUPTION;
//##protect##"writeTextOnce"
}

SEQUENCE_INTERRUPTION_LIST CGRuntime::closeSocket(int iSocket) {
//##protect##"closeSocket"
	NetSocket::closeSocket(iSocket);
	return NO_INTERRUPTION;
//##protect##"closeSocket"
}

bool CGRuntime::flushOutputToSocket(int iSocket) {
//##protect##"flushOutputToSocket"
	return NetSocket::flushOutputToSocket(iSocket);
//##protect##"flushOutputToSocket"
}

int CGRuntime::acceptSocket(int iServerSocket) {
//##protect##"acceptSocket"
	return NetSocket::acceptSocket(iServerSocket);
//##protect##"acceptSocket"
}

double CGRuntime::add(double dLeft, double dRight) {
//##protect##"add"
	return dLeft + dRight;
//##protect##"add"
}

std::string CGRuntime::addToDate(const std::string& sDate, const std::string& sFormat, const std::string& sShifting) {
//##protect##"addToDate"
	UtlDate theDate = UtlDate::getDateFromFormat(sDate, "%d%b%Y%| %H:%M:%S%|.%L");
	theDate.addDateFromFormat(sFormat, sShifting);
	if (theDate.getMillis() > 0) {
		return theDate.getFormattedDate("%d%b%Y %H:%M:%S.%L");
	}
	if ((theDate.getHour() == 0) && (theDate.getMin() == 0) && (theDate.getSec() == 0)) {
		return theDate.getFormattedDate("%d%b%Y");
	}
	return theDate.getFormattedDate("%d%b%Y %H:%M:%S");
//##protect##"addToDate"
}

std::string CGRuntime::byteToChar(const std::string& sByte) {
//##protect##"byteToChar"
	if (sByte.size() != 2) throw UtlException("'" + composeCLikeString(sByte) + "' isn't recognized as a byte");
	unsigned char c;
	char a = sByte[0];
	if (a <= '9') {
		if (a < '0') throw UtlException("byteToChar(\"" + sByte + "\"): invalid hexadecimal digit '" + composeCLikeString(std::string(1, a)) + "' encountered");
		c = a - '0';
	} else if (a <= 'F') {
		if (a < 'A') throw UtlException("byteToChar(\"" + sByte + "\"): invalid hexadecimal digit '" + std::string(1, a) + "' encountered");
		c = (a - 'A') + 10;
	} else if ((a >= 'a') && (a <= 'f')) {
		c = (a - 'a') + 10;
	} else {
		throw UtlException("byteToChar((\"" + sByte + "\"): invalid hexadecimal digit '" + std::string(1, a) + "' encountered");
	}
	c <<= 4;
	a = sByte[1];
	if (a <= '9') {
		if (a < '0') throw UtlException("byteToChar(\"" + sByte + "\"): invalid hexadecimal digit '" + composeCLikeString(std::string(1, a)) + "' encountered");
		c += a - '0';
	} else if (a <= 'F') {
		if (a < 'A') throw UtlException("byteToChar(\"" + sByte + "\"): invalid hexadecimal digit '" + std::string(1, a) + "' encountered");
		c += (a - 'A') + 10;
	} else if ((a >= 'a') && (a <= 'f')) {
		c += (a - 'a') + 10;
	} else {
		throw UtlException("byteToChar((\"" + sByte + "\"): invalid hexadecimal digit '" + std::string(1, a) + "' encountered");
	}
	if (c == '\0') return "";
	return std::string(1, (char) c);
//##protect##"byteToChar"
}

unsigned long CGRuntime::bytesToLong(const std::string& sBytes) {
//##protect##"bytesToLong"
	if (sBytes.size() != 8) throw UtlException("'" + composeCLikeString(sBytes) + "' isn't recognized as a 4-bytes sequence");
	unsigned long iLong;
	if (!convertBytesToChars(sBytes, (unsigned char*) &iLong, 4)) throw UtlException("invalid hexadecimal digit in the sequence of bytes '" + composeCLikeString(sBytes) + "'");
	return iLong;
//##protect##"bytesToLong"
}

unsigned short CGRuntime::bytesToShort(const std::string& sBytes) {
//##protect##"bytesToShort"
	if (sBytes.size() != 4) throw UtlException("'" + composeCLikeString(sBytes) + "' isn't recognized as a 4-bytes sequence");
	unsigned short iShort;
	if (!convertBytesToChars(sBytes, (unsigned char*) &iShort, 2)) throw UtlException("invalid hexadecimal digit in the sequence of bytes '" + composeCLikeString(sBytes) + "'");
	return iShort;
//##protect##"bytesToShort"
}

std::string CGRuntime::canonizePath(const std::string& sPath) {
//##protect##"canonizePath"
	char tcReference[MAX_PATH];
	std::string sDirectory = getCurrentDirectory();
	strcpy(tcReference, sDirectory.c_str());
	toSlashes_(tcReference);
	return canonize_(sPath.c_str(), tcReference);
//##protect##"canonizePath"
}

bool CGRuntime::changeDirectory(const std::string& sPath) {
//##protect##"changeDirectory"
#ifdef WIN32
	return (_chdir(sPath.c_str()) == 0);
#else
	return (chdir(sPath.c_str()) == 0);
#endif
//##protect##"changeDirectory"
}

int CGRuntime::changeFileTime(const std::string& sFilename, const std::string& sAccessTime, const std::string& sModificationTime) {
//##protect##"changeFileTime"
	UtlDate theAccessTime = UtlDate::getDateFromFormat(sAccessTime, "%d%b%Y%| %H:%M:%S%|.%L");
	UtlDate theModificationTime = UtlDate::getDateFromFormat(sModificationTime, "%d%b%Y%| %H:%M:%S%|.%L");
	struct utimbuf theTimes;
	struct tm theDate;
	theDate.tm_mday = theAccessTime.getDay();
	theDate.tm_mon  = theAccessTime.getMonth() - 1;
	theDate.tm_year = theAccessTime.getYear() - 1900;
	theDate.tm_hour = theAccessTime.getHour();
	theDate.tm_min  = theAccessTime.getMin();
	theDate.tm_sec  = theAccessTime.getSec();
	theTimes.actime  = mktime(&theDate);
	theDate.tm_mday = theModificationTime.getDay();
	theDate.tm_mon  = theModificationTime.getMonth() - 1;
	theDate.tm_year = theModificationTime.getYear() - 1900;
	theDate.tm_hour = theModificationTime.getHour();
	theDate.tm_min  = theModificationTime.getMin();
	theDate.tm_sec  = theModificationTime.getSec();
	theTimes.modtime = mktime(&theDate);
	if (utime(sFilename.c_str(), &theTimes) == -1) {
		switch(errno) {
			case EACCES: return -2;
			case EMFILE: return -3;
			case ENOENT: return -4;
			case EINVAL: return -5;
			default: return -1;
		}
	}
	return 0;
//##protect##"changeFileTime"
}

std::string CGRuntime::charAt(const std::string& sText, int iIndex) {
//##protect##"charAt"
	if ((iIndex < 0) || (iIndex >= (int) sText.size())) return "";
	std::string sResult(1, sText[iIndex]);
	return sResult;
//##protect##"charAt"
}

std::string CGRuntime::charToByte(const std::string& sChar) {
//##protect##"charToByte"
	if (sChar.size() > 1) throw UtlException("charToByte('" + composeCLikeString(sChar) + "'): the parameter is bigger than a character");
	unsigned char c = sChar[0];
	char tcContent[] = {0, 0, 0};
	tcContent[0] = _tcHexa[c >> 4];
	tcContent[1] = _tcHexa[c & 0x0F];
	return tcContent;
//##protect##"charToByte"
}

int CGRuntime::charToInt(const std::string& sChar) {
//##protect##"charToInt"
	if (sChar.size() != 1) return 0;
	return (int) ((unsigned char) (sChar[0]));
//##protect##"charToInt"
}

bool CGRuntime::chmod(const std::string& sFilename, const std::string& sMode) {
//##protect##"chmod"
	int iMode = 0;
	for (std::string::size_type i = 0; i < sMode.size(); i++) {
		char a = sMode[i];
#ifdef WIN32
		if ((a == 'R') || (a == 'r')) iMode = iMode | _S_IREAD;
		else if ((a == 'W') || (a == 'w')) iMode = iMode | _S_IWRITE;
		else if ((a == 'X') || (a == 'x')) iMode = iMode | _S_IEXEC;
#else
		if ((a == 'R') || (a == 'r')) iMode = iMode | S_IRUSR;
		else if ((a == 'W') || (a == 'w')) iMode = iMode | S_IWUSR;
		else if ((a == 'X') || (a == 'x')) iMode = iMode | S_IXUSR;
#endif
		else throw UtlException("invalid access mode '" + sMode + "' encountered by function 'chmod'");
	}
	if (iMode != 0) return (::chmod(sFilename.c_str(), iMode) == 0);
	return true;
//##protect##"chmod"
}

int CGRuntime::ceil(double dNumber) {
//##protect##"ceil"
	return (int) ::ceil(dNumber);
//##protect##"ceil"
}

int CGRuntime::compareDate(const std::string& sDate1, const std::string& sDate2) {
//##protect##"compareDate"
	UtlDate theDate1 = UtlDate::getDateFromFormat(sDate1, "%d%b%Y%| %H:%M:%S%|.%L");
	UtlDate theDate2 = UtlDate::getDateFromFormat(sDate2, "%d%b%Y%| %H:%M:%S%|.%L");
	if (theDate1 < theDate2) return -1;
	if (theDate1 > theDate2) return 1;
	return 0;
//##protect##"compareDate"
}

std::string CGRuntime::completeDate(const std::string& sDate, const std::string& sFormat) {
//##protect##"completeDate"
	UtlDate theDate = UtlDate::getDateFromFormat(sDate, sFormat);
	if (theDate.getMillis() > 0) {
		return theDate.getFormattedDate("%d%b%Y %H:%M:%S.%L");
	}
	if ((theDate.getHour() == 0) && (theDate.getMin() == 0) && (theDate.getSec() == 0)) {
		return theDate.getFormattedDate("%d%b%Y");
	}
	return theDate.getFormattedDate("%d%b%Y %H:%M:%S");
//##protect##"completeDate"
}

std::string CGRuntime::completeLeftSpaces(const std::string& sText, int iLength) {
//##protect##"completeLeftSpaces"
	if ((int) sText.size() >= iLength) return sText;
	std::string sSpaces(iLength - sText.size(), ' ');
	return sSpaces + sText;
//##protect##"completeLeftSpaces"
}

std::string CGRuntime::completeRightSpaces(const std::string& sText, int iLength) {
//##protect##"completeRightSpaces"
	if ((int) sText.size() >= iLength) return sText;
	std::string sSpaces(iLength - sText.size(), ' ');
	return sText + sSpaces;
//##protect##"completeRightSpaces"
}

std::string CGRuntime::composeAdaLikeString(const std::string& sText) {
//##protect##"composeAdaLikeString"
	std::string sResult;
	char* u = (char*) sText.c_str();
	while (*u != '\0') {
		if (*u == '"') {
			sResult += "\"\"";
		} else {
			sResult += *u;
		}
		u++;
	}
	return sResult;
//##protect##"composeAdaLikeString"
}

std::string CGRuntime::composeCLikeString(const std::string& sText) {
//##protect##"composeCLikeString"
	std::string sResult;
	char* u = (char*) sText.c_str();
	while (*u != '\0') {
		switch(*u) {
			case '\\': sResult += "\\\\";break;
			case '\"': sResult += "\\\"";break;
			case '\a': sResult += "\\a";break;
			case '\b': sResult += "\\b";break;
			case '\f': sResult += "\\f";break;
			case '\n': sResult += "\\n";break;
			case '\r': sResult += "\\r";break;
			case '\t': sResult += "\\t";break;
			case '\v': sResult += "\\v";break;
			default:
				sResult += *u;
		}
		u++;
	}
	return sResult;
//##protect##"composeCLikeString"
}

std::string CGRuntime::composeHTMLLikeString(const std::string& sText) {
//##protect##"composeHTMLLikeString"
	static std::map<char, std::string> mapOfCodes;
	if (mapOfCodes.empty()) {
		mapOfCodes['\''] = "&#39;";
		mapOfCodes['«'] = "&#147;";
		mapOfCodes['»'] = "&#148;";
//		mapOfCodes['-'] = "&#150;";
		mapOfCodes['°'] = "&#176;";

		mapOfCodes['à'] = "&agrave;";
		mapOfCodes['â'] = "&acirc;";
		mapOfCodes['å'] = "&aring;";
		mapOfCodes['ä'] = "&auml;";
		mapOfCodes['ã'] = "&atilde;";

		mapOfCodes['é'] = "&eacute;";
		mapOfCodes['è'] = "&egrave;";
		mapOfCodes['ê'] = "&ecirc;";
		mapOfCodes['ë'] = "&euml;";
		
		mapOfCodes['ì'] = "&igrave;";
		mapOfCodes['í'] = "&iacute;";
		mapOfCodes['î'] = "&icirc;";
		mapOfCodes['ï'] = "&iuml;";

		mapOfCodes['ô'] = "&ocirc;";
		mapOfCodes['Ø'] = "&Oslash;";
		mapOfCodes['ó'] = "&oacute;";
		mapOfCodes['ö'] = "&ouml;";

		mapOfCodes['û'] = "&ucirc;";
		mapOfCodes['ù'] = "&ugrave;";
		mapOfCodes['ü'] = "&uuml;";

		mapOfCodes['ÿ'] = "&yuml;";

		mapOfCodes['ç'] = "&ccedil;";
		mapOfCodes['ñ'] = "&ntilde;";
		mapOfCodes['ß'] = "&szlig;";
		mapOfCodes['Þ'] = "&THORN;";

		mapOfCodes['&'] = "&amp;";
		mapOfCodes['\"'] = "&quot;";
		mapOfCodes['<'] = "&lt;";
		mapOfCodes['>'] = "&gt;";
		mapOfCodes['¨'] = "&uml;";
		mapOfCodes['¢'] = "&cent;";
		mapOfCodes['£'] = "&pound;";
		mapOfCodes['©'] = "&copy;";
	}
	std::string sHTMLText;
	std::map<char, std::string>::iterator cursor;
	for (std::string::size_type i = 0; i < sText.size(); i++) {
		char a = sText[i];
		cursor = mapOfCodes.find(a);
		if (cursor != mapOfCodes.end()) sHTMLText += cursor->second;
		else sHTMLText += a;
	}
	return sHTMLText;
//##protect##"composeHTMLLikeString"
}

std::string CGRuntime::composeSQLLikeString(const std::string& sText) {
//##protect##"composeSQLLikeString"
	std::string sResult = composeCLikeString(sText);
	std::string::size_type iLastPos = 0;
	std::string::size_type iPos = sResult.find('\'');
	std::string sReplace;
	while (iPos != std::string::npos) {
		iPos++;
		if (iPos == sResult.size()) sReplace += sResult.substr(iLastPos);
		else sReplace += sResult.substr(iLastPos, iPos - iLastPos);
		sReplace += "'";
		iLastPos = iPos;
		iPos = sResult.find('\'', iLastPos);
	}
	if (iLastPos != sResult.size()) sReplace += sResult.substr(iLastPos);
	return sReplace;
//##protect##"composeSQLLikeString"
}

std::string CGRuntime::computeMD5(const std::string& sText) {
//##protect##"computeMD5"
	md5_context ctx;
	unsigned char tcMD5sum[16];
	md5_starts(&ctx);
	md5_update(&ctx, (unsigned char *) sText.c_str(), sText.size());
	md5_finish(&ctx, tcMD5sum);

	char tcResult[33];
	for(int i = 0; i < 16; ++i)
		sprintf(tcResult + i * 2, "%02X", tcMD5sum[i]);

	return tcResult;
//##protect##"computeMD5"
}

bool CGRuntime::copySmartFile(const std::string& sSourceFileName, const std::string& sDestinationFileName) {
//##protect##"copySmartFile"
	std::auto_ptr<ScpStream> pInput(new ScpStream(sSourceFileName, ScpStream::IN | ScpStream::PATH));
	std::auto_ptr<ScpStream> pOutput(new ScpStream(sDestinationFileName, ScpStream::IN | ScpStream::OUT));
	int iPosition;
	bool bCopy = !DtaScript::equalsIgnoringGenerationHeader(*pInput, *pOutput, iPosition);
	if (bCopy) pInput->saveIntoFile(sDestinationFileName, true);
	return bCopy;
//##protect##"copySmartFile"
}

std::string CGRuntime::coreString(const std::string& sText, int iPos, int iLastRemoved) {
//##protect##"coreString"
	register int iSum = iPos + iLastRemoved;
	if ((iPos < 0) || (iLastRemoved < 0) || (iSum >= (int) sText.size())) return "";
	if (iLastRemoved == 0) return sText.substr(iPos);
	return sText.substr(iPos, ((int) sText.size()) - iSum);
//##protect##"coreString"
}

int CGRuntime::countStringOccurences(const std::string& sString, const std::string& sText) {
//##protect##"countStringOccurences"
	std::string::size_type iPos = sString.find(sText);
	int iOccurences = 0;
	while (iPos != std::string::npos) {
		iOccurences++;
		iPos = sString.find(sText, iPos + 1);
	}
	return iOccurences;
//##protect##"countStringOccurences"
}

bool CGRuntime::createDirectory(const std::string& sPath) {
//##protect##"createDirectory"
	if (sPath.empty()) return false;
	char a = sPath[sPath.size() - 1];
	std::string sTemp = sPath;
	if ((a != '\\') && (a != '/')) sTemp += "/empty.dir";
	else sTemp += "empty.dir";
	return createDirectoriesForFile(sTemp);
//##protect##"createDirectory"
}

int CGRuntime::createINETClientSocket(const std::string& sRemoteAddress, int iPort) {
//##protect##"createINETClientSocket"
	return NetSocket::createINETClientSocket(sRemoteAddress.c_str(), iPort);
//##protect##"createINETClientSocket"
}

int CGRuntime::createINETServerSocket(int iPort, int iBackLog) {
//##protect##"createINETServerSocket"
	return NetSocket::createINETServerSocket(iPort, iBackLog);
//##protect##"createINETServerSocket"
}

bool CGRuntime::createIterator(DtaScriptVariable* pI, DtaScriptVariable* pList) {
//##protect##"createIterator"
	if (pList->getArray() == NULL) return false;
	pI->setValue(new DtaListIterator(*pList->getArray()));
	return true;
//##protect##"createIterator"
}

bool CGRuntime::createIterator(CppParsingTree_var pI, CppParsingTree_var pList) {
	bool result = createIterator(pI._pInternalNode, pList._pInternalNode);
	return result;
}

bool CGRuntime::createReverseIterator(DtaScriptVariable* pI, DtaScriptVariable* pList) {
//##protect##"createReverseIterator"
	if (pList->getArray() == NULL) return false;
	pI->setValue(new DtaReverseListIterator(*pList->getArray()));
	return true;
//##protect##"createReverseIterator"
}

bool CGRuntime::createReverseIterator(CppParsingTree_var pI, CppParsingTree_var pList) {
	bool result = createReverseIterator(pI._pInternalNode, pList._pInternalNode);
	return result;
}

bool CGRuntime::createVirtualFile(const std::string& sHandle, const std::string& sContent) {
//##protect##"createVirtualFile"
	return ScpStream::createVirtualFile(sHandle, sContent);
//##protect##"createVirtualFile"
}

std::string CGRuntime::createVirtualTemporaryFile(const std::string& sContent) {
//##protect##"createVirtualTemporaryFile"
	return ScpStream::createVirtualTemporaryFile(sContent);
//##protect##"createVirtualTemporaryFile"
}

std::string CGRuntime::decodeURL(const std::string& sURL) {
//##protect##"decodeURL"
	std::string sResult;
	for (std::string::size_type i = 0; i < sURL.size(); i++) {
		char a = (unsigned char) sURL[i];
		if (a == '%') {
			unsigned char c;
			i++;
			a = (unsigned char) sURL[i];
			if (a <= '9') c = (a - '0') << 4;
			else if (a <= 'F') c = (a - ('A' - (char) 10)) << 4;
			else c = (a - ('a' - (char) 10)) << 4;
			i++;
			a = (unsigned char) sURL[i];
			if (a <= '9') c += (a - '0');
			else if (a <= 'F') c += (a - ('A' - (char) 10));
			else c += (a - ('a' - (char) 10));
			sResult += c;
		} else if (a == '+') {
			sResult += ' ';
		} else {
			sResult += a;
		}
	}
	return sResult;
//##protect##"decodeURL"
}

double CGRuntime::decrement(double& dNumber) {
//##protect##"decrement"
	dNumber = dNumber - 1.0;
	return dNumber;
//##protect##"decrement"
}

double CGRuntime::decrement(CppParsingTree_var pNumber) {
	double dNumber = pNumber.getDoubleValue();
	double result = decrement(dNumber);
	pNumber.setValue(dNumber);
	return result;
}

bool CGRuntime::deleteFile(const std::string& sFilename) {
//##protect##"deleteFile"
	return (unlink(sFilename.c_str()) == 0);
//##protect##"deleteFile"
}

bool CGRuntime::deleteVirtualFile(const std::string& sHandle) {
//##protect##"deleteVirtualFile"
	return ScpStream::deleteVirtualFile(sHandle);
//##protect##"deleteVirtualFile"
}

double CGRuntime::div(double dDividend, double dDivisor) {
//##protect##"div"
	return dDividend / dDivisor;
//##protect##"div"
}

bool CGRuntime::duplicateIterator(DtaScriptVariable* pOldIt, DtaScriptVariable* pNewIt) {
//##protect##"duplicateIterator"
	if (pOldIt->getIteratorData() == NULL) return false;
	pNewIt->setValue(pOldIt->getIteratorData()->clone());
	return true;
//##protect##"duplicateIterator"
}

bool CGRuntime::duplicateIterator(CppParsingTree_var pOldIt, CppParsingTree_var pNewIt) {
	bool result = duplicateIterator(pOldIt._pInternalNode, pNewIt._pInternalNode);
	return result;
}

std::string CGRuntime::encodeURL(const std::string& sURL) {
//##protect##"encodeURL"
	std::string sResult;
	char percentChar[4];
	percentChar[0] = '%';
	percentChar[3] = '\0';
	for (std::string::size_type i = 0; i < sURL.size(); i++) {
		unsigned char a = (unsigned char) (sURL[i]);
		if (((a < 'a') || (a > 'z')) && ((a < 'A') || (a > 'Z')) && ((a < '0') || (a > '9'))) {
			percentChar[1] = _tcHexa[a >> 4];
			percentChar[2] = _tcHexa[a & 0x0F];
			sResult += percentChar;
		} else {
			sResult += (char) a;
		}
	}
	return sResult;
//##protect##"encodeURL"
}

std::string CGRuntime::endl() {
//##protect##"endl"
	return ScpStream::ENDL;
//##protect##"endl"
}

bool CGRuntime::endString(const std::string& sText, const std::string& sEnd) {
//##protect##"endString"
	int iPos = sText.size() - sEnd.size();
	if (iPos >= 0) {
		if (strncmp(sText.c_str() + iPos, sEnd.c_str(), sEnd.size()) == 0) return true;
	}
	return false;
//##protect##"endString"
}

bool CGRuntime::equal(double dLeft, double dRight) {
//##protect##"equal"
	return dLeft == dRight;
//##protect##"equal"
}

bool CGRuntime::equalsIgnoreCase(const std::string& sLeft, const std::string& sRight) {
//##protect##"equalsIgnoreCase"
	return (stricmp(sLeft.c_str(), sRight.c_str()) == 0);
//##protect##"equalsIgnoreCase"
}

bool CGRuntime::equalTrees(DtaScriptVariable* pFirstTree, DtaScriptVariable* pSecondTree) {
//##protect##"equalTrees"
	if (pFirstTree == NULL) {
		throw UtlException("the first tree parameter passed to the procedure 'equalTrees' doesn't exist");
	}
	if (pSecondTree == NULL) {
		throw UtlException("the second tree parameter passed to the procedure 'equalTrees' doesn't exist");
	}
	return pFirstTree->equal(*pSecondTree);
//##protect##"equalTrees"
}

bool CGRuntime::equalTrees(CppParsingTree_var pFirstTree, CppParsingTree_var pSecondTree) {
	bool result = equalTrees(pFirstTree._pInternalNode, pSecondTree._pInternalNode);
	return result;
}

std::string CGRuntime::executeStringQuiet(DtaScriptVariable* pThis, const std::string& sCommand) {
//##protect##"executeStringQuiet"
	CGQuietOutput quiet;
	ScpStream theCommand(sCommand);
	DtaScript script/*(pThis)*/;
	script.parseStream(theCommand);
	script.execute(*pThis);
	return quiet.getOutput();
//##protect##"executeStringQuiet"
}

std::string CGRuntime::executeStringQuiet(CppParsingTree_var pThis, const std::string& sCommand) {
	std::string result = executeStringQuiet(pThis._pInternalNode, sCommand);
	return result;
}

bool CGRuntime::existDirectory(const std::string& sPath) {
//##protect##"existDirectory"
	if (access(sPath.c_str(), 0) == 0) {
		struct stat status;
		stat(sPath.c_str(), &status);
		return (status.st_mode & S_IFDIR) != 0;
	}
	return false;
//##protect##"existDirectory"
}

bool CGRuntime::existEnv(const std::string& sVariable) {
//##protect##"existEnv"
	const char* tcVariable = ::getenv(sVariable.c_str());
	return (tcVariable != NULL);
//##protect##"existEnv"
}

bool CGRuntime::existFile(const std::string& sFileName) {
//##protect##"existFile"
	std::string sCompleteFileName;
	std::ifstream* pStream = openInputFileFromIncludePath(sFileName.c_str(), sCompleteFileName);
	if (pStream != NULL) {
		pStream->close();
		delete pStream;
		return true;
	}
	return false;
//##protect##"existFile"
}

bool CGRuntime::existVirtualFile(const std::string& sHandle) {
//##protect##"existVirtualFile"
	return ScpStream::existVirtualFile(sHandle);
//##protect##"existVirtualFile"
}

bool CGRuntime::existVariable(DtaScriptVariable* pVariable) {
//##protect##"existVariable"
	return (pVariable != NULL);
//##protect##"existVariable"
}

bool CGRuntime::existVariable(CppParsingTree_var pVariable) {
	bool result = existVariable(pVariable._pInternalNode);
	return result;
}

double CGRuntime::exp(double dX) {
//##protect##"exp"
	return ::exp(dX);
//##protect##"exp"
}

bool CGRuntime::exploreDirectory(DtaScriptVariable* pDirectory, const std::string& sPath, bool bSubfolders) {
//##protect##"exploreDirectory"
	UtlDirectory theDirectory(sPath);
	bool bSuccess;
	if (bSubfolders) bSuccess = theDirectory.scanRecursively();
	else bSuccess = theDirectory.scan();
	if (bSuccess) {
		pDirectory->clearContent();
		populateDirectory(pDirectory, theDirectory, bSubfolders);
	}
	return bSuccess;
//##protect##"exploreDirectory"
}

bool CGRuntime::exploreDirectory(CppParsingTree_var pDirectory, const std::string& sPath, bool bSubfolders) {
	bool result = exploreDirectory(pDirectory._pInternalNode, sPath, bSubfolders);
	return result;
}

std::string CGRuntime::extractGenerationHeader(const std::string& sFilename, std::string& sGenerator, std::string& sVersion, std::string& sDate) {
//##protect##"extractGenerationHeader"
	std::string sComment;
	std::string sCompleteFileName;
	ScpStream* pStream = ScpStream::openInputFileFromIncludePath(sFilename.c_str(), sCompleteFileName);
	if (pStream != NULL) {
		extractGenerationHeader(*pStream, sGenerator, sVersion, sDate, sComment);
		delete pStream;
	}
	return sComment;
//##protect##"extractGenerationHeader"
}

std::string CGRuntime::extractGenerationHeader(const std::string& sFilename, CppParsingTree_var pGenerator, CppParsingTree_var pVersion, CppParsingTree_var pDate) {
	std::string sGenerator = pGenerator.getValue();
	std::string sVersion = pVersion.getValue();
	std::string sDate = pDate.getValue();
	std::string result = extractGenerationHeader(sFilename, sGenerator, sVersion, sDate);
	pGenerator.setValue(sGenerator);
	pVersion.setValue(sVersion);
	pDate.setValue(sDate);
	return result;
}

std::string CGRuntime::fileCreation(const std::string& sFilename) {
//##protect##"fileCreation"
	struct stat theStat;
	int iFileHandle = open(sFilename.c_str(), _O_RDONLY);
	if (iFileHandle == -1) {
		switch(errno) {
			case EACCES:
				return "-2";
			case EMFILE:
				return "-3";
			case ENOENT:
				return "-4";
			default: return "-1";
		}
	}
	fstat(iFileHandle, &theStat);
	close(iFileHandle);
	const char* tcTime = ctime(&theStat.st_ctime);
	std::string sTemp(tcTime);
	std::string sTime = sTemp.substr(8, 2) + sTemp.substr(4, 3) + sTemp.substr(20, 4) + " " + sTemp.substr(11, 8);
	if (sTime[2] <= 'Z') sTime[2] += ' ';
	return sTime;
//##protect##"fileCreation"
}

std::string CGRuntime::fileLastAccess(const std::string& sFilename) {
//##protect##"fileLastAccess"
	struct stat theStat;
	int iFileHandle = open(sFilename.c_str(), _O_RDONLY);
	if (iFileHandle == -1) {
		switch(errno) {
			case EACCES:
				return "-2";
			case EMFILE:
				return "-3";
			case ENOENT:
				return "-4";
			default: return "-1";
		}
	}
	fstat(iFileHandle, &theStat);
	close(iFileHandle);
	const char* tcTime = ctime(&theStat.st_atime);
	std::string sTemp(tcTime);
	std::string sTime = sTemp.substr(8, 2) + sTemp.substr(4, 3) + sTemp.substr(20, 4) + " " + sTemp.substr(11, 8);
	if (sTime[2] <= 'Z') sTime[2] += ' ';
	return sTime;
//##protect##"fileLastAccess"
}

std::string CGRuntime::fileLastModification(const std::string& sFilename) {
//##protect##"fileLastModification"
	struct stat theStat;
	int iFileHandle = open(sFilename.c_str(), _O_RDONLY);
	if (iFileHandle == -1) {
		switch(errno) {
			case EACCES:
				return "-2";
			case EMFILE:
				return "-3";
			case ENOENT:
				return "-4";
			default: return "-1";
		}
	}
	fstat(iFileHandle, &theStat);
	close(iFileHandle);
	const char* tcTime = ctime(&theStat.st_mtime);
	std::string sTemp(tcTime);
	std::string sTime = sTemp.substr(8, 2) + sTemp.substr(4, 3) + sTemp.substr(20, 4) + " " + sTemp.substr(11, 8);
	if (sTime[2] <= 'Z') sTime[2] += ' ';
	return sTime;
//##protect##"fileLastModification"
}

int CGRuntime::fileLines(const std::string& sFilename) {
//##protect##"fileLines"
	std::string sCompleteFileName;
	std::auto_ptr<ScpStream> pStream(ScpStream::openInputFileFromIncludePath(sFilename.c_str(), sCompleteFileName));
	if (pStream.get() != NULL) {
		pStream->setInputLocation(pStream->size());
		return pStream->getLineCount();
	}
	return -1;
//##protect##"fileLines"
}

std::string CGRuntime::fileMode(const std::string& sFilename) {
//##protect##"fileMode"
	struct stat theStat;
	int iFileHandle = open(sFilename.c_str(), _O_RDONLY);
	if (iFileHandle == -1) {
		switch(errno) {
			case EACCES:
				return "-2";
			case EMFILE:
				return "-3";
			case ENOENT:
				return "-4";
			default: return "-1";
		}
	}
	fstat(iFileHandle, &theStat);
	close(iFileHandle);
	std::string sMode;
#ifdef WIN32
	if (theStat.st_mode & _S_IREAD)  sMode  = "R";
	if (theStat.st_mode & _S_IWRITE) sMode += "W";
	if (theStat.st_mode & _S_IEXEC)  sMode += "X";
#else
	if (theStat.st_mode & S_IRUSR) sMode  = "R";
	if (theStat.st_mode & S_IWUSR) sMode += "W";
	if (theStat.st_mode & S_IXUSR) sMode += "X";
#endif
	return sMode;
//##protect##"fileMode"
}

int CGRuntime::fileSize(const std::string& sFilename) {
//##protect##"fileSize"
	struct stat theStat;
	int iFileHandle = open(sFilename.c_str(), _O_RDONLY);
	if (iFileHandle == -1) {
		switch(errno) {
			case EACCES:
				return -2;
			case EMFILE:
				return -3;
			case ENOENT:
				return -4;
			default: return -1;
		}
	}
	fstat(iFileHandle, &theStat);
	close(iFileHandle);
	return theStat.st_size;
//##protect##"fileSize"
}

bool CGRuntime::findElement(const std::string& sValue, DtaScriptVariable* pVariable) {
//##protect##"findElement"
	return ((pVariable != NULL) && (pVariable->getArrayElement(sValue) != NULL));
//##protect##"findElement"
}

bool CGRuntime::findElement(const std::string& sValue, CppParsingTree_var pVariable) {
	bool result = findElement(sValue, pVariable._pInternalNode);
	return result;
}

int CGRuntime::findFirstChar(const std::string& sText, const std::string& sSomeChars) {
//##protect##"findFirstChar"
	std::string::size_type iFirstPos = std::string::npos;
	for (std::string::size_type i = 0; i < sSomeChars.size(); i++) {
		std::string::size_type iPos = sText.find(sSomeChars[i]);
		if ((iPos != std::string::npos) && ((iFirstPos == std::string::npos) || (iFirstPos > iPos))) iFirstPos = iPos;
	}
	if (iFirstPos == std::string::npos) return -1;
	return iFirstPos;
//##protect##"findFirstChar"
}

int CGRuntime::findFirstSubstringIntoKeys(const std::string& sSubstring, DtaScriptVariable* pArray) {
//##protect##"findFirstSubstringIntoKeys"
	if (pArray != NULL) {
		const std::list<DtaScriptVariable*>* listOfElements = pArray->getArray();
		if (listOfElements != NULL) {
			int iIndex = 0;
			for (std::list<DtaScriptVariable*>::const_iterator i = listOfElements->begin(); i != listOfElements->end(); i++) {
				if (strstr((*i)->getName(), sSubstring.c_str()) != NULL) return iIndex;
				iIndex++;
			}
		}
	}
	return -1;
//##protect##"findFirstSubstringIntoKeys"
}

int CGRuntime::findFirstSubstringIntoKeys(const std::string& sSubstring, CppParsingTree_var pArray) {
	int result = findFirstSubstringIntoKeys(sSubstring, pArray._pInternalNode);
	return result;
}

int CGRuntime::findLastString(const std::string& sText, const std::string& sFind) {
//##protect##"findLastString"
	std::string::size_type iPos = sText.rfind(sFind);
	if ((iPos == std::string::npos) || (iPos >= sText.size())) return -1;
	return iPos;
//##protect##"findLastString"
}

int CGRuntime::findNextString(const std::string& sText, const std::string& sFind, int iPosition) {
//##protect##"findNextString"
	std::string::size_type iPos = sText.find(sFind, iPosition);
	if ((iPos == std::string::npos) || (iPos >= sText.size())) return -1;
	return iPos;
//##protect##"findNextString"
}

int CGRuntime::findNextSubstringIntoKeys(const std::string& sSubstring, DtaScriptVariable* pArray, int iNext) {
//##protect##"findNextSubstringIntoKeys"
	if (pArray != NULL) {
		const std::list<DtaScriptVariable*>* listOfElements = pArray->getArray();
		if ((listOfElements != NULL) && (iNext < (int) listOfElements->size())) {
			int iIndex = 0;
			std::list<DtaScriptVariable*>::const_iterator i = listOfElements->begin();
			while (iNext >= 0) {
				i++;
				iIndex++;
				iNext--;
			}
			for (; i != listOfElements->end(); i++) {
				if (strstr((*i)->getName(), sSubstring.c_str()) != NULL) return iIndex;
				iIndex++;
			}
		}
	}
	return -1;
//##protect##"findNextSubstringIntoKeys"
}

int CGRuntime::findNextSubstringIntoKeys(const std::string& sSubstring, CppParsingTree_var pArray, int iNext) {
	int result = findNextSubstringIntoKeys(sSubstring, pArray._pInternalNode, iNext);
	return result;
}

int CGRuntime::findString(const std::string& sText, const std::string& sFind) {
//##protect##"findString"
	std::string::size_type iPos = sText.find(sFind);
	if (iPos == std::string::npos) return -1;
	return iPos;
//##protect##"findString"
}

int CGRuntime::floor(double dNumber) {
//##protect##"floor"
	return (int) ::floor(dNumber);
//##protect##"floor"
}

std::string CGRuntime::formatDate(const std::string& sDate, const std::string& sFormat) {
//##protect##"formatDate"
	UtlDate theDate = UtlDate::getDateFromFormat(sDate, "%d%b%Y%| %H:%M:%S%|.%L");
	return theDate.getFormattedDate(sFormat);
//##protect##"formatDate"
}

int CGRuntime::getArraySize(DtaScriptVariable* pVariable) {
//##protect##"getArraySize"
	if (pVariable == NULL) return 0;
	return pVariable->getArraySize();
//##protect##"getArraySize"
}

int CGRuntime::getArraySize(CppParsingTree_var pVariable) {
	int result = getArraySize(pVariable._pInternalNode);
	return result;
}

std::string CGRuntime::getCommentBegin() {
//##protect##"getCommentBegin"
	return DtaProject::getInstance().getCommentBegin();
//##protect##"getCommentBegin"
}

std::string CGRuntime::getCommentEnd() {
//##protect##"getCommentEnd"
	return DtaProject::getInstance().getCommentEnd();
//##protect##"getCommentEnd"
}

std::string CGRuntime::getCurrentDirectory() {
//##protect##"getCurrentDirectory"
	char tcPWD[MAX_PATH];
	if (getcwd(tcPWD, MAX_PATH - 1) == NULL) return "";
	toSlashes_(tcPWD);
	std::string sPWD = tcPWD;
	if (tcPWD[sPWD.size() - 1] != '/') sPWD += "/";
	return sPWD;
//##protect##"getCurrentDirectory"
}

std::string CGRuntime::getEnv(const std::string& sVariable) {
//##protect##"getEnv"
	const char* tcVariable = ::getenv(sVariable.c_str());
	if (tcVariable == NULL) throw UtlException("variable '" + sVariable + "' not found in the environment table");
	return tcVariable;
//##protect##"getEnv"
}

std::string CGRuntime::getGenerationHeader() {
//##protect##"getGenerationHeader"
	return DtaProject::getInstance().getGenerationHeader();
//##protect##"getGenerationHeader"
}

std::string CGRuntime::getHTTPRequest(const std::string& sURL, DtaScriptVariable* pHTTPSession, DtaScriptVariable* pArguments) {
//##protect##"getHTTPRequest"
	HTTPRequest request;
	return request.get(sURL, pHTTPSession, pArguments);
//##protect##"getHTTPRequest"
}

std::string CGRuntime::getHTTPRequest(const std::string& sURL, CppParsingTree_var pHTTPSession, CppParsingTree_var pArguments) {
	std::string result = getHTTPRequest(sURL, pHTTPSession._pInternalNode, pArguments._pInternalNode);
	return result;
}

std::string CGRuntime::getIncludePath() {
//##protect##"getIncludePath"
	std::string sPath;
	const std::list<std::string>& includePath = ScpStream::getListOfIncludePaths();
	for (std::list<std::string>::const_iterator i = includePath.begin(); i != includePath.end(); i++) {
		if (!sPath.empty()) sPath += ";";
		sPath += *i;
	}
	return sPath;
//##protect##"getIncludePath"
}

double CGRuntime::getLastDelay() {
//##protect##"getLastDelay"
	return DtaProject::getInstance().getLastDelay();
//##protect##"getLastDelay"
}

std::string CGRuntime::getNow() {
//##protect##"getNow"
	if (_sFrozenTime.empty()) {
		UtlDate today;
		return today.getString();
	}
	return _sFrozenTime;
//##protect##"getNow"
}

std::string CGRuntime::getProperty(const std::string& sDefine) {
//##protect##"getProperty"
	return DtaProject::getInstance().getDefineTarget(sDefine);
//##protect##"getProperty"
}

std::string CGRuntime::getShortFilename(const std::string& sPathFilename) {
//##protect##"getShortFilename"
	std::string::size_type iIndex = sPathFilename.find_last_of("/\\");
	if (iIndex == std::string::npos) return sPathFilename;
	else if (iIndex == sPathFilename.size()) return "";
	return sPathFilename.substr(iIndex + 1);
//##protect##"getShortFilename"
}

std::string CGRuntime::getTextMode() {
//##protect##"getTextMode"
	switch(DtaProject::getInstance().getTextMode()) {
		case DtaProject::DOS_MODE: return "DOS";
		case DtaProject::UNIX_MODE: return "UNIX";
		case DtaProject::BINARY_MODE: return "BINARY";
	}
	throw UtlException("internal error into CGRuntime::getTextMode(): unhandled text mode enum");
//##protect##"getTextMode"
}

int CGRuntime::getVariableAttributes(DtaScriptVariable* pVariable, DtaScriptVariable* pList) {
//##protect##"getVariableAttributes"
	if (pVariable == NULL) return -1;
	int iSize = 0;
	DtaScriptVariableList* pAttributes = pVariable->getAttributes();
	while (pAttributes != NULL) {
		DtaScriptVariable* pNode = pAttributes->getNode();
		DtaScriptVariable* pItem = pList->addElement(pNode->getName());
		if (pNode->getReferencedVariable() != NULL) {
			std::string sValue = pNode->getReferencedVariable()->getCompleteName();
			pItem->setValue(sValue.c_str());
		}
		pAttributes = pAttributes->getNext();
		iSize++;
	}
	return iSize;
//##protect##"getVariableAttributes"
}

int CGRuntime::getVariableAttributes(CppParsingTree_var pVariable, CppParsingTree_var pList) {
	int result = getVariableAttributes(pVariable._pInternalNode, pList._pInternalNode);
	return result;
}

std::string CGRuntime::getVersion() {
//##protect##"getVersion"
	std::string sVersion = DtaProject::getInstance().getVersion();
	if (sVersion.empty()) sVersion = EXECUTABLE_VERSION;
	return sVersion;
//##protect##"getVersion"
}

std::string CGRuntime::getWorkingPath() {
//##protect##"getWorkingPath"
	return DtaProject::getInstance().getWorkingPath();
//##protect##"getWorkingPath"
}

std::string CGRuntime::getWriteMode() {
//##protect##"getWriteMode"
	if (_pOutputStream == NULL) return "";
	return ((_pOutputStream->insertMode()) ? "insert" : "overwrite");
//##protect##"getWriteMode"
}

int CGRuntime::hexaToDecimal(const std::string& sHexaNumber) {
//##protect##"hexaToDecimal"
	int iResult = 0;
	for (std::string::size_type i = 0; i < sHexaNumber.size(); i++) {
		char a = sHexaNumber[i];
		iResult *= 16;
		if (a <= '9') {
			if (a < '0') throw UtlException("hexaToDecimal(\"" + composeCLikeString(sHexaNumber) + "\"): invalid hexadecimal digit '" + composeCLikeString(std::string(1, a)) + "' encountered");
			iResult += a - '0';
		} else if (a <= 'F') {
			if (a < 'A') throw UtlException("hexaToDecimal(\"" + composeCLikeString(sHexaNumber) + "\"): invalid hexadecimal digit '" + std::string(1, a) + "' encountered");
			iResult += 10 + (a - 'A');
		} else if ((a >= 'a') && (a <= 'f')) {
			iResult += 10 + (a - 'a');
		} else {
			throw UtlException("hexaToDecimal(\"" + composeCLikeString(sHexaNumber) + "\"): invalid hexadecimal digit '" + std::string(1, a) + "' encountered");;
		}
	}
	return iResult;
//##protect##"hexaToDecimal"
}

std::string CGRuntime::hostToNetworkLong(const std::string& sBytes) {
//##protect##"hostToNetworkLong"
	if (sBytes.size() != 8) throw UtlException("'" + composeCLikeString(sBytes) + "' isn't recognized as a 4-bytes sequence");
	char tcLong[4];
	if (!convertBytesToChars(sBytes, (unsigned char*) tcLong, 4)) throw UtlException("invalid hexadecimal representation in '" + composeCLikeString(sBytes) + "'");
	unsigned long iLong = NetSocket::nToHl(*((unsigned long*) &tcLong));
	char tcBytes[9];
	convertCharsToBytes((const unsigned char*) &iLong, tcBytes, 4);
	return tcBytes;
//##protect##"hostToNetworkLong"
}

std::string CGRuntime::hostToNetworkShort(const std::string& sBytes) {
//##protect##"hostToNetworkShort"
	if (sBytes.size() != 4) throw UtlException("'" + composeCLikeString(sBytes) + "' isn't recognized as a 2-bytes sequence");
	char tcShort[2];
	if (!convertBytesToChars(sBytes, (unsigned char*) tcShort, 2)) throw UtlException("invalid hexadecimal representation in '" + composeCLikeString(sBytes) + "'");
	unsigned short iShort = NetSocket::nToHs(*((unsigned short*) &tcShort));
	char tcBytes[5];
	convertCharsToBytes((const unsigned char*) &iShort, tcBytes, 2);
	return tcBytes;
//##protect##"hostToNetworkShort"
}

double CGRuntime::increment(double& dNumber) {
//##protect##"increment"
	dNumber = dNumber + 1.0;
	return dNumber;
//##protect##"increment"
}

double CGRuntime::increment(CppParsingTree_var pNumber) {
	double dNumber = pNumber.getDoubleValue();
	double result = increment(dNumber);
	pNumber.setValue(dNumber);
	return result;
}

bool CGRuntime::indentFile(const std::string& sFile, const std::string& sMode) {
//##protect##"indentFile"
	std::auto_ptr<ScpStream> pFile(new ScpStream(sFile, ScpStream::IN | ScpStream::PATH));
	std::string::size_type iIndex = sFile.find_last_of('.');
	if ((iIndex == std::string::npos) || (iIndex == sFile.size())) throw UtlException("function 'indentFile(\"" + sFile + "\")' expects a file extension to determine its type for indentation");
	std::string sExtension = sFile.substr(iIndex + 1);
	bool bSuccess;
	if ((stricmp(sMode.c_str(), "c++") == 0) || (stricmp(sExtension.c_str(), "cpp") == 0) || (stricmp(sExtension.c_str(), "cxx") == 0) || (stricmp(sExtension.c_str(), "hxx") == 0) || (stricmp(sExtension.c_str(), "h") == 0)) bSuccess = pFile->indentAsCpp();
	else if ((stricmp(sMode.c_str(), "java") == 0) || stricmp(sExtension.c_str(), "java") == 0) bSuccess = pFile->indentAsCpp();
	else throw UtlException("function 'indentFile(\"" + sFile + "\")' works on C++ and JAVA only at the present time, so '" + sExtension + "' extension is refused");
	if (bSuccess) pFile->saveIntoFile(pFile->getFilename(), false);
	return bSuccess;
//##protect##"indentFile"
}

bool CGRuntime::inf(double dLeft, double dRight) {
//##protect##"inf"
	return (dLeft < dRight);
//##protect##"inf"
}

std::string CGRuntime::inputKey(bool bEcho) {
//##protect##"inputKey"
	if (getExternalHandling() != NULL) return getExternalHandling()->inputKey(bEcho);
	char tcText[] = {0, 0};
	int iKbHit = kbhit();
	if (iKbHit != 0) {
#ifdef WIN32
		tcText[0] = (char) getch();
#else
		tcText[0] = (char) readch();
#endif
	}
	return tcText;
//##protect##"inputKey"
}

std::string CGRuntime::inputLine(bool bEcho, const std::string& sPrompt) {
//##protect##"inputLine"
	if (getExternalHandling() != NULL) return getExternalHandling()->inputLine(bEcho);
#ifdef CODEWORKER_GNU_READLINE
	char *tcText = (char*)NULL;
	tcText = readline(sPrompt.c_str());
	
	// Save history
	if (tcText && *tcText)
		add_history(tcText);
#else
	char tcText[16384];
#	ifndef WIN32
	closeKeyboard();
#	endif
	if (!sPrompt.empty()) CGRuntime::traceText(sPrompt.c_str());
	std::cin.getline(tcText, 16383);
#	ifndef WIN32
	initKeyboard();
#	endif
#endif
	return tcText;
//##protect##"inputLine"
}

bool CGRuntime::isEmpty(DtaScriptVariable* pArray) {
//##protect##"isEmpty"
	return ((pArray == NULL) || (pArray->getArraySize() == 0));
//##protect##"isEmpty"
}

bool CGRuntime::isEmpty(CppParsingTree_var pArray) {
	bool result = isEmpty(pArray._pInternalNode);
	return result;
}

bool CGRuntime::isIdentifier(const std::string& sIdentifier) {
//##protect##"isIdentifier"
	char a = sIdentifier[0];
	if (((a >= 'a') && (a <= 'z')) || ((a >= 'A') && (a <= 'Z')) || (a == '_')) {
		std::string::size_type iIndex = 1;
		a = sIdentifier[1];
		while ((iIndex < sIdentifier.size()) && (((a >= 'a') && (a <= 'z')) || ((a >= 'A') && (a <= 'Z')) || ((a >= '0') && (a <= '9')) || (a == '_'))) {
			iIndex++;
			a = sIdentifier[iIndex];
		}
		return (iIndex == sIdentifier.size());
	}
	return false;
//##protect##"isIdentifier"
}

bool CGRuntime::isNegative(double dNumber) {
//##protect##"isNegative"
	return (dNumber < 0.0);
//##protect##"isNegative"
}

bool CGRuntime::isNumeric(const std::string& sNumber) {
//##protect##"isNumeric"
	int iIndex = 0;
	char a = sNumber[iIndex];
	if (a == '-') {
		iIndex++;
		a = sNumber[iIndex];
	}
	bool bLeft = false;
	while (a >= '0' && a <= '9') {
		bLeft = true;
		iIndex++;
		a = sNumber[iIndex];
	}
	if (a == '.') {
		iIndex++;
		a = sNumber[iIndex];
		bool bRight = false;
		while (a >= '0' && a <= '9') {
			bRight = true;
			iIndex++;
			a = sNumber[iIndex];
		}
		if (!bLeft && !bRight) return false;
	} else if (!bLeft) {
		return false;
	}
	if (a == 'e' || a == 'E') {
		iIndex++;
		a = sNumber[iIndex];
		if (a == '+' || a == '-') {
			iIndex++;
			a = sNumber[iIndex];
		}
		bool bExponent = false;
		while (a >= '0' && a <= '9') {
			bExponent = true;
			iIndex++;
			a = sNumber[iIndex];
		}
		if (!bExponent) return false;
	}
	return (a == '\0');
//##protect##"isNumeric"
}

bool CGRuntime::isPositive(double dNumber) {
//##protect##"isPositive"
	return (dNumber > 0.0);
//##protect##"isPositive"
}

std::string CGRuntime::joinStrings(DtaScriptVariable* pList, const std::string& sSeparator) {
//##protect##"joinStrings"
	std::string sResult;
	const std::list<DtaScriptVariable*>* slList = pList->getArray();
	if (slList != NULL) {
		for (std::list<DtaScriptVariable*>::const_iterator i = slList->begin(); i != slList->end(); ++i) {
			if (i != slList->begin()) {
				sResult += sSeparator;
			}
			const char* tcValue = (*i)->getValue();
			if (tcValue != NULL) sResult += tcValue;
		}
	}
	return sResult;
//##protect##"joinStrings"
}

std::string CGRuntime::joinStrings(CppParsingTree_var pList, const std::string& sSeparator) {
	std::string result = joinStrings(pList._pInternalNode, sSeparator);
	return result;
}

std::string CGRuntime::leftString(const std::string& sText, int iLength) {
//##protect##"leftString"
	if (iLength < 0) return "";
	if (iLength > (int) sText.size()) iLength = sText.size();
	return sText.substr(0, iLength);
//##protect##"leftString"
}

int CGRuntime::lengthString(const std::string& sText) {
//##protect##"lengthString"
	return sText.size();
//##protect##"lengthString"
}

std::string CGRuntime::loadBinaryFile(const std::string& sFile, int iLength) {
//##protect##"loadBinaryFile"
	std::auto_ptr<ScpStream> pFile;
	if (iLength < 0) {
		pFile = std::auto_ptr<ScpStream>(new ScpStream(sFile, ScpStream::IN | ScpStream::PATH));
		iLength = pFile->size();
	} else {
		pFile = std::auto_ptr<ScpStream>(new ScpStream(sFile, ScpStream::IN | ScpStream::PATH, iLength, iLength));
	}
	const char* tcBinary = pFile->readBuffer();
	char* tcContent = new char[1 + (iLength << 1)];
	convertCharsToBytes((const unsigned char*) tcBinary, tcContent, iLength);
	pFile->close();
	std::string sContent = tcContent;
	delete [] tcContent;
	return sContent;
//##protect##"loadBinaryFile"
}

std::string CGRuntime::loadFile(const std::string& sFile, int iLength) {
//##protect##"loadFile"
	std::auto_ptr<ScpStream> pFile;
	if (iLength < 0) {
		pFile = std::auto_ptr<ScpStream>(new ScpStream(sFile, ScpStream::IN | ScpStream::PATH));
	} else {
		pFile = std::auto_ptr<ScpStream>(new ScpStream(sFile, ScpStream::IN | ScpStream::PATH, iLength, iLength));
	}
	std::string sContent = pFile->readBuffer();
	pFile->close();
	return sContent;
//##protect##"loadFile"
}

std::string CGRuntime::loadVirtualFile(const std::string& sHandle) {
//##protect##"loadVirtualFile"
	std::string sContent;
	if (!ScpStream::loadVirtualFile(sHandle, sContent)) throw UtlException("unable to load the virtual file called '" + sHandle + "'");
	return sContent;
//##protect##"loadVirtualFile"
}

double CGRuntime::log(double dX) {
//##protect##"log"
	if (dX < 0.0) throw UtlException("the logarithm of a negative floating-point doesn't exist");
	return ::log(dX);
//##protect##"log"
}

std::string CGRuntime::longToBytes(unsigned long ulLong) {
//##protect##"longToBytes"
	char tcBytes[9];
	convertCharsToBytes((const unsigned char*) &ulLong, tcBytes, 4);
	return tcBytes;
//##protect##"longToBytes"
}

std::string CGRuntime::midString(const std::string& sText, int iPos, int iLength) {
//##protect##"midString"
	if ((iPos < 0) || (iPos >= (int) sText.size()) || (iLength <= 0)) return "";
	if (iPos + iLength > (int) sText.size()) iLength = sText.size() - iPos;
	return sText.substr(iPos, iLength);
//##protect##"midString"
}

int CGRuntime::mod(int iDividend, int iDivisor) {
//##protect##"mod"
	return iDividend % iDivisor;
//##protect##"mod"
}

double CGRuntime::mult(double dLeft, double dRight) {
//##protect##"mult"
	return dLeft * dRight;
//##protect##"mult"
}

std::string CGRuntime::networkLongToHost(const std::string& sBytes) {
//##protect##"networkLongToHost"
	if (sBytes.size() != 8) throw UtlException("'" + composeCLikeString(sBytes) + "' isn't recognized as a 4-bytes sequence");
	char tcLong[4];
	if (!convertBytesToChars(sBytes, (unsigned char*) tcLong, 4)) throw UtlException("invalid hexadecimal representation in '" + composeCLikeString(sBytes) + "'");
	unsigned long iLong = NetSocket::hToNl(*((unsigned long*) &tcLong));
	char tcBytes[9];
	convertCharsToBytes((const unsigned char*) &iLong, tcBytes, 4);
	return tcBytes;
//##protect##"networkLongToHost"
}

std::string CGRuntime::networkShortToHost(const std::string& sBytes) {
//##protect##"networkShortToHost"
	if (sBytes.size() != 4) throw UtlException("'" + composeCLikeString(sBytes) + "' isn't recognized as a 2-bytes sequence");
	char tcShort[2];
	if (!convertBytesToChars(sBytes, (unsigned char*) tcShort, 2)) throw UtlException("invalid hexadecimal representation in '" + composeCLikeString(sBytes) + "'");
	unsigned short iShort = NetSocket::hToNs(*((unsigned short*) &tcShort));
	char tcBytes[5];
	convertCharsToBytes((const unsigned char*) &iShort, tcBytes, 2);
	return tcBytes;
//##protect##"networkShortToHost"
}

int CGRuntime::octalToDecimal(const std::string& sOctalNumber) {
//##protect##"octalToDecimal"
	int iResult = 0;
	for (std::string::size_type i = 0; i < sOctalNumber.size(); i++) {
		char a = sOctalNumber[i];
		iResult *= 8;
		if ((a < '0') || (a > '7')) throw UtlException("octalToDecimal(\"" + composeCLikeString(sOctalNumber) + "\"): invalid hexadecimal digit '" + composeCLikeString(std::string(1, a)) + "' encountered");
		iResult += a - '0';
	}
	return iResult;
//##protect##"octalToDecimal"
}

std::string CGRuntime::pathFromPackage(const std::string& sPackage) {
//##protect##"pathFromPackage"
	std::string sPath = sPackage;
	std::string::size_type iPos = sPath.find(".");
	while (iPos != std::string::npos) {
		sPath[iPos] = '/';
		iPos = sPath.find(".", iPos + 1);
	}
	if (sPath.empty() || ((sPath[sPath.size() - 1] != '\\') && (sPath[sPath.size() - 1] != '/'))) sPath += "/";
	return sPath;
//##protect##"pathFromPackage"
}

std::string CGRuntime::postHTTPRequest(const std::string& sURL, DtaScriptVariable* pHTTPSession, DtaScriptVariable* pArguments) {
//##protect##"postHTTPRequest"
	HTTPRequest request;
	return request.post(sURL, pHTTPSession, pArguments);
//##protect##"postHTTPRequest"
}

std::string CGRuntime::postHTTPRequest(const std::string& sURL, CppParsingTree_var pHTTPSession, CppParsingTree_var pArguments) {
	std::string result = postHTTPRequest(sURL, pHTTPSession._pInternalNode, pArguments._pInternalNode);
	return result;
}

double CGRuntime::pow(double dX, double dY) {
//##protect##"pow"
	return ::pow(dX, dY);
//##protect##"pow"
}

int CGRuntime::randomInteger() {
//##protect##"randomInteger"
	return rand();
//##protect##"randomInteger"
}

std::string CGRuntime::receiveBinaryFromSocket(int iSocket, int iLength) {
//##protect##"receiveBinaryFromSocket"
	char* tcBuffer = new char[iLength];
	int iNbChars = 0;
	int iOffset = 0;
	do {
		iOffset += iNbChars;
		iNbChars = NetSocket::receiveFromSocket(iSocket, tcBuffer + iOffset, iLength - iOffset);
		if (iNbChars < 0) {
			delete [] tcBuffer;
			return "";
		}
	} while (iOffset + iNbChars < iLength);
	char* tcContent = new char[iLength * 2 + 1];
	convertCharsToBytes((const unsigned char*) tcBuffer, tcContent, iLength);
	std::string sContent = tcContent;
	delete [] tcBuffer;
	delete [] tcContent;
	return sContent;
//##protect##"receiveBinaryFromSocket"
}

std::string CGRuntime::receiveFromSocket(int iSocket, bool& bIsText) {
//##protect##"receiveFromSocket"
	char tcBuffer[2048];
	int iLength = NetSocket::receiveFromSocket(iSocket, tcBuffer, 2047);
	unsigned char* u = (unsigned char*) tcBuffer;
	int i = iLength;
	while (i-- > 0) {
		if (*u < 2) {
			bIsText = false;
			char tcContent[4096];
			convertCharsToBytes((const unsigned char*) tcBuffer, tcContent, iLength);
			return tcContent;
		}
		u++;
	}
	bIsText = true;
	tcBuffer[iLength] = '\0';
	return tcBuffer;
//##protect##"receiveFromSocket"
}

std::string CGRuntime::receiveFromSocket(int iSocket, CppParsingTree_var pIsText) {
	bool bIsText = pIsText.getBooleanValue();
	std::string result = receiveFromSocket(iSocket, bIsText);
	pIsText.setValue(bIsText);
	return result;
}

std::string CGRuntime::receiveTextFromSocket(int iSocket, int iLength) {
//##protect##"receiveTextFromSocket"
	char* tcBuffer = new char[iLength + 1];
	int iNbChars = 0;
	int iOffset = 0;
	do {
		iOffset += iNbChars;
		iNbChars = NetSocket::receiveFromSocket(iSocket, tcBuffer + iOffset, iLength - iOffset);
		if (iNbChars < 0) {
			delete [] tcBuffer;
			return "";
		}
	} while (iOffset + iNbChars < iLength);
	tcBuffer[iLength] = '\0';
	std::string sContent = tcBuffer;
	delete [] tcBuffer;
	return sContent;
//##protect##"receiveTextFromSocket"
}

std::string CGRuntime::relativePath(const std::string& sPath, const std::string& sReference) {
//##protect##"relativePath"
	std::string sPath1 = canonizePath(sPath);
	std::string sReference1 = canonizePath(sReference);
#ifdef WIN32
	if ((sPath1.size() < 2) || (sReference1.size() < 2) ||
		(sPath1[1] != ':') || (sReference1[1] != ':') ||
		((sPath1[0] & 0xDF) != (sReference1[0] & 0xDF))) return sPath1;
#endif
	char a = sPath1[sPath1.size() - 1];
	if ((a == '/') || (a == '\\')) sPath1 = sPath1.substr(0, sPath1.size() - 1);
	a = sReference1[sReference1.size() - 1];
	if ((a == '/') || (a == '\\')) sReference1 = sReference1.substr(0, sReference1.size() - 1);
	std::string::size_type iIndex1 = sPath1.find_first_of("/\\");
	std::string::size_type iIndex2 = sReference1.find_first_of("/\\");
	while ((iIndex1 == iIndex2) && (iIndex1 != std::string::npos)) {
		if (strncmp(sPath1.c_str(), sReference1.c_str(), iIndex1) != 0) break;
		sPath1 = sPath1.substr(iIndex1 + 1);
		sReference1 = sReference1.substr(iIndex2 + 1);
		iIndex1 = sPath1.find_first_of("/\\");
		iIndex2 = sReference1.find_first_of("/\\");
	}
	if (iIndex1 == std::string::npos) {
		if (iIndex2 == std::string::npos) {
			if (sPath1 == sReference1) return ".";
			return "../" + sPath1;
		} else if ((iIndex2 == sPath1.size()) && (strncmp(sPath1.c_str(), sReference1.c_str(), iIndex2) == 0)) {
			sPath1 = "";
			sReference1 = sReference1.substr(iIndex2 + 1);
			iIndex2 = sReference1.find_first_of("/\\");
		}
	} else {
		if (iIndex2 == std::string::npos) {
			if ((iIndex1 == sReference1.size()) && (strncmp(sPath1.c_str(), sReference1.c_str(), iIndex1) == 0)) {
				return sPath1.substr(iIndex1 + 1);
			}
			return "../" + sPath1;
		}
	}
	std::string sRelativePath;
	sRelativePath = "../";
	while (iIndex2 != std::string::npos) {
		sRelativePath += "../";
		sReference1 = sReference1.substr(iIndex2 + 1);
		iIndex2 = sReference1.find_first_of("/\\");
	}
	sRelativePath += sPath1;
	return sRelativePath;
//##protect##"relativePath"
}

bool CGRuntime::removeDirectory(const std::string& sPath) {
//##protect##"removeDirectory"
	if (sPath.empty()) return false;
	UtlDirectory theDirectory(sPath);
	return theDirectory.remove();
//##protect##"removeDirectory"
}

bool CGRuntime::removeGenerationTagsHandler(const std::string& sKey) {
//##protect##"removeGenerationTagsHandler"
	return DtaProject::getInstance().removeGenerationTagsHandler(sKey);
//##protect##"removeGenerationTagsHandler"
}

std::string CGRuntime::repeatString(const std::string& sText, int iOccurrences) {
//##protect##"repeatString"
	std::string sResult;
	while (iOccurrences-- > 0) sResult += sText;
	return sResult;
//##protect##"repeatString"
}

std::string CGRuntime::replaceString(const std::string& sOld, const std::string& sNew, const std::string& sText) {
//##protect##"replaceString"
	if (sOld.empty()) return sText;
	std::string::size_type iLastPos = 0;
	std::string::size_type iPos = sText.find(sOld);
	std::string sReplace;
	while (iPos != std::string::npos) {
		sReplace += sText.substr(iLastPos, iPos - iLastPos);
		sReplace += sNew;
		iLastPos = iPos + sOld.size();
		iPos = sText.find(sOld, iLastPos);
	}
	if (iLastPos != sText.size()) sReplace += sText.substr(iLastPos);
	return sReplace;
//##protect##"replaceString"
}

std::string CGRuntime::replaceTabulations(const std::string& sText, int iTab) {
//##protect##"replaceTabulations"
	std::string::size_type iLastPos = 0;
	std::string::size_type iPos = sText.find_first_of("\t\n");
	std::string sReplace;
	while (iPos != std::string::npos) {
		if (sText[iPos] == '\n') iPos++;
		sReplace += sText.substr(iLastPos, iPos - iLastPos);
		if (sText[iPos] == '\t') {
			int iNbSpaces = iTab - ((iPos - iLastPos) % iTab);
			sReplace += std::string(iNbSpaces, ' ');
			iLastPos = iPos + 1;
		} else {
			iLastPos = iPos;
		}
		iPos = sText.find_first_of("\t\n", iLastPos);
	}
	sReplace += sText.substr(iLastPos);
	return sReplace;
//##protect##"replaceTabulations"
}

std::string CGRuntime::resolveFilePath(const std::string& sFilename) {
//##protect##"resolveFilePath"
	if (ScpStream::existVirtualFile(sFilename)) return sFilename;
	std::string sCompleteFileName;
	if (openInputFileFromIncludePath(sFilename.c_str(), sCompleteFileName) == NULL) return "";
	return sCompleteFileName;
//##protect##"resolveFilePath"
}

std::string CGRuntime::rightString(const std::string& sText, int iLength) {
//##protect##"rightString"
	if (iLength < 0) return "";
	if (iLength > (int) sText.size()) iLength = sText.size();
	return sText.substr(sText.size() - iLength, iLength);
//##protect##"rightString"
}

std::string CGRuntime::rsubString(const std::string& sText, int iPos) {
//##protect##"rsubString"
	int iLength = ((int) sText.size()) - iPos;
	if ((iLength <= 0) || (iLength >= (int) sText.size())) return "";
	return sText.substr(0, iLength);
//##protect##"rsubString"
}

bool CGRuntime::scanDirectories(DtaScriptVariable* pDirectory, const std::string& sPath, const std::string& sPattern) {
//##protect##"scanDirectories"
	UtlDirectory theDirectory(sPath);
	bool bSuccess;
	bSuccess = theDirectory.scanRecursively(sPattern);
	if (bSuccess) {
		pDirectory->clearContent();
		populateDirectory(pDirectory, theDirectory, true);
	}
	return bSuccess;
//##protect##"scanDirectories"
}

bool CGRuntime::scanDirectories(CppParsingTree_var pDirectory, const std::string& sPath, const std::string& sPattern) {
	bool result = scanDirectories(pDirectory._pInternalNode, sPath, sPattern);
	return result;
}

bool CGRuntime::scanFiles(DtaScriptVariable* pFiles, const std::string& sPath, const std::string& sPattern, bool bSubfolders) {
//##protect##"scanFiles"
	bool bSuccess;
	std::string sNewPath = sPath;
	std::string sNewPattern = sPattern;
	if (!bSubfolders) {
		std::string::size_type iIndex = sPattern.find_last_of("/\\");
		if (iIndex != std::string::npos) {
			// a directory in the pattern, special case if non recursive
			std::string sNewPath = sPath;
			if (!sNewPath.empty() && (sNewPath.find_last_of("/\\") != sNewPath.size() - 1)) {
				sNewPath += "/";
			}
			iIndex++;
			sNewPath += sPattern.substr(0, iIndex);
			sNewPattern = sPattern.substr(iIndex);
		}
	}
	UtlDirectory theDirectory(sNewPath);
	if (bSubfolders) {
		bSuccess = theDirectory.scanRecursively(sNewPattern);
	} else {
		bSuccess = theDirectory.scan(sNewPattern);
	}
	if (bSuccess) {
		pFiles->clearContent();
		populateFileScan(pFiles, theDirectory, bSubfolders);
	}
	return bSuccess;
//##protect##"scanFiles"
}

bool CGRuntime::scanFiles(CppParsingTree_var pFiles, const std::string& sPath, const std::string& sPattern, bool bSubfolders) {
	bool result = scanFiles(pFiles._pInternalNode, sPath, sPattern, bSubfolders);
	return result;
}

bool CGRuntime::sendBinaryToSocket(int iSocket, const std::string& sBytes) {
//##protect##"sendBinaryToSocket"
	int iLength = sBytes.size();
	if ((iLength & 1) != 0) throw UtlException("sequence of bytes expected for 'sendBinaryToSocket()'");
	iLength >>= 1;
	char* tcBuffer = new char[iLength];
	if (!convertBytesToChars(sBytes, (unsigned char*) tcBuffer, iLength)) {
		delete [] tcBuffer;
		throw UtlException("invalid hexadecimal digit encountered");
	}
	bool bSuccess = NetSocket::sendToSocket(iSocket, tcBuffer, iLength);
	delete [] tcBuffer;
	return bSuccess;
//##protect##"sendBinaryToSocket"
}

std::string CGRuntime::sendHTTPRequest(const std::string& sURL, DtaScriptVariable* pHTTPSession) {
//##protect##"sendHTTPRequest"
	HTTPRequest request;
	return request.send(sURL, pHTTPSession);
//##protect##"sendHTTPRequest"
}

std::string CGRuntime::sendHTTPRequest(const std::string& sURL, CppParsingTree_var pHTTPSession) {
	std::string result = sendHTTPRequest(sURL, pHTTPSession._pInternalNode);
	return result;
}

bool CGRuntime::sendTextToSocket(int iSocket, const std::string& sText) {
//##protect##"sendTextToSocket"
	return NetSocket::sendToSocket(iSocket, sText.c_str(), sText.size());
//##protect##"sendTextToSocket"
}

bool CGRuntime::selectGenerationTagsHandler(const std::string& sKey) {
//##protect##"selectGenerationTagsHandler"
	return DtaProject::getInstance().selectGenerationTagsHandler(sKey);
//##protect##"selectGenerationTagsHandler"
}

std::string CGRuntime::shortToBytes(unsigned short ulShort) {
//##protect##"shortToBytes"
	char tcBytes[5];
	convertCharsToBytes((const unsigned char*) &ulShort, tcBytes, 2);
	return tcBytes;
//##protect##"shortToBytes"
}

double CGRuntime::sqrt(double dX) {
//##protect##"sqrt"
	return ::sqrt(dX);
//##protect##"sqrt"
}

bool CGRuntime::startString(const std::string& sText, const std::string& sStart) {
//##protect##"startString"
	return (strncmp(sText.c_str(), sStart.c_str(), sStart.size()) == 0);
//##protect##"startString"
}

double CGRuntime::sub(double dLeft, double dRight) {
//##protect##"sub"
	return dLeft - dRight;
//##protect##"sub"
}

std::string CGRuntime::subString(const std::string& sText, int iPos) {
//##protect##"subString"
	return ((iPos < (int) sText.size()) ? sText.substr(iPos) : "");
//##protect##"subString"
}

bool CGRuntime::sup(double dLeft, double dRight) {
//##protect##"sup"
	return (dLeft > dRight);
//##protect##"sup"
}

std::string CGRuntime::system(const std::string& sCommand) {
//##protect##"system"
#ifdef WIN32
	std::string sAdjustedCommand = sCommand;
#endif
	const char* tcCommand = NULL;
	if (!sCommand.empty()) {
#ifdef WIN32
		char cFinal = ((sCommand[0] == '"') ? '"' : ' ');
		int i = ((cFinal == '"') ? 1 : 0);
		char a = sAdjustedCommand[i];
		while ((a != cFinal) && (a != '\0')) {
			if (a == '/') sAdjustedCommand[i] = '\\';
			a = sAdjustedCommand[++i];
		}
		tcCommand = sAdjustedCommand.c_str();
#else
		tcCommand = sCommand.c_str();
#endif
	}
	int iResult = ::system(tcCommand);
	if (iResult == -1) {
		switch(errno) {
			case E2BIG: return "Argument list is too big for command interpreter";
			case ENOENT: return "Command interpreter cannot be found";
			case ENOEXEC: return "Command interpreter file has invalid format and is not executable";
			case ENOMEM: return "Not enough memory is available to execute command; or available memory has been corrupted; or invalid block exists, indicating that process making call was not allocated properly";
			default: return "Unknown error encountered by command interpreter";
		}
	} else if (iResult != 0) {
		return "The name specified isn't recognized as an executable";
	}
	return "";
//##protect##"system"
}

std::string CGRuntime::toLowerString(const std::string& sText) {
//##protect##"toLowerString"
	std::string sCopy(sText.c_str());
	for (std::string::size_type i = 0; i < sCopy.size(); i++) {
		char a = sCopy[i];
		if ((a >= 'A') && (a <= 'Z')) sCopy[i] = a + ' ';
	}
	return sCopy;
//##protect##"toLowerString"
}

std::string CGRuntime::toUpperString(const std::string& sText) {
//##protect##"toUpperString"
	std::string sCopy(sText.c_str());
	for (std::string::size_type i = 0; i < sCopy.size(); i++) {
		char a = sCopy[i];
		if ((a >= 'a') && (a <= 'z')) sCopy[i] = a - ' ';
	}
	return sCopy;
//##protect##"toUpperString"
}

int CGRuntime::trimLeft(std::string& sString) {
//##protect##"trimLeft"
	int i = 0;
	while ((sString[i] > '\0') && (sString[i] <= ' ')) i++;
	if (i > 0) {
		if (i < (int) sString.size()) {
			sString = sString.substr(i);
		} else {
			sString = "";
		}
	}
	return i;
//##protect##"trimLeft"
}

int CGRuntime::trimLeft(CppParsingTree_var pString) {
	std::string sString = pString.getValue();
	int result = trimLeft(sString);
	pString.setValue(sString);
	return result;
}

int CGRuntime::trimRight(std::string& sString) {
//##protect##"trimRight"
	int i = sString.size() - 1;
	int iCounter = i;
	while ((i >= 0) && (sString[i] > '\0') && (sString[i] <= ' ')) i--;
	iCounter -= i;
	if (iCounter > 0) {
		if (i >= 0) {
			sString = sString.substr(0, i + 1);
		} else {
			sString = "";
		}
	}
	return iCounter;
//##protect##"trimRight"
}

int CGRuntime::trimRight(CppParsingTree_var pString) {
	std::string sString = pString.getValue();
	int result = trimRight(sString);
	pString.setValue(sString);
	return result;
}

int CGRuntime::trim(std::string& sString) {
//##protect##"trim"
	return trimLeft(sString) + trimRight(sString);
//##protect##"trim"
}

int CGRuntime::trim(CppParsingTree_var pString) {
	std::string sString = pString.getValue();
	int result = trim(sString);
	pString.setValue(sString);
	return result;
}

std::string CGRuntime::truncateAfterString(DtaScriptVariable* pVariable, const std::string& sText) {
//##protect##"truncateAfterString"
	std::string sString = pVariable->getValue();
	std::string::size_type iPos = sString.find(sText);
	if (iPos == std::string::npos) return "";
	iPos += sText.size();
	if (iPos >= sString.size()) pVariable->setValue("");
	else {
		std::string sNewValue = sString.substr(iPos);
		pVariable->setValue(sNewValue.c_str());
	}
	return sString.substr(0, iPos);
//##protect##"truncateAfterString"
}

std::string CGRuntime::truncateAfterString(CppParsingTree_var pVariable, const std::string& sText) {
	std::string result = truncateAfterString(pVariable._pInternalNode, sText);
	return result;
}

std::string CGRuntime::truncateBeforeString(DtaScriptVariable* pVariable, const std::string& sText) {
//##protect##"truncateBeforeString"
	std::string sString = pVariable->getValue();
	std::string::size_type iPos = sString.find(sText);
	if (iPos == std::string::npos) return "";
	if (iPos == 0) pVariable->setValue("");
	else {
		std::string sNewValue = sString.substr(0, iPos);
		pVariable->setValue(sNewValue.c_str());
	}
	return sString.substr(iPos);
//##protect##"truncateBeforeString"
}

std::string CGRuntime::truncateBeforeString(CppParsingTree_var pVariable, const std::string& sText) {
	std::string result = truncateBeforeString(pVariable._pInternalNode, sText);
	return result;
}

std::string CGRuntime::UUID() {
//##protect##"UUID"
	return HTTPRequest::UUID();
//##protect##"UUID"
}

int CGRuntime::countInputCols() {
//##protect##"countInputCols"
	return _pInputStream->getColCount();
//##protect##"countInputCols"
}

int CGRuntime::countInputLines() {
//##protect##"countInputLines"
	return _pInputStream->getLineCount();
//##protect##"countInputLines"
}

std::string CGRuntime::getInputFilename() {
//##protect##"getInputFilename"
	return _pInputStream->getFilename();
//##protect##"getInputFilename"
}

std::string CGRuntime::getLastReadChars(int iLength) {
//##protect##"getLastReadChars"
	std::string sLastChars;
	_pInputStream->readLastChars(iLength, sLastChars);
	return sLastChars;
//##protect##"getLastReadChars"
}

int CGRuntime::getInputLocation() {
//##protect##"getInputLocation"
	return _pInputStream->getInputLocation();
//##protect##"getInputLocation"
}

bool CGRuntime::lookAhead(const std::string& sText) {
//##protect##"lookAhead"
	if (!_pInputStream->isEqualTo(sText)) return false;
	_pInputStream->setInputLocation(_pInputStream->getInputLocation() - sText.size());
	return true;
//##protect##"lookAhead"
}

std::string CGRuntime::peekChar() {
//##protect##"peekChar"
	static char tcResult[] = {'\0', '\0'};
	int iChar = _pInputStream->peekChar();
	if (iChar > 0) tcResult[0] = (char) iChar;
	else tcResult[0] = '\0';
	return tcResult;
//##protect##"peekChar"
}

bool CGRuntime::readAdaString(std::string& sText) {
//##protect##"readAdaString"
	return _pInputStream->readAdaString(sText);
//##protect##"readAdaString"
}

bool CGRuntime::readAdaString(CppParsingTree_var pText) {
	std::string sText = pText.getValue();
	bool result = readAdaString(sText);
	pText.setValue(sText);
	return result;
}

std::string CGRuntime::readByte() {
//##protect##"readByte"
	char tcText[] = {'\0', '\0', '\0'};
	int iChar = _pInputStream->readChar();
	if (iChar >= 0) {
		tcText[0] = _tcHexa[iChar >> 4];
		tcText[1] = _tcHexa[iChar & 0x0F];
	}
	return tcText;
//##protect##"readByte"
}

std::string CGRuntime::readBytes(int iLength) {
//##protect##"readBytes"
	char* tcText = new char[iLength*2 + 1];
	int iLocation = _pInputStream->getInputLocation();
	int j = 0;
	int iChar;
	for (int i = 0; i < iLength; i++) {
		iChar = _pInputStream->readChar();
		if (iChar < 0) {
			delete [] tcText;
			_pInputStream->setInputLocation(iLocation);
			return "";
		}
		tcText[j++] = _tcHexa[iChar >> 4];
		tcText[j++] = _tcHexa[iChar & 0x0F];
	}
	tcText[j] = '\0';
	std::string sBytes = tcText;
	delete [] tcText;
	return sBytes;
//##protect##"readBytes"
}

std::string CGRuntime::readCChar() {
//##protect##"readCChar"
	static char tcResult[] = {'\0', '\0'};
	int iChar;
	if (_pInputStream->readCharLiteral(iChar)) tcResult[0] = (char) iChar;
	else tcResult[0] = '\0';
	return tcResult;
//##protect##"readCChar"
}

std::string CGRuntime::readChar() {
//##protect##"readChar"
	static char tcResult[] = {'\0', '\0'};
	int iChar = _pInputStream->readChar();
	if (iChar > 0) tcResult[0] = (char) iChar;
	else tcResult[0] = '\0';
	return tcResult;
//##protect##"readChar"
}

int CGRuntime::readCharAsInt() {
//##protect##"readCharAsInt"
	return _pInputStream->readChar();
//##protect##"readCharAsInt"
}

std::string CGRuntime::readChars(int iLength) {
//##protect##"readChars"
	std::string sText;
	_pInputStream->readChars(iLength, sText);
	return sText;
//##protect##"readChars"
}

std::string CGRuntime::readIdentifier() {
//##protect##"readIdentifier"
	std::string sIdentifier;
	if (!_pInputStream->readIdentifier(sIdentifier)) return "";
	return sIdentifier;
//##protect##"readIdentifier"
}

bool CGRuntime::readIfEqualTo(const std::string& sText) {
//##protect##"readIfEqualTo"
	return _pInputStream->isEqualTo(sText);
//##protect##"readIfEqualTo"
}

bool CGRuntime::readIfEqualToIgnoreCase(const std::string& sText) {
//##protect##"readIfEqualToIgnoreCase"
	return _pInputStream->isEqualToIgnoreCase(sText);
//##protect##"readIfEqualToIgnoreCase"
}

bool CGRuntime::readIfEqualToIdentifier(const std::string& sIdentifier) {
//##protect##"readIfEqualToIdentifier"
	return _pInputStream->isEqualToIdentifier(sIdentifier.c_str());
//##protect##"readIfEqualToIdentifier"
}

bool CGRuntime::readLine(std::string& sText) {
//##protect##"readLine"
	return _pInputStream->readLine(sText);
//##protect##"readLine"
}

bool CGRuntime::readLine(CppParsingTree_var pText) {
	std::string sText = pText.getValue();
	bool result = readLine(sText);
	pText.setValue(sText);
	return result;
}

bool CGRuntime::readNextText(const std::string& sText) {
//##protect##"readNextText"
	return _pInputStream->findString(sText);
//##protect##"readNextText"
}

bool CGRuntime::readNumber(double& dNumber) {
//##protect##"readNumber"
	return _pInputStream->readDouble(dNumber);
//##protect##"readNumber"
}

bool CGRuntime::readNumber(CppParsingTree_var pNumber) {
	double dNumber = pNumber.getDoubleValue();
	bool result = readNumber(dNumber);
	pNumber.setValue(dNumber);
	return result;
}

bool CGRuntime::readPythonString(std::string& sText) {
//##protect##"readPythonString"
	return _pInputStream->readPythonString(sText);
//##protect##"readPythonString"
}

bool CGRuntime::readPythonString(CppParsingTree_var pText) {
	std::string sText = pText.getValue();
	bool result = readPythonString(sText);
	pText.setValue(sText);
	return result;
}

bool CGRuntime::readString(std::string& sText) {
//##protect##"readString"
	return _pInputStream->readString(sText);
//##protect##"readString"
}

bool CGRuntime::readString(CppParsingTree_var pText) {
	std::string sText = pText.getValue();
	bool result = readString(sText);
	pText.setValue(sText);
	return result;
}

std::string CGRuntime::readUptoJustOneChar(const std::string& sOneAmongChars) {
//##protect##"readUptoJustOneChar"
	std::string sText;
	if (!_pInputStream->readUptoChar(sOneAmongChars, sText)) return "";
	return sText;
//##protect##"readUptoJustOneChar"
}

std::string CGRuntime::readWord() {
//##protect##"readWord"
	std::string sWord;
	if (!_pInputStream->readWord(sWord)) return "";
	return sWord;
//##protect##"readWord"
}

bool CGRuntime::skipBlanks() {
//##protect##"skipBlanks"
	return _pInputStream->skipBlanks();
//##protect##"skipBlanks"
}

bool CGRuntime::skipSpaces() {
//##protect##"skipSpaces"
	return _pInputStream->skipSpaces();
//##protect##"skipSpaces"
}

bool CGRuntime::skipEmptyCpp() {
//##protect##"skipEmptyCpp"
	return _pInputStream->skipEmpty();
//##protect##"skipEmptyCpp"
}

bool CGRuntime::skipEmptyCppExceptDoxygen() {
//##protect##"skipEmptyCppExceptDoxygen"
	return _pInputStream->skipEmptyCppExceptDoxygen();
//##protect##"skipEmptyCppExceptDoxygen"
}

bool CGRuntime::skipEmptyHTML() {
//##protect##"skipEmptyHTML"
	return _pInputStream->skipEmptyHTML();
//##protect##"skipEmptyHTML"
}

bool CGRuntime::skipEmptyLaTeX() {
//##protect##"skipEmptyLaTeX"
	return _pInputStream->skipEmptyLaTeX();
//##protect##"skipEmptyLaTeX"
}

int CGRuntime::countOutputCols() {
//##protect##"countOutputCols"
	return _pOutputStream->getOutputColCount();
//##protect##"countOutputCols"
}

int CGRuntime::countOutputLines() {
//##protect##"countOutputLines"
	return _pOutputStream->getOutputLineCount();
//##protect##"countOutputLines"
}

bool CGRuntime::decrementIndentLevel(int iLevel) {
//##protect##"decrementIndentLevel"
	return _pOutputStream->decrementIndentation(iLevel);
//##protect##"decrementIndentLevel"
}

bool CGRuntime::equalLastWrittenChars(const std::string& sText) {
//##protect##"equalLastWrittenChars"
	std::string sLastChars = _pOutputStream->getLastWrittenChars(sText.length());
	return sLastChars == sText;
//##protect##"equalLastWrittenChars"
}

bool CGRuntime::existFloatingLocation(const std::string& sKey, bool bParent) {
//##protect##"existFloatingLocation"
	ScpStream* pOwner;
	int iPosition = _pOutputStream->getFloatingLocation(sKey, pOwner);
	return ((bParent) ? (pOwner != NULL) : (pOwner == _pOutputStream));
//##protect##"existFloatingLocation"
}

int CGRuntime::getFloatingLocation(const std::string& sKey) {
//##protect##"getFloatingLocation"
	ScpStream* pOwner;
	int iPosition = _pOutputStream->getFloatingLocation(sKey, pOwner);
	if (pOwner == NULL) throw UtlException("the floating area '" + sKey + "' doesn't exist.");
	if (pOwner != _pOutputStream) throw UtlException("the floating area '" + sKey + "' was put in a precedent markup area, but not in this one.");
	return iPosition;
//##protect##"getFloatingLocation"
}

std::string CGRuntime::getLastWrittenChars(int iNbChars) {
//##protect##"getLastWrittenChars"
	std::string sLastChars = _pOutputStream->getLastWrittenChars(iNbChars);
	return sLastChars;
//##protect##"getLastWrittenChars"
}

std::string CGRuntime::getMarkupKey() {
//##protect##"getMarkupKey"
	return DtaProject::getInstance().getMarkupKey();
//##protect##"getMarkupKey"
}

std::string CGRuntime::getMarkupValue() {
//##protect##"getMarkupValue"
	return DtaProject::getInstance().getMarkupValue();
//##protect##"getMarkupValue"
}

std::string CGRuntime::getOutputFilename() {
//##protect##"getOutputFilename"
	return _pOutputStream->getFilename();
//##protect##"getOutputFilename"
}

int CGRuntime::getOutputLocation() {
//##protect##"getOutputLocation"
	return _pOutputStream->getOutputLocation();
//##protect##"getOutputLocation"
}

std::string CGRuntime::getProtectedArea(const std::string& sProtection) {
//##protect##"getProtectedArea"
	DtaPatternScript* pPatternScript = _listOfPatternScripts.front();
	return pPatternScript->getProtectedAreasBag().getProtection(sProtection.c_str());
//##protect##"getProtectedArea"
}

int CGRuntime::getProtectedAreaKeys(DtaScriptVariable* pKeys) {
//##protect##"getProtectedAreaKeys"
	if (pKeys == NULL) return -1;
	pKeys->clearContent();
	std::list<std::string> listOfKeys = _listOfPatternScripts.front()->getProtectionKeys();
	for (std::list<std::string>::const_iterator i = listOfKeys.begin(); i != listOfKeys.end(); ++i) {
		pKeys->addElement(*i)->setValue(i->c_str());
	}
	return listOfKeys.size();
//##protect##"getProtectedAreaKeys"
}

int CGRuntime::getProtectedAreaKeys(CppParsingTree_var pKeys) {
	int result = getProtectedAreaKeys(pKeys._pInternalNode);
	return result;
}

bool CGRuntime::indentText(const std::string& sMode) {
//##protect##"indentText"
	if (stricmp(sMode.c_str(), "c++") == 0) return _pOutputStream->indentAsCpp();
	if (stricmp(sMode.c_str(), "java") == 0) return _pOutputStream->indentAsCpp();
	throw UtlException("function 'indentText(<mode>)' works on C++ and JAVA only at the present time, so '" + sMode + "' is refused");
//##protect##"indentText"
}

bool CGRuntime::newFloatingLocation(const std::string& sKey) {
//##protect##"newFloatingLocation"
	return _pOutputStream->newFloatingLocation(sKey);
//##protect##"newFloatingLocation"
}

int CGRuntime::remainingProtectedAreas(DtaScriptVariable* pKeys) {
//##protect##"remainingProtectedAreas"
	if (pKeys == NULL) return -1;
	pKeys->clearContent();
	std::list<std::string> listOfKeys = _listOfPatternScripts.front()->remainingProtectionKeys();
	for (std::list<std::string>::const_iterator i = listOfKeys.begin(); i != listOfKeys.end(); ++i) {
		pKeys->addElement(*i)->setValue(i->c_str());
	}
	return listOfKeys.size();
//##protect##"remainingProtectedAreas"
}

int CGRuntime::remainingProtectedAreas(CppParsingTree_var pKeys) {
	int result = remainingProtectedAreas(pKeys._pInternalNode);
	return result;
}

int CGRuntime::removeFloatingLocation(const std::string& sKey) {
//##protect##"removeFloatingLocation"
	ScpStream* pOwner;
	int iPosition = _pOutputStream->removeFloatingLocation(sKey, pOwner);
	return iPosition;
//##protect##"removeFloatingLocation"
}

bool CGRuntime::removeProtectedArea(const std::string& sProtectedAreaName) {
//##protect##"removeProtectedArea"
	return _listOfPatternScripts.front()->removeProtectedArea(sProtectedAreaName);
//##protect##"removeProtectedArea"
}

//##end##"functions and procedures"

	bool CGRuntime::extractGenerationHeader(ScpStream& theStream, std::string& sGenerator, std::string& sVersion, std::string& sDate, std::string& sComment) {
		bool bSuccess = false;
		std::string sCommentBegin = DtaProject::getInstance().getCommentBegin();
		std::string sBeginningMarker = sCommentBegin + "##generation header##";
		theStream.setInputLocation(0);
		if (theStream.isEqualTo(sBeginningMarker)) {
			if (theStream.readUptoChar('#', sGenerator) && theStream.isEqualTo("##")) {
				if (theStream.readUptoChar('#', sVersion) && theStream.isEqualTo("##")) {
					if (theStream.readUptoChar('#', sDate) && theStream.isEqualTo("##")) {
						std::string sFile;
						bool bCommentExtracted = false;
						if (theStream.readString(sFile)) {
							if (!theStream.isEqualTo("##")) {
								sComment = sFile;
								sFile.clear();
								bCommentExtracted = true;
							}
						}
						if (bCommentExtracted || theStream.readString(sComment)) {
							bSuccess = true;
						} else {
							std::string sCommentEnd = DtaProject::getInstance().getCommentEnd();
							if (theStream.findString(sCommentBegin + "##header start##") && theStream.findString(sCommentEnd)) {
								if (sCommentEnd.find('\n') == std::string::npos) theStream.findString("\n");
								int iStart = theStream.getInputLocation();
								std::string sMarker = sCommentBegin + "##header end##";
								if (theStream.findString(sMarker)) {
									int iEnd = theStream.getInputLocation();
									theStream.readLastChars(iEnd - iStart, sComment);
									sComment = replaceString("\r", "", sComment.substr(0, iEnd - iStart - sMarker.size()));
									if (sCommentEnd.find('\n') == std::string::npos) sCommentEnd += "\n";
									std::string sSeparator = sCommentEnd + sCommentBegin;
									sComment = sComment.substr(sCommentBegin.size(), sComment.size() - sSeparator.size());
									sComment = replaceString(sSeparator, "\n", sComment);
									bSuccess = true;
								}
							}
						}
					}
				}
			}
		}
		if (bSuccess) {
			std::string sCommentEnd = DtaProject::getInstance().getCommentEnd();
			if (sCommentEnd.find('\n') != std::string::npos) {
				if (!theStream.findString("\n")) {
					theStream.setInputLocation(0);
					bSuccess = false;
				}
			} else if (!theStream.isEqualTo(sCommentEnd)) {
				theStream.setInputLocation(0);
				bSuccess = false;
			}
		} else theStream.setInputLocation(0);
		return bSuccess;
	}

	void CGRuntime::populateDirectory(DtaScriptVariable* pDirectory, UtlDirectory& theDirectory, bool bSubfolders) {
		pDirectory->setValue(theDirectory.getRelativePath().c_str());
		if (!theDirectory.getFiles().empty()) {
			DtaScriptVariable* pFiles = pDirectory->insertNode("files");
			for (std::list<UtlFile*>::const_iterator i = theDirectory.getFiles().begin(); i != theDirectory.getFiles().end(); ++i) {
				const std::string& sName = (*i)->getFileName();
				DtaScriptVariable* pNode = pFiles->addElement(sName);
				pNode->setValue(sName.c_str());
			}
		}
		if (!theDirectory.getDirectories().empty()) {
			DtaScriptVariable* pDirectories = pDirectory->insertNode("directories");
			for (std::list<UtlDirectory*>::const_iterator i = theDirectory.getDirectories().begin(); i != theDirectory.getDirectories().end(); ++i) {
				DtaScriptVariable* pNode = pDirectories->addElement((*i)->getDirectoryName());
				if (bSubfolders) populateDirectory(pNode, *(*i), true);
				else pNode->setValue((*i)->getRelativePath().c_str());
			}
		}
	}

	void CGRuntime::populateFileScan(DtaScriptVariable* pFiles, UtlDirectory& theDirectory, bool bSubfolders) {
		if (!theDirectory.getFiles().empty()) {
			for (std::list<UtlFile*>::const_iterator i = theDirectory.getFiles().begin(); i != theDirectory.getFiles().end(); ++i) {
				std::string sRelativePath = (*i)->getDirectory()->getRelativePath() + (*i)->getFileName();
				DtaScriptVariable* pNode = pFiles->addElement(sRelativePath);
				pNode->setValue(sRelativePath.c_str());
			}
		}
		if (!theDirectory.getDirectories().empty() && bSubfolders) {
			for (std::list<UtlDirectory*>::const_iterator i = theDirectory.getDirectories().begin(); i != theDirectory.getDirectories().end(); ++i) {
				populateFileScan(pFiles, *(*i), true);
			}
		}
	}

	void CGRuntime::copySmartDirectory(UtlDirectory& theDirectory, const std::string& sDestinationPath) {
		if (!theDirectory.getFiles().empty()) {
			for (std::list<UtlFile*>::const_iterator i = theDirectory.getFiles().begin(); i != theDirectory.getFiles().end(); ++i) {
				copySmartFile(theDirectory.getFullPath() + (*i)->getFileName(), sDestinationPath + (*i)->getFileName());
			}
		}
		if (!theDirectory.getDirectories().empty()) {
			for (std::list<UtlDirectory*>::const_iterator i = theDirectory.getDirectories().begin(); i != theDirectory.getDirectories().end(); ++i) {
				copySmartDirectory(*(*i), sDestinationPath + (*i)->getDirectoryName() + "/");
			}
		}
	}

	bool CGRuntime::convertBytesToChars(const std::string& sBytes, unsigned char* tcBuffer, int iLength) {
		int i = 0;
		for (int j = 0; j < iLength; j++) {
			unsigned char c;
			char a = sBytes[i++];
			if (a <= '9') {
				if (a < '0') return false;
				c = a - '0';
			} else if (a <= 'F') {
				if (a < 'A') return false;
				c = (a - 'A') + 10;
			} else if ((a >= 'a') && (a <= 'f')) {
				c = (a - 'a') + 10;
			} else return false;
			c <<= 4;
			a = sBytes[i++];
			if (a <= '9') {
				if (a < '0') return false;
				c += a - '0';
			} else if (a <= 'F') {
				if (a < 'A') return false;
				c += (a - 'A') + 10;
			} else if ((a >= 'a') && (a <= 'f')) {
				c += (a - 'a') + 10;
			} else return false;
			tcBuffer[j] = c;
		}
		return true;
	}

	void CGRuntime::convertCharsToBytes(const unsigned char* tcBuffer, char* tcContent, int iLength) {
		int j = 0;
		for (int i = 0; i < iLength; i++) {
			unsigned char c = tcBuffer[i];
			tcContent[j++] = _tcHexa[c >> 4];
			tcContent[j++] = _tcHexa[c & 0x0F];
		}
		tcContent[j] = '\0';
	}


	std::map<std::string, EXTERNAL_FUNCTION>& CGRuntime::getExternalFunctionsRegister() {
		static std::map<std::string, EXTERNAL_FUNCTION> theRegister;
		return theRegister;
	}

	std::map<std::string, EXTERNAL_TEMPLATE_DISPATCHER_FUNCTION>& CGRuntime::getExternalTemplateDispatcherFunctionsRegister() {
		static std::map<std::string, EXTERNAL_TEMPLATE_DISPATCHER_FUNCTION> theRegister;
		return theRegister;
	}



	CGRuntimeInputStream::CGRuntimeInputStream(ScpStream* pNewStream) {
		_pOldStream = CGRuntime::_pInputStream;
		CGRuntime::_pInputStream = pNewStream;
	}

	CGRuntimeInputStream::~CGRuntimeInputStream() {
		CGRuntime::_pInputStream = _pOldStream;
	}

	CGRuntimeOutputStream::CGRuntimeOutputStream(ScpStream* pNewStream) {
		_pOldStream = CGRuntime::_pOutputStream;
		CGRuntime::_pOutputStream = pNewStream;
	}

	CGRuntimeOutputStream::~CGRuntimeOutputStream() {
		CGRuntime::_pOutputStream = _pOldStream;
	}


	CGRuntimeInputFile::CGRuntimeInputFile(const std::string& sFile) : _sFile(sFile), _pOldInputStream(NULL), _pNewStream(NULL) {
		_pOldInputStream = CGRuntime::_pInputStream;
		_pNewStream = new ScpStream(sFile, ScpStream::IN | ScpStream::PATH);
		CGRuntime::_pInputStream = _pNewStream;
	}

	CGRuntimeInputFile::~CGRuntimeInputFile() {
		CGRuntime::_pInputStream = _pOldInputStream;
		if (_pNewStream != NULL) {
			_pNewStream->close();
			delete _pNewStream;
		}
	}

	std::string CGRuntimeInputFile::onCatchedException(const UtlException& exception) {
		int iLine = CGRuntime::_pInputStream->getLineCount();
		std::string sException = exception.getMessage();
		std::string sMessage = _sFile;
		char tcNumber[32];
		sprintf(tcNumber, "(%d):", iLine);
		sMessage += tcNumber;
		sMessage += CGRuntime::endl() + sException;
		return sMessage;
	}


	CGRuntimeInputString::CGRuntimeInputString(const std::string& sText) : _pOldInputStream(NULL), _pNewStream(NULL) {
		_pOldInputStream = CGRuntime::_pInputStream;
		_pNewStream = new ScpStream(sText);
		CGRuntime::_pInputStream = _pNewStream;
	}

	CGRuntimeInputString::~CGRuntimeInputString() {
		CGRuntime::_pInputStream = _pOldInputStream;
		if (_pNewStream != NULL) {
			_pNewStream->close();
			delete _pNewStream;
		}
	}

	std::string CGRuntimeInputString::onCatchedException(const UtlException& exception) {
		int iLine = CGRuntime::_pInputStream->getLineCount();
		std::string sException = exception.getMessage();
		std::string sMessage;
		if (!CGRuntime::_pInputStream->empty()) {
			sMessage +=	CGRuntime::endl() + "----------------- content -----------------" +
						CGRuntime::endl() + CGRuntime::_pInputStream->readBuffer() +
						CGRuntime::endl() + "-------------------------------------------" + CGRuntime::endl();
		}
		char tcNumber[40];
		sprintf(tcNumber, "line %d", iLine);
		sMessage += tcNumber;
		sMessage += ":" + CGRuntime::endl() + sException;
		return sMessage;
	}


	CGRuntimeOutputString::CGRuntimeOutputString() {
		_pGeneratedString = new DtaOutputFile(0);
		_pGeneratedString->openGenerate(true, NULL, _pOldOutputStream);
	}

	CGRuntimeOutputString::~CGRuntimeOutputString() {
		delete _pGeneratedString;
	}

	void CGRuntimeOutputString::onCatchedException(UtlException& exception) {
		_pGeneratedString->catchGenerateExecution(true, _pOldOutputStream, &exception);
	}

	std::string CGRuntimeOutputString::getResult() const {
		return _pGeneratedString->closeGenerate(true, NULL, _pOldOutputStream);
	}


	CGRuntimeOutputFile::CGRuntimeOutputFile(const std::string& sFile, bool bAppendMode) : _bAppendMode(bAppendMode) {
		_pGeneratedString = new DtaOutputFile(0);
		if (_bAppendMode) {
			_pGeneratedString->openAppend(true, sFile.c_str(), _pOldOutputStream);
		} else {
			_pGeneratedString->openGenerate(true, sFile.c_str(), _pOldOutputStream);
		}
	}

	CGRuntimeOutputFile::~CGRuntimeOutputFile() {
		delete _pGeneratedString;
	}

	void CGRuntimeOutputFile::onCatchedException(UtlException& exception) {
		_pGeneratedString->catchGenerateExecution(true, _pOldOutputStream, &exception);
	}

	void CGRuntimeOutputFile::closeGenerate() {
		_pGeneratedString->closeGenerate(true, CGRuntime::_pOutputStream->getFilename().c_str(), _pOldOutputStream);
	}

	CGRuntimeNewProject::CGRuntimeNewProject() {
		_pNewProject = new DtaProject;
	}

	CGRuntimeNewProject::~CGRuntimeNewProject() {
		delete _pNewProject;
	}



	struct CGRuntimeTemporaryMatchingStorage {
		bool bBegin;
		DtaScriptVariable* pClauseNode;
		CGRuntimeTemporaryMatchingStorage(bool b, DtaScriptVariable* p) : bBegin(b), pClauseNode(p) {}
	};

	CGBNFRuntimeEnvironment::CGMatchingAreas::~CGMatchingAreas() {
		for (std::list<CGMatchingAreas*>::iterator i = childs.begin(); i != childs.end(); ++i) {
			delete *i;
		}
	}

	void CGBNFRuntimeEnvironment::CGMatchingAreas::purgeChildsAfterPosition(int iPosition) {
		while (!childs.empty() && (childs.back()->endPosition > iPosition)) {
			delete childs.back();
			childs.pop_back();
		}
	}

	CGBNFRuntimeEnvironment::~CGBNFRuntimeEnvironment() {
		delete _pMatchingAreas;
	}

	void CGBNFRuntimeEnvironment::pushIgnoreMode(CGBNFRuntimeIgnore& ignoreMode, int iNewIgnoreMode, EXECUTE_CLAUSE* newExecuteClause) {
		ignoreMode._executeClause = _executeClause;
		ignoreMode._iIgnoreMode = _iIgnoreMode;
		_iIgnoreMode = iNewIgnoreMode;
		_executeClause = newExecuteClause;
	}

	void CGBNFRuntimeEnvironment::popIgnoreMode(const CGBNFRuntimeIgnore& ignoreMode) {
		_executeClause = ignoreMode._executeClause;
		_iIgnoreMode = ignoreMode._iIgnoreMode;
	}

	void CGBNFRuntimeEnvironment::pushImplicitCopy(CGBNFRuntimeTransformationMode& transformationMode, bool bNewImplicitCopy) {
		transformationMode._bImplicitCopy = _bImplicitCopy;
		_bImplicitCopy = bNewImplicitCopy;
	}

	void CGBNFRuntimeEnvironment::popImplicitCopy(const CGBNFRuntimeTransformationMode& transformationMode) {
		_bImplicitCopy = transformationMode._bImplicitCopy;
	}

	void CGBNFRuntimeEnvironment::activateMatchingAreas() {
		_pMatchingAreas = new CGMatchingAreas(NULL, -1);
	}

	void CGBNFRuntimeEnvironment::storeClauseMatching(DtaScriptVariable& ruleNames, std::map<int, std::map<int, std::list<CGRuntimeTemporaryMatchingStorage*> > >& mapOfAreas, CGMatchingAreas* pClauseMatching) {
		if (pClauseMatching == NULL) return;
		int iBegin = pClauseMatching->beginPosition;
		int iEnd = pClauseMatching->endPosition;
		if (iBegin != iEnd) {
			DtaScriptVariable* pClauseNode = ruleNames.getArrayElement(pClauseMatching->clause);
			mapOfAreas[iBegin][iEnd].push_back(new CGRuntimeTemporaryMatchingStorage(true, pClauseNode));
			for (std::list<CGMatchingAreas*>::iterator i = pClauseMatching->childs.begin(); i != pClauseMatching->childs.end(); ++i) {
				storeClauseMatching(ruleNames, mapOfAreas, *i);
			}
			mapOfAreas[iEnd][iBegin].push_back(new CGRuntimeTemporaryMatchingStorage(false, pClauseNode));
		}
	}

	void CGBNFRuntimeEnvironment::storeMatchingAreas(CppParsingTree_var& pStorage) {
		DtaScriptVariable* pAreas = pStorage.insertNode("areas").getInternalNode();
		DtaScriptVariable* pRules = pStorage.insertNode("rules").getInternalNode();
		for (std::list<std::string>::iterator i = _clauseSignatures.begin(); i != _clauseSignatures.end(); ++i) {
			DtaScriptVariable* pClauseNode = pRules->addElement(*i);
			pClauseNode->setValue(i->c_str());
		}
		std::map<int, std::map<int, std::list<CGRuntimeTemporaryMatchingStorage*> > > mapOfAreas;
		if ((_pMatchingAreas != NULL) && !_pMatchingAreas->childs.empty()) storeClauseMatching(*pRules, mapOfAreas, _pMatchingAreas->childs.front());
		{
			for (std::map<int, std::map<int, std::list<CGRuntimeTemporaryMatchingStorage*> > >::iterator i = mapOfAreas.begin(); i != mapOfAreas.end(); ++i) {
				DtaScriptVariable* pPosition = pAreas->addElement(i->first);
				for (std::map<int, std::list<CGRuntimeTemporaryMatchingStorage*> >::reverse_iterator j = i->second.rbegin(); j != i->second.rend(); ++j) {
					{
						for (std::list<CGRuntimeTemporaryMatchingStorage*>::iterator k = j->second.begin(); k != j->second.end(); ++k) {
							if ((*k)->bBegin) {
								pPosition->insertNode("begin")->addElement(j->first)->pushItem("")->setValue((*k)->pClauseNode);
							}
						}
					}
					{
						for (std::list<CGRuntimeTemporaryMatchingStorage*>::iterator k = j->second.begin(); k != j->second.end(); ++k) {
							if (!(*k)->bBegin) {
								pPosition->insertNode("end")->addElement(j->first)->pushItem("")->setValue((*k)->pClauseNode);
							}
						}
					}
					{
						// delete the temporary storage
						for (std::list<CGRuntimeTemporaryMatchingStorage*>::iterator k = j->second.begin(); k != j->second.end(); ++k) {
							delete *k;
						}
					}
				}
			}
		}
	}

	int CGBNFRuntimeEnvironment::skipEmptyChars() {
		int iImplicitCopyPosition;
		int iLocation;
		if (_bImplicitCopy) {
			iImplicitCopyPosition = CGRuntime::getOutputLocation();
			iLocation = CGRuntime::getInputLocation();
		} else {
			iImplicitCopyPosition = -1;
		}
		if ((_iIgnoreMode != (int) NOT_IGNORE) && (_iIgnoreMode != (int) UNDEFINED_IGNORE)) {
			bool bCopyImplicitly = _bImplicitCopy;
			switch(_iIgnoreMode) {
				case IGNORE_CPP:
				case IGNORE_JAVA:
					CGRuntime::_pInputStream->skipEmpty();
					break;
				case IGNORE_HTML:
					CGRuntime::_pInputStream->skipEmptyHTML();
					break;
				case IGNORE_BLANKS:
					CGRuntime::_pInputStream->skipBlanks();
					break;
				case IGNORE_SPACES:
					CGRuntime::_pInputStream->skipSpaces();
					break;
				case IGNORE_ADA:
					CGRuntime::_pInputStream->skipEmptyAda();
					break;
				case IGNORE_LATEX:
					CGRuntime::_pInputStream->skipEmptyLaTeX();
					break;
				case IGNORE_CPP_EXCEPT_DOXYGEN:
					CGRuntime::_pInputStream->skipEmptyCppExceptDoxygen();
					break;
				case IGNORE_CLAUSE:
					bCopyImplicitly = false;
					if (_executeClause != NULL) {
						CGBNFRuntimeEnvironment theEnvironment(NULL, NOT_IGNORE, _bImplicitCopy);
						_executeClause->run(theEnvironment);
					}
					break;
				default:
					throw UtlException("internal error in CGBNFRuntimeEnvironment::skipEmptyChars(): unrecognized ignore mode encountered");
			}
			if (bCopyImplicitly) {
				int iLastLocation = CGRuntime::getInputLocation();
				if (iLastLocation > iLocation) {
					std::string sText = CGRuntime::getLastReadChars(iLastLocation - iLocation);
					CGRuntime::writeBinaryData(sText.c_str(), sText.size());
				}
			}
		}
		return iImplicitCopyPosition;
	}

	void CGBNFRuntimeEnvironment::writeBinaryData(const char* tcText, int iLength) {
		CGRuntime::writeBinaryData(tcText, iLength);
	}


	CGBNFRuntimeClauseMatchingAreaValidator::CGBNFRuntimeClauseMatchingAreaValidator(const char* tcClause, CGBNFRuntimeEnvironment* pBNFScript) {
		if (pBNFScript->_pMatchingAreas == NULL) {
			pBNFScript_ = NULL;
		} else {
			pBNFScript_ = pBNFScript;
			pOld_ = pBNFScript->_pMatchingAreas;
			pBNFScript->_pMatchingAreas = new CGBNFRuntimeEnvironment::CGMatchingAreas(tcClause, CGRuntime::getInputLocation());
		}
	}

	CGBNFRuntimeClauseMatchingAreaValidator::~CGBNFRuntimeClauseMatchingAreaValidator() {
		if (pBNFScript_ != NULL) {
			if (pBNFScript_->_pMatchingAreas->endPosition < 0) {
				// the clause has failed!
				delete pBNFScript_->_pMatchingAreas;
			}
			pBNFScript_->_pMatchingAreas = pOld_;
		}
	}

	void CGBNFRuntimeClauseMatchingAreaValidator::validate() {
		if (pBNFScript_ != NULL) {
			CGBNFRuntimeEnvironment::CGMatchingAreas* pNew = pBNFScript_->_pMatchingAreas;
			if (pNew->endPosition < 0) {
				// prevents aginst multiple call to 'validate()' (shouldn't)
				pNew->endPosition = CGRuntime::getInputLocation();
				pOld_->pushChild(pNew);
			}
		}
	}

	void CGBNFRuntimeClauseMatchingAreaValidator::purgeChildsAfterPosition(int iLocation) {
		if (pBNFScript_ != NULL) {
			pBNFScript_->_pMatchingAreas->purgeChildsAfterPosition(iLocation);
		}
	}


	CGBNFRuntimeResizeInput::CGBNFRuntimeResizeInput(int iFinalLocation) {
		_pSizeAttributes = new ScpStream::SizeAttributes(CGRuntime::_pInputStream->resize(iFinalLocation));
	}

	CGBNFRuntimeResizeInput::~CGBNFRuntimeResizeInput() {
		ScpStream::SizeAttributes* pSizeAttributes = (ScpStream::SizeAttributes*) _pSizeAttributes;
		CGRuntime::_pInputStream->restoreSize(*pSizeAttributes);
		delete pSizeAttributes;
	}


	CGJointPointStack::~CGJointPointStack() {
		CGRuntime::_pJointPoint = _pOldJointPoint;
	}


	CGBNFClauseScope::~CGBNFClauseScope() {
		if (_pLocalVariables != NULL) delete _pLocalVariables;
	}

	CppParsingTree_var CGBNFClauseScope::getNode(const std::string& sName) const {
		if (_pLocalVariables != NULL) {
			CppParsingTree_var theNode = _pLocalVariables->getNode(sName.c_str());
			if (!theNode.isNull()) return theNode;
		}
		return CGRuntime::getThisTree().getNode(sName);
	}

	CppParsingTree_var CGBNFClauseScope::getEvaluatedNode(const std::string& sDynamicVariable) const {
		if (_pLocalVariables != NULL) {
			CppParsingTree_var theNode = _pLocalVariables->getEvaluatedNode(sDynamicVariable);
			if (!theNode.isNull()) return theNode;
		}
		return CGRuntime::getThisTree().getEvaluatedNode(sDynamicVariable);
	}

	CppParsingTree_var CGBNFClauseScope::insertNode(const std::string& sName) {
		if (_pLocalVariables != NULL) {
			DtaScriptVariable* pNode = _pLocalVariables->getNode(sName.c_str());
			if (pNode != NULL) return pNode;
		} else {
			_pLocalVariables = new DtaScriptVariable;
		}
		return _pLocalVariables->insertNode(sName.c_str());
	}

	CppParsingTree_var CGBNFClauseScope::insertClassicalNode(const std::string& sName) {
		if (_pLocalVariables != NULL) {
			DtaScriptVariable* pNode = _pLocalVariables->getNode(sName.c_str());
			if (pNode != NULL) return pNode;
		}
		return CGRuntime::getThisTree().insertNode(sName);
	}

	CppParsingTree_var CGBNFClauseScope::insertEvaluatedNode(const std::string& sDynamicVariable) {
		if (_pLocalVariables == NULL) _pLocalVariables = new DtaScriptVariable;
		return _pLocalVariables->insertEvaluatedNode(sDynamicVariable);
	}

	CppParsingTree_var CGBNFClauseScope::insertClassicalEvaluatedNode(const std::string& sDynamicVariable) {
		if (_pLocalVariables == NULL) _pLocalVariables = new DtaScriptVariable;
		return _pLocalVariables->insertClassicalEvaluatedNode(sDynamicVariable);
	}

	CppParsingTree_var CGBNFClauseScope::getOrCreateLocalNode(const std::string& sName) {
		if (_pLocalVariables != NULL) {
			DtaScriptVariable* pNode = _pLocalVariables->getNode(sName.c_str());
			if (pNode != NULL) return pNode;
		}
		CppParsingTree_var theNode = CGRuntime::getThisTree().getNode(sName);
		if (theNode.isNull()) {
			if (_pLocalVariables == NULL) _pLocalVariables = new DtaScriptVariable(NULL, "##stack## clause");
			theNode = _pLocalVariables->insertNode(sName.c_str());
		}
		return theNode;
	}
}
