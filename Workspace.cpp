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
#	pragma warning (disable : 4786)
#endif

#include <iostream>
#include <fstream>
#include <memory>

#include "UtlException.h"

#ifndef WIN32
#	include "UtlString.h" // for Debian/gcc 2.95.4
#endif

#include "ScpStream.h"
#include "UtlTimer.h"

#include "DtaProject.h"
#include "CGRuntime.h"
#include "DtaConsoleScript.h"
#include "DtaPatternScript.h"
#include "CppCompilerEnvironment.h"
#include "CGExternalHandling.h"

#include "ExprScriptVariable.h"
#include "GrfFunction.h"
#include "Workspace.h"

namespace CodeWorker {
	std::string Workspace::_sTraceFileName;
	bool Workspace::_bWarningOnUnknownVariables = false;
	std::list<GrfFunction*> Workspace::_listOfExternalFunctions;


	Workspace::Workspace() : _bConsole(true), _bDebugMode(false), _bTimeMeasure(false), _iScriptMode(NONE), _bQuietMode(false), _pQuietCGIOutput(NULL), _bNoLogo(false) {
#ifndef WIN32
		initKeyboard();
#endif
	}

	Workspace::~Workspace() {
		if (isCGIMode()) {
			std::string sOutputDocument = CGRuntime::loadVirtualFile(_sGeneratedFileName);
			CGRuntime::deleteVirtualFile(_sGeneratedFileName);
			CGRuntime::deleteVirtualFile(_sScriptFileName);
			if (!CGRuntime::getProperty("HTTP_STATUS_CODE").empty()) {
				std::cout << CGRuntime::getEnv("SERVER_PROTOCOL") << " " << CGRuntime::getProperty("HTTP_STATUS_CODE") << " " << CGRuntime::getProperty("HTTP_REASON_LINE") << CGRuntime::endl();
			}
			bool bTypeReclaimed = true;
			if (!CGRuntime::getProperty("HTTP_LOCATION").empty()) {
				bTypeReclaimed = false;
				std::cout << "Location: " << CGRuntime::getProperty("HTTP_LOCATION") << CGRuntime::endl();
			}
			if (bTypeReclaimed && CGRuntime::getProperty("HTTP_CONTENT_TYPE").empty()) {
				CGRuntime::setProperty("HTTP_CONTENT_TYPE", "text/html");
			}
			if (!CGRuntime::getProperty("HTTP_CONTENT_TYPE").empty()) {
				std::cout << "Content-type: " << CGRuntime::getProperty("HTTP_CONTENT_TYPE") << CGRuntime::endl();
			}
			std::cout << CGRuntime::endl();
			std::cout << sOutputDocument << std::flush;
			delete _pQuietCGIOutput;
		}
#ifndef WIN32
		closeKeyboard();
#endif
	}

	int Workspace::executionHasFailed() {
		if (isCGIMode()) {
			CGRuntime::deleteVirtualFile(_sGeneratedFileName);
			CGRuntime::deleteVirtualFile(_sScriptFileName);
			if (!CGRuntime::getProperty("HTTP_STATUS_CODE").empty()) {
				std::cout << CGRuntime::getEnv("SERVER_PROTOCOL") << " " << CGRuntime::getProperty("HTTP_STATUS_CODE") << " " << CGRuntime::getProperty("HTTP_REASON_LINE") << CGRuntime::endl();
			}
			std::cout << "Content-type: text/html" << CGRuntime::endl();
			std::cout << CGRuntime::endl();
			std::cout << "<HTML>" << std::endl;
			std::cout << "\t<BODY>" << std::endl;
			std::string sError = _pQuietCGIOutput->getOutput();
			sError = CGRuntime::replaceString("\n", "<BR/>", sError);
			sError = CGRuntime::replaceString("\r", "", sError);
			std::cout << "\t\t" << sError << std::endl;
			std::cout << "\t</BODY>" << std::endl;
			std::cout << "</HTML>" << std::endl;
			delete _pQuietCGIOutput;
			_pQuietCGIOutput = NULL;
		}
		return -1;
	}

	void Workspace::setTraceFileName(const std::string& sTraceFileName) {
		_sTraceFileName = sTraceFileName;
		std::ofstream* pFile = CodeWorker::openOutputFile(_sTraceFileName.c_str());
		if (pFile != NULL) {
			pFile->close();
			delete pFile;
		}
	}

	std::string Workspace::readArgument(char** tsArgs, int& i) {
		return tsArgs[i];
	}

	int Workspace::readInteger(char* tcArgs, int& i) {
		int iResult = 0;
		char a = tcArgs[i];
		if ((a < '0') || (a > '9')) {
			throw UtlException("command-line processing: integer expected somewhere on the command line");
		}
		while ((a >= '0') && (a <= '9')) {
			iResult *= 10;
			iResult += (a - '0');
			a = tcArgs[++i];
		}
		return iResult;
	}

	bool Workspace::checkArguments(int iNargs, char** tsArgs, EXECUTE_FUNCTION* executeFunction) {
		if (CGRuntime::existEnv("GATEWAY_INTERFACE") && CGRuntime::existEnv("REQUEST_METHOD")) {
			// CGI mode
			_bQuietMode = true;
			_pQuietCGIOutput = new CGQuietOutput;
			_sScriptFileName = CGRuntime::getEnv("PATH_TRANSLATED");
			_iScriptMode = GENERATE;
			_bConsole = false;
			std::string sRequestMethod = CGRuntime::getEnv("REQUEST_METHOD");
			std::string sQueryString;
			if ((stricmp(sRequestMethod.c_str(), "post") == 0) || (stricmp(sRequestMethod.c_str(), "put") == 0)) {
				std::string sContentLength = CGRuntime::getEnv("CONTENT_LENGTH");
				int iLength = atoi(sContentLength.c_str());
				char* tcQueryString = new char[iLength + 1];
				int iRemainingSize = iLength;
				while (iRemainingSize > 0) {
					int iBytes = fread(tcQueryString, 1, iRemainingSize, stdin);
					if (iBytes == 0) {
						delete [] tcQueryString;
						std::string sMessage = "CGI processing failed at initialization: ";
						if (iRemainingSize != iLength) {
							char tcNumber[32];
							sprintf(tcNumber, "%d", (iLength - iRemainingSize));
							sMessage += "incomplete extraction of the " + sRequestMethod + " query (have read " + std::string(tcNumber) + "/" + CGRuntime::getEnv("CONTENT_LENGTH") + " bytes)";
						} else {
							sMessage += "extraction of the " + sRequestMethod + " query didn't work (had " + CGRuntime::getEnv("CONTENT_LENGTH") + " bytes to read)";
						}
						throw UtlException(sMessage);
					}
					iRemainingSize -= iBytes;
				}
				tcQueryString[iLength] = '\0';
				sQueryString = tcQueryString;
				delete [] tcQueryString;
			} else if (CGRuntime::existEnv("QUERY_STRING")) {
				sQueryString = CGRuntime::getEnv("QUERY_STRING");
			}
			DtaScriptVariable* pREQUEST = DtaProject::getInstance().setGlobalVariable("_REQUEST");
			std::string::size_type iIndex = sQueryString.find('=');
			while (iIndex != std::string::npos) {
				std::string sAttribute = sQueryString.substr(0, iIndex);
				sAttribute = CGRuntime::decodeURL(sAttribute);
				std::string sValue;
				iIndex++;
				if (iIndex < sQueryString.size()) sQueryString = sQueryString.substr(iIndex);
				else sQueryString = ""; // bug VC++ 6 on substr()
				iIndex = sQueryString.find('&');
				if (iIndex == std::string::npos) {
					sValue = sQueryString;
					sQueryString = "";
				} else {
					sValue = sQueryString.substr(0, iIndex);
					iIndex++;
					if (iIndex < sQueryString.size()) sQueryString = sQueryString.substr(iIndex);
					else sQueryString = ""; // bug VC++ 6 on substr()
				}
				sValue = CGRuntime::decodeURL(sValue);
				if (CGRuntime::isIdentifier(sAttribute)) {
					DtaScriptVariable* pAttribute = DtaProject::getInstance().setGlobalVariable(sAttribute);
					pAttribute->setValue(sValue.c_str());
					pREQUEST->addElement(sAttribute)->setValue(pAttribute);
				} else {
					pREQUEST->addElement(sAttribute)->setValue(sValue.c_str());
				}
				iIndex = sQueryString.find('=');
			}
			if (!sQueryString.empty()) {
				sQueryString = CGRuntime::decodeURL(sQueryString);
				if (CGRuntime::isIdentifier(sQueryString)) {
					DtaScriptVariable* pAttribute = DtaProject::getInstance().setGlobalVariable(sQueryString);
					pREQUEST->addElement(sQueryString)->setValue(pAttribute);
				} else {
					pREQUEST->addElement(sQueryString);
				}
			}
			_sGeneratedFileName = CGRuntime::createVirtualTemporaryFile("");
		} else {
			if ((iNargs == 2) && (strncmp(tsArgs[1], "-cut=", 5) == 0)) {
				int i = 5;
				std::vector<int> lengths;
				do {
					lengths.push_back(readInteger(tsArgs[1], i));
				} while(tsArgs[1][i++] == '|');
				if (tsArgs[1][i - 1] != ' ') {
					throw UtlException("command-line processing: '-cut' ends with a space character");
				}
				int iNewNargs = lengths.size() + 2;
				char** tcNewArgs = new char*[iNewNargs];
				const char* tcRemaningCmdLine = tsArgs[1] + i;
				i = 0;
				tcNewArgs[i++] = NULL;
				for (std::vector<int>::const_iterator iter = lengths.begin(); iter != lengths.end(); ++iter) {
					tcNewArgs[i] = new char[*iter + 1];
					strncpy(tcNewArgs[i++], tcRemaningCmdLine, *iter);
					tcRemaningCmdLine += *iter + 1;
				}
				tcNewArgs[i] = new char[strlen(tcRemaningCmdLine) + 1];
				strcpy(tcNewArgs[i], tcRemaningCmdLine);
				return executeArguments(iNewNargs, tcNewArgs, executeFunction);
			}
			return executeArguments(iNargs, tsArgs, executeFunction);
		}
		return true;
	}

	bool Workspace::executeArguments(int iNargs, char** tsArgs, EXECUTE_FUNCTION* executeFunction) {
		// parsing the command line
		std::string sWorkingPath;
		std::list<std::string> listOfIncludePaths;
		std::map<std::string, std::string> mapOfDefinedTargets;
		bool bSpeed = false;
		int i = 1;
		bool bConsoleExplicitlyRequired = false;
		_bConsole = (executeFunction == NULL);
		_sScriptFileName = "";
		_sParsedFileName = "";
		_sGeneratedFileName = "";
		_iScriptMode = NONE;
		_sCppCompilation = "";
		_bQuietMode = false;
		std::string sFileExtension; // if the script filename is passed as the first argument
		if ((iNargs >= 2) && (tsArgs[1][0] != '-') && (executeFunction == 0)) {
			std::string sFile = tsArgs[1];
			if (sFile.size() > 4) {
				sFileExtension = sFile.substr(sFile.size() - 4);
				if ((sFileExtension != ".cwt") && (sFileExtension != ".cwp") && (sFileExtension != ".cws") && (sFileExtension != ".gen")) {
					sFileExtension = "";
				} else {
					_sScriptFileName = sFile;
					if (sFileExtension == ".cwp") {
						i++;
						if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: input filename expected after the BNF parsing script");
						_sParsedFileName = readArgument(tsArgs, i);
						_iScriptMode = PARSE_BNF;
					} else {
						_iScriptMode = SCRIPT;
					}
					i++;
				}
			}
		}
		if ((iNargs >= 2) && (stricmp(tsArgs[1], "-commands") == 0)) {
			// it isn't a script without arguments, but a command-line file
			if (iNargs != 3) throw UtlException("command-line processing: '-commands' expects a file name and nothing else behind");
			std::string sFile = tsArgs[2];
			return checkArgumentsFromFile(sFile, executeFunction);
		}
		if (!sFileExtension.empty() || (executeFunction != 0)) {
			_bConsole = false;
			if ((i < iNargs) && (tsArgs[i][0] != '-')) {
				DtaScriptVariable* pArgs = DtaProject::getInstance().setGlobalVariable("_ARGS");
				do {
					pArgs->pushItem(tsArgs[i]);
					i++;
				} while ((i < iNargs) && (tsArgs[i][0] != '-'));
			}
		}
		while (i < iNargs) {
			if (tsArgs[i][0] == '-') {
				const char* tcCommand = tsArgs[i] + 1;
				if (tcCommand[0] == '-') tcCommand++;
				if (stricmp(tcCommand, "script") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: script filename expected after '-script'");
					if (_iScriptMode != NONE) throw UtlException("command-line processing: '-script' isn't allowed: another script mode has already been specified");
					_sScriptFileName = readArgument(tsArgs, i);
					_iScriptMode = SCRIPT;
					_bConsole = false;
				} else if (stricmp(tcCommand, "compile") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: script filename expected after '-compile'");
					if (_iScriptMode != NONE) throw UtlException("command-line processing: '-compile' isn't allowed: another script mode has already been specified");
					_sScriptFileName = readArgument(tsArgs, i);
					_iScriptMode = COMPILE;
					_bConsole = false;
				} else if (stricmp(tcCommand, "parsebnf") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: script filename expected after '-parseBNF'");
					if (_iScriptMode != NONE) throw UtlException("command-line processing: '-parseBNF' isn't allowed: another script mode has already been specified");
					_sScriptFileName = readArgument(tsArgs, i);
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: input filename expected after '-parseBNF <BNF-parsing-script>'");
					_sParsedFileName = readArgument(tsArgs, i);
					_iScriptMode = PARSE_BNF;
					_bConsole = false;
				} else if (stricmp(tcCommand, "generate") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: script filename expected after '-generate'");
					if (_iScriptMode != NONE) throw UtlException("command-line processing: '-generate' isn't allowed: another script mode has already been specified");
					_sScriptFileName = readArgument(tsArgs, i);
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: generated filename expected after '-generate <patternFile>'");
					_sGeneratedFileName = readArgument(tsArgs, i);
					_iScriptMode = GENERATE;
					_bConsole = false;
				} else if (stricmp(tcCommand, "expand") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: script filename expected after '-expand'");
					if (_iScriptMode != NONE) throw UtlException("command-line processing: '-expand' isn't allowed: another script mode has already been specified");
					_sScriptFileName = readArgument(tsArgs, i);
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: generated filename expected after '-expand <patternFile>'");
					_sGeneratedFileName = readArgument(tsArgs, i);
					_iScriptMode = EXPAND;
					_bConsole = false;
				} else if (stricmp(tcCommand, "autoexpand") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: filename expected after '-autoexpand'");
					if (_iScriptMode != NONE) throw UtlException("command-line processing: '-autoexpand' isn't allowed: another script mode has already been specified");
					_sScriptFileName = readArgument(tsArgs, i);
					_iScriptMode = AUTO_EXPAND;
					_bConsole = false;
				} else if (stricmp(tcCommand, "translate") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: filename of a BNF parsing script expected after '-translate'");
					if (_iScriptMode != NONE) throw UtlException("command-line processing: '-translate' isn't allowed: another script mode has already been specified");
					_sScriptFileName = readArgument(tsArgs, i);
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: source file expected after '-translate <BNF-script>'");
					_sParsedFileName = readArgument(tsArgs, i);
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: generated file expected after '-translate <BNF-script> <source-file>'");
					_sGeneratedFileName = readArgument(tsArgs, i);
					_iScriptMode = TRANSLATE;
					_bConsole = false;
				} else if (stricmp(tcCommand, "args") == 0) {
					DtaScriptVariable* pArgs = DtaProject::getInstance().setGlobalVariable("_ARGS");
					i++;
					while ((i < iNargs) && (tsArgs[i][0] != '-')) {
						pArgs->pushItem(tsArgs[i]);
						i++;
					}
					continue;
				} else if (stricmp(tcCommand, "insert") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: variable expected after '-insert'");
					std::string sVariable = readArgument(tsArgs, i);
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: value expected after '-insert'");
					std::string sValue = readArgument(tsArgs, i);
					DtaScript script;
					ScpStream theCommand(sVariable);
					std::auto_ptr<ExprScriptVariable> pExprVariable(script.parseVariableExpression(script.getBlock(), theCommand));
					if (theCommand.getInputLocation() != (int) sVariable.size()) throw UtlException("option '-insert': syntax error on the variable expression");
					DtaProject::getInstance().getVariable(*pExprVariable)->setValue(sValue.c_str());
				} else if (stricmp(tcCommand, "console") == 0) {
					bConsoleExplicitlyRequired = true;
				} else if (stricmp(tcCommand, "debug") == 0) {
					_bDebugMode = true;
					DtaScript::requiresParsingInformation(true);
					++i;
					if ((i < iNargs) && (tsArgs[i][0] != '-')) {
						std::string sRemote = readArgument(tsArgs, i);
						std::string::size_type iIndex = sRemote.find(':');
						if (iIndex == std::string::npos) throw UtlException("command-line processing: ':' expected to separate host and port after '-debug'");
						std::string sHost = sRemote.substr(0, iIndex);
						std::string sPort = sRemote.substr(iIndex + 1);
						int iPort = atoi(sPort.c_str());
						if (iPort <= 0) throw UtlException("command-line processing: valid socket port expected after '-debug'");
						DtaProject::getInstance().setRemoteDebug(sHost, iPort);
					} else {
						--i;
					}
				} else if (stricmp(tcCommand, "time") == 0) {
					_bTimeMeasure = true;
					DtaScript::requiresParsingInformation(true);
				} else if (stricmp(tcCommand, "varexist") == 0) {
					_bWarningOnUnknownVariables = true;
				} else if (stricmp(tcCommand, "nologo") == 0) {
					_bNoLogo = true;
				} else if (stricmp(tcCommand, "verbose") == 0) {
					DtaProject::getInstance().setVerbose(true);
				} else if (stricmp(tcCommand, "version") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: version name expected after '-version'");
					DtaProject::getInstance().setVersion(tsArgs[i]);
				} else if (stricmp(tcCommand, "fast") == 0) {
					bSpeed = true;
				} else if (stricmp(tcCommand, "c++") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: directory expected after '-c++'");
					if (!_sCppCompilation.empty()) throw UtlException("command-line processing: C++ project directory is given twice (switch '-c++')");
					_sCppCompilation = readArgument(tsArgs, i);
					i++;
					if ((i < iNargs) && (tsArgs[i][0] != '-')) {
						DtaProject::getInstance().setCodeWorkerHome(readArgument(tsArgs, i));
					} else {
						i--;
					}
					_bConsole = false;
				} else if (stricmp(tcCommand, "c++2target") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: script filename expected after '-c++2target'");
					if (_iScriptMode != NONE) throw UtlException("command-line processing: '-c++2target' isn't allowed: another script mode has already been specified");
					_sScriptFileName = readArgument(tsArgs, i);
					_iScriptMode = TARGET;
					_bConsole = false;
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: directory expected after '-c++2target'");
					if (!_sCppCompilation.empty()) throw UtlException("command-line processing: C++ project directory is given twice (switch '-c++2target')");
					_sCppCompilation = readArgument(tsArgs, i);
					i++;
					if ((i < iNargs) && (tsArgs[i][0] != '-')) {
						std::string sTargetLanguage = readArgument(tsArgs, i);
						std::string::size_type iIndex = sTargetLanguage.find('=');
						if (iIndex != std::string::npos) {
							std::string sProperty = sTargetLanguage.substr(iIndex + 1);
							mapOfDefinedTargets["c++2target-property"] = sProperty;
							sTargetLanguage = sTargetLanguage.substr(0, iIndex);
						}
						if (DtaProject::getInstance().hasTargetLanguage() && (DtaProject::getInstance().getTargetLanguage() != sTargetLanguage)) throw UtlException("command-line processing: do not specify more than one target language (switch '-c++2target')");
						DtaProject::getInstance().setTargetLanguage(sTargetLanguage);
					} else {
						i--;
					}
				} else if (stricmp(tcCommand, "c++external") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: C++ filename expected after '-c++sp'");
					if (!_sCppServerPage.empty()) throw UtlException("command-line processing: C++ filename is given twice for server pages");
					_sCppServerPage = readArgument(tsArgs, i);
				} else if (stricmp(tcCommand, "home") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: CodeWorker path expected after '-home'");
					DtaProject::getInstance().setCodeWorkerHome(readArgument(tsArgs, i));
				} else if (stricmp(tcCommand, "genheader") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: comment expected after '-genheader'");
					DtaProject::getInstance().setGenerationHeader(tsArgs[i]);
				} else if (stricmp(tcCommand, "quantify") == 0) {
					i++;
					if ((i < iNargs) && (tsArgs[i][0] != '-')) {
						_sQuantifyFile = readArgument(tsArgs, i);
					} else {
						_sQuantifyFile = "true";
					}
					DtaScript::requiresParsingInformation(true);
				} else if (stricmp(tcCommand, "stdin") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: filename expected after '-stdin'");
					_sStandardInput = readArgument(tsArgs, i);
				} else if (stricmp(tcCommand, "stdout") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: filename expected after '-stdout'");
					_sStandardOutput = readArgument(tsArgs, i);
				} else if (stricmp(tcCommand, "stack") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: depth expected after '-stack'");
					int iStackDepth = atoi(tsArgs[i]);
					if (iStackDepth <= 0)  throw UtlException("command-line processing: invalid depth given after '-stack'");
					GrfFunction::_iMaxStackDepth = iStackDepth;
				} else if (stricmp(tcCommand, "path") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: working path expected after '-path'");
					sWorkingPath = readArgument(tsArgs, i);
					if ((sWorkingPath[sWorkingPath.size() - 1] != '\\') && (sWorkingPath[sWorkingPath.size() - 1] != '/')) sWorkingPath += '/';
				} else if ((stricmp(tcCommand, "define") == 0) || (stricmp(tcCommand, "D") == 0)) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: define target expected after '-define'");
					std::string sTargetName = readArgument(tsArgs, i);
					std::string sTargetValue;
					std::string::size_type iIndex = sTargetName.find("=");
					if (iIndex != std::string::npos) {
						if (iIndex == 0) throw UtlException("target name expected on the left side of '=' for define directive");
						if (iIndex + 1 < sTargetName.size()) {
							sTargetValue = sTargetName.substr(iIndex + 1);
							sTargetName = sTargetName.substr(0, iIndex);
						} else {
							sTargetName = sTargetName.substr(0, iIndex);
						}
					} else {
						sTargetValue = sTargetName;
					}
					mapOfDefinedTargets[sTargetName] = sTargetValue;
				} else if (stricmp(tcCommand, "I") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: include path expected after '-I'");
					std::string sIncludePath = readArgument(tsArgs, i);
					if ((sIncludePath[sIncludePath.size() - 1] != '\\') && (sIncludePath[sIncludePath.size() - 1] != '/')) sIncludePath += '/';
					listOfIncludePaths.push_back(sIncludePath);
				} else if (stricmp(tcCommand, "commentBegin") == 0) {
					i++;
					if (i >= iNargs) throw UtlException("command-line processing: format of a beginning of comment expected after '-commentBegin'");
					std::string sCommentBegin = readArgument(tsArgs, i);
					DtaProject::getInstance().setCommentBegin(sCommentBegin);
				} else if (stricmp(tcCommand, "commentEnd") == 0) {
					i++;
					if (i >= iNargs) throw UtlException("command-line processing: end of comment format expected after '-commentEnd'");
					std::string sCommentEnd = readArgument(tsArgs, i);
					DtaProject::getInstance().setCommentEnd(sCommentEnd);
				} else if (stricmp(tcCommand, "report") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: output file name expected after '-report'");
					std::string sOutputFile = readArgument(tsArgs, i);
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: flag expected as the second argument of '-report'");
					std::string sFlag = readArgument(tsArgs, i);
					int iFlag = atoi(sFlag.c_str());
					if (iFlag <= 0) throw UtlException("command-line processing: invalid flag passed to '-report'");
					DtaProject::getInstance().setFinalInfo(sOutputFile, iFlag);
				} else if (stricmp(tcCommand, "nowarn") == 0) {
					i++;
					if ((i >= iNargs) || (tsArgs[i][0] == '-')) throw UtlException("command-line processing: warning types expected after '-nowarn'");
					std::string sNoWarns = readArgument(tsArgs, i);
					std::string::size_type iIndex = sNoWarns.find('|');
					while (iIndex != std::string::npos) {
						std::string sNoWarn = sNoWarns.substr(0, iIndex);
						if (stricmp(sNoWarn.c_str(), "undeclvar") != 0) {
							throw UtlException("command-line processing: invalid warning type passed to '-nowarn'");
						}
						DtaProject::getInstance().addNoWarning(sNoWarn);
						sNoWarns = sNoWarns.substr(iIndex + 1);
						iIndex = sNoWarns.find('|');
					}
					DtaProject::getInstance().addNoWarning(sNoWarns);
				} else if (stricmp(tcCommand, "help") == 0) {
					displayHelp();
					_bConsole = false;
					_bQuietMode = true;
				} else if (stricmp(tcCommand, "quiet") == 0) {
					_bQuietMode = true;
				} else if (stricmp(tcCommand, "-commands") == 0) {
					throw UtlException("command-line processing: '-commands' must be the first argument");
				} else throw UtlException(std::string("command-line processing: unknown option '-") + (tcCommand) + "'");
			} else if (strcmp(tsArgs[i], "?") == 0) {
				displayHelp();
				_bConsole = false;
				_bQuietMode = true;
			} else {
				std::string sMessage = "a command always begins with a '-', so invalid parameter '";
				sMessage += readArgument(tsArgs, i);
				sMessage += "'";
				throw UtlException(sMessage);
			}
			i++;
		}

		if (!_sTraceFileName.empty()) {
			std::ofstream* pFile = CodeWorker::openAppendFile(_sTraceFileName.c_str());
			if (pFile != NULL) {
				(*pFile) << "commandLine" << std::endl;
				for (i = 1; i < iNargs; i++) {
					(*pFile) << "\t" << i << ":" << tsArgs[i] << std::endl;
				}
				(*pFile) << std::endl;
				pFile->close();
				delete pFile;
			}
		}

		if (sWorkingPath.empty()) sWorkingPath = "./";
		DtaProject::getInstance().setWorkingPath(sWorkingPath.c_str());
		ScpStream::setListOfIncludePaths(listOfIncludePaths);
		DtaProject::getInstance().setMapOfDefinedTargets(mapOfDefinedTargets);
		DtaProject::getInstance().setSpeed(bSpeed);
		if (bConsoleExplicitlyRequired) _bConsole = true;
		return true;
	}

	bool Workspace::checkArgumentsFromFile(const std::string& sFilename, EXECUTE_FUNCTION* executeFunction) {
		bool bSuccess = false;
		std::ifstream* pFile = ScpStream::openSTLInputFile(sFilename.c_str());
		if (pFile == NULL) {
			throw UtlException("unable to open command file \"" + sFilename + "\" for loading arguments");
		}
		try {
			while (CodeWorker::skipEmpty(*pFile)) {
				std::string sIdentifier;
				if (CodeWorker::readIdentifier(*pFile, sIdentifier)) {
					if (sIdentifier == "commandLine") {
						std::vector<std::string> listOfArguments;
						int iValue;
						int iIndex = 1;
						while (CodeWorker::skipEmpty(*pFile) && CodeWorker::readInt(*pFile, iValue)) {
							if (iValue != iIndex) {
								char tcMessage[80];
								sprintf(tcMessage, "bad argument index, '%d:' expected", iIndex);
								throw UtlException(*pFile, tcMessage);
							}
							CodeWorker::skipEmpty(*pFile);
							if (!CodeWorker::isEqualTo(*pFile, ':')) throw UtlException(*pFile, "':' expected");
							std::string sArgument;
							if (!CodeWorker::readLine(*pFile, sArgument)) throw UtlException(*pFile, "argument expected after ':'");
							listOfArguments.push_back(sArgument);
							iIndex++;
						}
						int iNbArgs = listOfArguments.size() + 1;
						char** tsArgs = new char*[iNbArgs];
						try {
							tsArgs[0] = NULL;
							for (int i = 1; i < iNbArgs; i++) tsArgs[i] = (char*) listOfArguments[i - 1].c_str();
							bSuccess = checkArguments(listOfArguments.size() + 1, tsArgs, executeFunction);
						} catch(UtlException&) {
							delete [] tsArgs;
							throw;
						} catch(std::exception&) {
							delete [] tsArgs;
							throw;
						} catch(...) {
							delete [] tsArgs;
							throw UtlException("ellipsis exception encountered while checking arguments of command line");
						}
						break;
					} else {
						throw UtlException(*pFile, "file of command-line switches: 'commandLine' keyword expected");
					}
				} else {
					throw UtlException(*pFile, "file of command-line switches: identifier expected");
				}
			}
		} catch(std::exception&) {
			pFile->close();
			delete pFile;
			throw;
		} catch(...) {
			pFile->close();
			delete pFile;
			throw UtlException("ellipsis exception encountered while checking arguments of command line");
		}
		pFile->close();
		delete pFile;
		return bSuccess;
	}

	void Workspace::displayHelp() {
		CGRuntime::traceLine("Common usages:");
		CGRuntime::traceLine("\t<scriptFile> <arg1> ... <argN> [<switch>]*");
		CGRuntime::traceLine("\t-script <scriptFile> -I ... -D ...");
		CGRuntime::traceLine("\t-generate <patternFile> <generatedFile>");
		CGRuntime::traceLine("\t-expand <patternFile> <generatedFile>");
		CGRuntime::traceLine("\t-parseAsBNF <BNF-File> <parsedFile>");
		CGRuntime::traceLine("Alphabetical detailed list of all switches:");
//##markup##"display switches"
//##script##
//local sOptions;
//quiet(sOptions) parseAsBNF(
//	{
//		injector	::= #continue #ignore(blanks) [->table]4;
//		table	::=
//				"\\begin{tableii}{l|l}{.6}{Switch}{Description}"
//				#continue
//				[line]*
//				"\\end{tableii}"
//				;
//		line	::=
//				"\\lineii"
//				#continue
//				=> traceText("  ");
//				bracket => traceText(": ");
//				bracket => traceLine("");
//				;
//		bracket	::= #continue '{' #!ignore bracket_content '}';
//		bracket_content ::=
//				[
//						"\\\\"
//					|
//						"\\_"
//						=> traceText('_');
//					|
//						"\\#"
//						=> traceText('#');
//					|
//						'\\' #readIdentifier:sCommand
//						command<sCommand>
//					|
//						"\""
//						=> traceText("\\\"");
//					|
//						" \\ref" ->'}'
//					|
//						['\r']? '\n' [' ' | '\t']*
//						=> traceText(' ');
//					|
//						[' ' | '\t']+
//						=> traceText(' ');
//					|
//						~'}':cChar
//						=> traceText(cChar);
//				]*
//				;
//		command<"samp">	::= bracket;
//		command<"textbf">	::=
//				=> traceText("'");
//				bracket
//				=> traceText("'");
//				;
//		command<"textit">	::=
//				=> traceText('<');
//				bracket
//				=> traceText('>');
//				;
//		command<"begin">	::= #continue "{itemize}" bracket_content '}';
//		command<"item">	::= => traceText(endl() + "      *"); ;
//		command<"end">	::= #continue "{itemize" => traceText(endl() + "    "); ;
//	}, project, "Documentation/CodeWorker.tex");
//local i = 0;
//local iLastLine = -1;
//local sText;
//@	CGRuntime::traceLine("@
//while $i < sOptions.length()$ {
//	local c = sOptions.charAt(i);
//  if $i - iLastLine >= 80$ {
//			set iLastLine = $i - (4 + sText.length())$;
//			@");@endl()@	CGRuntime::traceLine("   @
//	}
//	if c == ' ' {
//		@@sText@@
//		set sText = " ";
//	} else if c == '\n' {
//		set iLastLine = i;
//		@@sText@");@endl()@	CGRuntime::traceLine("@
//		set sText = "";
//	} else if c == '\r' {
//		increment(iLastLine);
//	} else {
//		set sText += c;
//	}
//	increment(i);
//}
//@@sText@");@endl()@@
//##script##
//##begin##"display switches"
	CGRuntime::traceLine("  -args '['<arg>']*': Pass some arguments to the command line. The list of");
	CGRuntime::traceLine("    arguments stops at the end of the command line or as soon as an option is");
	CGRuntime::traceLine("    encountered. The arguments are stored in a global array variable called");
	CGRuntime::traceLine("    _ARGS.");
	CGRuntime::traceLine("  -autoexpand <file-to-expand>: The file <file-to-expand> is explored for");
	CGRuntime::traceLine("    expanding code at markups, executing a <template-based> script inserted");
	CGRuntime::traceLine("    just below each markup. It is identical to execute the script function");
	CGRuntime::traceLine("    'autoexpand('<file-to-expand>', project)'.");
	CGRuntime::traceLine("  -c++ <generated-project-path> <CodeWorker-path>?: To translate the leader");
	CGRuntime::traceLine("    script and all its dependencies in C++ source code, once the execution of");
	CGRuntime::traceLine("    the leader script has achieved (same job as compileToCpp()). The");
	CGRuntime::traceLine("    <CodeWorker-path> is optional and gives the path through includes and");
	CGRuntime::traceLine("    libraries of the software. However, it is now recommended to specify");
	CGRuntime::traceLine("    <CodeWorker-path> by the switch -home.");
	CGRuntime::traceLine("  -c++2target <script-file> <generated-project-path> <target-language>?:  To");
	CGRuntime::traceLine("    translate the leader script and all its dependencies in C++ source code.");
	CGRuntime::traceLine("    Hence, the C++ is translated to a target language, all that once the");
	CGRuntime::traceLine("    execution of the leader script has achieved. Do not forget to give the");
	CGRuntime::traceLine("    path through includes and libraries of CodeWorker, setting the switch");
	CGRuntime::traceLine("    -home. A preprocessor definition called '\"c++2target-path\"' is");
	CGRuntime::traceLine("    automatically created. It contains the path of the generated project. Call");
	CGRuntime::traceLine("    getProperty(\"c++2target-path\") to retrieve the path value.");
	CGRuntime::traceLine("    <target-language> is optional if at least one script of the project holds");
	CGRuntime::traceLine("    the target into its filename, just before the extension. Example:");
	CGRuntime::traceLine("    \"myscript.java.cwt\" means that the target language of this script is");
	CGRuntime::traceLine("    \"java\". A property can follow the name of the target language, separated");
	CGRuntime::traceLine("    by a '=' symbol. The property is accessible via");
	CGRuntime::traceLine("    getProperty(\"c++2target-property\"), and its nature depends on the");
	CGRuntime::traceLine("    target. For instance, in Java, this property represents the package the");
	CGRuntime::traceLine("    generated classes will belong to. Example: java=org.landscape.mountains.");
	CGRuntime::traceLine("  -c++external <filename>: To generate C++ source code for implementing all");
	CGRuntime::traceLine("    functions declared as external into scripts.");
	CGRuntime::traceLine("  -commentBegin <format>: To specify the format of a beginning of comment.");
	CGRuntime::traceLine("  -commentEnd <format>: To specify the format of a comment's end.");
	CGRuntime::traceLine("  -compile <scriptFile>: To compile a script file, just to check whether the");
	CGRuntime::traceLine("    syntax is correct.");
	CGRuntime::traceLine("  -commands <commandFile>: To load all arguments processed ordinary on the");
	CGRuntime::traceLine("    command-line. It must be the only switch or else passed on the");
	CGRuntime::traceLine("    command-line.");
	CGRuntime::traceLine("  -console: To open a console session (default mode if no script to interpret");
	CGRuntime::traceLine("    is specified via -script or -compile or -generate or -expand.");
	CGRuntime::traceLine("  -debug [<remote>]?: To debug a script in a console while executing it. The");
	CGRuntime::traceLine("    optional argument <remote> defines parameters for a remote socket control");
	CGRuntime::traceLine("    of the debugging session. <remote> looks like '<hostname>:<port>'. If");
	CGRuntime::traceLine("    <hostname> is empty, CodeWorker runs as a socket server.");
	CGRuntime::traceLine("  -define VAR=<value> or -D ...: To define some variables, as when using the");
	CGRuntime::traceLine("    C++ preprocessor or when passing properties to the JAVA compiler. These");
	CGRuntime::traceLine("    variables are similar to properties, insofar as they aren't exploited");
	CGRuntime::traceLine("    during the preprocessing of scripts to interpret. This option conforms to");
	CGRuntime::traceLine("    the format -define VAR when no value has to be assigned ; in that case,");
	CGRuntime::traceLine("    <\"true\"> is assigned by default to variable VAR. The script function");
	CGRuntime::traceLine("    'getProperty('\"VAR\"')' gives the value of variable VAR.");
	CGRuntime::traceLine("  -expand <pattern-script> <file-to-expand>: Script file <pattern-script> is");
	CGRuntime::traceLine("    executed to expand file <file-to-expand> into markups. It is identical to");
	CGRuntime::traceLine("    execute script function 'expand('<pattern-script>', project,");
	CGRuntime::traceLine("    '<file-to-expand>')'.");
	CGRuntime::traceLine("  -fast: To optimize speed. While processing generation, the output file is");
	CGRuntime::traceLine("    built into memory, instead of into a temporary file.");
	CGRuntime::traceLine("  -generate <pattern-script> <file-to-generate>: Script file <pattern-script>");
	CGRuntime::traceLine("    is executed to generate file <file-to-generate>. It is identical to");
	CGRuntime::traceLine("    execute script function 'generate('<pattern-script>', project,");
	CGRuntime::traceLine("    '<file-to-generate>')'.");
	CGRuntime::traceLine("  -genheader <text>: Adds a header at the beginning of all generated files,");
	CGRuntime::traceLine("    followed by a text (see procedure setGenerationHeader()).");
	CGRuntime::traceLine("  -help or ?: Help about the command line.");
	CGRuntime::traceLine("  -home <CodeWorker-path>: Specifies the path to the home directory of");
	CGRuntime::traceLine("    CodeWorker.");
	CGRuntime::traceLine("  -I <path>: Specify a path to explore when trying to find a file while");
	CGRuntime::traceLine("    invoking include or parseFree or parseAsBNF or generate or expand or ...");
	CGRuntime::traceLine("    This option may be repeated to specify more than one <path>.");
	CGRuntime::traceLine("  -insert <variable_expression> <value>: Creates a new node in the main parse");
	CGRuntime::traceLine("    tree project and assigns a constant value to it. It is identical to");
	CGRuntime::traceLine("    execute the statement 'insert' <variable_expression> '= \"' <value> '\"");
	CGRuntime::traceLine("    ;'.");
	CGRuntime::traceLine("  -nologo: The interpreter doesn't write the copyright in the shell at the");
	CGRuntime::traceLine("    beginning.");
	CGRuntime::traceLine("  -nowarn <warnings>: Specified warning types are ignored. They are separated");
	CGRuntime::traceLine("    by pipe symbols. Today, the only recognized type is 'undeclvar', which");
	CGRuntime::traceLine("    prevents the developer against the use of a undeclared variable.");
	CGRuntime::traceLine("  -parseBNF <BNF-parsing-script> <source-file>: The script file");
	CGRuntime::traceLine("    <BNF-parsing-script> parses <source-file> from an extended BNF grammar. It");
	CGRuntime::traceLine("    is identical to execute the script function");
	CGRuntime::traceLine("    'parseAsBNF('<BNF-parsing-script>', project, '<source-file>')'.");
	CGRuntime::traceLine("  -path <path>: Output directory, returned by the script function");
	CGRuntime::traceLine("    'getWorkingPath()', and used ordinary to specify where to generate or copy");
	CGRuntime::traceLine("    a file.");
	CGRuntime::traceLine("  -quantify [<outputFile>]?: To execute scripts into quantify mode that");
	CGRuntime::traceLine("    consists of measuring the coverage and the time consuming. Results are");
	CGRuntime::traceLine("    saved to HTML file <outputFile> or displayed to the console if not");
	CGRuntime::traceLine("    present.");
	CGRuntime::traceLine("  -report <report-file> <request-flag>: To generate a report once the");
	CGRuntime::traceLine("    execution has achieved. The report is saved to file <report-file> and");
	CGRuntime::traceLine("    nature of information depends on the flag <request-flag>. This flag must");
	CGRuntime::traceLine("    be built by computing a bitwise OR for one or several of the following");
	CGRuntime::traceLine("    integer constants:  ");
	CGRuntime::traceLine("      * '1': provides every output file written by a template-based script");
	CGRuntime::traceLine("    (<generate()>, <expand()> or <translate>) ");
	CGRuntime::traceLine("      * '2': provides every input file scanned by a BNF parse script");
	CGRuntime::traceLine("    (<parseAsBNF()> or <translate()>) ");
	CGRuntime::traceLine("      * '4': provides details of coverage recording for every output file");
	CGRuntime::traceLine("    using the '#coverage' directive ");
	CGRuntime::traceLine("      * '8': provides details of coverage recording for every input file using");
	CGRuntime::traceLine("    the '#matching' directive ");
	CGRuntime::traceLine("      * '16': provides details of coverage recording for every output file");
	CGRuntime::traceLine("    written by a template-based script ");
	CGRuntime::traceLine("      * '32': provides details of coverage recording for every input file");
	CGRuntime::traceLine("    scanned by a BNF parse script ");
	CGRuntime::traceLine("     Notice that flags <16> and <32> may become highly time and memory");
	CGRuntime::traceLine("    consuming, depending both on how many input/output files you have to");
	CGRuntime::traceLine("    process and on their size.");
	CGRuntime::traceLine("  -script <script-file>: Defines the <leader> script, which will be executed");
	CGRuntime::traceLine("    first.");
	CGRuntime::traceLine("  -stack <depth>: To limit the recursive call of functions, for avoiding an");
	CGRuntime::traceLine("    overflow stack memory. By default, the <depth> is set to 1000.");
	CGRuntime::traceLine("  -stdin <filename>: To change the standard input for reading from an existing");
	CGRuntime::traceLine("    file. It may be useful for running a scenario.");
	CGRuntime::traceLine("  -stdout <filename>: To change the standard output for writing it to a file.");
	CGRuntime::traceLine("  -time: To display the execution time expressed in milliseconds, just before");
	CGRuntime::traceLine("    exiting.");
	CGRuntime::traceLine("  -translate <translation-script> <source-file> <file-to-generate>: Script");
	CGRuntime::traceLine("    file <translation-script> processes a <source-to-source> translation. It");
	CGRuntime::traceLine("    is identical to execute the script function");
	CGRuntime::traceLine("    'translate('<translation-script>', project, '<source-file>',");
	CGRuntime::traceLine("    '<file-to-generate>')'.");
	CGRuntime::traceLine("  -varexist: To trigger a warning when the value of a variable that doesn't");
	CGRuntime::traceLine("    exist is required into a script.");
	CGRuntime::traceLine("  -verbose: To display internal messages of the interpreter (information).");
	CGRuntime::traceLine("  -version <version-name>: To force interpreted scripts as written in a");
	CGRuntime::traceLine("    precedent version given by <version-name>.");
	CGRuntime::traceLine("");
//##end##"display switches"
	}


	bool Workspace::execute(EXECUTE_FUNCTION* executeFunction) {
		bool bSuccess = true;
		std::auto_ptr<CGFileStandardInputOutput> theStandardInputOutput;
		if (!_sStandardInput.empty() || !_sStandardOutput.empty()) {
			std::auto_ptr<CGFileStandardInputOutput> pFile(new CGFileStandardInputOutput(_sStandardInput, _sStandardOutput));
			theStandardInputOutput = pFile;
		}
		std::auto_ptr<CGRemoteDebugInputOutput> theRemoteDebugIO;
		if (DtaProject::getInstance().getRemoteDebugPort() > 0) {
			theRemoteDebugIO = std::auto_ptr<CGRemoteDebugInputOutput>(new CGRemoteDebugInputOutput(DtaProject::getInstance().getRemoteDebugHost(), DtaProject::getInstance().getRemoteDebugPort()));
		}
		UtlTimer myTimer;
		if (_bTimeMeasure) myTimer.start();
		try {
			if ((_iScriptMode != AUTO_EXPAND) && (_sScriptFileName.empty() || (executeFunction != NULL))) {
				if (executeFunction != NULL) {
					if		(_iScriptMode == PARSE_BNF)	CGRuntime::parseAsBNF(executeFunction, CGRuntime::getRootTree(), _sParsedFileName);
					else if	(_iScriptMode == EXPAND)	CGRuntime::expand(executeFunction, CGRuntime::getRootTree(), _sGeneratedFileName);
					else if	(_iScriptMode == TRANSLATE)	CGRuntime::translate(executeFunction, CGRuntime::getRootTree(), _sParsedFileName, _sGeneratedFileName);
					else if	(_iScriptMode == GENERATE)	CGRuntime::generate(executeFunction, CGRuntime::getRootTree(), _sGeneratedFileName);
					else executeFunction->run();
				}
			} else {
				if (executeFunction == NULL) {
					if (!_sGeneratedFileName.empty()) {
						ScpStream sCommand(8192);
						if (_iScriptMode == EXPAND) sCommand << "expand(";
						else if (_iScriptMode == TRANSLATE) sCommand << "translate(";
						else sCommand << "generate(";
						sCommand.writeString(_sScriptFileName.c_str());
						sCommand << ", project, ";
						if (_iScriptMode == TRANSLATE) {
							sCommand.writeString(_sParsedFileName.c_str());
							sCommand << ", ";
						}
						sCommand.writeString(_sGeneratedFileName.c_str());
						sCommand << ");";
						_script.parseStream(sCommand, _bDebugMode, _sQuantifyFile);
					} else if (!_sParsedFileName.empty()) {
						ScpStream sCommand(8192);
						sCommand << "parseAsBNF(";
						sCommand.writeString(_sScriptFileName.c_str());
						sCommand << ", project, ";
						sCommand.writeString(_sParsedFileName.c_str());
						sCommand << ");";
						_script.parseStream(sCommand, _bDebugMode, _sQuantifyFile);
					} else if (_iScriptMode == AUTO_EXPAND) {
						ScpStream sCommand(8192);
						sCommand << "autoexpand(";
						sCommand.writeString(_sScriptFileName.c_str());
						sCommand << ", project);";
						_script.parseStream(sCommand, _bDebugMode, _sQuantifyFile);
					} else {
						_script.parseFile(_sScriptFileName.c_str(), NULL, _bDebugMode, _sQuantifyFile);
					}
					if ((_iScriptMode != COMPILE) && (_iScriptMode != TARGET)) {
						_script.execute(CGRuntime::getThisInternalNode());
					}
					if (!_sCppCompilation.empty()) {
						CppCompilerEnvironment theCompilerEnvironment(_sCppCompilation);
						_script.compileCpp(theCompilerEnvironment, _sScriptFileName);
						theCompilerEnvironment.optimizeSources();
						if (_iScriptMode == TARGET) {
							std::string sTargetLanguage = DtaProject::getInstance().getTargetLanguage();
							if (sTargetLanguage.empty()) throw UtlException("unknown target language: should have been specified on switch -c++2target");
							theCompilerEnvironment.translateToTargetLanguage(sTargetLanguage);
						}
					}
				}
			}
			if (_sCppServerPage.empty()) {
				if (!_listOfExternalFunctions.empty()) {
					CGRuntime::traceLine("warning: some functions have been declared external, but option '-c++external' wasn't required on the command line!");
				}
			} else {
				std::string sRadical;
				std::string::size_type iIndex = _sCppServerPage.find_last_of('.');
				if (iIndex != std::string::npos) sRadical = _sCppServerPage.substr(0, iIndex);
				else sRadical = _sCppServerPage;
				std::string sClassName;
				iIndex = sRadical.find_last_of("/\\");
				if (iIndex != std::string::npos) sClassName = sRadical.substr(iIndex + 1);
				else sClassName = sRadical;
				CppParsingTree_value theContext;
				theContext.setValue(sClassName);
				ExecuteServerPageCppHeader executeServerPageCppHeader(*this);
				ExecuteServerPageCppBody executeServerPageCppBody(*this);
				CGRuntime::generate(&executeServerPageCppHeader, theContext, sRadical + ".h");
				CGRuntime::generate(&executeServerPageCppBody, theContext, sRadical + ".cpp");
			}
		} catch(UtlException& exception) {
			if (CGRuntime::getExternalHandling() == NULL) {
				CGRuntime::traceLine(exception.getMessage());
				CGRuntime::traceText(exception.getTraceStack());
			} else {
				std::string sMessage = exception.getMessage() + "\n" +  exception.getTraceStack();
				CGRuntime::getExternalHandling()->traceLine(sMessage);
			}
			bSuccess = false;
		}
		if (_bConsole) {
			DtaConsoleScript console(&(_script.getBlock()));
			console.execute(CGRuntime::getThisInternalNode());
		}
		if (!DtaProject::getInstance().getFinalInfoOutputFilename().empty()) {
			ScpStream stream;
			// list of all generated files?
			if ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_GENERATED_FILES) != 0) {
				stream << "generated_files {";stream.endl();
				const std::map<std::string, std::set<std::string> >& generatedFiles = DtaProject::getInstance().getCapturedOutputFiles();
				for (std::map<std::string, std::set<std::string> >::const_iterator iterTemplates = generatedFiles.begin(); iterTemplates != generatedFiles.end(); ++iterTemplates) {
					stream << "\t\"" << CGRuntime::composeCLikeString(iterTemplates->first) << "\"";
					if (iterTemplates->second.size() == 1) {
						stream << " = \"" << CGRuntime::composeCLikeString(*(iterTemplates->second.begin())) << "\"";
					} else {
						stream << "{";stream.endl();
						for (std::set<std::string>::const_iterator iterFiles = iterTemplates->second.begin(); iterFiles != iterTemplates->second.end(); ++iterFiles) {
							stream << "\t\t\"" << CGRuntime::composeCLikeString(*(iterTemplates->second.begin())) << "\"";stream.endl();
						}
						stream << "\t}";
					}
					if ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_TEMPLATE_COVERAGE) != 0) {
						stream << " {";stream.endl();
						stream.incrementIndentation();
						DtaScriptVariable* pStorage = DtaProject::getInstance().getGlobalVariable("_FINAL_INFO_TEMPLATE_COVERAGE");
						if (pStorage != NULL) {
							writeFinalInfoGenerateCoverage(stream, pStorage->getArrayElement(iterTemplates->first));
						}
						stream.decrementIndentation();
						stream << "}";
					}
					stream.endl();
				}
				stream << "}";stream.endl();
			}
			// list of all parsed files?
			if ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_PARSED_FILES) != 0) {
				stream << "parsed_files {";stream.endl();
				stream.incrementIndentation();
				const std::map<std::string, std::set<std::string> >& parsedFiles = DtaProject::getInstance().getCapturedInputFiles();
				for (std::map<std::string, std::set<std::string> >::const_iterator iterTemplates = parsedFiles.begin(); iterTemplates != parsedFiles.end(); ++iterTemplates) {
					stream << "\"" << CGRuntime::composeCLikeString(iterTemplates->first) << "\"";
					if (iterTemplates->second.size() == 1) {
						stream << " = \"" << CGRuntime::composeCLikeString(*(iterTemplates->second.begin())) << "\"";
					} else {
						stream << "{";stream.endl();
						for (std::set<std::string>::const_iterator iterFiles = iterTemplates->second.begin(); iterFiles != iterTemplates->second.end(); ++iterFiles) {
							stream << "\t\"" << CGRuntime::composeCLikeString(*(iterTemplates->second.begin())) << "\"";stream.endl();
						}
						stream << "}";
					}
					if ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_PARSE_COVERAGE) != 0) {
						stream << " {";stream.endl();
						stream.incrementIndentation();
						DtaScriptVariable* pStorage = DtaProject::getInstance().getGlobalVariable("_FINAL_INFO_PARSE_COVERAGE");
						if (pStorage != NULL) {
							writeFinalInfoParseCoverage(stream, pStorage->getArrayElement(iterTemplates->first));
						}
						stream.decrementIndentation();
						stream << "}";
					}
					stream.endl();
				}
				stream.decrementIndentation();
				stream << "}";stream.endl();
			}
			stream.saveIntoFile(DtaProject::getInstance().getFinalInfoOutputFilename(), true);
		}
		if (_bTimeMeasure) {
			myTimer.stop();
			if (CGRuntime::getExternalHandling() == NULL) {
				char tcNumber[32];
				sprintf(tcNumber, "%d", myTimer.getTimeInMillis());
				CGRuntime::traceLine("time execution = " + std::string(tcNumber) + "ms");
			} else {
				char tcMessage[80];
				sprintf(tcMessage, "time execution = %dms", myTimer.getTimeInMillis());
				CGRuntime::getExternalHandling()->traceLine(tcMessage);
			}
		}
		return bSuccess;
	}


	class ParseCoverageRecordingIterator {
	private:
		const std::list<DtaScriptVariable*>& allPositions_;
		std::list<DtaScriptVariable*>::const_iterator iterPositions_;
		mutable DtaScriptVariable* pCurrentPosition_;
		std::list<DtaScriptVariable*>::const_iterator iterRuleBegin_;
		const std::list<DtaScriptVariable*>* pAllRulesBegin_;

	public:
		ParseCoverageRecordingIterator(const std::list<DtaScriptVariable*>& allPositions) : pCurrentPosition_(NULL), pAllRulesBegin_(NULL), allPositions_(allPositions) {
			iterPositions_ = allPositions.begin();
		}

		DtaScriptVariable* getCurrentPosition() const {
			if (pCurrentPosition_ == NULL) pCurrentPosition_ = *iterPositions_;
			return pCurrentPosition_;
		}
		
		DtaScriptVariable* getCurrentRulesBegin() {
			if (pAllRulesBegin_ == NULL) {
				DtaScriptVariable* pCurrentPosition = getCurrentPosition();
				if (pCurrentPosition == NULL) return NULL;
				DtaScriptVariable* pBeginningRules = pCurrentPosition->getNode("begin");
				if (pBeginningRules == NULL) return NULL;
				pAllRulesBegin_ = pBeginningRules->getArray();
				iterRuleBegin_ = pAllRulesBegin_->begin();
			}
			if (iterRuleBegin_ == pAllRulesBegin_->end()) return NULL;
			return *iterRuleBegin_;
		}

		bool nextCurrentRulesBegin() {
			++iterRuleBegin_;
			if (iterRuleBegin_ == pAllRulesBegin_->end()) return nextPosition();
			return true;
		}

		bool matchCurrentRulesEnd(const std::string& sEndOffset) {
			if (iterPositions_ == allPositions_.end()) {
				return true;
			}
			return (getCurrentPosition()->getName() == sEndOffset);
		}
			
		bool nextPosition() {
			++iterPositions_;
			pCurrentPosition_ = NULL;
			pAllRulesBegin_ = NULL;
			return iterPositions_ != allPositions_.end();
		}
	};

	void Workspace::writeFinalInfoParseCoverage(ScpStream& stream, DtaScriptVariable* pCoverage) {
		if (pCoverage == NULL) return;
		DtaScriptVariable* pAreas = pCoverage->getNode("areas");
		if (pAreas == NULL) return;
		const std::list<DtaScriptVariable*>* pAllPositions = pAreas->getArray();
		if (pAllPositions == NULL) return;
		ParseCoverageRecordingIterator it(*pAllPositions);
		writeFinalInfoParseCoverage(stream, it);
	}

	bool Workspace::writeFinalInfoParseCoverage(ScpStream& stream, ParseCoverageRecordingIterator& it) {
		DtaScriptVariable* pThisRuleSet = it.getCurrentRulesBegin();
		if (pThisRuleSet == NULL) return false;
		DtaScriptVariable* pFirstRuleName = pThisRuleSet->getArray()->front()->getReferencedVariable();
		std::string sBeginningOffset = it.getCurrentPosition()->getName();
		std::string sEndOffset = pThisRuleSet->getName();
		stream << pFirstRuleName->getValue() << " [" << sBeginningOffset << ", " << sEndOffset << "]";
		stream.endl();
		if (it.nextCurrentRulesBegin()) {
			while (!it.matchCurrentRulesEnd(sEndOffset)) {
				if (!writeFinalInfoParseCoverage(stream, it)) {
					if (it.matchCurrentRulesEnd(sEndOffset)) break;
					if (!it.nextPosition()) break;
				}
			}
		}
		return true;
	}

	void Workspace::writeFinalInfoGenerateCoverage(ScpStream& stream, DtaScriptVariable* pCoverage) {
		if (pCoverage == NULL) return;
		const std::list<DtaScriptVariable*>* pAllPositions = pCoverage->getArray();
		if (pAllPositions == NULL) return;
		int iIndex = 0;
		for (std::list<DtaScriptVariable*>::const_iterator i = pAllPositions->begin(); i != pAllPositions->end(); ++i) {
			DtaScriptVariable* pOutput = (*i)->getNode("output");
			DtaScriptVariable* pScript = (*i)->getNode("script");
			if (pOutput != NULL && pScript != NULL) {
				stream << pOutput->getValue() << (*i)->getValue() << pScript->getValue() << " ";
				++iIndex;
				if ((iIndex % 8) == 0) {
					stream.endl();
				}
			}
		}
	}


	ExecuteServerPageCppHeader::~ExecuteServerPageCppHeader() {}

	void ExecuteServerPageCppHeader::run() {
		std::string sClassName;
		const char* tcClassName = CGRuntime::getThisTree().getValue();
		if (tcClassName == NULL) throw UtlException("while generating the C++ external stub: no class name specified!");
		sClassName = tcClassName;
		CGRuntime::writeText("#ifndef _" + sClassName + "_h_" + CGRuntime::endl());
		CGRuntime::writeText("#define _" + sClassName + "_h_" + CGRuntime::endl());
		CGRuntime::writeText(CGRuntime::endl());
		CGRuntime::writeText("#include <string>" + CGRuntime::endl());
		if (CGRuntime::getProtectedArea("INCLUDE FILES").empty()) {
			CGRuntime::populateProtectedArea("INCLUDE FILES", "#include \"UtlException.h\"" + CGRuntime::endl());
		} else {
			CGRuntime::setProtectedArea("INCLUDE FILES");
		}
		CGRuntime::writeText(CGRuntime::endl());
		CGRuntime::writeText("namespace CodeWorker {" + CGRuntime::endl());
		CGRuntime::writeText("\tclass CppParsingTree_var;" + CGRuntime::endl());
		CGRuntime::writeText("\tclass CppParsingTree_value;" + CGRuntime::endl());
		CGRuntime::writeText("}" + CGRuntime::endl() + CGRuntime::endl());
		CGRuntime::writeText("class " + sClassName + " {" + CGRuntime::endl());
		CGRuntime::writeText("\tprivate:" + CGRuntime::endl());
		CGRuntime::setProtectedArea("attributes");
		CGRuntime::writeText(CGRuntime::endl());
		CGRuntime::writeText("\tpublic:" + CGRuntime::endl());
		CGRuntime::writeText("\t\tstatic void init();" + CGRuntime::endl() + CGRuntime::endl());
		for (std::list<GrfFunction*>::const_iterator i = workspace_._listOfExternalFunctions.begin(); i != workspace_._listOfExternalFunctions.end(); ++i) {
			GrfFunction* pFunction = *i;
			std::string sFunctionName = pFunction->getFunctionName();
			if (pFunction->isATemplateInstantiation()) {
				if (pFunction->isATemplateDispatcher()) {
					CGRuntime::writeText("\t\t// template dispatcher '" + pFunction->getSignature() + "':" + CGRuntime::endl());
				} else {
					CGRuntime::writeText("\t\t// template function '" + pFunction->getSignature() + "':" + CGRuntime::endl());
				}
				sFunctionName = "_compilerTemplateFunction_" + sFunctionName + "_compilerInstantiation_" + CppCompilerEnvironment::convertTemplateKey(pFunction->getTemplateInstantiation());
			} else {
				CGRuntime::writeText("\t\t// function '" + pFunction->getSignature() + "':" + CGRuntime::endl());
			}
			CGRuntime::writeText("\t\tstatic std::string " + sFunctionName + "(");
			if (pFunction->isATemplateDispatcher()) {
				CGRuntime::writeText("const std::string& _compilerTemplateFunction_dispatching");
			}
			int iIndex = 0;
			for (std::list<std::string>::const_iterator j = pFunction->getParameters().begin(); j != pFunction->getParameters().end(); j++, iIndex++) {
				if ((j != pFunction->getParameters().begin()) || pFunction->isATemplateDispatcher()) CGRuntime::writeText(", ");
				switch(pFunction->getParameterType(iIndex) & 0x00FF) {
					case VALUE_EXPRTYPE: CGRuntime::writeText("CodeWorker::CppParsingTree_value ");break;
					case ITERATOR_EXPRTYPE:
					case REFERENCE_EXPRTYPE: CGRuntime::writeText("const CodeWorker::CppParsingTree_var& ");break;
					case NODE_EXPRTYPE:	CGRuntime::writeText("const CodeWorker::CppParsingTree_var& ");break;
				}
				CGRuntime::writeText((*j));
			}
			CGRuntime::writeText(") throw(CodeWorker::UtlException);" + CGRuntime::endl());
			CGRuntime::writeText("\t\tstatic std::string " + sFunctionName + "_compilerVarlist(");
			if (pFunction->isATemplateDispatcher()) {
				CGRuntime::writeText("const std::string& _compilerTemplateFunction_dispatching, ");
			}
			CGRuntime::writeText("CodeWorker::CppParsingTree_var** tParameters) throw(CodeWorker::UtlException);" + CGRuntime::endl());
			if (pFunction->isATemplateDispatcher()) {
				CGRuntime::writeText("\t\tstatic std::string ");
				CGRuntime::writeText(pFunction->getFunctionName());
				CGRuntime::writeText("_compilerVarlist(CodeWorker::CppParsingTree_var** tParameters) throw(CodeWorker::UtlException);" + CGRuntime::endl());
			}
		}
		CGRuntime::writeText(CGRuntime::endl());
		CGRuntime::setProtectedArea("interfaces");
		CGRuntime::writeText("};" + CGRuntime::endl());
		CGRuntime::writeText(CGRuntime::endl());
		CGRuntime::writeText("#endif" + CGRuntime::endl());
	}


	ExecuteServerPageCppBody::~ExecuteServerPageCppBody() {}

	void ExecuteServerPageCppBody::run() {
		std::string sClassName;
		const char* tcClassName = CGRuntime::getThisTree().getValue();
		if (tcClassName == NULL) throw UtlException("while generating the C++ external stub: no class name specified!");
		sClassName = tcClassName;
		CGRuntime::writeText("#ifdef WIN32" + CGRuntime::endl());
		CGRuntime::writeText("#pragma warning(disable: 4786)" + CGRuntime::endl());
		CGRuntime::writeText("#endif" + CGRuntime::endl());
		CGRuntime::writeText(CGRuntime::endl());
		if (CGRuntime::getProtectedArea("INCLUDE FILES").empty()) {
			CGRuntime::populateProtectedArea("INCLUDE FILES", "#include \"CppParsingTree.h\"" + CGRuntime::endl() + "#include \"CGRuntime.h\"" + CGRuntime::endl());
		} else {
			CGRuntime::setProtectedArea("INCLUDE FILES");
		}
		CGRuntime::writeText("#include \"" + sClassName + ".h\"" + CGRuntime::endl());
		CGRuntime::writeText(CGRuntime::endl());
		CGRuntime::writeText("void " + sClassName + "::init() {" + CGRuntime::endl());
		std::list<GrfFunction*>::const_iterator i;
		for (i = workspace_._listOfExternalFunctions.begin(); i != workspace_._listOfExternalFunctions.end(); ++i) {
			GrfFunction* pFunction = *i;
			std::string sFunctionName = pFunction->getFunctionName();
			if (pFunction->isATemplateInstantiation()) {
				sFunctionName = "_compilerTemplateFunction_" + sFunctionName + "_compilerInstantiation_" + CppCompilerEnvironment::convertTemplateKey(pFunction->getTemplateInstantiation());
			}
			if (pFunction->isATemplateDispatcher()) {
				CGRuntime::writeText("\tCodeWorker::CGRuntime::registerExternalFunction(\"");
				CGRuntime::writeText(pFunction->getSignature());
				CGRuntime::writeText("\", " + sClassName + "::");
				CGRuntime::writeText(pFunction->getFunctionName());
				CGRuntime::writeText("_compilerVarlist);" + CGRuntime::endl());
				CGRuntime::writeText("\tCodeWorker::CGRuntime::registerExternalTemplateDispatcherFunction(");
			} else {
				CGRuntime::writeText("\tCodeWorker::CGRuntime::registerExternalFunction(");
			}
			CGRuntime::writeText("\"" + pFunction->getSignature() + "\", " + sClassName + "::" + sFunctionName + "_compilerVarlist);" + CGRuntime::endl());
		}
		CGRuntime::writeText("}" + CGRuntime::endl() + CGRuntime::endl());
		for (i = workspace_._listOfExternalFunctions.begin(); i != workspace_._listOfExternalFunctions.end(); ++i) {
			GrfFunction* pFunction = *i;
			std::string sFunctionName = pFunction->getFunctionName();
			std::string sDisplayName = pFunction->getSignature();
			if (pFunction->isATemplateInstantiation()) {
				if (pFunction->isATemplateDispatcher()) {
					CGRuntime::writeText("// template dispatcher '" + sDisplayName + "':" + CGRuntime::endl());
				} else {
					CGRuntime::writeText("// template function '" + sDisplayName + "':" + CGRuntime::endl());
				}
				sFunctionName = "_compilerTemplateFunction_" + sFunctionName + "_compilerInstantiation_" + CppCompilerEnvironment::convertTemplateKey(pFunction->getTemplateInstantiation());
			} else {
				CGRuntime::writeText("// function '" + sDisplayName + "':" + CGRuntime::endl());
			}
			CGRuntime::writeText("std::string " + sClassName + "::" + sFunctionName + "(");
			if (pFunction->isATemplateDispatcher()) {
				CGRuntime::writeText("const std::string& _compilerTemplateFunction_dispatching");
			}
			int iIndex = 0;
			std::list<std::string>::const_iterator j;
			for (j = pFunction->getParameters().begin(); j != pFunction->getParameters().end(); j++, iIndex++) {
				if ((j != pFunction->getParameters().begin()) || pFunction->isATemplateDispatcher()) CGRuntime::writeText(", ");
				switch(pFunction->getParameterType(iIndex) & 0x00FF) {
					case VALUE_EXPRTYPE: CGRuntime::writeText("CodeWorker::CppParsingTree_value ");break;
					case ITERATOR_EXPRTYPE:
					case REFERENCE_EXPRTYPE: CGRuntime::writeText("const CodeWorker::CppParsingTree_var& ");break;
					case NODE_EXPRTYPE:	CGRuntime::writeText("const CodeWorker::CppParsingTree_var& ");break;
				}
				CGRuntime::writeText((*j));
			}
			CGRuntime::writeText(") throw(CodeWorker::UtlException) {" + CGRuntime::endl());
			if (pFunction->isATemplateDispatcher()) {
				for (std::map<std::string, GrfFunction*>::const_iterator k = pFunction->getInstantiatedFunctions().begin(); k != pFunction->getInstantiatedFunctions().end(); k++) {
					CGRuntime::writeText("\tif (_compilerTemplateFunction_dispatching == \"" + k->first + "\") return " + sFunctionName + k->first + "(");
					for (j = pFunction->getParameters().begin(); j != pFunction->getParameters().end(); j++) {
						if (j != pFunction->getParameters().begin()) CGRuntime::writeText(", ");
						CGRuntime::writeText((*j));
					}
					CGRuntime::writeText(");" + CGRuntime::endl());
				}
				CGRuntime::writeText("\tif (!_compilerTemplateFunction_dispatching.empty()) throw CodeWorker::UtlException(\"template function '");
				CGRuntime::writeText(pFunction->getFunctionName());
				CGRuntime::writeText("<\\\"\" + _compilerTemplateFunction_dispatching + \"\\\">' hasn't been implemented\");" + CGRuntime::endl());
			}
			CGRuntime::setProtectedArea(sFunctionName);
			if (CGRuntime::getProtectedArea(sFunctionName).empty()) {
				CGRuntime::writeText("\treturn \"\";" + CGRuntime::endl());
			}
			CGRuntime::writeText("}" + CGRuntime::endl() + CGRuntime::endl());
			CGRuntime::writeText("std::string " + sClassName + "::" + sFunctionName + "_compilerVarlist(");
			if (pFunction->isATemplateDispatcher()) {
				CGRuntime::writeText("const std::string& _compilerTemplateFunction_dispatching, ");
			}
			CGRuntime::writeText("CodeWorker::CppParsingTree_var** tParameters) throw(CodeWorker::UtlException) {" + CGRuntime::endl());
			CGRuntime::writeText("\treturn " + sFunctionName + "(");
			if (pFunction->isATemplateDispatcher()) {
				CGRuntime::writeText("_compilerTemplateFunction_dispatching");
			}
			iIndex = 0;
			for (j = pFunction->getParameters().begin(); j != pFunction->getParameters().end(); j++, iIndex++) {
				if ((j != pFunction->getParameters().begin()) || pFunction->isATemplateDispatcher()) CGRuntime::writeText(", ");
				char tcFormat[64];
				sprintf(tcFormat, "*(tParameters[%d])", iIndex);
				CGRuntime::writeText(tcFormat);
			}
			CGRuntime::writeText(");" + CGRuntime::endl());
			CGRuntime::writeText("}" + CGRuntime::endl() + CGRuntime::endl());
			if (pFunction->isATemplateDispatcher()) {
				CGRuntime::writeText("std::string " + sClassName + "::" + pFunction->getFunctionName() + "_compilerVarlist(CodeWorker::CppParsingTree_var** tParameters) throw(CodeWorker::UtlException) {" + CGRuntime::endl());
				CGRuntime::writeText("\treturn " + sFunctionName + "(\"\", ");
				iIndex = 0;
				for (j = pFunction->getParameters().begin(); j != pFunction->getParameters().end(); j++, iIndex++) {
					if (j != pFunction->getParameters().begin()) CGRuntime::writeText(", ");
					char tcFormat[64];
					sprintf(tcFormat, "*(tParameters[%d])", iIndex);
					CGRuntime::writeText(tcFormat);
				}
				CGRuntime::writeText(");" + CGRuntime::endl());
				CGRuntime::writeText("}" + CGRuntime::endl() + CGRuntime::endl());
			}
		}
		CGRuntime::writeText(CGRuntime::endl());
		CGRuntime::setProtectedArea("implementation");
	}
}
