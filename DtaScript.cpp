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

// function 'mkdir'
#ifdef WIN32
#	include <direct.h>
#else
#	include <sys/stat.h>
#endif

#include "ScpStream.h"
#include "UtlException.h"
#include "UtlTrace.h"
#include "UtlDirectory.h"

#include "CGRuntime.h"
#include "Workspace.h"
#include "GrfForeach.h"
#include "GrfSelect.h"
#include "GrfIfThenElse.h"
#include "GrfDelay.h"
#include "GrfNewProject.h"
#include "GrfDebugExecution.h"
#include "GrfQuantifyExecution.h"
#include "GrfQuiet.h"
#include "GrfFileAsStandardInput.h"
#include "GrfStringAsStandardInput.h"
#include "GrfDo.h"
#include "GrfSwitch.h"
#include "GrfTryCatch.h"
#include "GrfWhile.h"
#include "GrfContinue.h"
#include "GrfBreak.h"
#include "GrfReturn.h"
#include "GrfExit.h"
#include "GrfFunction.h"
#include "GrfReadonlyHook.h"
#include "GrfWritefileHook.h"
#include "GrfWriteText.h"
#include "DtaClass.h"
#include "GrfNop.h"
#include "GrfLocalReference.h"
#include "GrfLocalVariable.h"
#include "GrfGlobalVariable.h"
#include "GrfSetAssignment.h"
#include "GrfInsertAssignment.h"
#include "GrfSetAll.h"
#include "GrfMerge.h"
#include "GrfPushItem.h"
#include "GrfReference.h"
#include "GrfParsedFile.h"
#include "GrfParsedString.h"
#include "GrfAlienInstruction.h"
#include "CGExternalHandling.h" // for CGQuietOutput
//##markup##"include files"
//##begin##"include files"
#include "GrfAppendFile.h"
#include "GrfAutoexpand.h"
#include "GrfClearVariable.h"
#include "GrfCompileToCpp.h"
#include "GrfCopyFile.h"
#include "GrfCopyGenerableFile.h"
#include "GrfCopySmartDirectory.h"
#include "GrfCutString.h"
#include "GrfEnvironTable.h"
#include "GrfError.h"
#include "GrfExecuteString.h"
#include "GrfExpand.h"
#include "GrfExtendExecutedScript.h"
#include "GrfGenerate.h"
#include "GrfGenerateString.h"
#include "GrfInsertElementAt.h"
#include "GrfInvertArray.h"
#include "GrfListAllGeneratedFiles.h"
#include "GrfLoadProject.h"
#include "GrfOpenLogFile.h"
#include "GrfParseAsBNF.h"
#include "GrfParseStringAsBNF.h"
#include "GrfParseFree.h"
#include "GrfProduceHTML.h"
#include "GrfPutEnv.h"
#include "GrfRandomSeed.h"
#include "GrfRemoveAllElements.h"
#include "GrfRemoveElement.h"
#include "GrfRemoveFirstElement.h"
#include "GrfRemoveLastElement.h"
#include "GrfRemoveRecursive.h"
#include "GrfRemoveVariable.h"
#include "GrfSaveBinaryToFile.h"
#include "GrfSaveProject.h"
#include "GrfSaveProjectTypes.h"
#include "GrfSaveToFile.h"
#include "GrfSetCommentBegin.h"
#include "GrfSetCommentEnd.h"
#include "GrfSetGenerationHeader.h"
#include "GrfSetIncludePath.h"
#include "GrfSetNow.h"
#include "GrfSetProperty.h"
#include "GrfSetTextMode.h"
#include "GrfSetVersion.h"
#include "GrfSetWriteMode.h"
#include "GrfSetWorkingPath.h"
#include "GrfSleep.h"
#include "GrfSlideNodeContent.h"
#include "GrfSortArray.h"
#include "GrfTraceEngine.h"
#include "GrfTraceLine.h"
#include "GrfTraceObject.h"
#include "GrfTraceStack.h"
#include "GrfTraceText.h"
#include "GrfTranslate.h"
#include "GrfCloseSocket.h"
//##end##"include files"
#include "ExprScriptVariable.h"
#include "ExprScriptMotif.h"
#include "ExprScriptFunction.h"
#include "ExprScriptBinaryExpression.h"
#include "DtaProject.h"
#include "CppCompilerEnvironment.h"
#include "DtaBNFScript.h" // for alien languages
#include "DynFunction.h"
#include "DynPackage.h"

#include "DtaScript.h"
#include "DtaPatternScript.h"

namespace CodeWorker {
	bool DtaScript::_bParseWithSymbols = true;
	std::vector<const char*> DtaScript::_listOfAllFilesAlreadyParsed;
	std::map<std::string, EXECUTE_FUNCTION*> DtaScript::_mapOfScriptFunctions;
	std::map<std::string, std::string> DtaScript::_mapOfSyntaxModes;
	std::map<std::string, int> DtaScript::_mapOfReferenceMagicNumbers;


	SEQUENCE_INTERRUPTION_LIST GrfScriptBlock::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		GrfCommand* pCommand = NULL;
		try {
			if (hasLocalVariables()) {
				DtaScriptVariable localVariables(&visibility, "##stack## block");
				for (std::vector<GrfCommand*>::iterator i = _commands.begin(); i != _commands.end(); i++) {
					pCommand = *i;
					if ((result = pCommand->execute(localVariables)) != NO_INTERRUPTION) break;
				}
			} else {
				for (std::vector<GrfCommand*>::iterator i = _commands.begin(); i != _commands.end(); i++) {
					pCommand = *i;
					if ((result = pCommand->execute(visibility)) != NO_INTERRUPTION) break;
				}
			}
		} catch(UtlException& exception) {
			std::string sLineStack;
			if (pCommand->_sParsingFilePtr != NULL) {
				char tcNumber[1024];
				std::string sCompleteFileName;
				std::auto_ptr<std::ifstream> pFile(openInputFileFromIncludePath(pCommand->_sParsingFilePtr, sCompleteFileName));
				if (pFile.get() == NULL) {
					sprintf(tcNumber, "\"%s\": offset[%d], ", pCommand->_sParsingFilePtr, pCommand->_iFileLocation);
				} else {
					setLocation(*pFile, pCommand->_iFileLocation);
					int iLine = getLineCount(*pFile);
					std::string sScriptFilename = pCommand->_sParsingFilePtr;
					std::string::size_type iIndex = sScriptFilename.find_last_of("\\/");
					if (iIndex != std::string::npos) sScriptFilename = sScriptFilename.substr(iIndex + 1);
					sprintf(tcNumber, "%s(%d): ", sScriptFilename.c_str(), iLine);
					pFile->close();
				}
				sLineStack += tcNumber;
			}
			sLineStack += "main scope";
			sLineStack += "\n";
			std::string sTraceStack = exception.getTraceStack();
			if (sTraceStack.substr(0, sLineStack.size()) != sLineStack) {
				sTraceStack = sLineStack + sTraceStack;
			}
			throw UtlException(sTraceStack, exception.getMessage());
		}
		return result;
	}




	DtaScript::DtaScript(/*DtaScriptVariable* pVisibility, */GrfBlock* pParentBlock, const char* sFilename) : _graph(pParentBlock), /*_pVisibility(pVisibility), */_pPreviousScript(NULL), _bNumericalExpression(false), _bCurrentFunctionGenericTemplateKey(false) {
		setFilename(sFilename);
		_graph.setScript(this);
		allocateLocalParameters();
	}

	DtaScript::~DtaScript() {
		freeLocalParameters();
	}

	void DtaScript::allocateLocalParameters() {
		_sPreviousVersion = DtaProject::getInstance().getVersion();
		_pPreviousScript = DtaProject::getInstance().getScript();
		_sPreviousMarkupKey = DtaProject::getInstance().getMarkupKey();
		_sPreviousMarkupValue = DtaProject::getInstance().getMarkupValue();
		DtaProject::getInstance().setScript(this);
	}

	void DtaScript::freeLocalParameters() {
		DtaProject::getInstance().setVersion(_sPreviousVersion);
		DtaProject::getInstance().setScript(_pPreviousScript);
		DtaProject::getInstance().setMarkupKey(_sPreviousMarkupKey);
		DtaProject::getInstance().setMarkupValue(_sPreviousMarkupValue);
	}

	DtaScriptFactory::SCRIPT_TYPE DtaScript::getType() const { return DtaScriptFactory::COMMON_SCRIPT; }
	bool DtaScript::isAParseScript() const { return false; }
	bool DtaScript::isAGenerateScript() const { return false; }

	void DtaScript::setFilename(const char* sFilename) {
		if ((sFilename == NULL) || (sFilename[0] == '\0')) _tcFilePtr = NULL;
		else {
			// determine the target language
			if (!DtaProject::getInstance().getCodeWorkerHome().empty()) {
				std::string sFileForTarget = sFilename;
				std::string::size_type iIndex = sFileForTarget.find_last_of("/\\");
				if (iIndex != std::string::npos) sFileForTarget = sFileForTarget.substr(iIndex + 1);
				iIndex = sFileForTarget.find_last_of(".");
				if (iIndex != std::string::npos) {
					sFileForTarget = sFileForTarget.substr(0, iIndex);
					iIndex = sFileForTarget.find_last_of(".");
					if (iIndex != std::string::npos) {
						sFileForTarget = sFileForTarget.substr(iIndex + 1);
						UtlDirectory theTargetDir(DtaProject::getInstance().getCodeWorkerHome() + sFileForTarget);
						if (theTargetDir.scan()) {
							_sTargetLanguage = sFileForTarget;
							if (DtaProject::getInstance().hasTargetLanguage()) {
								if (DtaProject::getInstance().getTargetLanguage() != _sTargetLanguage) {
									throw UtlException("you can't specify two different target languages");
								}
							} else {
								DtaProject::getInstance().setTargetLanguage(_sTargetLanguage);
							}
						}
					}
				}
			}
			// register the filename pointer
			for (std::vector<const char*>::const_iterator i = _listOfAllFilesAlreadyParsed.begin(); i != _listOfAllFilesAlreadyParsed.end(); i++) {
				if (*i == sFilename) {
					_tcFilePtr = *i;
					return;
				}
			}
#ifdef WIN32
			_tcFilePtr = _strdup(sFilename);
#else
			_tcFilePtr = strdup(sFilename);
#endif
			_listOfAllFilesAlreadyParsed.push_back(_tcFilePtr);
		}
	}

	void DtaScript::traceEngine() const {
		if (_tcFilePtr == NULL) CGRuntime::traceLine("Common script (no filename):");
		else CGRuntime::traceLine("Common script \"" + std::string(_tcFilePtr) + "\":");
		traceInternalEngine();
	}

	void DtaScript::traceInternalEngine() const {
		if (_pPreviousScript != NULL) {
			CGRuntime::traceLine("--------------");
			_pPreviousScript->traceEngine();
		}
	}

	std::fstream* DtaScript::openOutputFile(const char* sFileName, bool bThrowOnError) {
		std::fstream* pFile = openIOFile(sFileName);
		if (pFile == NULL) {
			std::string sFile = sFileName;
			int iEndPos = sFile.find_first_of("\\/", 1);
			while (iEndPos > 0) {
				std::string sDirectory = sFile.substr(0, iEndPos);
#ifdef WIN32
				mkdir(sDirectory.c_str());
#else
				mkdir(sDirectory.c_str(), S_IRWXU);
#endif
				iEndPos = sFile.find_first_of("\\/", iEndPos + 1);
			}
			pFile = openIOFile(sFileName);
		}
		if ((pFile == NULL) && bThrowOnError) throw UtlException(std::string("Unable to create file \"") + sFileName + "\"");
		return pFile;
	}

	void DtaScript::registerScript(const char* sRegistration, EXECUTE_FUNCTION* executeFunction) {
		_mapOfScriptFunctions[sRegistration] = executeFunction;
	}

	EXECUTE_FUNCTION* DtaScript::getRegisteredScript(const char* sRegistration) {
		if (sRegistration == NULL) return NULL;
		std::string sModule(sRegistration);
		std::string::size_type iIndex = sModule.find('.');
		if (iIndex != std::string::npos) sModule = sModule.substr(0, iIndex);
		std::map<std::string, EXECUTE_FUNCTION*>::const_iterator cursor = _mapOfScriptFunctions.find(sModule);
		if (cursor == _mapOfScriptFunctions.end()) return NULL;
		return cursor->second;
	}

	void DtaScript::parseFile(const char* tcScriptFileName, GrfBlock* pIncluded, bool bDebugMode, const std::string& sQuantifyFile) {
		ScpStream* pFile = NULL;
		ScpStream theFileContent;
		std::string sCompleteFileName;
		if (ScpStream::existVirtualFile(tcScriptFileName)) {
			std::string sContent;
			if (!ScpStream::loadVirtualFile(tcScriptFileName, sContent)) throw UtlException(std::string("unable to load the virtual script file \"") + tcScriptFileName + "\"");
			theFileContent << sContent;
		} else {
			pFile = ScpStream::openInputFileFromIncludePath(tcScriptFileName, sCompleteFileName);
			if (pFile == NULL) {
				throw UtlException(std::string("unable to open script file \"") + tcScriptFileName + "\"");
			}
			sCompleteFileName = CGRuntime::canonizePath(sCompleteFileName);
		}
		const char* sOldFile = getFilenamePtr();
		setFilename(sCompleteFileName.c_str());
		_mapOfAttachedAreas = std::map<int, int>();
		std::string sOldCurrentDirectory;
		try {
			if (pFile != NULL) {
				sOldCurrentDirectory = CGRuntime::getCurrentDirectory();
				std::string sNewCurrentDirectory = sCompleteFileName.substr(0, 1 + sCompleteFileName.find_last_of('/'));
				CGRuntime::changeDirectory(sNewCurrentDirectory);
				if (pIncluded != NULL) parseScript(*pFile, *pIncluded);
				else parseStream(*pFile, bDebugMode, sQuantifyFile);
				CGRuntime::changeDirectory(sOldCurrentDirectory);
			} else {
				if (pIncluded != NULL) parseScript(theFileContent, *pIncluded);
				else parseStream(theFileContent, bDebugMode, sQuantifyFile);
			}
		} catch(UtlExitException&) {
			if (pFile != NULL) {
				CGRuntime::changeDirectory(sOldCurrentDirectory);
				pFile->close();
				delete pFile;
			}
			throw;
		} catch(UtlException& e) {
			std::string sMessage;
			sMessage += tcScriptFileName;
			setFilenamePtr(sOldFile);
			char tcNumber[64];
			sprintf(tcNumber, "(%d):", ((pFile == NULL) ? theFileContent.getLineCount() : pFile->getLineCount()));
			sMessage += tcNumber + CGRuntime::endl();
			if (pFile != NULL) {
				CGRuntime::changeDirectory(sOldCurrentDirectory);
				pFile->close();
				delete pFile;
			}
			throw UtlException(e.getTraceStack(), sMessage + e.getMessage());
		}
		if (sOldFile != NULL) {
			// we were including a script file here
			setFilenamePtr(sOldFile);
		}
		if (pFile != NULL) {
			pFile->close();
			delete pFile;
		} 
		if (!_mapOfAttachedAreas.empty()) {
			std::auto_ptr<ScpStream> pStream;
			if (ScpStream::existVirtualFile(tcScriptFileName)) {
				std::auto_ptr<ScpStream> pExistingFile(new ScpStream);
				pStream = pExistingFile;
				std::string sContent;
				if (!ScpStream::loadVirtualFile(tcScriptFileName, sContent)) throw UtlException(std::string("unable to load the virtual script file \"") + tcScriptFileName + "\"");
				pStream->setFilename(tcScriptFileName);
				(*pStream) << sContent;
			} else {
				std::auto_ptr<ScpStream> pNewFile(new ScpStream(tcScriptFileName, ScpStream::IN | ScpStream::PATH));
				pStream = pNewFile;
			}
			std::map<int, int>::const_iterator i;
			char tcNumber[32];
			for (i = _mapOfAttachedAreas.begin(); i != _mapOfAttachedAreas.end(); ++i) {
				pStream->setOutputLocation(i->first);
				sprintf(tcNumber, "%d", i->first);
				pStream->newFloatingLocation(tcNumber);
			}
			char tcMagicNumber[32];
			ScpStream* pOwner;
			for (i = _mapOfAttachedAreas.begin(); i != _mapOfAttachedAreas.end(); ++i) {
				sprintf(tcMagicNumber, ", %d", i->second);
				sprintf(tcNumber, "%d", i->first);
				pStream->insertText(tcMagicNumber, pStream->getFloatingLocation(tcNumber, pOwner));
			}
			GrfWritefileHook* pHook = DtaProject::getInstance().getWritefileHook();
			if (pHook != NULL) {
				std::string sSuccess;
				try {
					sSuccess = pHook->executeHook(DtaProject::getInstance(), tcScriptFileName, -1, false);
				} catch(UtlException& exc) {
					throw UtlException(std::string("FATAL ERROR! Before writing the script file \"") + tcScriptFileName + "\", a call to 'writefileHook' has raised the following error:" + CGRuntime::endl() + "\"" + exc.getMessage() + "\"");
				}
				if (sSuccess.empty()) return;
			}
			try {
				pStream->saveIntoFile(tcScriptFileName, false);
			} catch(UtlException&/* exception*/) {
				GrfReadonlyHook* pHook = DtaProject::getInstance().getReadonlyHook();
				if (pHook != NULL) {
					try {
						pHook->executeHook(DtaProject::getInstance(), tcScriptFileName);
					} catch(UtlException& exc) {
						throw UtlException(std::string("FATAL ERROR! Script file \"") + tcScriptFileName + "\" is read-only and call to the hook has launched the following error:\n\"" + exc.getMessage() + "\"");
					}
					pStream->saveIntoFile(tcScriptFileName, false);
				} else {
					throw/* UtlException(exception.getTraceStack(), exception.getMessage())*/;
				}
			}
		}
	}

	void DtaScript::parseFile(const char* sScriptName, const std::string& sCurrentDirectoryAtCompileTime) {
		std::string sOldCurrentDirectory;
		try {
			sOldCurrentDirectory = CGRuntime::getCurrentDirectory();
			CGRuntime::changeDirectory(sCurrentDirectoryAtCompileTime);
			parseFile(sScriptName);
			CGRuntime::changeDirectory(sOldCurrentDirectory);
		} catch(std::exception&) {
			CGRuntime::changeDirectory(sOldCurrentDirectory);
			throw;
		}
	}

	void DtaScript::parseStream(ScpStream& myStream, bool bDebugMode, const std::string& sQuantifyFile) {
		GrfBlock* pRoot = &_graph;
		if (!sQuantifyFile.empty()) {
			GrfQuantifyExecution* pQuantify = new GrfQuantifyExecution(pRoot);
			if (sQuantifyFile != "true") pQuantify->setFilename(new ExprScriptConstant(sQuantifyFile.c_str()));
			pRoot->add(pQuantify);
			pRoot = pQuantify;
		}
		if (bDebugMode) {
			GrfDebugExecution* pDebug = new GrfDebugExecution(pRoot);
			pRoot->add(pDebug);
			pRoot = pDebug;
		}
		parseScript(myStream, *pRoot);
	}

	void DtaScript::handleNotAWordCommand(ScpStream& script, GrfBlock& block) {
		int iChar = script.readChar();
		if (iChar == (int) '{') {
			script.goBack();
			GrfBlock* pBlock = new GrfBlock(&block);
			if (requiresParsingInformation()) pBlock->setParsingInformation(getFilenamePtr(), script);
			block.add(pBlock);
			parseBlock(script, *pBlock);
		} else if (iChar == (int) '#') {
			std::string sDirective;
			if (!script.readIdentifier(sDirective)) throw UtlException(script, "preprocessor directive expected after '#' symbol");
			sDirective = "#" + sDirective;
			parsePreprocessorDirective(sDirective, script, block);
		} else {
			std::string sMessage = "command or procedure or function expected, instead of ";
			if (iChar < 0) {
				throw UtlException(script, sMessage + "end of file");
			} else {
				throw UtlException(script, sMessage + "'" + (char) iChar + "'");
			}
		}
	}

	void DtaScript::parsePreprocessorDirective(const std::string& sDirective, ScpStream& script, GrfBlock& block) {
		if (sDirective == "#include") {
			script.skipEmpty();
			std::string sScriptFileName;
			if (!script.readString(sScriptFileName)) throw UtlException(script, "syntax error: constant string expected");
			// protect against the multi-inclusion in the same script file
			std::set<std::string>::const_iterator	itIncluded;
			itIncluded = _setOfIncludedFiles.find(sScriptFileName);
			if (itIncluded == _setOfIncludedFiles.end()) {
				_setOfIncludedFiles.insert(sScriptFileName);
				parseFile(sScriptFileName.c_str(), &block);
			}
		} else if (sDirective == "#use") {
			script.skipEmpty();
			std::string sPackageName;
			if (!script.readIdentifier(sPackageName)) throw UtlException(script, "syntax error: module name expected as a constant string");
			DynPackage::addPackage(sPackageName, block);
		} else if (sDirective == "#line") {
			script.skipEmpty();
			int iLine;
			if (!script.readInt(iLine)) throw UtlException(script, "syntax error: line number expected as a constant integer");
			int iChar = script.readChar();
			while ((iChar == (int) ' ') && (iChar == (int) '\t')) iChar = script.readChar();
			if (iChar == (int) '/') {
				iChar = script.readChar();
				if (iChar != (int) '/') throw UtlException(script, "syntax error: end of line or line comment expected");
				do {
					iChar = script.readChar();
				} while ((iChar >= 0) && (iChar != (int)'\r') && (iChar != (int)'\n'));
			}
			if (iChar == '\r') iChar = script.readChar();
			if (iChar != '\n') throw UtlException(script, "syntax error: end of line expected");
			script.setLineDirective(iLine);
		} else if (sDirective == "#attach") {
			script.skipLineBlanks();
			std::string sKey;
			std::string sReferenceFile;
			if (!script.readString(sReferenceFile)) throw UtlException(script, "syntax error on '#attach': reference script file expected");
			script.skipLineBlanks();
			if (!script.isEqualTo(':')) throw UtlException(script, "syntax error on '#attach', ':' expected");
			script.skipLineBlanks();
			if (!script.readIdentifier(sKey)) throw UtlException(script, "syntax error on '#attach': identifier expected");
			int iMagicNumberLocation = script.getInputLocation();
			script.skipLineBlanks();
			if (script.isEqualTo(',')) {
				script.skipLineBlanks();
				int iMagicNumber;
				if (!script.readInt(iMagicNumber)) throw UtlException(script, "syntax error on '#attach': magic number expected after the comma");
				int iReferenceMagicNumber;
				if (extractReferenceMagicNumber(sReferenceFile, sKey, iReferenceMagicNumber)) {
					if (iReferenceMagicNumber != iMagicNumber) {
						std::string sErrorMessage = "warning: attached area '" + sKey + "' isn't equivalent to the same area in the reference script file \"" + sReferenceFile + "\"" + CGRuntime::endl();
						sErrorMessage += DtaProject::getInstance().getTraceStack(script);
						if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
					}
				} else {
					std::string sErrorMessage = "warning: attached area '" + sKey + "' refers to an unexisting script file \"" + sReferenceFile + "\" or key" + CGRuntime::endl();
					sErrorMessage += DtaProject::getInstance().getTraceStack(script);
					if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
				}
			} else {
				if (!script.findString("\n")) throw UtlException(script, "syntax error on '#attach': end of line expected");
				int iReferenceMagicNumber;
				if (extractReferenceMagicNumber(sReferenceFile, sKey, iReferenceMagicNumber)) {
					_mapOfAttachedAreas[iMagicNumberLocation] = iReferenceMagicNumber;
				} else {
					std::string sErrorMessage = "warning: attached area '" + sKey + "' refers to an unexisting script file \"" + sReferenceFile + "\" or key" + CGRuntime::endl();
					sErrorMessage += DtaProject::getInstance().getTraceStack(script);
					if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
				}
			}
		} else if (sDirective == "#reference") {
			if (getFilenamePtr() == NULL) throw UtlException(script, "a '#reference' area must be put into a script file; this isn't the case here");
			script.skipLineBlanks();
			std::string sKey;
			if (!script.readIdentifier(sKey)) throw UtlException(script, "syntax error on '#reference': identifier expected");
			if (!script.findString("\n")) throw UtlException(script, "syntax error on '#reference': end of line expected");
			int iLocation = script.getInputLocation();
			computeReferenceMagicNumber(script, getFilenamePtr(), sKey);
			script.setInputLocation(iLocation);
		} else if (sDirective == "#end") {
			if (!script.findString("\n")) throw UtlException(script, "syntax error on '#end': end of line expected");
		} else if (sDirective == "#syntax") {
			parseSyntax(block, script);
		} else {
			throw UtlException(script, "unknown preprocessor directive '" + sDirective + "' found");
		}
	}

	void DtaScript::handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block) {
		int iLocation = script.getInputLocation() - sCommand.size();
		DynPackage* pPackage = NULL;
		std::string sPackageMember;
		script.skipEmpty();
		int iChar = script.readChar();
		if (iChar == (int) ':') {
			if (!script.isEqualTo(':')) {
				script.goBack();
				throw UtlException(script, "unexpected identifier \"" + sCommand + "\"");
			} else {
				pPackage = DynPackage::getPackage(sCommand);
				if (pPackage == NULL) throw UtlException(script, "unknown module '" + sCommand +"'; type '#use' to load the dynamic library");
				script.skipEmpty();
				if (!script.readIdentifier(sPackageMember)) throw UtlException(script, "function or variable name expected after '" + sCommand + "::'");
				script.skipEmpty();
				iChar = script.readChar();
			}
		}
		if (iChar == (int) '(') {
			if (pPackage != NULL) {
				// the variable comes from a module
				DynFunction* pFunction = pPackage->getFunction(sPackageMember);
				if (pFunction == NULL) throw UtlException(script, "the module '" + sCommand + "' doesn't export the function '" + sCommand + "::" + sPackageMember + "'");
				GrfNop* pNop = new GrfNop;
				if (requiresParsingInformation()) pNop->setParsingInformation(getFilenamePtr(), script);
				block.add(pNop);
				pNop->setExpression(parseFunctionExpression(block, script, NULL, new ExprScriptFunction(pFunction), NULL, true));
			} else {
				// classical variable kind
				ExprScriptFunction* pFunction = ExprScriptFunction::create(block, script, sCommand, "", false);
				if (pFunction == NULL) throw UtlException(script, "unknown command or procedure or function \"" + sCommand + "\"");
				checkIfAllowedFunction(script, *pFunction);
				GrfNop* pNop = new GrfNop;
				if (requiresParsingInformation()) pNop->setParsingInformation(getFilenamePtr(), script);
				block.add(pNop);
				pNop->setExpression(parseFunctionExpression(block, script, NULL, pFunction, NULL, true));
			}
			script.skipEmpty();
			if (!script.isEqualTo(';')) throw UtlException(script, "';' expected after calling a procedure");
		} else if ((iChar == (int) '.') || (iChar == (int) '#') || (iChar == (int) '[') || (iChar == (int) '$' && script.peekChar() == (int) '[')) {
			script.goBack();
			std::string sVariable;
			if (pPackage != NULL) {
				if (pPackage->getVariable(sPackageMember) == NULL) throw UtlException(script, "the module '" + sCommand + "' doesn't export the variable '" + sCommand + "::" + sPackageMember + "'");
				sVariable = sPackageMember;
			} else {
				sVariable = sCommand;
			}
			ExprScriptExpression* pMethodFunction;
			GrfCommand* pMethodProc;
			std::auto_ptr<ExprScriptVariable> pVariable(parseVariableExpression(block, script, new ExprScriptVariable(sVariable.c_str(), pPackage), pMethodFunction, pMethodProc));
			script.skipEmpty();
			if (pMethodFunction != NULL) {
				pVariable.release();
				GrfNop* pNop = new GrfNop;
				if (requiresParsingInformation()) pNop->setParsingInformation(getFilenamePtr(), script);
				block.add(pNop);
				pNop->setExpression(pMethodFunction);
				if (!script.isEqualTo(';')) throw UtlException(script, "';' expected to close an instruction (function call)");
			} else if (pMethodProc != NULL) {
				pVariable.release();
				// nothing to do: the procedure has been added
			} else {
				GrfSetAssignment* pAssignment = new GrfSetAssignment;
				if (requiresParsingInformation()) pAssignment->setParsingInformation(getFilenamePtr(), script);
				block.add(pAssignment);
				pAssignment->setVariable(pVariable.release());
				bool bConcat = script.isEqualTo('+');
				if (!script.isEqualTo('=')) throw UtlException(script, "syntax error: assignment expected ('+=' or '=' operator)");
				script.skipEmpty();
				pAssignment->setValue(parseExpression(block, script), bConcat);
				script.skipEmpty();
				if (!script.isEqualTo(';')) throw UtlException(script, "';' expected to close an assignment instruction");
			}
		} else if ((iChar == (int) '=') || ((iChar == (int) '+') && script.isEqualTo('='))) {
			script.setInputLocation(iLocation);
			parseSetAssignment(block, script);
		} else if (iChar == (int) '<') {
			std::auto_ptr<ExprScriptExpression> pTemplate(parseKeyTemplateExpression(block, script));
			script.skipEmpty();
			if (script.isEqualTo('>')) {
				script.skipEmpty();
				if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected after instantiating a template procedure");
				ExprScriptFunction* pFunction = ExprScriptFunction::create(block, script, sCommand, "", false);
				if (pFunction == NULL) throw UtlException(script, "unknown command or procedure or function \"" + sCommand + "\"");
				checkIfAllowedFunction(script, *pFunction);
				GrfNop* pNop = new GrfNop;
				if (requiresParsingInformation()) pNop->setParsingInformation(getFilenamePtr(), script);
				block.add(pNop);
				pNop->setExpression(parseFunctionExpression(block, script, NULL, pFunction, pTemplate.release(), true));
				if (!script.isEqualTo(';')) throw UtlException(script, "';' expected after calling a function or procedure");
			} else {
				script.setInputLocation(iLocation);
				throw UtlException(script, "unknown command or procedure or function \"" + sCommand + "\"");
			}
		} else {
			throw UtlException(script, "unexpected identifier \"" + sCommand + "\"");
		}
	}

	bool DtaScript::betweenCommands(ScpStream& /*script*/, GrfBlock& /*block*/) { return false; }

	void DtaScript::parseScript(ScpStream& script, GrfBlock& block) {
		_stackOfParsedBlocks.push_front(&block);
		if (hasTargetLanguage()) {
			// alien programming language
			parseAlienScript(script, block);
		} else {
			// ordinary scripting
			betweenCommands(script, block);
			while (script.skipEmpty()) parseInstruction(script, block);
		}
		_stackOfParsedBlocks.pop_front();
	}

	void DtaScript::parseEmbeddedScript(ScpStream& script) {
		if (!script.isEqualTo('{')) throw UtlException(script, "'{' expected to start an embedded script");
		_stackOfParsedBlocks.push_front(&_graph);
		betweenCommands(script, _graph);
		script.skipEmpty();
		while (!script.isEqualTo('}')) {
			parseInstruction(script, _graph);
			script.skipEmpty();
		}
		_stackOfParsedBlocks.pop_front();
	}

	void DtaScript::beforeParsingABlock(ScpStream& /*script*/, GrfBlock& /*block*/) {}
	void DtaScript::afterParsingABlock(ScpStream& /*script*/, GrfBlock& /*block*/) {}

	void DtaScript::parseBlock(ScpStream& script, GrfBlock& block) {
		script.skipEmpty();
		if (!script.isEqualTo('{')) {
			if (script.isEqualTo(';')) return;
			if (!betweenCommands(script, block)) {
				if (hasTargetLanguage()) {
					// alien programming language
					parseAlienInstruction(script, block);
				} else {
					// standard CodeWorker scripting
					std::string sCommand;
					script.skipEmpty();
					if (!script.readIdentifier(sCommand)) handleNotAWordCommand(script, block);
					else if (!parseKeyword(sCommand, script, block)) {
						handleUnknownCommand(sCommand, script, block);
					}
					script.skipEmpty();
				}
			}
		} else {
			_stackOfParsedBlocks.push_front(&block);
			beforeParsingABlock(script, block);
			if (hasTargetLanguage()) {
				// alien programming language
				script.goBack();
				parseAlienBlock(script, block);
			} else {
				betweenCommands(script, block);
				script.skipEmpty();
				while (!script.isEqualTo('}')) {
					if (!script.skipEmpty()) throw UtlException(script, "unexpected end of file: '}' expected");
					parseInstruction(script, block);
					script.skipEmpty();
				}
			}
			afterParsingABlock(script, block);
			_stackOfParsedBlocks.pop_front();
			betweenCommands(script, *(_stackOfParsedBlocks.front()));
		}
	}

	void DtaScript::parseInstruction(ScpStream& script, GrfBlock& block) {
		std::string sCommand;
		script.skipEmpty();
		if (!script.readIdentifier(sCommand)) handleNotAWordCommand(script, block);
		else if (!parseKeyword(sCommand, script, block)) {
			handleUnknownCommand(sCommand, script, block);
		}
		script.skipEmpty();
		betweenCommands(script, block);
	}

	DtaBNFScript& DtaScript::getAlienParser() const {
		DtaBNFScript* pScript = DtaProject::getInstance().getCommonAlienParser();
		if (pScript == NULL) throw UtlException("compiling a common script in " + _sTargetLanguage + " requires \"($CODEWORKER_HOME)/" + _sTargetLanguage + "/default-scripts/" + _sTargetLanguage + "CommonScript.cwp\"");
		return *pScript;
	}

	void DtaScript::parseAlienInstructions(DtaScriptVariable& parseTree, ScpStream& script, GrfBlock& block) {
		const std::list<DtaScriptVariable*>* pInstructions = parseTree.getArray();
		if (pInstructions == NULL) return;
		for (std::list<DtaScriptVariable*>::const_iterator i = pInstructions->begin(); i != pInstructions->end(); ++i) {
			std::string sType = (*i)->getValue();
			if (sType == "alien") {
				DtaScriptVariable* pCode = (*i)->getNode("code");
				if (pCode == NULL) {
					throw UtlException(script, "instruction type 'alien' is expected to have the attribute 'code'");
				}
				DtaScriptVariable* pLine = (*i)->getNode("line");
				if (pLine == NULL) {
					throw UtlException(script, "instruction type 'alien' is expected to have the attribute 'line'");
				}
				GrfAlienInstruction* pAlien = new GrfAlienInstruction(_sTargetLanguage, pLine->getIntValue(), pCode->getValue());
				if (requiresParsingInformation()) pAlien->setParsingInformation(getFilenamePtr(), script);
				block.add(pAlien);
			} else if (sType == "write_alien_expr") {
				DtaScriptVariable* pCode = (*i)->getNode("code");
				if (pCode == NULL) {
					throw UtlException(script, "instruction type 'write_alien_expr' is expected to have the attribute 'code'");
				}
				GrfWriteText* pWriteText = new GrfWriteText;
				pWriteText->setText(new ExprScriptAlien(pCode->getValue(), _sTargetLanguage));
				if (requiresParsingInformation()) pWriteText->setParsingInformation(getFilenamePtr(), script);
				block.add(pWriteText);
			} else if (sType == "text") {
				DtaScriptVariable* pText = (*i)->getNode("text");
				if (pText == NULL) {
					throw UtlException(script, "instruction type 'text' is expected to have the attribute 'text'");
				}
				GrfWriteText* pWriteText = new GrfWriteText;
				pWriteText->setText(new ExprScriptConstant(pText->getValue()));
				if (requiresParsingInformation()) pWriteText->setParsingInformation(getFilenamePtr(), script);
				block.add(pWriteText);
			} else {
				throw UtlException(script, "unknown instruction type '" + sType + "' to add in the parse tree");
			}
		}
	}

	ExprScriptExpression* DtaScript::parseAlienExpression(DtaScriptVariable& parseTree, ScpStream& script, GrfBlock& block) {
		DtaScriptVariable* pExpression = parseTree.getNode("expression");
		if (pExpression == NULL) return NULL;
		ExprScriptExpression* pResult;
		std::string sType = pExpression->getValue();
		if (sType == "alien_expr") {
			DtaScriptVariable* pCode = pExpression->getNode("code");
			if (pCode == NULL) {
				throw UtlException(script, "Expression type 'alien_expr' is expected to have the attribute 'code'");
			}
			pResult = new ExprScriptAlien(pCode->getValue(), _sTargetLanguage);
		} else if (sType == "alien_variable_expr") {
			DtaScriptVariable* pCode = pExpression->getNode("code");
			if (pCode == NULL) {
				throw UtlException(script, "Expression type 'alien_variable_expr' is expected to have the attribute 'code'");
			}
			pResult = new ExprScriptAlienVariable(pCode->getValue(), _sTargetLanguage);
		} else {
			throw UtlException(script, "unknown Expression type '" + sType + "' to add in the parse tree");
		}
		return pResult;
	}

	void DtaScript::parseAlienScript(ScpStream& script, GrfBlock& block) {
		for (;;) {
			DtaScriptVariable parseTree(NULL, "parseTree");
			parseTree.insertNode("type")->setValue("script");
			getAlienParser().generate(script, parseTree);
			parseAlienInstructions(parseTree, script, block);
			if (!parseTree.getBooleanValue()) break;
			parseInstruction(script, block);
		}
	}

	void DtaScript::parseAlienBlock(ScpStream& script, GrfBlock& block) {
		DtaScriptVariable parseTree(NULL, "parseTree");
		parseTree.insertNode("type")->setValue("block");
		getAlienParser().generate(script, parseTree);
		parseAlienInstructions(parseTree, script, block);
	}

	void DtaScript::parseAlienInstruction(ScpStream& script, GrfBlock& block) {
		DtaScriptVariable parseTree(NULL, "parseTree");
		parseTree.insertNode("type")->setValue("instruction");
		getAlienParser().generate(script, parseTree);
		parseAlienInstructions(parseTree, script, block);
	}

	ExprScriptVariable* DtaScript::parseAlienVariableExpression(GrfBlock& block, ScpStream& script) {
		DtaScriptVariable parseTree(NULL, "parseTree");
		parseTree.insertNode("type")->setValue("variable");
		getAlienParser().generate(script, parseTree);
		ExprScriptExpression* pResult = parseAlienExpression(parseTree, script, block);
		ExprScriptVariable* pVariable = dynamic_cast<ExprScriptVariable*>(pResult);
		if (pVariable == NULL) {
			if (pResult != NULL) {
				throw UtlException(script, "the target language parser should have returned a variable expression");
			}
		}
		return pVariable;
	}

	ExprScriptExpression* DtaScript::parseAlienExpression(GrfBlock& block, ScpStream& script) {
		DtaScriptVariable parseTree(NULL, "parseTree");
		parseTree.insertNode("type")->setValue("expression");
		getAlienParser().generate(script, parseTree);
		return parseAlienExpression(parseTree, script, block);
	}

	ExprScriptExpression* DtaScript::parseConstantNodeExpression(GrfBlock& block, ScpStream& script) {
		// parse the tree and array part of the node, not the value
		if (!script.isEqualTo('{')) return NULL;
		std::auto_ptr<ExprScriptConstantTree> pConstantTree(new ExprScriptConstantTree);
		script.skipEmpty();
		if (!script.isEqualTo('}')) {
			int iDeprecatedSyntax = -1;
			do {
				script.skipEmpty();
				if (script.isEqualTo('.')) {
					if (iDeprecatedSyntax == 1) {
						throw UtlException(script, "syntax error: cannot melt both deprecated and legacy constant tree syntax");
					}
					iDeprecatedSyntax = 0;
					std::string sAttribute;
					if (!script.readIdentifier(sAttribute)) {
						throw UtlException(script, "syntax error: attribute name expected");
					}
					script.skipEmpty();
					if (!script.isEqualTo('=')) {
						throw UtlException(script, "syntax error: '=' expected");
					}
					script.skipEmpty();
					if (script.isEqualTo('&')) {
						script.skipEmpty();
						if (!script.isEqualToIdentifier("ref")) {
							throw UtlException(script, "syntax error: '&ref' expected to announce a reference node");
						}
						script.skipEmpty();
						ExprScriptVariable* pRefExpr = parseVariableExpression(block, script);
						pConstantTree->addAttribute("&"+sAttribute, pRefExpr);
					} else {
						pConstantTree->addAttribute(sAttribute, parseAssignmentExpression(block, script));
					}
				} else if (script.isEqualTo('[')) {
					if (iDeprecatedSyntax == 1) {
						throw UtlException(script, "syntax error: cannot melt both deprecated and legacy constant tree syntax");
					}
					iDeprecatedSyntax = 0;
					do {
						script.skipEmpty();
						if (iDeprecatedSyntax == 1) {
							throw UtlException(script, "syntax error: cannot melt both deprecated and legacy constant tree syntax");
						}
						iDeprecatedSyntax = 0;
						ExprScriptExpression* pItem = parseAssignmentExpression(block, script);
						if (pItem == NULL) {
							throw UtlException(script, "syntax error: key or item expression expected");
						}
						script.skipEmpty();
						if (script.isEqualTo(':')) {
							script.skipEmpty();
							ExprScriptExpression* pKey = pItem;
							if (dynamic_cast<ExprScriptConstantTree*>(pKey) != NULL) {
								delete pKey;
								throw UtlException(script, "syntax error: a dictionary key cannot be a constant tree");
							}
							pItem = parseAssignmentExpression(block, script);
							if (pItem == NULL) {
								delete pKey;
								throw UtlException(script, "syntax error: malformed item expression");
							}
							pConstantTree->addItem(pKey, pItem);
							script.skipEmpty();
						} else {
							pConstantTree->addItem(pItem);
						}
					} while (script.isEqualTo(','));
					if (!script.isEqualTo(']')) throw UtlException(script, "']' expected to end a constant array");
				} else {
					// deprecated way of descripting a constant tree
					if (iDeprecatedSyntax == 0) {
						throw UtlException(script, "syntax error: cannot melt both deprecated and legacy constant tree syntax");
					}
					iDeprecatedSyntax = 1;
					pConstantTree->addItem(parseAssignmentExpression(block, script));
				}
				script.skipEmpty();
			} while (script.isEqualTo(','));
			if (!script.isEqualTo('}')) throw UtlException(script, "'}' expected to end a constant tree");
		}
		return pConstantTree.release();
	}

	ExprScriptExpression* DtaScript::parseAssignmentExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptExpression* expr = NULL;
		if (script.peekChar() != '{') {
			expr = parseExpression(block, script);
		}
		ExprScriptConstantTree* pTree = (ExprScriptConstantTree*) parseConstantNodeExpression(block, script);
		if (pTree != NULL) {
			pTree->setValueExpression(expr);
			expr = pTree;
		}
		return expr;
	}

	std::map<std::string, int>& DtaScript::getParsingFunctionRegister() {
		static std::map<std::string, int> _register;
		if (_register.empty()) {
			_register["foreach"] = 1;
			_register["if"] = 2;
			_register["do"] = 3;
			_register["switch"] = 4;
			_register["try"] = 5;
			_register["while"] = 6;
			_register["function"] = 7;
			_register["continue"] = 8;
			_register["break"] = 9;
			_register["finally"] = 10;
			_register["return"] = 11;
			_register["external"] = 12;
			_register["file_as_standard_input"] = 13;
			_register["string_as_standard_input"] = 14;
			_register["nop"] = 15;
			_register["declare"] = 16;
			_register["delay"] = 17;
			_register["debug"] = 18;
			_register["quantify"] = 19;
			_register["quiet"] = 20;
			_register["local"] = 21;
			_register["localref"] = 22;
			_register["set"] = 23;
			_register["setall"] = 24;
			_register["ref"] = 25;
			_register["pushItem"] = 26;
			_register["new_project"] = 27;
			_register["generated_file"] = 28;
			_register["appended_file"] = 29;
			_register["generated_string"] = 30;
			_register["forfile"] = 31;
			_register["insert"] = 32;
			_register["readonlyHook"] = 33;
			_register["writefileHook"] = 34;
			_register["merge"] = 35;
			_register["parsed_file"] = 36;
			_register["select"] = 37;
			_register["global"] = 38;
			_register["parsed_string"] = 39;
			_register["exit"] = 40;
			_register["class"] = 41;
//##markup##"getParsingFunctionRegister()"
//##begin##"getParsingFunctionRegister()"
		if (_register.find("appendFile") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('appendFile') already exists");
		_register["appendFile"] = 179126;
		if (_register.find("autoexpand") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('autoexpand') already exists");
		_register["autoexpand"] = 2911241;
		if (_register.find("clearVariable") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('clearVariable') already exists");
		_register["clearVariable"] = 7413225;
		if (_register.find("compileToCpp") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('compileToCpp') already exists");
		_register["compileToCpp"] = 18027701;
		if (_register.find("copyFile") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('copyFile') already exists");
		_register["copyFile"] = 2106257;
		if (_register.find("copyGenerableFile") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('copyGenerableFile') already exists");
		_register["copyGenerableFile"] = 9921918;
		if (_register.find("copySmartDirectory") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('copySmartDirectory') already exists");
		_register["copySmartDirectory"] = 18645881;
		if (_register.find("cutString") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('cutString') already exists");
		_register["cutString"] = 5833203;
		if (_register.find("environTable") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('environTable') already exists");
		_register["environTable"] = 19353497;
		if (_register.find("error") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('error') already exists");
		_register["error"] = 16784904;
		if (_register.find("executeString") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('executeString') already exists");
		_register["executeString"] = 10410726;
		if (_register.find("expand") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('expand') already exists");
		_register["expand"] = 5800090;
		if (_register.find("extendExecutedScript") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('extendExecutedScript') already exists");
		_register["extendExecutedScript"] = 11255540;
		if (_register.find("generate") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('generate') already exists");
		_register["generate"] = 3100533;
		if (_register.find("generateString") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('generateString') already exists");
		_register["generateString"] = 17410534;
		if (_register.find("insertElementAt") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('insertElementAt') already exists");
		_register["insertElementAt"] = 13901206;
		if (_register.find("invertArray") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('invertArray') already exists");
		_register["invertArray"] = 7040035;
		if (_register.find("listAllGeneratedFiles") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('listAllGeneratedFiles') already exists");
		_register["listAllGeneratedFiles"] = 3779115;
		if (_register.find("loadProject") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('loadProject') already exists");
		_register["loadProject"] = 15590483;
		if (_register.find("openLogFile") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('openLogFile') already exists");
		_register["openLogFile"] = 4625718;
		if (_register.find("parseAsBNF") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('parseAsBNF') already exists");
		_register["parseAsBNF"] = 8623893;
		if (_register.find("parseStringAsBNF") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('parseStringAsBNF') already exists");
		_register["parseStringAsBNF"] = 12713924;
		if (_register.find("parseFree") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('parseFree') already exists");
		_register["parseFree"] = 3007935;
		if (_register.find("produceHTML") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('produceHTML') already exists");
		_register["produceHTML"] = 2453643;
		if (_register.find("putEnv") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('putEnv') already exists");
		_register["putEnv"] = 18042462;
		if (_register.find("randomSeed") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('randomSeed') already exists");
		_register["randomSeed"] = 15941076;
		if (_register.find("removeAllElements") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('removeAllElements') already exists");
		_register["removeAllElements"] = 15019924;
		if (_register.find("removeElement") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('removeElement') already exists");
		_register["removeElement"] = 15071448;
		if (_register.find("removeFirstElement") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('removeFirstElement') already exists");
		_register["removeFirstElement"] = 19507408;
		if (_register.find("removeLastElement") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('removeLastElement') already exists");
		_register["removeLastElement"] = 18768482;
		if (_register.find("removeRecursive") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('removeRecursive') already exists");
		_register["removeRecursive"] = 5023246;
		if (_register.find("removeVariable") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('removeVariable') already exists");
		_register["removeVariable"] = 7468320;
		if (_register.find("saveBinaryToFile") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('saveBinaryToFile') already exists");
		_register["saveBinaryToFile"] = 3078933;
		if (_register.find("saveProject") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('saveProject') already exists");
		_register["saveProject"] = 18607068;
		if (_register.find("saveProjectTypes") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('saveProjectTypes') already exists");
		_register["saveProjectTypes"] = 730621;
		if (_register.find("saveToFile") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('saveToFile') already exists");
		_register["saveToFile"] = 14995060;
		if (_register.find("setCommentBegin") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setCommentBegin') already exists");
		_register["setCommentBegin"] = 14972940;
		if (_register.find("setCommentEnd") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setCommentEnd') already exists");
		_register["setCommentEnd"] = 9300734;
		if (_register.find("setGenerationHeader") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setGenerationHeader') already exists");
		_register["setGenerationHeader"] = 3237703;
		if (_register.find("setIncludePath") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setIncludePath') already exists");
		_register["setIncludePath"] = 5288747;
		if (_register.find("setNow") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setNow') already exists");
		_register["setNow"] = 9162260;
		if (_register.find("setProperty") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setProperty') already exists");
		_register["setProperty"] = 10303831;
		if (_register.find("setTextMode") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setTextMode') already exists");
		_register["setTextMode"] = 3850482;
		if (_register.find("setVersion") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setVersion') already exists");
		_register["setVersion"] = 12168214;
		if (_register.find("setWriteMode") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setWriteMode') already exists");
		_register["setWriteMode"] = 5067744;
		if (_register.find("setWorkingPath") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setWorkingPath') already exists");
		_register["setWorkingPath"] = 16222516;
		if (_register.find("sleep") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('sleep') already exists");
		_register["sleep"] = 9522647;
		if (_register.find("slideNodeContent") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('slideNodeContent') already exists");
		_register["slideNodeContent"] = 13707430;
		if (_register.find("sortArray") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('sortArray') already exists");
		_register["sortArray"] = 9410907;
		if (_register.find("traceEngine") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('traceEngine') already exists");
		_register["traceEngine"] = 13561575;
		if (_register.find("traceLine") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('traceLine') already exists");
		_register["traceLine"] = 16138969;
		if (_register.find("traceObject") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('traceObject') already exists");
		_register["traceObject"] = 8856036;
		if (_register.find("traceStack") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('traceStack') already exists");
		_register["traceStack"] = 7087939;
		if (_register.find("traceText") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('traceText') already exists");
		_register["traceText"] = 16373778;
		if (_register.find("translate") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('translate') already exists");
		_register["translate"] = 4934734;
		if (_register.find("attachInputToSocket") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('attachInputToSocket') already exists");
		_register["attachInputToSocket"] = 4116339;
		if (_register.find("detachInputFromSocket") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('detachInputFromSocket') already exists");
		_register["detachInputFromSocket"] = 16057876;
		if (_register.find("goBack") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('goBack') already exists");
		_register["goBack"] = 13375983;
		if (_register.find("setInputLocation") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setInputLocation') already exists");
		_register["setInputLocation"] = 17901405;
		if (_register.find("allFloatingLocations") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('allFloatingLocations') already exists");
		_register["allFloatingLocations"] = 16157783;
		if (_register.find("attachOutputToSocket") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('attachOutputToSocket') already exists");
		_register["attachOutputToSocket"] = 2539668;
		if (_register.find("detachOutputFromSocket") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('detachOutputFromSocket') already exists");
		_register["detachOutputFromSocket"] = 10159185;
		if (_register.find("incrementIndentLevel") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('incrementIndentLevel') already exists");
		_register["incrementIndentLevel"] = 9232969;
		if (_register.find("insertText") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('insertText') already exists");
		_register["insertText"] = 2938502;
		if (_register.find("insertTextOnce") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('insertTextOnce') already exists");
		_register["insertTextOnce"] = 10767911;
		if (_register.find("insertTextToFloatingLocation") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('insertTextToFloatingLocation') already exists");
		_register["insertTextToFloatingLocation"] = 2074012;
		if (_register.find("insertTextOnceToFloatingLocation") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('insertTextOnceToFloatingLocation') already exists");
		_register["insertTextOnceToFloatingLocation"] = 19491581;
		if (_register.find("overwritePortion") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('overwritePortion') already exists");
		_register["overwritePortion"] = 4193756;
		if (_register.find("populateProtectedArea") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('populateProtectedArea') already exists");
		_register["populateProtectedArea"] = 19875729;
		if (_register.find("resizeOutputStream") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('resizeOutputStream') already exists");
		_register["resizeOutputStream"] = 3517365;
		if (_register.find("setFloatingLocation") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setFloatingLocation') already exists");
		_register["setFloatingLocation"] = 11884637;
		if (_register.find("setOutputLocation") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setOutputLocation') already exists");
		_register["setOutputLocation"] = 8004824;
		if (_register.find("setProtectedArea") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('setProtectedArea') already exists");
		_register["setProtectedArea"] = 14826777;
		if (_register.find("writeBytes") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('writeBytes') already exists");
		_register["writeBytes"] = 6512844;
		if (_register.find("writeText") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('writeText') already exists");
		_register["writeText"] = 16211116;
		if (_register.find("writeTextOnce") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('writeTextOnce') already exists");
		_register["writeTextOnce"] = 8521805;
		if (_register.find("closeSocket") != _register.end()) throw UtlException("DtaScript::getParsingFunctionRegister('closeSocket') already exists");
		_register["closeSocket"] = 19888011;
//##end##"getParsingFunctionRegister()"
		}
		return _register;
	}

	bool DtaScript::parseKeyword(const std::string& sCommand, ScpStream& script, GrfBlock& block, ExprScriptVariable* pMethodCaller) {
		std::map<std::string, int>::const_iterator cursor = getParsingFunctionRegister().find(sCommand);
		bool bSuccess = (cursor != getParsingFunctionRegister().end());
		if (bSuccess) {
			switch(cursor->second) {
				case  1: parseForeach(block, script);break;
				case  2: parseIfThenElse(block, script);break;
				case  3: parseDo(block, script);break;
				case  4: parseSwitch(block, script);break;
				case  5: parseTryCatch(block, script);break;
				case  6: parseWhile(block, script);break;
				case  7: parseFunction(block, script);break;
				case  8: parseContinue(block, script);break;
				case  9: parseBreak(block, script);break;
				case 10: parseFinally(block, script);break;
				case 11: parseReturn(block, script);break;
				case 12: parseExternal(block, script);break;
				case 13: parseFileAsStandardInput(block, script);break;
				case 14: parseStringAsStandardInput(block, script);break;
				case 15: parseNop(block, script);break;
				case 16: parseDeclare(block, script);break;
				case 17: parseDelay(block, script);break;
				case 18: parseDebug(block, script);break;
				case 19: parseQuantify(block, script);break;
				case 20: parseQuiet(block, script);break;
				case 21: parseLocalVariable(block, script);break;
				case 22: parseLocalReference(block, script);break;
				case 23: parseSetAssignment(block, script);break;
				case 24: parseSetAll(block, script);break;
				case 25: parseReference(block, script);break;
				case 26: parsePushItem(block, script);break;
				case 27: parseNewProject(block, script);break;
				case 28: parseGeneratedFile(block, script);break;
				case 29: parseAppendedFile(block, script);break;
				case 30: parseGeneratedString(block, script);break;
				case 31: parseForfile(block, script);break;
				case 32: parseInsertAssignment(block, script);break;
				case 33: parseReadonlyHook(block, script);break;
				case 34: parseWritefileHook(block, script);break;
				case 35: parseMerge(block, script);break;
				case 36: parseParsedFile(block, script);break;
				case 37: parseSelect(block, script);break;
				case 38: parseGlobalVariable(block, script);break;
				case 39: parseParsedString(block, script);break;
				case 40: parseExit(block, script);break;
				case 41: parseClass(block, script);break;
//##markup##"parseKeyword()"
//##begin##"parseKeyword()"
			case 179126: parseAppendFile(block, script, pMethodCaller);break;
			case 2911241: parseAutoexpand(block, script, pMethodCaller);break;
			case 7413225: parseClearVariable(block, script, pMethodCaller);break;
			case 18027701: parseCompileToCpp(block, script, pMethodCaller);break;
			case 2106257: parseCopyFile(block, script, pMethodCaller);break;
			case 9921918: parseCopyGenerableFile(block, script, pMethodCaller);break;
			case 18645881: parseCopySmartDirectory(block, script, pMethodCaller);break;
			case 5833203: parseCutString(block, script, pMethodCaller);break;
			case 19353497: parseEnvironTable(block, script, pMethodCaller);break;
			case 16784904: parseError(block, script, pMethodCaller);break;
			case 10410726: parseExecuteString(block, script, pMethodCaller);break;
			case 5800090: parseExpand(block, script, pMethodCaller);break;
			case 11255540: parseExtendExecutedScript(block, script, pMethodCaller);break;
			case 3100533: parseGenerate(block, script, pMethodCaller);break;
			case 17410534: parseGenerateString(block, script, pMethodCaller);break;
			case 13901206: parseInsertElementAt(block, script, pMethodCaller);break;
			case 7040035: parseInvertArray(block, script, pMethodCaller);break;
			case 3779115: parseListAllGeneratedFiles(block, script, pMethodCaller);break;
			case 15590483: parseLoadProject(block, script, pMethodCaller);break;
			case 4625718: parseOpenLogFile(block, script, pMethodCaller);break;
			case 8623893: parseParseAsBNF(block, script, pMethodCaller);break;
			case 12713924: parseParseStringAsBNF(block, script, pMethodCaller);break;
			case 3007935: parseParseFree(block, script, pMethodCaller);break;
			case 2453643: parseProduceHTML(block, script, pMethodCaller);break;
			case 18042462: parsePutEnv(block, script, pMethodCaller);break;
			case 15941076: parseRandomSeed(block, script, pMethodCaller);break;
			case 15019924: parseRemoveAllElements(block, script, pMethodCaller);break;
			case 15071448: parseRemoveElement(block, script, pMethodCaller);break;
			case 19507408: parseRemoveFirstElement(block, script, pMethodCaller);break;
			case 18768482: parseRemoveLastElement(block, script, pMethodCaller);break;
			case 5023246: parseRemoveRecursive(block, script, pMethodCaller);break;
			case 7468320: parseRemoveVariable(block, script, pMethodCaller);break;
			case 3078933: parseSaveBinaryToFile(block, script, pMethodCaller);break;
			case 18607068: parseSaveProject(block, script, pMethodCaller);break;
			case 730621: parseSaveProjectTypes(block, script, pMethodCaller);break;
			case 14995060: parseSaveToFile(block, script, pMethodCaller);break;
			case 14972940: parseSetCommentBegin(block, script, pMethodCaller);break;
			case 9300734: parseSetCommentEnd(block, script, pMethodCaller);break;
			case 3237703: parseSetGenerationHeader(block, script, pMethodCaller);break;
			case 5288747: parseSetIncludePath(block, script, pMethodCaller);break;
			case 9162260: parseSetNow(block, script, pMethodCaller);break;
			case 10303831: parseSetProperty(block, script, pMethodCaller);break;
			case 3850482: parseSetTextMode(block, script, pMethodCaller);break;
			case 12168214: parseSetVersion(block, script, pMethodCaller);break;
			case 5067744: parseSetWriteMode(block, script, pMethodCaller);break;
			case 16222516: parseSetWorkingPath(block, script, pMethodCaller);break;
			case 9522647: parseSleep(block, script, pMethodCaller);break;
			case 13707430: parseSlideNodeContent(block, script, pMethodCaller);break;
			case 9410907: parseSortArray(block, script, pMethodCaller);break;
			case 13561575: parseTraceEngine(block, script, pMethodCaller);break;
			case 16138969: parseTraceLine(block, script, pMethodCaller);break;
			case 8856036: parseTraceObject(block, script, pMethodCaller);break;
			case 7087939: parseTraceStack(block, script, pMethodCaller);break;
			case 16373778: parseTraceText(block, script, pMethodCaller);break;
			case 4934734: parseTranslate(block, script, pMethodCaller);break;
			case 4116339: parseAttachInputToSocket(block, script, pMethodCaller);break;
			case 16057876: parseDetachInputFromSocket(block, script, pMethodCaller);break;
			case 13375983: parseGoBack(block, script, pMethodCaller);break;
			case 17901405: parseSetInputLocation(block, script, pMethodCaller);break;
			case 16157783: parseAllFloatingLocations(block, script, pMethodCaller);break;
			case 2539668: parseAttachOutputToSocket(block, script, pMethodCaller);break;
			case 10159185: parseDetachOutputFromSocket(block, script, pMethodCaller);break;
			case 9232969: parseIncrementIndentLevel(block, script, pMethodCaller);break;
			case 2938502: parseInsertText(block, script, pMethodCaller);break;
			case 10767911: parseInsertTextOnce(block, script, pMethodCaller);break;
			case 2074012: parseInsertTextToFloatingLocation(block, script, pMethodCaller);break;
			case 19491581: parseInsertTextOnceToFloatingLocation(block, script, pMethodCaller);break;
			case 4193756: parseOverwritePortion(block, script, pMethodCaller);break;
			case 19875729: parsePopulateProtectedArea(block, script, pMethodCaller);break;
			case 3517365: parseResizeOutputStream(block, script, pMethodCaller);break;
			case 11884637: parseSetFloatingLocation(block, script, pMethodCaller);break;
			case 8004824: parseSetOutputLocation(block, script, pMethodCaller);break;
			case 14826777: parseSetProtectedArea(block, script, pMethodCaller);break;
			case 6512844: parseWriteBytes(block, script, pMethodCaller);break;
			case 16211116: parseWriteText(block, script, pMethodCaller);break;
			case 8521805: parseWriteTextOnce(block, script, pMethodCaller);break;
			case 19888011: parseCloseSocket(block, script, pMethodCaller);break;
//##end##"parseKeyword()"
				default:
					throw UtlException("DtaScript::parseKeyword() unhandled command");
			}
		} else if (sCommand == "include") {
			std::string sErrorMessage = "warning: procedure-like syntax of the 'include' directive is obsolete since version 1.40 -> replace it by the preprocessor directive '#include'" + CGRuntime::endl();
			sErrorMessage += UtlTrace::getTraceStack();
			if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
			script.skipEmpty();
			if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
			script.skipEmpty();
			std::string sScriptFileName;
			if (!script.readString(sScriptFileName)) throw UtlException(script, "syntax error: constant string expected");
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
			script.skipEmpty();
			if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
			parseFile(sScriptFileName.c_str(), &block);
			bSuccess = true;
//##markup##"parseKeyword()::deprecated"
//##begin##"parseKeyword()::deprecated"
	} else if (sCommand == "clearNode") {
		std::string sErrorMessage = "warning: procedure 'clearNode' has been deprecated since version 3.8.7 -> replace it by procedure 'clearVariable'" + CGRuntime::endl();
		sErrorMessage += UtlTrace::getTraceStack();
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		parseClearVariable(block, script, pMethodCaller);
		bSuccess = true;
	} else if (sCommand == "loadDesign") {
		std::string sErrorMessage = "warning: procedure 'loadDesign' has been deprecated since version 1.6 -> replace it by procedure 'parseFree'" + CGRuntime::endl();
		sErrorMessage += UtlTrace::getTraceStack();
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		parseParseFree(block, script, pMethodCaller);
		bSuccess = true;
	} else if (sCommand == "setDefineTarget") {
		std::string sErrorMessage = "warning: procedure 'setDefineTarget' has been deprecated since version 1.30 -> replace it by procedure 'setProperty'" + CGRuntime::endl();
		sErrorMessage += UtlTrace::getTraceStack();
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		parseSetProperty(block, script, pMethodCaller);
		bSuccess = true;
	} else if (sCommand == "setLocation") {
		std::string sErrorMessage = "warning: procedure 'setLocation' has been deprecated since version 3.7.1 -> replace it by procedure 'setInputLocation'" + CGRuntime::endl();
		sErrorMessage += UtlTrace::getTraceStack();
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		parseSetInputLocation(block, script, pMethodCaller);
		bSuccess = true;
//##end##"parseKeyword()::deprecated"
		}
		return bSuccess;
	}

	void DtaScript::parseForeachListDeclaration(GrfBlock& block, ScpStream& script, GrfForeach* pForeach) {
		script.skipEmpty();
		pForeach->setIndex(parseIndexExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualToIdentifier("in")) throw UtlException(script, "syntax error, 'in' expected");
		script.skipEmpty();
		for (;;) {
			if (script.isEqualToIdentifier("sorted")) {
				if (pForeach->getSorted()) throw UtlException(script, "'sorted' encountered twice");
				script.skipEmpty();
				if (script.isEqualToIdentifier("no_case")) pForeach->setSortedNoCase(true);
				else pForeach->setSorted(true);
			} else if (script.isEqualToIdentifier("reverse")) {
				pForeach->setReverse(true);
			} else if (script.isEqualToIdentifier("cascading")) {
				if (pForeach->getCascading() != GrfForeach::NO_CASCADING) throw UtlException(script, "'cascading' encountered twice");
				script.skipEmpty();
				if (script.isEqualToIdentifier("first")) pForeach->setCascading(GrfForeach::CASCADING_FIRST);
				else if (script.isEqualToIdentifier("last")) pForeach->setCascading(GrfForeach::CASCADING_LAST);
				else pForeach->setCascading(GrfForeach::CASCADING_LAST);
			} else if (script.isEqualToIdentifier("by_value")) {
				if (!pForeach->getSorted()) throw UtlException(script, "'sorted' keywork expected before 'by_value'");
				pForeach->setSortOnValue(true);
			} else break;
			script.skipEmpty();
		}
		pForeach->setList(parseVariableExpression(block, script));
	}

	void DtaScript::parseForeach(GrfBlock& block, ScpStream& script) {
		GrfForeach* pForeach = new GrfForeach(&block);
		if (requiresParsingInformation()) pForeach->setParsingInformation(getFilenamePtr(), script);
		block.add(pForeach);
		parseForeachListDeclaration(block, script, pForeach);
		parseBlock(script, *pForeach);
	}

	void DtaScript::parseSelect(GrfBlock& block, ScpStream& script) {
		GrfSelect* pSelect = new GrfSelect(&block);
		if (requiresParsingInformation()) pSelect->setParsingInformation(getFilenamePtr(), script);
		block.add(pSelect);
		script.skipEmpty();
		pSelect->setIndex(parseIndexExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualToIdentifier("in")) throw UtlException(script, "syntax error, 'in' expected");
		script.skipEmpty();
		for (;;) {
			if (script.isEqualToIdentifier("sorted")) {
				throw UtlException("select-sorted not implemented yet!");
			} else break;
			script.skipEmpty();
		}
		pSelect->setMotif(parseMotifExpression(block, script));
		parseBlock(script, *pSelect);
	}

	void DtaScript::parseForfile(GrfBlock& block, ScpStream& script) {
		script.skipEmpty();
		std::auto_ptr<ExprScriptVariable> pIndex(parseIndexExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualToIdentifier("in")) throw UtlException(script, "syntax error, 'in' expected");
		script.skipEmpty();
		bool bSorted = false;
		bool bNoCase = false;
		GrfForeach::CASCADING iCascading = GrfForeach::NO_CASCADING;
		for (;;) {
			if (script.isEqualToIdentifier("sorted")) {
				if (bSorted) throw UtlException(script, "'sorted' encountered twice");
				script.skipEmpty();
				bSorted = true;
				if (script.isEqualToIdentifier("no_case")) bNoCase = true;
			} else if (script.isEqualToIdentifier("cascading")) {
				if (iCascading != GrfForeach::NO_CASCADING) throw UtlException(script, "'cascading' encountered twice");
				script.skipEmpty();
				if (script.isEqualToIdentifier("first")) iCascading = GrfForeach::CASCADING_FIRST;
				else if (script.isEqualToIdentifier("last")) iCascading = GrfForeach::CASCADING_LAST;
				else iCascading = GrfForeach::CASCADING_LAST;
			} else break;
			script.skipEmpty();
		}
		std::auto_ptr<ExprScriptExpression> pPattern(parseExpression(block, script));
		char tcCursor[32];
		sprintf(tcCursor, "%d", script.getInputLocation());
		std::string sCursor = tcCursor;
		ScpStream theStream;
		std::string sPattern = "_compilerForfile_pattern" + sCursor;
		std::string sPosition = "_compilerForfile_position" + sCursor;
		std::string sTmpPosition = "_compilerForfile_tmpPosition" + sCursor;
		std::string sPath = "_compilerForfile_path" + sCursor;
		std::string sFiles = "_compilerForfile_files" + sCursor;
		theStream << "\tlocal " << sPattern << " = " << pPattern->toString() << ";\n";
		theStream << "\tlocal " << sPosition << " = findLastString(" << sPattern << ", \"\\\\\");\n";
		theStream << "\tlocal " << sTmpPosition << " = findLastString(" << sPattern << ", \"/\");\n";
		theStream << "\tif $" << sTmpPosition << " > " << sPosition << "$ set " << sPosition << " = " << sTmpPosition << ";\n";
		theStream << "\tlocal " << sPath << ";\n";
		theStream << "\tif isPositive(" << sPosition << ") {\n";
		theStream << "\t\tset " << sPath << " = leftString(" << sPattern << ", " << sPosition << ");\n";
		theStream << "\t\tset " << sPattern << " = subString(" << sPattern << ", $" << sPosition << " + 1$);\n";
		theStream << "\t}\n";
		theStream << "\tlocal " << sFiles << ";\n";
		if (iCascading == GrfForeach::NO_CASCADING) {
			theStream << "\tscanFiles(" << sFiles << ", "  << sPath << ", " << sPattern << ", false);\n";
			theStream << "\tforeach " << pIndex->toString() << " in ";
			if (bSorted) {
				theStream << "sorted ";
				if (bNoCase) theStream << "no_case ";
			}
			theStream << sFiles << " ;\n";
		} else {
			std::string sDirectories = sFiles + ".directories";
			std::string sIndex = "_compilerForfile_index" + sCursor;
			theStream << "\tpushItem " << sDirectories << ";\n";
			theStream << "\tscanDirectories(" << sDirectories << "#front, "  << sPath << ", " << sPattern << ");\n";
			theStream << "\tforeach " << sIndex << " in cascading ";
			if (bSorted) {
				theStream << "sorted ";
				if (bNoCase) theStream << "no_case ";
			}
			theStream << sDirectories << "{\n";
			theStream << "\t\tforeach " << pIndex->toString() << " in ";
			if (bSorted) {
				theStream << "sorted ";
				if (bNoCase) theStream << "no_case ";
			}
			theStream << sIndex << ".files {\n";
			theStream << "\t\t\tlocal _compilerForfile_file = " << sIndex << " + " << pIndex->toString() << ";\n";
			theStream << "\t\t\tinsert " << sIndex << ".list[_compilerForfile_file] = _compilerForfile_file;\n";
			theStream << "\t\t}\n";
			theStream << "\t\tforeach " << pIndex->toString() << " in ";
			theStream << sIndex << ".list ;\n";
			theStream << "\t}\n";
		}
		bool bRequiresParsingInformation = requiresParsingInformation();
		try {
			requiresParsingInformation(false);
			parseScript(theStream, block);
			requiresParsingInformation(bRequiresParsingInformation);
		} catch(UtlException& exception) {
			requiresParsingInformation(bRequiresParsingInformation);
			std::string sMessage = "internal error while parsing the following expanded expression of 'forfile':" + CGRuntime::endl();
			sMessage += theStream.readBuffer();
			sMessage += exception.getMessage();
			throw UtlException(exception.getTraceStack(), sMessage);
		}
		GrfForeach* pForeach = dynamic_cast<GrfForeach*>(block.getCommands()[block.getNbCommands() - 1]);
		if (pForeach == NULL) throw UtlException(script, "internal error on parsing of 'forfile'");
		if (iCascading != GrfForeach::NO_CASCADING) {
			pForeach = dynamic_cast<GrfForeach*>(pForeach->getCommands()[pForeach->getNbCommands() - 1]);
			if (pForeach == NULL) throw UtlException(script, "internal error on parsing of 'forfile'");
		}
		if (requiresParsingInformation()) pForeach->setParsingInformation(getFilenamePtr(), script);
		parseBlock(script, *pForeach);
	}

	void DtaScript::parseDo(GrfBlock& block, ScpStream& script) {
		GrfDo* pDo = new GrfDo(&block);
		if (requiresParsingInformation()) pDo->setParsingInformation(getFilenamePtr(), script);
		block.add(pDo);
		parseBlock(script, *pDo);
		script.skipEmpty();
		std::string sWord;
		if (!script.readIdentifier(sWord)) throw UtlException(script, "syntax error, 'while' expected after a 'do' statement");
		pDo->setCondition(parseExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error, ';' expected");
	}

	void DtaScript::parseWhile(GrfBlock& block, ScpStream& script) {
		GrfWhile* pWhile = new GrfWhile(&block);
		if (requiresParsingInformation()) pWhile->setParsingInformation(getFilenamePtr(), script);
		block.add(pWhile);
		script.skipEmpty();
		pWhile->setCondition(parseExpression(block, script));
		parseBlock(script, *pWhile);
	}

	void DtaScript::parseSwitch(GrfBlock& block, ScpStream& script) {
		GrfSwitch* pSwitch = new GrfSwitch(&block);
		if (requiresParsingInformation()) pSwitch->setParsingInformation(getFilenamePtr(), script);
		block.add(pSwitch);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error, '(' expected");
		pSwitch->setExpression(parseExpression(block, script));
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error, ')' expected");
		script.skipEmpty();
		if (!script.isEqualTo('{')) throw UtlException(script, "syntax error, '}' expected");
		script.skipEmpty();
		bool bDefault = false;
		_stackOfParsedBlocks.push_front(pSwitch);
		beforeParsingABlock(script, *pSwitch);
		while (!script.isEqualTo('}')) {
			std::string sWord;
			if ((!script.readIdentifier(sWord)) || ((sWord != "case") && (sWord != "start") && (sWord != "default"))) {
				if (bDefault) throw UtlException(script, "syntax error, 'case' or 'start' expected");
				else throw UtlException(script, "syntax error, 'case', 'start' or 'default' expected");
			}
			if (sWord == "default") {
				pSwitch->insertDefault();
			} else {
				std::string sConstant;
				script.skipEmpty();
				if (!script.readStringOrCharLiteral(sConstant)) throw UtlException(script, "syntax error, constant string expected after a '" + sWord + "'");
				if (sWord == "case") {
					if (pSwitch->equalString(sConstant.c_str())) throw UtlException(script, "syntax error, constant string \"" + sConstant + "\" already registered as a 'case'");
					pSwitch->insertEqualString(sConstant.c_str());
				} else {
					if (pSwitch->startString(sConstant.c_str())) throw UtlException(script, "syntax error, constant string \"" + sConstant + "\" already registered as a 'start'");
					pSwitch->insertStartString(sConstant.c_str());
				}
			}
			script.skipEmpty();
			if (!script.isEqualTo(':')) throw UtlException(script, "syntax error, ':' expected after a '" + sWord + "'");
			script.skipEmpty();
			betweenCommands(script, *pSwitch);
			while (script.peekChar() != (int) '}') {
				if (script.readIdentifier(sWord)) {
					script.setInputLocation(script.getInputLocation() - sWord.size());
					if ((sWord == "case") || (sWord == "start")) break;
					if (sWord == "default") {
						if (bDefault) throw UtlException(script, "'default' statement already defined");
						bDefault = true;
						break;
					}
				}
				parseInstruction(script, *pSwitch);
				script.skipEmpty();
			}
		}
		afterParsingABlock(script, *pSwitch);
		_stackOfParsedBlocks.pop_front();
	}

	void DtaScript::parseTryCatch(GrfBlock& block, ScpStream& script) {
		GrfTryCatch* pTryCatch = new GrfTryCatch;
		if (requiresParsingInformation()) pTryCatch->setParsingInformation(getFilenamePtr(), script);
		block.add(pTryCatch);
		script.skipEmpty();
		GrfBlock* pTryBlock = new GrfBlock(&block);
		if (requiresParsingInformation()) pTryBlock->setParsingInformation(getFilenamePtr(), script);
		pTryCatch->setTryBlock(pTryBlock);
		parseBlock(script, *pTryBlock);
		script.skipEmpty();
		std::string sWord;
		if (!script.readIdentifier(sWord)|| (sWord != "catch")) throw UtlException(script, "syntax error, 'catch' expected");
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error, '(' expected after a 'catch'");
		script.skipEmpty();
		std::string sVariableName;
		if (script.readIdentifier(sVariableName)) {
			if (block.getVariable(sVariableName) == UNKNOWN_EXPRTYPE) block.addLocalVariable(sVariableName, VALUE_EXPRTYPE);
			pTryCatch->setErrorVariable(parseVariableExpression(block, script, new ExprScriptVariable(sVariableName.c_str())));
		} else {
			pTryCatch->setErrorVariable(parseVariableExpression(block, script));
		}
		script.skipEmpty();
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error, ')' expected after a 'catch'");
		script.skipEmpty();
		GrfBlock* pCatchBlock = new GrfBlock(&block);
		if (requiresParsingInformation()) pCatchBlock->setParsingInformation(getFilenamePtr(), script);
		pTryCatch->setCatchBlock(pCatchBlock);
		parseBlock(script, *pCatchBlock);
	}

	void DtaScript::parseIfThenElse(GrfBlock& block, ScpStream& script) {
		GrfIfThenElse* pIfThenElse = new GrfIfThenElse;
		if (requiresParsingInformation()) pIfThenElse->setParsingInformation(getFilenamePtr(), script);
		block.add(pIfThenElse);
		script.skipEmpty();
		pIfThenElse->setCondition(parseExpression(block, script));
		script.skipEmpty();
		GrfBlock* pThenBlock = new GrfBlock(&block);
		if (requiresParsingInformation()) pThenBlock->setParsingInformation(getFilenamePtr(), script);
		pIfThenElse->setThenBlock(pThenBlock);
		parseBlock(script, *pThenBlock);
		script.skipEmpty();
		std::string sWord;
		if (script.readIdentifier(sWord)) {
			if (sWord == "else") {
				script.skipEmpty();
				GrfBlock* pElseBlock = new GrfBlock(&block);
				if (requiresParsingInformation()) pElseBlock->setParsingInformation(getFilenamePtr(), script);
				pIfThenElse->setElseBlock(pElseBlock);
				parseBlock(script, *pElseBlock);
			} else {
				script.setInputLocation(script.getInputLocation() - sWord.size());
			}
		}
	}

	void DtaScript::parseContinue(GrfBlock& block, ScpStream& script) {
		GrfContinue* pContinue = new GrfContinue;
		if (requiresParsingInformation()) pContinue->setParsingInformation(getFilenamePtr(), script);
		block.add(pContinue);
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error, ';' expected");
	}

	void DtaScript::parseBreak(GrfBlock& block, ScpStream& script) {
		GrfBreak* pBreak = new GrfBreak;
		if (requiresParsingInformation()) pBreak->setParsingInformation(getFilenamePtr(), script);
		block.add(pBreak);
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error, ';' expected");
	}

	void DtaScript::parseFinally(GrfBlock& block, ScpStream& script) {
		if (_sCurrentFunctionBody.empty()) throw UtlException(script, "'finally' must be called inside a function's body!");
		std::auto_ptr<ExprScriptFunction> pFuncExpr(ExprScriptFunction::create(block, script, _sCurrentFunctionBody, _sCurrentFunctionTemplateBody, _bCurrentFunctionGenericTemplateKey));
		GrfFunction* pFunction = pFuncExpr->getUserBody();
		if (pFunction->getFinally() != NULL) {
			std::string sMessage = "an occurrence of 'finally' already exists inside function \"" + _sCurrentFunctionBody;
			if (_sCurrentFunctionTemplateBody.empty()) sMessage += "\"";
			else if (_bCurrentFunctionGenericTemplateKey) sMessage += "<" + _sCurrentFunctionTemplateBody + ">";
			else sMessage += "<\"" + _sCurrentFunctionTemplateBody + "\">";
			throw UtlException(script, sMessage);
		}
		GrfBlock* pFinallyBlock = new GrfBlock(pFunction);
		if (requiresParsingInformation()) pFinallyBlock->setParsingInformation(getFilenamePtr(), script);
		pFunction->setFinally(pFinallyBlock);
		parseBlock(script, *pFinallyBlock);
	}

	void DtaScript::parseReturn(GrfBlock& block, ScpStream& script) {
		if (_sCurrentFunctionBody.empty()) throw UtlException(script, "'return' must be called inside a function's body!");
		GrfReturn* pReturn = new GrfReturn(_sCurrentFunctionBody.c_str());
		if (requiresParsingInformation()) pReturn->setParsingInformation(getFilenamePtr(), script);
		block.add(pReturn);
		script.skipEmpty();
		if (!script.isEqualTo(';')) {
			pReturn->setExpression(parseExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(';')) throw UtlException(script, "syntax error, ';' expected");
		}
	}

	void DtaScript::parseExit(GrfBlock& block, ScpStream& script) {
		GrfExit* pExit = new GrfExit;
		if (requiresParsingInformation()) pExit->setParsingInformation(getFilenamePtr(), script);
		block.add(pExit);
		script.skipEmpty();
		pExit->setCode(parseExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error, ';' expected");
	}

	void DtaScript::parseDeclare(GrfBlock& block, ScpStream& script) {
		std::string sWord;
		script.skipEmpty();
		if (!script.readIdentifier(sWord)) throw UtlException(script, "syntax error, type of declaration expected");
		if (sWord == "function") {
			parseFunction(block, script, true);
		} else throw UtlException(script, std::string("unknown type of declaration: \"") + sWord + "\"");
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error, ';' expected");
	}

	void DtaScript::parseExternal(GrfBlock& block, ScpStream& script) {
		std::string sWord;
		script.skipEmpty();
		if (!script.readIdentifier(sWord)) throw UtlException(script, "syntax error, type of external expected");
		if (sWord == "function") {
			GrfFunction* pFunction = parseFunction(block, script, true);
			if (pFunction->isBodyDefined()) throw UtlException(script, "syntax error, function \"" + pFunction->getName() + "\" is expected external, but it has already a body");
			if (!pFunction->isExternal()) {
				pFunction->isExternal(true);
				Workspace::addExternalFunction(pFunction);
			}
			if (pFunction->getTemplateFunction() != NULL) {
				if (pFunction->getTemplateFunction()->isBodyDefined()) throw UtlException(script, "syntax error, function \"" + pFunction->getName() + "\" is expected external, but it has already a body");
				if (!pFunction->getTemplateFunction()->isExternal()) {
					pFunction->getTemplateFunction()->isExternal(true);
					Workspace::addExternalFunction(pFunction->getTemplateFunction());
				}
			}
		} else throw UtlException(script, std::string("unknown type of declaration: \"") + sWord + "\"");
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error, ';' expected");
	}

	void DtaScript::parseDelay(GrfBlock& block, ScpStream& script) {
		GrfDelay* pDelayBlock = new GrfDelay(&block);
		if (requiresParsingInformation()) pDelayBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pDelayBlock);
		parseBlock(script, *pDelayBlock);
	}

	void DtaScript::parseNewProject(GrfBlock& block, ScpStream& script) {
		GrfNewProject* pNewProjectBlock = new GrfNewProject(&block);
		if (requiresParsingInformation()) pNewProjectBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pNewProjectBlock);
		parseBlock(script, *pNewProjectBlock);
	}

	void DtaScript::parseDebug(GrfBlock& block, ScpStream& script) {
		GrfDebugExecution* pDebug = new GrfDebugExecution(&block);
		if (requiresParsingInformation()) pDebug->setParsingInformation(getFilenamePtr(), script);
		block.add(pDebug);
		parseBlock(script, *pDebug);
	}

	void DtaScript::parseQuantify(GrfBlock& block, ScpStream& script) {
		GrfQuantifyExecution* pQuantify = new GrfQuantifyExecution(&block);
		if (requiresParsingInformation()) pQuantify->setParsingInformation(getFilenamePtr(), script);
		block.add(pQuantify);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pQuantify->setFilename(parseExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		}
		parseBlock(script, *pQuantify);
	}

	void DtaScript::parseQuiet(GrfBlock& block, ScpStream& script) {
		GrfQuiet* pQuietBlock = new GrfQuiet(&block);
		if (requiresParsingInformation()) pQuietBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pQuietBlock);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pQuietBlock->setVariable(parseVariableExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		}
		parseBlock(script, *pQuietBlock);
	}

	void DtaScript::parseFileAsStandardInput(GrfBlock& block, ScpStream& script) {
		GrfFileAsStandardInput* pFileAsStandardInputBlock = new GrfFileAsStandardInput(&block);
		if (requiresParsingInformation()) pFileAsStandardInputBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pFileAsStandardInputBlock);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pFileAsStandardInputBlock->setFilename(parseExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		}
		parseBlock(script, *pFileAsStandardInputBlock);
	}

	void DtaScript::parseStringAsStandardInput(GrfBlock& block, ScpStream& script) {
		GrfStringAsStandardInput* pStringAsStandardInputBlock = new GrfStringAsStandardInput(&block);
		if (requiresParsingInformation()) pStringAsStandardInputBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pStringAsStandardInputBlock);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pStringAsStandardInputBlock->setText(parseExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		}
		parseBlock(script, *pStringAsStandardInputBlock);
	}

	void DtaScript::parseParsedFile(GrfBlock& block, ScpStream& script) {
		if (!isAParseScript()) throw UtlException(script, "statement modifier 'parsed_file' is available on parsing scripts only");
		GrfParsedFile* pParsedFileBlock = new GrfParsedFile(&block);
		if (requiresParsingInformation()) pParsedFileBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pParsedFileBlock);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pParsedFileBlock->setInputFile(parseExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		}
		parseBlock(script, *pParsedFileBlock);
	}

	void DtaScript::parseParsedString(GrfBlock& block, ScpStream& script) {
		if (!isAParseScript()) throw UtlException(script, "statement modifier 'parsed_string' is available on parsing scripts only");
		GrfParsedString* pParsedStringBlock = new GrfParsedString(&block);
		if (requiresParsingInformation()) pParsedStringBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pParsedStringBlock);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pParsedStringBlock->setInputString(parseExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		}
		parseBlock(script, *pParsedStringBlock);
	}

	void DtaScript::parseGeneratedFile(GrfBlock& /*block*/, ScpStream& script) {
		throw UtlException(script, "statement modifier \"generated_file\" must be used into pattern scripts only");
	}

	void DtaScript::parseAppendedFile(GrfBlock& /*block*/, ScpStream& script) {
		throw UtlException(script, "statement modifier \"appended_file\" must be used into pattern scripts only");
	}

	void DtaScript::parseGeneratedString(GrfBlock& /*block*/, ScpStream& script) {
		throw UtlException(script, "statement modifier \"generated_string\" must be used into pattern scripts only");
	}

	EXPRESSION_TYPE DtaScript::parseVariableType(GrfBlock& block, ScpStream& script) {
		EXPRESSION_TYPE exprType = VALUE_EXPRTYPE;
		script.skipEmpty();
		std::string sType;
		if (!script.readIdentifier(sType)) {
			throw UtlException(script, "syntax error, variable/parameter type expected (value, node, reference and index eventually)");
		}
		if (sType == "value") exprType = VALUE_EXPRTYPE;
		else if (sType == "iterator") exprType = ITERATOR_EXPRTYPE;
		else if (sType == "index") {
			std::string sErrorMessage = script.getMessagePrefix() + "warning! parameter type 'index' is obsolete since version 3.11 -> replace it by 'iterator'";
			if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
			exprType = ITERATOR_EXPRTYPE;
		} else if (sType == "node") exprType = NODE_EXPRTYPE;
		else if (sType == "reference") exprType = REFERENCE_EXPRTYPE;
		else if (sType == "variable") {
			std::string sErrorMessage = script.getMessagePrefix() + "warning! parameter type 'variable' is obsolete since version 3.8.7 -> replace it by 'node'";
			if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
			exprType = NODE_EXPRTYPE;
		} else throw UtlException(script, std::string("Unknown parameter type \"") + sType + "\"");
		return parseClassType(block, script, exprType);
	}

	EXPRESSION_TYPE DtaScript::parseClassType(GrfBlock& block, ScpStream& script, EXPRESSION_TYPE exprType) {
		script.skipEmpty();
		if (script.isEqualTo('<')) {
			if (exprType == VALUE_EXPRTYPE) throw UtlException(script, "only a node or a reference parameter can point to a class");
			script.skipEmpty();
			if (hasTargetLanguage()) {
				DtaScriptVariable parseTree(NULL, "parseTree");
				parseTree.insertNode("type")->setValue("target_type");
				getAlienParser().generate(script, parseTree);
				DtaScriptVariable* pNode = parseTree.getNode("target_type");
				if (pNode == NULL) throw UtlException(script, "instruction type 'target_type' is expected to have the attribute 'target_type'");
				DtaTargetLanguageTypeSpecifier* pTypeSpec = block.addTargetLanguageTypeSpecifier(pNode->getValue());
				exprType = pTypeSpec->composeExprType(exprType);
			} else {
				std::string sClassName;
				if (!script.readIdentifier(sClassName)) throw UtlException(script, "syntax error, class name expected");
				DtaClass* pClass = block.getClass(sClassName);
				if (pClass == NULL) throw UtlException(script, "undefined class '" + sClassName + "'");
				exprType = pClass->composeExprType(exprType);
			}
			script.skipEmpty();
			if (!script.isEqualTo('>')) throw UtlException(script, "syntax error, '>' expected");
		}
		return exprType;
	}

	// declaration / definition of a user function function.
	//   - bPrototypeOnly = true  -> declaration
	//   - bPrototypeOnly = false -> definition
	GrfFunction* DtaScript::parseFunction(GrfBlock& block, ScpStream& script, bool bPrototypeOnly) {
		std::string sFunctionName;
		script.skipEmpty();
		if (!script.readIdentifier(sFunctionName)) throw UtlException(script, "syntax error, function name expected");
		if (getParsingFunctionRegister().find(sFunctionName) != getParsingFunctionRegister().end()) {
			throw UtlException(script, "cannot use the reserved keyword '" + sFunctionName + "' as a function name");
		}
		script.skipEmpty();
		std::string sTemplate;
		bool bGenericKey = false;
		if (script.isEqualTo('<')) {
			script.skipEmpty();
			if (!script.readString(sTemplate)) {
				if (script.readIdentifier(sTemplate)) bGenericKey = true;
				else {
					double dValue;
					if (script.readDouble(dValue)) {
						sTemplate = CGRuntime::toString(dValue);
					} else throw UtlException(script, "syntax error, template function \"" + sFunctionName + "\" expects a constant string or an identifier for instantiating");
				}
			}
			script.skipEmpty();
			if (!script.isEqualTo('>')) throw UtlException(script, "syntax error, template function \"" + sFunctionName + "\" expects '>' after constant string \"" + sTemplate + "\"");
		}
		int iArity;
		GrfFunction* pFunction = NULL;
		bool bAlreadyExist = false;
		ExprScriptFunction* pFuncExpr = ExprScriptFunction::create(block, script, sFunctionName, sTemplate, bGenericKey);
		if (pFuncExpr != NULL) {
			// the function already exists
			pFunction = pFuncExpr->getUserBody();
			delete pFuncExpr;
			if (pFunction == NULL) {
				// this function isn't user-declared, so it's a reserved one:
				// don't have the right to redefine a reserved function
				throw UtlException(script, "syntax error, function \"" + sFunctionName + "\" is reserved");
			}
			iArity = pFunction->getArity();
		}
		GrfFunction* pTemplateFunction = NULL;
		if (pFunction == NULL) {
			// this function was never declared before
			if (sTemplate.empty()) {
				// a standard function: we'll discover its arity now!
				iArity = -1;
			} else {
				// a generic/instantiated function: look for the root template function
				pTemplateFunction = block.getFunction(sFunctionName, "", false);
				if (pTemplateFunction == NULL) {
					// it doesn't exist: no derivative template form of this function
					// already declared, so we'll discover its arity now
					iArity = -1;
					pTemplateFunction = block.addFunction(sFunctionName, "", false);
				} else {
					// we know its arity
					iArity = pTemplateFunction->getArity();
				}
			}
			// declare this new function
			pFunction = block.addFunction(sFunctionName, sTemplate, bGenericKey);
			if (requiresParsingInformation()) pFunction->setParsingInformation(getFilenamePtr(), script);
		} else {
			// this function was already declared: we know the arity
			bAlreadyExist = true;
			iArity = pFunction->getArity();
		}
		std::string sOldFunctionBody = _sCurrentFunctionBody;
		std::string sOldFunctionTemplateBody = _sCurrentFunctionTemplateBody;
		bool bOldCurrentFunctionGenericTemplateKey = _bCurrentFunctionGenericTemplateKey;
		_sCurrentFunctionBody = sFunctionName;
		_sCurrentFunctionTemplateBody = sTemplate;
		_bCurrentFunctionGenericTemplateKey = bGenericKey;
		if (bAlreadyExist && (!bPrototypeOnly) && pFunction->isBodyDefined()) throw UtlException(script, "syntax error, function \"" + sFunctionName + "\" has already been defined");
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error, '(' expected");
		script.skipEmpty();
		unsigned int i = 0;
		if (!script.isEqualTo(')')) {
			bool bHasDefaultValues = false;
			do {
				if (bAlreadyExist && (i >= pFunction->getArity())) throw UtlException(script, "too many parameters");
				// parameter name
				std::string sWord;
				script.skipEmpty();
				if (!script.readIdentifier(sWord)) throw UtlException(script, "syntax error, parameter name expected");
				// parameter type
				script.skipEmpty();
				EXPRESSION_TYPE exprType = VALUE_EXPRTYPE;
				if (script.isEqualTo(':')) {
					exprType = parseVariableType(block, script);
					script.skipEmpty();
				}
				// default value
				script.skipEmpty();
				ExprScriptExpression* pDefaultValue = NULL;
				if (script.isEqualTo('=')) {
					script.skipEmpty();
					if ((exprType & 0xFF) == VALUE_EXPRTYPE) {
						pDefaultValue = parseConstantLiteral(block, script);
					} else if ((exprType & 0xFF) == NODE_EXPRTYPE) {
						std::string sVarName;
						if (!script.readIdentifier(sVarName) || ((sVarName != "this") && (sVarName != "project") && (sVarName != "null") && (DtaProject::getInstance().getGlobalVariableType(sVarName) == UNKNOWN_EXPRTYPE))) throw UtlException(script, "global variable name or 'null' expected");
						pDefaultValue = new ExprScriptVariable(sVarName.c_str());
					}
					bHasDefaultValues = true;
					script.skipEmpty();
				} else if (bHasDefaultValues) {
					throw UtlException(script, "default value expected for argument '" + sWord + "'");
				}
				if (bAlreadyExist) {
					// we already know the prototype of the function: check only
					if (pFunction->getParameterType(i) != exprType) throw UtlException(script, "wrong parameter type");
				} else {
					if (!pFunction->addParameterAndType(sWord.c_str(), exprType, pDefaultValue)) throw UtlException(script, "syntax error, two parameters with the same name");
					if (pTemplateFunction != NULL) {
						if (iArity < 0) {
							if (pDefaultValue == NULL) {
								pTemplateFunction->addParameterAndType(sWord.c_str(), exprType, NULL);
							} else {
								pTemplateFunction->addParameterAndType(sWord.c_str(), exprType, pDefaultValue->clone());
							}
						} else if ((unsigned int) iArity <= i) throw UtlException(script, "too many parameters");
						else if (pTemplateFunction->getParameterType(i) != exprType) throw UtlException(script, "wrong parameter type");
					}
				}
				i++;
			} while (script.isEqualTo(','));
    		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error, ')' expected");
		}
		if (iArity >= 0) {
			// prototype was already given
			if (i < (unsigned int) iArity) throw UtlException(script, "parameters are lacking");
			if (i > (unsigned int) iArity) throw UtlException(script, "too many parameters");
		}
		if (!bPrototypeOnly) {
			// definition of the function
			if (pFunction->isExternal()) throw UtlException(script, "syntax error, function \"" + sFunctionName + "\" is external, so it shouldn't implement a body here");
			script.skipEmpty();
			pFunction->addLocalVariable(sFunctionName, VALUE_EXPRTYPE);
			if (bGenericKey) pFunction->addLocalVariable(sTemplate, VALUE_EXPRTYPE);
			if (bGenericKey && script.isEqualTo("{{")) {
				script.goBack();
				std::auto_ptr<DtaPatternScript> pPatternScript(new DtaPatternScript(&block));
				pPatternScript->parseEmbeddedScript(script);
    			if (!script.isEqualTo('}')) throw UtlException(script, "syntax error, '}}' expected to close the pattern-template function");
				pFunction->setPatternTemplateBody(getType(), pPatternScript.release());
			} else {
				pFunction->isBodyDefined(true);
				parseBlock(script, *pFunction);
			}
		}
		_sCurrentFunctionBody = sOldFunctionBody;
		_sCurrentFunctionTemplateBody = sOldFunctionTemplateBody;
		_bCurrentFunctionGenericTemplateKey = bOldCurrentFunctionGenericTemplateKey;
		return pFunction;
	}

	DtaClass* DtaScript::parseClass(GrfBlock& block, ScpStream& script) {
		std::string sClassName;
		script.skipEmpty();
		if (!script.readIdentifier(sClassName)) throw UtlException(script, "syntax error, class name expected");
		DtaClass* pClass = block.getClass(sClassName);
		if (pClass != NULL) {
			if (!pClass->isForwardDeclaration()) throw UtlException(script, "the class '" + sClassName + "' cannot be declared twice");
		} else {
			pClass = block.addClass(sClassName);
		}
		script.skipEmpty();
		if (script.isEqualTo(':')) {
			script.skipEmpty();
			std::string sExtendedClass;
			if (!script.readIdentifier(sExtendedClass)) throw UtlException(script, "syntax error, extended class expected after ':'");
			DtaClass* pExtendedClass = block.getClass(sExtendedClass);
			if (pExtendedClass == NULL) throw UtlException(script, "the extended class '" + sExtendedClass + "' doesn't exist");
			pClass->setExtendedClass(pExtendedClass);
			script.skipEmpty();
		}
		if (script.isEqualTo(';')) {
			// forward declaration
			if (pClass->getExtendedClass() != NULL) throw UtlException(script, "a forward declaration of a class doesn't admit the specification of the extended class");
		} else {
			// a body
			if (!script.isEqualTo('{')) throw UtlException(script, "syntax error, '{' expected");
			script.skipEmpty();
			while (!script.isEqualTo('}')) {
				std::string sAttribute;
				if (!script.readIdentifier(sAttribute)) {
					if (script.isEqualTo('[') && script.skipEmpty() && script.isEqualTo(']')) {
						sAttribute = "[]";
					} else {
						throw UtlException(script, "syntax error, attribute name or '}' expected");
					}
				}
				if (pClass->getAttributeType(sAttribute) != UNKNOWN_EXPRTYPE) throw UtlException(script, "the attribute '" + sClassName + "::" + sAttribute + "' is declared twice");
				script.skipEmpty();
				if (!script.isEqualTo(':')) throw UtlException(script, "syntax error, ':' expected");
				script.skipEmpty();
				EXPRESSION_TYPE attributeType = parseVariableType(block, script);
				DtaClass* pClassAttribute = DtaClass::getClass(attributeType);
				script.skipEmpty();
				if (script.isEqualTo('[') && script.skipEmpty() && script.isEqualTo(']')) {
					pClass->addAttribute(sAttribute, (EXPRESSION_TYPE) (((int) attributeType) | ((int) ARRAY_EXPRTYPE)), pClassAttribute);
					script.skipEmpty();
				} else {
					pClass->addAttribute(sAttribute, attributeType, pClassAttribute);
				}
				if (!script.isEqualTo(';')) throw UtlException(script, "syntax error, ';' expected");
				script.skipEmpty();
			}
			script.skipEmpty();
			script.isEqualTo(';');
		}
		return pClass;
	}

	ExprScriptExpression* DtaScript::parsePreprocessorExpression(GrfBlock& block, ScpStream& script) {
		std::string sWord;
		int iLocation = script.getInputLocation();
		script.skipEmpty();
		if (!script.readIdentifier(sWord)) throw UtlException(script, "syntax error: preprocessor-like variable expected");
		if (sWord == "LINE") {
			return new ExprScriptConstant(script.getLineCount());
		} else if (sWord == "FILE") {
			if (getFilenamePtr() == NULL) return new ExprScriptConstant("");
			return new ExprScriptConstant(getFilenamePtr());
		} else if (sWord == "evaluateVariable") {
			script.setInputLocation(iLocation - 1);
			return parseVariableExpression(block, script);
		} else {
			throw UtlException(script, "syntax error: preprocessor-like variable expected");
		}
		return NULL;
	}

	ExprScriptExpression* DtaScript::parseConstantLiteral(GrfBlock& block, ScpStream& script) {
		std::string sWord;
		if (script.readPythonString(sWord)) {
			std::string sConstantString = sWord;
			script.skipEmpty();
			while (script.readPythonString(sWord)) {
				sConstantString += sWord;
				script.skipEmpty();
			}
			return new ExprScriptConstant(sConstantString.c_str());
		}
		int iChar;
		if (script.readCharLiteral(iChar)) {
			char tcText[] = {0, 0};
			tcText[0] = (char) iChar;
			return new ExprScriptConstant(tcText);
		}
		if (script.readIdentifier(sWord)) {
			if (sWord == "true") return new ExprScriptConstant("true");
			if (sWord == "false") return new ExprScriptConstant("");
			if ((sWord == "this") || (sWord == "project") || (sWord == "null") || (DtaProject::getInstance().getGlobalVariableType(sWord) != UNKNOWN_EXPRTYPE)) {
				return new ExprScriptVariable(sWord.c_str());
			}
			throw UtlException(script, "'" + sWord + "' isn't recognized as a constant literal");
		}
		double dValue;
		if (script.readDouble(dValue)) {
			return new ExprScriptConstant(dValue);
		}
		throw UtlException(script, "constant literal expected");
	}

	ExprScriptExpression* DtaScript::parseLiteralUnsignedExpression(GrfBlock& block, ScpStream& script) {
		std::string sWord;
		ExprScriptExpression* pExpr = NULL;
		if (script.readPythonString(sWord)) {
			std::string sConstantString = sWord;
			script.skipEmpty();
			while (script.readPythonString(sWord)) {
				sConstantString += sWord;
				script.skipEmpty();
			}
			pExpr = new ExprScriptConstant(sConstantString.c_str());
		} else if (script.isEqualTo('(')) {
			pExpr = parseExpression(block, script);
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error, ')' expected");
		} else if (script.isEqualTo('$')) {
			if (_bNumericalExpression) throw UtlException(script, "syntax error, misplaced '$' character");
			_bNumericalExpression = true;
			try {
				pExpr = parseExpression(block, script);
			} catch(UtlException&/* exception*/) {
				_bNumericalExpression = false;
				throw/* UtlException(exception)*/;
			}
			_bNumericalExpression = false;
			script.skipEmpty();
			if (!script.isEqualTo('$')) throw UtlException(script, "syntax error, '$' expected to close a numerical expression");
		} else if (script.isEqualTo('#')) {
			pExpr = parsePreprocessorExpression(block, script);
		} else if (script.isEqualTo('~')) {
			if (!_bNumericalExpression) throw UtlException(script, "syntax error, bitwise not is an arithmetic operator to put between '$'");
			pExpr = parseLiteralExpression(block, script);
			pExpr = new ExprScriptBitwiseNot(pExpr);
		} else {
			int iChar;
			if (script.readCharLiteral(iChar)) {
				char tcText[] = {0, 0};
				tcText[0] = (char) iChar;
				pExpr = new ExprScriptConstant(tcText);
			} else {
				bool bWithoutBrackets = false;
				if (script.isEqualTo('!')) {
					bWithoutBrackets = true;
					sWord = "not";
				} else if (script.readIdentifier(sWord)) {
					if (sWord == "true") return new ExprScriptConstant("true");
					if (sWord == "false") return new ExprScriptConstant("");
				} else {
					double dValue;
					if (script.readDouble(dValue))
						return new ExprScriptConstant(dValue);
					else {
						std::string sMessage = "syntax error: literal expected (variable, function, string or number), instead of ";
						int iChar = script.readChar();
						if (iChar < 0) {
							sMessage += "end of file";
						} else {
							char tcChar[] = {'\'', '\0', '\'', '\0'};
							tcChar[1] = (char) iChar;
							sMessage += tcChar;
						}
						throw UtlException(script, sMessage);
					}
				}
				script.skipEmpty();
				ExprScriptExpression* pTemplate = NULL;
				int iLocation = script.getInputLocation();
				if (!bWithoutBrackets && script.isEqualTo('<')) {
					int a = script.peekChar();
					if ((a == (int) '=') || (a == (int) '<')) {
						script.goBack();
					} else {
						script.skipEmpty();
						pTemplate = parseKeyTemplateExpression(block, script);
						script.skipEmpty();
						if (script.isEqualTo('>')) {
							script.skipEmpty();
							if (!script.isEqualTo('(')) {
								delete pTemplate;
								throw UtlException(script, "syntax error: '(' expected after declaring template function call '" + sWord + "<" + pTemplate->toString() + ">'");
							}
							script.goBack();
						} else {
							delete pTemplate;
							pTemplate = NULL;
							script.setInputLocation(iLocation);
						}
					}
				}
				if (bWithoutBrackets || script.isEqualTo('(')) {
					ExprScriptFunction* pFunction;
					try {
						pFunction = ExprScriptFunction::create(block, script, sWord, "", false);
					} catch(UtlException& exception) {
						delete pTemplate;
						throw UtlException(script, exception.getTraceStack(), exception.getMessage());
					} catch(std::exception&) {
						delete pTemplate;
						throw;
					}
					if (pFunction == NULL) throw UtlException(script, "Unknown function name \"" + sWord + "\"");
					checkIfAllowedFunction(script, *pFunction);
					pExpr = parseFunctionExpression(block, script, NULL, pFunction, pTemplate, !bWithoutBrackets);
				} else if (script.isEqualTo("::")) {
					DynPackage* pPackage = DynPackage::getPackage(sWord);
					if (pPackage == NULL) throw UtlException(script, "unknown module '" + sWord +"'; type '#use' to load the dynamic library");
					script.skipEmpty();
					std::string sFunction;
					if (!script.readIdentifier(sFunction)) throw UtlException(script, "function or variable name expected after '" + sWord + "::'");
					script.skipEmpty();
					if (script.isEqualTo('(')) {
						DynFunction* pFunction = pPackage->getFunction(sFunction);
						if (pFunction == NULL) throw UtlException(script, "the module '" + sWord + "' doesn't export the function '" + sWord + "::" + sFunction + "'");
						pExpr = parseFunctionExpression(block, script, NULL, new ExprScriptFunction(pFunction), NULL, true);
					} else {
						if (pPackage->getVariable(sFunction) == NULL) throw UtlException(script, "the module '" + sWord + "' doesn't export the variable '" + sWord + "::" + sFunction + "'");
						ExprScriptExpression* pMethodFunction;
						GrfCommand* pMethodProc;
						pExpr = parseVariableExpression(block, script, new ExprScriptVariable(sFunction.c_str(), pPackage), pMethodFunction, pMethodProc);
						if (pMethodFunction != NULL) pExpr = pMethodFunction;
						else if (pMethodProc != NULL) throw UtlException("literal expected instead of procedure call as a method");
					}
				} else {
					ExprScriptExpression* pMethodFunction;
					GrfCommand* pMethodProc;
					pExpr = parseVariableExpression(block, script, new ExprScriptVariable(sWord.c_str()), pMethodFunction, pMethodProc);
					if (pMethodFunction != NULL) pExpr = pMethodFunction;
					else if (pMethodProc != NULL) {
						throw UtlException("literal expected instead of procedure call as a method");
					} else {
						// variable expression
						script.skipEmpty();
						if (script.isEqualTo("++")) {
							std::string sTemplate;
							ExprScriptFunction* pFunction = ExprScriptFunction::create(block, script, "increment", sTemplate, false);
							pFunction->addParameter(pExpr);
							pExpr = pFunction;
						} else if (script.isEqualTo("--")) {
							std::string sTemplate;
							ExprScriptFunction* pFunction = ExprScriptFunction::create(block, script, "decrement", sTemplate, false);
							pFunction->addParameter(pExpr);
							pExpr = pFunction;
/*						} else {
							std::string sOperator;
							if (script.isEqualTo("+=")) sOperator = "+=";
							else if (script.isEqualTo("-=")) sOperator = "-=";
							else if (script.isEqualTo("*=")) sOperator = "*=";
							else if (script.isEqualTo("/=")) sOperator = "/=";
*/						}
					}
				}
			}
		}
		return pExpr;
	}

	ExprScriptExpression* DtaScript::parseLiteralExpression(GrfBlock& block, ScpStream& script) {
		bool bUnaryMinus = script.isEqualTo('-');
		if (bUnaryMinus) {
			int a = script.peekChar();
			if ((a >= (int) '0') && (a <= (int) '9')) {
				script.goBack();
				bUnaryMinus = false;
			}
		}
		ExprScriptExpression* pExpr = parseLiteralUnsignedExpression(block, script);
		if (bUnaryMinus) pExpr = new ExprScriptUnaryMinus(pExpr);
		return pExpr;
	}

	ExprScriptExpression* DtaScript::parseConcatenationExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptExpression* pResultExpr = NULL;
		for (;;) {
    		ExprScriptExpression* pExpr = NULL;
			script.skipEmpty();
			try {
				pExpr = parseLiteralExpression(block, script);
			} catch(std::exception&/* e*/) {
				delete pResultExpr;
				throw/* UtlException(e)*/;
			}
			if (pResultExpr == NULL) pResultExpr = pExpr;
			else {
				ExprScriptConstant* pLeftConstant = dynamic_cast<ExprScriptConstant*>(pResultExpr);
				if (pLeftConstant != NULL) {
					ExprScriptConstant* pRightConstant = dynamic_cast<ExprScriptConstant*>(pExpr);
					if (pRightConstant != NULL) {
						std::string sConcatenation = pLeftConstant->getConstant() + pRightConstant->getConstant();
						pResultExpr = new ExprScriptConstant(sConcatenation.c_str());
						delete pLeftConstant;
						delete pRightConstant;
					} else {
						pResultExpr = new ExprScriptConcatenation(pResultExpr, pExpr);
					}
				} else {
					pResultExpr = new ExprScriptConcatenation(pResultExpr, pExpr);
				}
			}
			script.skipEmpty();
			if (script.isEqualTo('+')) {
				if (script.isEqualTo("=")) {
					script.goBack();
					script.goBack();
					break;
				}
			} else {
				break;
			}
		}
		return pResultExpr;
	}

	ExprScriptExpression* DtaScript::parseKeyTemplateExpression(GrfBlock& block, ScpStream& script) {
		if (!_bNumericalExpression) return parseConcatenationExpression(block, script);
		ExprScriptExpression* pExpr;
		try {
			_bNumericalExpression = false;
			pExpr = parseConcatenationExpression(block, script);
			_bNumericalExpression = true;
		} catch(UtlException&) {
			_bNumericalExpression = true;
			throw;
		}
		return pExpr;
	}

	ExprScriptExpression* DtaScript::parseFactorExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptExpression* pResultExpr = NULL;
		char cComparison = '\0';
		do {
    		ExprScriptExpression* pExpr = NULL;
			script.skipEmpty();
			try {
				pExpr = parseLiteralExpression(block, script);
			} catch(std::exception&/* e*/) {
				delete pResultExpr;
				throw/* UtlException(e)*/;
			}
			if (pResultExpr == NULL) pResultExpr = pExpr;
			else pResultExpr = new ExprScriptArithmetic(pResultExpr, pExpr, cComparison);
			script.skipEmpty();
			if (script.isEqualTo('*')) cComparison = '*';
			else if (script.isEqualTo('/')) cComparison = '/';
			else if (script.isEqualTo('%')) cComparison = '%';
			else break;
		} while (true);
		return pResultExpr;
	}

	ExprScriptExpression* DtaScript::parseShiftExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptExpression* pResultExpr = NULL;
		char cComparison = '\0';
		do {
    		ExprScriptExpression* pExpr = NULL;
			script.skipEmpty();
			try {
				pExpr = parseFactorExpression(block, script);
			} catch(std::exception&/* e*/) {
				delete pResultExpr;
				throw/* UtlException(e)*/;
			}
			if (pResultExpr == NULL) pResultExpr = pExpr;
			else pResultExpr = new ExprScriptShift(pResultExpr, pExpr, cComparison);
			script.skipEmpty();
			if (script.isEqualTo("<<")) cComparison = '<';
			else if (script.isEqualTo(">>")) cComparison = '>';
			else break;
		} while (true);
		return pResultExpr;
	}

	ExprScriptExpression* DtaScript::parseArithmeticExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptExpression* pResultExpr = NULL;
		char cComparison = '\0';
		do {
    		ExprScriptExpression* pExpr = NULL;
			script.skipEmpty();
			try {
				pExpr = parseShiftExpression(block, script);
			} catch(std::exception&/* e*/) {
				delete pResultExpr;
				throw/* UtlException(e)*/;
			}
			if (pResultExpr == NULL) pResultExpr = pExpr;
			else pResultExpr = new ExprScriptArithmetic(pResultExpr, pExpr, cComparison);
			script.skipEmpty();
			if (script.isEqualTo('+')) cComparison = '+';
			else if (script.isEqualTo('-')) cComparison = '-';
			else break;
		} while (true);
		return pResultExpr;
	}

	ExprScriptExpression* DtaScript::parseComparisonExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptExpression* pResultExpr = NULL;
		char cComparison = '\0';
		do {
    		ExprScriptExpression* pExpr = NULL;
			script.skipEmpty();
			try {
				if (_bNumericalExpression) pExpr = parseArithmeticExpression(block, script);
				else pExpr = parseConcatenationExpression(block, script);
			} catch(std::exception&/* e*/) {
				delete pResultExpr;
				throw/* UtlException(e)*/;
			}
			if (pResultExpr == NULL) pResultExpr = pExpr;
			else if (_bNumericalExpression) pResultExpr = new ExprScriptNumericComparison(pResultExpr, pExpr, cComparison);
			else pResultExpr = new ExprScriptComparison(pResultExpr, pExpr, cComparison);
			script.skipEmpty();
			if (script.isEqualTo('<')) {
				if (script.isEqualTo('=')) cComparison = 'i';
				else if (script.isEqualTo('>')) cComparison = '!';
				else cComparison = '<';
			} else if (script.isEqualTo('=')) {
				script.isEqualTo('=');
				cComparison = '=';
			} else if (script.isEqualTo('>')) {
				if (script.isEqualTo('=')) cComparison = 's';
				else cComparison = '>';
			} else if (script.isEqualTo('!')) {
				if (script.isEqualTo('=')) cComparison = '!';
				else {
					delete pResultExpr;
					throw UtlException(script, "binary operator '!=' expected, or syntax error on unary operator '!'");
				}
			} else if (script.isEqualToIdentifier("in")) {
				script.skipEmpty();
				if (!script.isEqualTo('{')) throw UtlException(script, "'{' expected");
				script.skipEmpty();
				std::set<std::string> listOfConstants;
				std::string sConstant;
				if (!script.readStringOrCharLiteral(sConstant)) throw UtlException(script, "syntax error, constant string expected");
				listOfConstants.insert(sConstant);
				script.skipEmpty();
				while (script.isEqualTo(',')) {
					script.skipEmpty();
					if (!script.readStringOrCharLiteral(sConstant)) throw UtlException(script, "syntax error, constant string expected");
					listOfConstants.insert(sConstant);
					script.skipEmpty();
				}
				if (!script.isEqualTo('}')) throw UtlException(script, "'{' expected");
				pResultExpr = new ExprScriptInSet(pResultExpr, listOfConstants);
				break;
			} else break;
		} while (true);
		return pResultExpr;
	}

	ExprScriptExpression* DtaScript::parseTernaryExpression(GrfBlock& block, ScpStream& script) {
   		ExprScriptExpression* pResultExpr = parseComparisonExpression(block, script);
		script.skipEmpty();
		if (script.isEqualTo('?')) {
			std::auto_ptr<ExprScriptExpression> noLeak(pResultExpr);
	   		ExprScriptExpression* pTrueExpr = parseExpression(block, script);
			script.skipEmpty();
			if (!script.isEqualTo(':')) throw UtlException(script, "':' expected in the ternary operator");
	   		ExprScriptExpression* pFalseExpr = parseExpression(block, script);
			pResultExpr = new ExprScriptTernaryOperator(pResultExpr, pTrueExpr, pFalseExpr);
			noLeak.release();
		}
		return pResultExpr;
	}

	ExprScriptExpression* DtaScript::parseBooleanExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptExpression* pResultExpr = NULL;
		char cOperator = '\0';
		do {
    		ExprScriptExpression* pExpr = NULL;
			script.skipEmpty();
			try {
				pExpr = parseTernaryExpression(block, script);
			} catch(std::exception&/* e*/) {
				delete pResultExpr;
				throw/* UtlException(e)*/;
			}
			if (pResultExpr == NULL) pResultExpr = pExpr;
			else pResultExpr = new ExprScriptBoolean(pResultExpr, pExpr, cOperator);
			script.skipEmpty();
			if (script.isEqualTo('&')) cOperator = '&';
			else if (script.isEqualTo('|')) cOperator = '|';
			else if (script.isEqualTo('^')) cOperator = '^';
			else break;
			script.isEqualTo(cOperator);
		} while (true);
		return pResultExpr;
	}

	ExprScriptExpression* DtaScript::parseExpression(GrfBlock& block, ScpStream& script) {
		if (hasTargetLanguage()) {
			// expression embedded in a script written in a target language
			if (script.isEqualTo('[')) {
				// CodeWorker expression
				std::auto_ptr<ExprScriptExpression> pExpr(parseBooleanExpression(block, script));
				if (!script.isEqualTo(']')) throw UtlException(script, "syntax error, ']' expected");
				return pExpr.release();
			}
			// alien expression
			return parseAlienExpression(block, script);
		}
		return parseBooleanExpression(block, script);
	}

	ExprScriptScriptFile* DtaScript::parseScriptFileExpression(GrfBlock& block, ScpStream& script, DtaScriptFactory::SCRIPT_TYPE eScriptType) {
		ExprScriptScriptFile* pExpr;
		script.skipEmpty();
		if (script.isEqualTo('{')) {
			script.goBack();
			std::auto_ptr<DtaScript> pScript(DtaScriptFactory::create(eScriptType, block));
			pScript->parseEmbeddedScript(script);
			pExpr = new ExprScriptScriptFile(pScript.release());
			script.skipEmpty();
		} else {
			ExprScriptExpression* pFileExpr = parseExpression(block, script);
			pExpr = new ExprScriptScriptFile(pFileExpr);
		}
		return pExpr;
	}

	ExprScriptVariable* DtaScript::parseIndexExpression(GrfBlock& /*block*/, ScpStream& script) {
		std::string sWord;
		script.skipEmpty();
		if (!script.readIdentifier(sWord)) throw UtlException(script, "syntax error: index name expected");
		ExprScriptVariable* pIndexExpr = new ExprScriptVariable(sWord.c_str());
		return pIndexExpr;
	}

	ExprScriptVariable* DtaScript::parseReferenceExpression(GrfBlock& /*block*/, ScpStream& script) {
		std::string sWord;
		script.skipEmpty();
		if (!script.readIdentifier(sWord)) throw UtlException(script, "syntax error: reference name expected");
		script.skipEmpty();
		if (script.isEqualTo('.') || script.isEqualTo('#') || script.isEqualTo('[') || (script.isEqualTo('$') && script.peekChar() == (int) '[')) throw UtlException(script, "syntax error: reference name expected (not a variable expression)");
		ExprScriptVariable* pReferenceExpr = new ExprScriptVariable(sWord.c_str());
		return pReferenceExpr;
	}

	ExprScriptVariable* DtaScript::parseVariableExpression(GrfBlock& block, ScpStream& script, ExprScriptVariable* pParentExpr) {
		ExprScriptExpression* pMethodExpr;
		GrfCommand* pMethodProc;
		ExprScriptVariable* pVariableExpr = parseVariableExpression(block, script, pParentExpr, pMethodExpr, pMethodProc);
		if ((pMethodExpr != NULL) || (pMethodProc != NULL)) {
			delete pVariableExpr;
			throw UtlException(script, "variable expected, instead of method call");
		}
		return pVariableExpr;
	}

	ExprScriptVariable* DtaScript::parseVariableExpression(GrfBlock& block, ScpStream& script, ExprScriptVariable* pParentExpr, ExprScriptExpression*& pMethodExpr, GrfCommand*& pMethodProc) {
		std::string sWord;
		pMethodExpr = NULL;
		pMethodProc = NULL;
		if (pParentExpr == NULL) {
			script.skipEmpty();
			bool bDirective = script.isEqualTo('#');
			if (!script.readIdentifier(sWord)) throw UtlException(script, "syntax error: variable expression expected");
			if (bDirective) {
				if (sWord == "evaluateVariable") {
					script.skipEmpty();
					if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
					pParentExpr = new ExprScriptVariable(sWord.c_str());
					pParentExpr->setEvaluation(parseExpression(block, script));
					script.skipEmpty();
					if (!script.isEqualTo(')')) {
						delete pParentExpr;
						throw UtlException(script, "syntax error: ')' expected");
					}
					script.skipEmpty();
					return pParentExpr;
				} else {
					throw UtlException(script, "syntax error: '#evaluateVariable' expected in a variable expression");
				}
			} else {
				if (script.isEqualTo("::")) {
					DynPackage* pPackage = DynPackage::getPackage(sWord);
					if (pPackage == NULL) throw UtlException(script, "unknown module '" + sWord +"'; type '#use' to load the dynamic library");
					script.skipEmpty();
					std::string sVariable;
					if (!script.readIdentifier(sVariable)) throw UtlException(script, "variable name expected after '" + sWord + "::'");
					pParentExpr = new ExprScriptVariable(sVariable.c_str(), pPackage);
				} else {
					pParentExpr = new ExprScriptVariable(sWord.c_str());
				}
			}
		} else {
			// just for the following message
			sWord = pParentExpr->getName();
		}
		if (!sWord.empty() && (&block != NULL) && (block.getVariable(sWord) == UNKNOWN_EXPRTYPE) && (pParentExpr->getPackage() == NULL)) {
			if (!DtaProject::getInstance().noWarning("undeclvar")) {
				std::string sErrorMessage = script.getMessagePrefix() + "warning! you haven't declared the variable '" + sWord + "' before ; interpreted as 'this." + sWord + "', but obsolete soon!";
				CGRuntime::traceLine(sErrorMessage);
			}
		}
		ExprScriptVariable* pExpr = pParentExpr;
		ExprScriptVariable* pPrecExpr = pParentExpr;
		bool bArrayIterator;
		bool bNext = true;
		do {
			bool bArray = false;
			bArrayIterator = script.isEqualTo('#');
			if (script.isEqualTo('[')) {
				bArray = true;
				try {
					if (bArrayIterator) pExpr->setArrayPosition(parseExpression(block, script));
					else pExpr->setArrayKey(parseExpression(block, script));
				} catch(std::exception&/* except*/) {
					delete pParentExpr;
					throw/* UtlException(except)*/;
				}
				if (!script.isEqualTo(']')) {
					delete pParentExpr;
					throw UtlException(script, "syntax error: ']' expected");
				}
			} else if (bArrayIterator) {
				if (!script.readIdentifier(sWord) || ((sWord != "front") && (sWord != "back") && (sWord != "parent") && (sWord != "root"))) throw UtlException(script, "'[' or 'front' or 'back' expected after '#' to precise an array index");
				if (sWord == "front") pExpr->setArrayPosition(new ExprScriptConstant("0"));
				else pExpr->setArrayPosition(new ExprScriptConstant(sWord.c_str()));
			}
			bNext = script.isEqualTo('.');
			if (bNext) {
				if (!script.readIdentifier(sWord)) {
					delete pParentExpr;
					throw UtlException(script, "syntax error: variable expected");
				}
				std::auto_ptr<ExprScriptExpression> pTemplate;
				int iLocation = script.getInputLocation();
				if (script.isEqualTo('<')) {
					std::auto_ptr<ExprScriptExpression> pTemplateExpression(parseKeyTemplateExpression(block, script));
					pTemplate = pTemplateExpression;
					script.skipEmpty();
					if (script.isEqualTo('>')) {
						script.skipEmpty();
						if (!script.isEqualTo('(')) throw UtlException(script, "'(' expected after instantiating a template method");
					} else {
						std::auto_ptr<ExprScriptExpression> pTemplateExpression;
						pTemplate = pTemplateExpression;
						script.setInputLocation(iLocation);
					}
				}
				if ((pTemplate.get() != NULL) || script.isEqualTo('(')) {
					ExprScriptFunction* pFunction = ExprScriptFunction::createMethod(block, script, sWord, "", false);
					if (pFunction != NULL) {
						checkIfAllowedFunction(script, *pFunction);
						pMethodExpr = parseFunctionExpression(block, script, pParentExpr, pFunction, pTemplate.release(), true);
					} else {
						script.goBack();
						if (!parseKeyword(sWord, script, block, pParentExpr)) throw UtlException(script, "unknown method \"" + sWord + "\"");
						pMethodProc = block.getCommands().back();
					}
					break;
				}
				pExpr = new ExprScriptVariable(pPrecExpr, sWord.c_str());
				pPrecExpr = pExpr;
			} else {
				int iChar = script.peekChar();
				bNext = (iChar == (int) '[') || (iChar == (int) '#');
				if (bNext && (bArray || bArrayIterator)) {
					pExpr = new ExprScriptVariable(pPrecExpr, "");
					pPrecExpr = pExpr;
				}
			}
		} while (bNext);
		return pParentExpr;
	}

	ExprScriptMotifStep* DtaScript::parseMotifStepExpression(GrfBlock& block, ScpStream& script, ExprScriptMotifPath*& pPathExpr) {
		std::auto_ptr<ExprScriptMotifStep> pParent;
		std::string sIdentifier;
		script.skipEmpty();
		if (script.isEqualTo('*')) sIdentifier = "*";
		else script.readIdentifier(sIdentifier);
		if (!sIdentifier.empty()) {
			std::auto_ptr<ExprScriptMotifStep> pMotifStep(new ExprScriptMotifStep(sIdentifier));
			pParent = pMotifStep;
			pPathExpr = pParent.get();
			script.skipEmpty();
			while (script.isEqualTo('[')) {
				script.skipEmpty();
				if (script.isEqualTo(']')) {
					pPathExpr = new ExprScriptMotifArray(pPathExpr);
				} else {
					pPathExpr = new ExprScriptMotifArray(pPathExpr, parseExpression(block, script));
					script.skipEmpty();
					if (!script.isEqualTo(']')) throw UtlException(script, "']' expected to close an array in a motif");
				}
				script.skipEmpty();
			}
		} else {
			throw UtlException(script, "location path expected");
		}
		return pParent.release();
	}

	ExprScriptMotifPath* DtaScript::parseMotifPathExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptMotifPath* pEndPath;
		std::auto_ptr<ExprScriptMotifPath> pParent(parseMotifStepExpression(block, script, pEndPath));
		script.skipEmpty();
		while (script.isEqualTo('.')) {
			ExprScriptMotifPath* pMotif = pEndPath;
			if (script.isEqualTo("..")) {
				new ExprScriptMotifEllipsis(pMotif, parseMotifStepExpression(block, script, pEndPath));
			} else {
				pMotif->setNextPath(parseMotifStepExpression(block, script, pEndPath));
			}
		}
		return pParent.release();
	}

	ExprScriptMotif* DtaScript::parseMotifConcatExpression(GrfBlock& block, ScpStream& script) {
		std::auto_ptr<ExprScriptMotif> pParent(parseMotifPathExpression(block, script));
		ExprScriptMotifBoolean* pBooleanMotif = NULL;
		script.skipEmpty();
		while (script.isEqualTo('+')) {
			if (pBooleanMotif == NULL) {
				pBooleanMotif = new ExprScriptMotifBoolean('+', pParent.release());
				std::auto_ptr<ExprScriptMotif> pRef(pBooleanMotif);
				pParent = pRef;
			}
			pBooleanMotif->addMember(parseMotifPathExpression(block, script));
			script.skipEmpty();
		}
		return pParent.release();
	}

	ExprScriptMotif* DtaScript::parseMotifAndExpression(GrfBlock& block, ScpStream& script) {
		std::auto_ptr<ExprScriptMotif> pParent(parseMotifConcatExpression(block, script));
		ExprScriptMotifBoolean* pBooleanMotif = NULL;
		script.skipEmpty();
		while (script.isEqualTo("&&") || script.isEqualTo('&')) {
			if (pBooleanMotif == NULL) {
				pBooleanMotif = new ExprScriptMotifBoolean('&', pParent.release());
				std::auto_ptr<ExprScriptMotif> pRef(pBooleanMotif);
				pParent = pRef;
			}
			pBooleanMotif->addMember(parseMotifConcatExpression(block, script));
			script.skipEmpty();
		}
		return pParent.release();
	}

	ExprScriptMotif* DtaScript::parseMotifExpression(GrfBlock& block, ScpStream& script) {
		ExprScriptMotif* pMotif;
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pMotif = parseMotifExpression(block, script);
			script.skipEmpty();
			if (!script.isEqualTo(')')) {
				delete pMotif;
				throw UtlException(script, "')' expected in a motif expression");
			}
			script.skipEmpty();
		} else {
			pMotif = parseMotifAndExpression(block, script);
		}
		std::auto_ptr<ExprScriptMotif> pParent(pMotif);
		ExprScriptMotifBoolean* pBooleanMotif = NULL;
		while (script.isEqualTo("||") || script.isEqualTo('|')) {
			if (pBooleanMotif == NULL) {
				pBooleanMotif = new ExprScriptMotifBoolean('|', pParent.release());
				std::auto_ptr<ExprScriptMotif> pRef(pBooleanMotif);
				pParent = pRef;
			}
			pBooleanMotif->addMember(parseMotifAndExpression(block, script));
			script.skipEmpty();
		}
		return pParent.release();
	}

	ExprScriptFunction* DtaScript::parseFunctionExpression(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller, ExprScriptFunction* pFunction, ExprScriptExpression* pTemplate, bool bBracket) {
		std::string sWord;
		unsigned int iIndex = 0;
		pFunction->setTemplate(pTemplate);
		if (pFunction->getArity() != 0) {
			if (bBracket) {
				script.skipEmpty();
				if ((pFunction->getMinArity() > 0) || (script.peekChar() != ')')) {
					// not in the case where the first parameter is optional and not provided
					do {
						try {
							if ((pMethodCaller != NULL) && (iIndex == pFunction->getThisPosition())) {
								EXPRESSION_TYPE parameterType = pFunction->getParameterType(pFunction->getThisPosition());
								pFunction->addParameter(pMethodCaller);
								iIndex++;
								script.skipEmpty();
								if (script.isEqualTo(')')) {
									script.goBack();
									break;
								}
							}
							bool bOldArithmeticState = _bNumericalExpression;
							_bNumericalExpression = false;
							switch(pFunction->getParameterType(iIndex) & 0x00FF) {
								case VALUE_EXPRTYPE: pFunction->addParameter(parseExpression(block, script));break;
								case REFERENCE_EXPRTYPE: pFunction->addParameter(parseVariableExpression(block, script));break;
								case NODE_EXPRTYPE: pFunction->addParameter(parseVariableExpression(block, script));break;
								case ITERATOR_EXPRTYPE: pFunction->addParameter(parseIndexExpression(block, script));break;
								case SCRIPTFILE_EXPRTYPE: pFunction->addParameter(parseScriptFileExpression(block, script, DtaScriptFactory::COMMON_SCRIPT));break;
								case SCRIPTFILE_PATTERN_EXPRTYPE: pFunction->addParameter(parseScriptFileExpression(block, script, DtaScriptFactory::PATTERN_SCRIPT));break;
								case SCRIPTFILE_FREE_EXPRTYPE: pFunction->addParameter(parseScriptFileExpression(block, script, DtaScriptFactory::FREE_SCRIPT));break;
								case SCRIPTFILE_BNF_EXPRTYPE: pFunction->addParameter(parseScriptFileExpression(block, script, DtaScriptFactory::BNF_SCRIPT));break;
								case SCRIPTFILE_TRANSLATE_EXPRTYPE: pFunction->addParameter(parseScriptFileExpression(block, script, DtaScriptFactory::TRANSLATE_SCRIPT));break;
							}
							_bNumericalExpression = bOldArithmeticState;
						} catch(UtlExitException&) {
							delete pFunction;
							throw;
						} catch(UtlException& e) {
							std::string sMessage = e.getMessage();
							char tcNumber[32];
							sprintf(tcNumber, "%d", iIndex + 1);
							sMessage += " while parsing parameter ";
							sMessage += tcNumber;
							sMessage += " of function '" + std::string(pFunction->getName()) + "'";
							delete pFunction;
							throw UtlException(e.getTraceStack(), sMessage);
						}
						script.skipEmpty();
						iIndex++;
					} while (script.isEqualTo(',') || ((pMethodCaller != NULL) && (iIndex == pFunction->getThisPosition())));
				}
			} else {
				pFunction->addParameter(parseLiteralExpression(block, script));
				iIndex++;
			}
		}
		if (iIndex < pFunction->getMinArity()) {
			std::string sMessageFormat = "Not enough parameters encountered";
			if (pFunction != NULL) sMessageFormat += " for call to function '" + std::string(pFunction->getName()) + "'";
			delete pFunction;
			throw UtlException(script, sMessageFormat);
		}
		if (iIndex > pFunction->getArity()) {
			std::string sMessageFormat = "too many parameters encountered";
			if (pFunction != NULL) sMessageFormat += " for call to function '" + std::string(pFunction->getName()) + "'";
			delete pFunction;
			throw UtlException(script, sMessageFormat);
		}
		if (iIndex < pFunction->getArity()) {
			// default parameters to populate
			do {
				ExprScriptExpression* pDefaultValue = pFunction->getDefaultParameter(iIndex);
				pFunction->addParameter(pDefaultValue->clone());
				++iIndex;
			} while (iIndex < pFunction->getArity());
		}
		if (bBracket && !script.isEqualTo(')')) {
			std::string sMessageFormat = "syntax error, ')' expected to close parameters";
			if (pFunction != NULL) sMessageFormat += " of '" + std::string(pFunction->getName()) + "'";
			int iChar = script.readChar();
			if (iChar < 0) sMessageFormat += ", instead of end of file";
			else {
				sMessageFormat += ", instead of '";
				sMessageFormat += (char) iChar;
				sMessageFormat += "'";
			}
			delete pFunction;
			throw UtlException(script, sMessageFormat);
		}
		pFunction->initializationDone();
		return pFunction;
	}

	void DtaScript::checkIfAllowedFunction(ScpStream& script, ExprScriptFunction& theFunction) {
		if (theFunction.isAGenerateFunction() && !isAGenerateScript()) throw UtlException(script, "function '" + std::string(theFunction.getName()) + "' reserved for template-based scripts only");
		if (theFunction.isAParseFunction() && !isAParseScript()) throw UtlException(script, "function '" + std::string(theFunction.getName()) + "' reserved for parse scripts only");
	}

	void DtaScript::parseReadonlyHook(GrfBlock& block, ScpStream& script) {
		if (DtaProject::getInstance().getReadonlyHook() != NULL) throw UtlException(script, "function 'readonlyHook(<filename>)' has already been defined and can't be implemented twice");
		GrfReadonlyHook* pReadonlyHook = new GrfReadonlyHook(&block);
		if (requiresParsingInformation()) pReadonlyHook->setParsingInformation(getFilenamePtr(), script);
		block.addFunction(pReadonlyHook);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
		script.skipEmpty();
		std::string sIdentifier;
		if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "syntax error: argument name expected");
		pReadonlyHook->setParameterName(sIdentifier.c_str());
		script.skipEmpty();
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		std::string _sOldFunctionBody = _sCurrentFunctionBody;
		std::string _sOldFunctionTemplateBody = _sCurrentFunctionTemplateBody;
		bool bOldCurrentFunctionGenericTemplateKey = _bCurrentFunctionGenericTemplateKey;
		_sCurrentFunctionBody = pReadonlyHook->getFunctionName();
		_sCurrentFunctionTemplateBody = "";
		_bCurrentFunctionGenericTemplateKey = false;
		pReadonlyHook->isBodyDefined(true);
		parseBlock(script, *pReadonlyHook);
		_sCurrentFunctionBody = _sOldFunctionBody;
		_sCurrentFunctionTemplateBody = _sOldFunctionTemplateBody;
		_bCurrentFunctionGenericTemplateKey = bOldCurrentFunctionGenericTemplateKey;
	}

	void DtaScript::parseWritefileHook(GrfBlock& block, ScpStream& script) {
		if (DtaProject::getInstance().getWritefileHook() != NULL) throw UtlException(script, "hook 'writefileHook(<filename>, <position>, <creation>)' has already been defined and can't be implemented twice");
		GrfWritefileHook* pWritefileHook = new GrfWritefileHook(&block);
		if (requiresParsingInformation()) pWritefileHook->setParsingInformation(getFilenamePtr(), script);
		block.addFunction(pWritefileHook);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
		script.skipEmpty();
		std::string sIdentifier;
		if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "syntax error: argument '<filename>' expected for hook 'writefileHook(<filename>, <position>, <creation>)'");
		pWritefileHook->setFileNameArgument(sIdentifier.c_str());
		script.skipEmpty();
		if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
		script.skipEmpty();
		if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "syntax error: argument '<position>' expected for hook 'writefileHook(<filename>, <position>, <creation>)'");
		pWritefileHook->setPositionArgument(sIdentifier.c_str());
		script.skipEmpty();
		if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
		script.skipEmpty();
		if (!script.readIdentifier(sIdentifier)) throw UtlException(script, "syntax error: argument '<creation>' expected for hook 'writefileHook(<filename>, <position>, <creation>)'");
		pWritefileHook->setCreationArgument(sIdentifier.c_str());
		script.skipEmpty();
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		std::string _sOldFunctionBody = _sCurrentFunctionBody;
		std::string _sOldFunctionTemplateBody = _sCurrentFunctionTemplateBody;
		bool bOldCurrentFunctionGenericTemplateKey = _bCurrentFunctionGenericTemplateKey;
		_sCurrentFunctionBody = pWritefileHook->getFunctionName();
		_sCurrentFunctionTemplateBody = "";
		_bCurrentFunctionGenericTemplateKey = false;
		pWritefileHook->isBodyDefined(true);
		parseBlock(script, *pWritefileHook);
		_sCurrentFunctionBody = _sOldFunctionBody;
		_sCurrentFunctionTemplateBody = _sOldFunctionTemplateBody;
		_bCurrentFunctionGenericTemplateKey = bOldCurrentFunctionGenericTemplateKey;
	}

//##markup##"parsing"
//##begin##"parsing"
void DtaScript::parseAppendFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfAppendFile* pAppendFile = new GrfAppendFile;
	if (requiresParsingInformation()) pAppendFile->setParsingInformation(getFilenamePtr(), script);
	block.add(pAppendFile);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pAppendFile->setFilename(parseExpression(block, script));
	else pAppendFile->setFilename(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pAppendFile->setContent(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseAutoexpand(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfAutoexpand* pAutoexpand = new GrfAutoexpand;
	if (requiresParsingInformation()) pAutoexpand->setParsingInformation(getFilenamePtr(), script);
	block.add(pAutoexpand);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pAutoexpand->setOutputFileName(parseExpression(block, script));
	else pAutoexpand->setOutputFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pAutoexpand->setThis(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseClearVariable(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfClearVariable* pClearVariable = new GrfClearVariable;
	if (requiresParsingInformation()) pClearVariable->setParsingInformation(getFilenamePtr(), script);
	block.add(pClearVariable);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pClearVariable->setNode(parseVariableExpression(block, script));
	else pClearVariable->setNode(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseCompileToCpp(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfCompileToCpp* pCompileToCpp = new GrfCompileToCpp;
	if (requiresParsingInformation()) pCompileToCpp->setParsingInformation(getFilenamePtr(), script);
	block.add(pCompileToCpp);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pCompileToCpp->setScriptFileName(parseExpression(block, script));
	else pCompileToCpp->setScriptFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pCompileToCpp->setProjectDirectory(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pCompileToCpp->setCodeWorkerDirectory(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseCopyFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfCopyFile* pCopyFile = new GrfCopyFile;
	if (requiresParsingInformation()) pCopyFile->setParsingInformation(getFilenamePtr(), script);
	block.add(pCopyFile);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pCopyFile->setSourceFileName(parseExpression(block, script));
	else pCopyFile->setSourceFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pCopyFile->setDestinationFileName(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseCopyGenerableFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfCopyGenerableFile* pCopyGenerableFile = new GrfCopyGenerableFile;
	if (requiresParsingInformation()) pCopyGenerableFile->setParsingInformation(getFilenamePtr(), script);
	block.add(pCopyGenerableFile);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pCopyGenerableFile->setSourceFileName(parseExpression(block, script));
	else pCopyGenerableFile->setSourceFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pCopyGenerableFile->setDestinationFileName(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseCopySmartDirectory(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfCopySmartDirectory* pCopySmartDirectory = new GrfCopySmartDirectory;
	if (requiresParsingInformation()) pCopySmartDirectory->setParsingInformation(getFilenamePtr(), script);
	block.add(pCopySmartDirectory);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pCopySmartDirectory->setSourceDirectory(parseExpression(block, script));
	else pCopySmartDirectory->setSourceDirectory(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pCopySmartDirectory->setDestinationPath(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseCutString(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfCutString* pCutString = new GrfCutString;
	if (requiresParsingInformation()) pCutString->setParsingInformation(getFilenamePtr(), script);
	block.add(pCutString);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pCutString->setText(parseExpression(block, script));
	else pCutString->setText(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pCutString->setSeparator(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pCutString->setList(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseEnvironTable(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfEnvironTable* pEnvironTable = new GrfEnvironTable;
	if (requiresParsingInformation()) pEnvironTable->setParsingInformation(getFilenamePtr(), script);
	block.add(pEnvironTable);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pEnvironTable->setTable(parseVariableExpression(block, script));
	else pEnvironTable->setTable(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseError(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfError* pError = new GrfError;
	if (requiresParsingInformation()) pError->setParsingInformation(getFilenamePtr(), script);
	block.add(pError);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pError->setErrorMessage(parseExpression(block, script));
	else pError->setErrorMessage(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseExecuteString(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfExecuteString* pExecuteString = new GrfExecuteString;
	if (requiresParsingInformation()) pExecuteString->setParsingInformation(getFilenamePtr(), script);
	block.add(pExecuteString);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pExecuteString->setThis(parseVariableExpression(block, script));
	else pExecuteString->setThis(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pExecuteString->setCommand(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseExpand(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfExpand* pExpand = new GrfExpand;
	if (requiresParsingInformation()) pExpand->setParsingInformation(getFilenamePtr(), script);
	block.add(pExpand);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pExpand->setPatternFileName(parseScriptFileExpression(block, script, DtaScriptFactory::PATTERN_SCRIPT));
	else pExpand->setPatternFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pExpand->setThis(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pExpand->setOutputFileName(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseExtendExecutedScript(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfExtendExecutedScript* pExtendExecutedScript = new GrfExtendExecutedScript;
	if (requiresParsingInformation()) pExtendExecutedScript->setParsingInformation(getFilenamePtr(), script);
	block.add(pExtendExecutedScript);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pExtendExecutedScript->setScriptContent(parseExpression(block, script));
	else pExtendExecutedScript->setScriptContent(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseGenerate(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfGenerate* pGenerate = new GrfGenerate;
	if (requiresParsingInformation()) pGenerate->setParsingInformation(getFilenamePtr(), script);
	block.add(pGenerate);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pGenerate->setPatternFileName(parseScriptFileExpression(block, script, DtaScriptFactory::PATTERN_SCRIPT));
	else pGenerate->setPatternFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pGenerate->setThis(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pGenerate->setOutputFileName(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseGenerateString(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfGenerateString* pGenerateString = new GrfGenerateString;
	if (requiresParsingInformation()) pGenerateString->setParsingInformation(getFilenamePtr(), script);
	block.add(pGenerateString);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pGenerateString->setPatternFileName(parseScriptFileExpression(block, script, DtaScriptFactory::PATTERN_SCRIPT));
	else pGenerateString->setPatternFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pGenerateString->setThis(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pGenerateString->setOutputString(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseInsertElementAt(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfInsertElementAt* pInsertElementAt = new GrfInsertElementAt;
	if (requiresParsingInformation()) pInsertElementAt->setParsingInformation(getFilenamePtr(), script);
	block.add(pInsertElementAt);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pInsertElementAt->setList(parseVariableExpression(block, script));
	else pInsertElementAt->setList(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pInsertElementAt->setKey(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pInsertElementAt->setPosition(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseInvertArray(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfInvertArray* pInvertArray = new GrfInvertArray;
	if (requiresParsingInformation()) pInvertArray->setParsingInformation(getFilenamePtr(), script);
	block.add(pInvertArray);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pInvertArray->setArray(parseVariableExpression(block, script));
	else pInvertArray->setArray(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseListAllGeneratedFiles(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfListAllGeneratedFiles* pListAllGeneratedFiles = new GrfListAllGeneratedFiles;
	if (requiresParsingInformation()) pListAllGeneratedFiles->setParsingInformation(getFilenamePtr(), script);
	block.add(pListAllGeneratedFiles);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pListAllGeneratedFiles->setFiles(parseVariableExpression(block, script));
	else pListAllGeneratedFiles->setFiles(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseLoadProject(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfLoadProject* pLoadProject = new GrfLoadProject;
	if (requiresParsingInformation()) pLoadProject->setParsingInformation(getFilenamePtr(), script);
	block.add(pLoadProject);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pLoadProject->setXMLorTXTFileName(parseExpression(block, script));
	else pLoadProject->setXMLorTXTFileName(pMethodCaller);
	do {
		if (pMethodCaller != NULL) {
			if (script.peekChar() == ')') break;
		} else {
			if (!script.isEqualTo(',')) break;
		}
		script.skipEmpty();
		pLoadProject->setNodeToLoad(parseVariableExpression(block, script));
		script.skipEmpty();
	} while (false);
	pLoadProject->populateDefaultParameters();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseOpenLogFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfOpenLogFile* pOpenLogFile = new GrfOpenLogFile;
	if (requiresParsingInformation()) pOpenLogFile->setParsingInformation(getFilenamePtr(), script);
	block.add(pOpenLogFile);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pOpenLogFile->setFilename(parseExpression(block, script));
	else pOpenLogFile->setFilename(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseParseAsBNF(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfParseAsBNF* pParseAsBNF = new GrfParseAsBNF;
	if (requiresParsingInformation()) pParseAsBNF->setParsingInformation(getFilenamePtr(), script);
	block.add(pParseAsBNF);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pParseAsBNF->setBNFFileName(parseScriptFileExpression(block, script, DtaScriptFactory::BNF_SCRIPT));
	else pParseAsBNF->setBNFFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pParseAsBNF->setThis(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pParseAsBNF->setInputFileName(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseParseStringAsBNF(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfParseStringAsBNF* pParseStringAsBNF = new GrfParseStringAsBNF;
	if (requiresParsingInformation()) pParseStringAsBNF->setParsingInformation(getFilenamePtr(), script);
	block.add(pParseStringAsBNF);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pParseStringAsBNF->setBNFFileName(parseScriptFileExpression(block, script, DtaScriptFactory::BNF_SCRIPT));
	else pParseStringAsBNF->setBNFFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pParseStringAsBNF->setThis(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pParseStringAsBNF->setContent(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseParseFree(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfParseFree* pParseFree = new GrfParseFree;
	if (requiresParsingInformation()) pParseFree->setParsingInformation(getFilenamePtr(), script);
	block.add(pParseFree);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pParseFree->setDesignFileName(parseScriptFileExpression(block, script, DtaScriptFactory::FREE_SCRIPT));
	else pParseFree->setDesignFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pParseFree->setThis(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pParseFree->setInputFileName(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseProduceHTML(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfProduceHTML* pProduceHTML = new GrfProduceHTML;
	if (requiresParsingInformation()) pProduceHTML->setParsingInformation(getFilenamePtr(), script);
	block.add(pProduceHTML);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pProduceHTML->setScriptFileName(parseExpression(block, script));
	else pProduceHTML->setScriptFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pProduceHTML->setHTMLFileName(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parsePutEnv(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfPutEnv* pPutEnv = new GrfPutEnv;
	if (requiresParsingInformation()) pPutEnv->setParsingInformation(getFilenamePtr(), script);
	block.add(pPutEnv);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pPutEnv->setName(parseExpression(block, script));
	else pPutEnv->setName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pPutEnv->setValue(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseRandomSeed(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfRandomSeed* pRandomSeed = new GrfRandomSeed;
	if (requiresParsingInformation()) pRandomSeed->setParsingInformation(getFilenamePtr(), script);
	block.add(pRandomSeed);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pRandomSeed->setSeed(parseExpression(block, script));
	else pRandomSeed->setSeed(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseRemoveAllElements(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfRemoveAllElements* pRemoveAllElements = new GrfRemoveAllElements;
	if (requiresParsingInformation()) pRemoveAllElements->setParsingInformation(getFilenamePtr(), script);
	block.add(pRemoveAllElements);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pRemoveAllElements->setVariable(parseVariableExpression(block, script));
	else pRemoveAllElements->setVariable(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseRemoveElement(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfRemoveElement* pRemoveElement = new GrfRemoveElement;
	if (requiresParsingInformation()) pRemoveElement->setParsingInformation(getFilenamePtr(), script);
	block.add(pRemoveElement);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pRemoveElement->setVariable(parseVariableExpression(block, script));
	else pRemoveElement->setVariable(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pRemoveElement->setKey(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseRemoveFirstElement(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfRemoveFirstElement* pRemoveFirstElement = new GrfRemoveFirstElement;
	if (requiresParsingInformation()) pRemoveFirstElement->setParsingInformation(getFilenamePtr(), script);
	block.add(pRemoveFirstElement);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pRemoveFirstElement->setList(parseVariableExpression(block, script));
	else pRemoveFirstElement->setList(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseRemoveLastElement(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfRemoveLastElement* pRemoveLastElement = new GrfRemoveLastElement;
	if (requiresParsingInformation()) pRemoveLastElement->setParsingInformation(getFilenamePtr(), script);
	block.add(pRemoveLastElement);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pRemoveLastElement->setList(parseVariableExpression(block, script));
	else pRemoveLastElement->setList(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseRemoveRecursive(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfRemoveRecursive* pRemoveRecursive = new GrfRemoveRecursive;
	if (requiresParsingInformation()) pRemoveRecursive->setParsingInformation(getFilenamePtr(), script);
	block.add(pRemoveRecursive);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pRemoveRecursive->setVariable(parseVariableExpression(block, script));
	else pRemoveRecursive->setVariable(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pRemoveRecursive->setAttribute(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseRemoveVariable(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfRemoveVariable* pRemoveVariable = new GrfRemoveVariable;
	if (requiresParsingInformation()) pRemoveVariable->setParsingInformation(getFilenamePtr(), script);
	block.add(pRemoveVariable);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pRemoveVariable->setNode(parseVariableExpression(block, script));
	else pRemoveVariable->setNode(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSaveBinaryToFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSaveBinaryToFile* pSaveBinaryToFile = new GrfSaveBinaryToFile;
	if (requiresParsingInformation()) pSaveBinaryToFile->setParsingInformation(getFilenamePtr(), script);
	block.add(pSaveBinaryToFile);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSaveBinaryToFile->setFilename(parseExpression(block, script));
	else pSaveBinaryToFile->setFilename(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pSaveBinaryToFile->setContent(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSaveProject(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSaveProject* pSaveProject = new GrfSaveProject;
	if (requiresParsingInformation()) pSaveProject->setParsingInformation(getFilenamePtr(), script);
	block.add(pSaveProject);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSaveProject->setXMLorTXTFileName(parseExpression(block, script));
	else pSaveProject->setXMLorTXTFileName(pMethodCaller);
	do {
		if (pMethodCaller != NULL) {
			if (script.peekChar() == ')') break;
		} else {
			if (!script.isEqualTo(',')) break;
		}
		script.skipEmpty();
		pSaveProject->setNodeToSave(parseVariableExpression(block, script));
		script.skipEmpty();
	} while (false);
	pSaveProject->populateDefaultParameters();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSaveProjectTypes(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSaveProjectTypes* pSaveProjectTypes = new GrfSaveProjectTypes;
	if (requiresParsingInformation()) pSaveProjectTypes->setParsingInformation(getFilenamePtr(), script);
	block.add(pSaveProjectTypes);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSaveProjectTypes->setXMLFileName(parseExpression(block, script));
	else pSaveProjectTypes->setXMLFileName(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSaveToFile(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSaveToFile* pSaveToFile = new GrfSaveToFile;
	if (requiresParsingInformation()) pSaveToFile->setParsingInformation(getFilenamePtr(), script);
	block.add(pSaveToFile);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSaveToFile->setFilename(parseExpression(block, script));
	else pSaveToFile->setFilename(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pSaveToFile->setContent(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetCommentBegin(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetCommentBegin* pSetCommentBegin = new GrfSetCommentBegin;
	if (requiresParsingInformation()) pSetCommentBegin->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetCommentBegin);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetCommentBegin->setCommentBegin(parseExpression(block, script));
	else pSetCommentBegin->setCommentBegin(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetCommentEnd(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetCommentEnd* pSetCommentEnd = new GrfSetCommentEnd;
	if (requiresParsingInformation()) pSetCommentEnd->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetCommentEnd);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetCommentEnd->setCommentEnd(parseExpression(block, script));
	else pSetCommentEnd->setCommentEnd(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetGenerationHeader(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetGenerationHeader* pSetGenerationHeader = new GrfSetGenerationHeader;
	if (requiresParsingInformation()) pSetGenerationHeader->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetGenerationHeader);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetGenerationHeader->setComment(parseExpression(block, script));
	else pSetGenerationHeader->setComment(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetIncludePath(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetIncludePath* pSetIncludePath = new GrfSetIncludePath;
	if (requiresParsingInformation()) pSetIncludePath->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetIncludePath);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetIncludePath->setPath(parseExpression(block, script));
	else pSetIncludePath->setPath(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetNow(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetNow* pSetNow = new GrfSetNow;
	if (requiresParsingInformation()) pSetNow->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetNow);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetNow->setConstantDateTime(parseExpression(block, script));
	else pSetNow->setConstantDateTime(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetProperty(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetProperty* pSetProperty = new GrfSetProperty;
	if (requiresParsingInformation()) pSetProperty->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetProperty);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetProperty->setDefine(parseExpression(block, script));
	else pSetProperty->setDefine(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pSetProperty->setValue(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetTextMode(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetTextMode* pSetTextMode = new GrfSetTextMode;
	if (requiresParsingInformation()) pSetTextMode->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetTextMode);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetTextMode->setTextMode(parseExpression(block, script));
	else pSetTextMode->setTextMode(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetVersion(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetVersion* pSetVersion = new GrfSetVersion;
	if (requiresParsingInformation()) pSetVersion->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetVersion);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetVersion->setVersion(parseExpression(block, script));
	else pSetVersion->setVersion(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetWriteMode(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetWriteMode* pSetWriteMode = new GrfSetWriteMode;
	if (requiresParsingInformation()) pSetWriteMode->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetWriteMode);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetWriteMode->setMode(parseExpression(block, script));
	else pSetWriteMode->setMode(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSetWorkingPath(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetWorkingPath* pSetWorkingPath = new GrfSetWorkingPath;
	if (requiresParsingInformation()) pSetWorkingPath->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetWorkingPath);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetWorkingPath->setPath(parseExpression(block, script));
	else pSetWorkingPath->setPath(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSleep(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSleep* pSleep = new GrfSleep;
	if (requiresParsingInformation()) pSleep->setParsingInformation(getFilenamePtr(), script);
	block.add(pSleep);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSleep->setMillis(parseExpression(block, script));
	else pSleep->setMillis(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSlideNodeContent(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSlideNodeContent* pSlideNodeContent = new GrfSlideNodeContent;
	if (requiresParsingInformation()) pSlideNodeContent->setParsingInformation(getFilenamePtr(), script);
	block.add(pSlideNodeContent);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSlideNodeContent->setOrgNode(parseVariableExpression(block, script));
	else pSlideNodeContent->setOrgNode(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	{
		CGQuietOutput quiet;
		pSlideNodeContent->setDestNode(parseVariableExpression(block, script));
		script.skipEmpty();
	}
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseSortArray(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSortArray* pSortArray = new GrfSortArray;
	if (requiresParsingInformation()) pSortArray->setParsingInformation(getFilenamePtr(), script);
	block.add(pSortArray);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSortArray->setArray(parseVariableExpression(block, script));
	else pSortArray->setArray(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseTraceEngine(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfTraceEngine* pTraceEngine = new GrfTraceEngine;
	if (requiresParsingInformation()) pTraceEngine->setParsingInformation(getFilenamePtr(), script);
	block.add(pTraceEngine);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseTraceLine(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfTraceLine* pTraceLine = new GrfTraceLine;
	if (requiresParsingInformation()) pTraceLine->setParsingInformation(getFilenamePtr(), script);
	block.add(pTraceLine);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pTraceLine->setLine(parseExpression(block, script));
	else pTraceLine->setLine(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseTraceObject(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfTraceObject* pTraceObject = new GrfTraceObject;
	if (requiresParsingInformation()) pTraceObject->setParsingInformation(getFilenamePtr(), script);
	block.add(pTraceObject);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pTraceObject->setObject(parseVariableExpression(block, script));
	else pTraceObject->setObject(pMethodCaller);
	do {
		if (pMethodCaller != NULL) {
			if (script.peekChar() == ')') break;
		} else {
			if (!script.isEqualTo(',')) break;
		}
		script.skipEmpty();
		pTraceObject->setDepth(parseExpression(block, script));
		script.skipEmpty();
	} while (false);
	pTraceObject->populateDefaultParameters();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseTraceStack(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfTraceStack* pTraceStack = new GrfTraceStack;
	if (requiresParsingInformation()) pTraceStack->setParsingInformation(getFilenamePtr(), script);
	block.add(pTraceStack);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseTraceText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfTraceText* pTraceText = new GrfTraceText;
	if (requiresParsingInformation()) pTraceText->setParsingInformation(getFilenamePtr(), script);
	block.add(pTraceText);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pTraceText->setText(parseExpression(block, script));
	else pTraceText->setText(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseTranslate(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfTranslate* pTranslate = new GrfTranslate;
	if (requiresParsingInformation()) pTranslate->setParsingInformation(getFilenamePtr(), script);
	block.add(pTranslate);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pTranslate->setPatternFileName(parseScriptFileExpression(block, script, DtaScriptFactory::TRANSLATE_SCRIPT));
	else pTranslate->setPatternFileName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pTranslate->setThis(parseVariableExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pTranslate->setInputFileName(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pTranslate->setOutputFileName(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaScript::parseAttachInputToSocket(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"attachInputToSocket\" must be used into design format files only");
}

void DtaScript::parseDetachInputFromSocket(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"detachInputFromSocket\" must be used into design format files only");
}

void DtaScript::parseGoBack(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"goBack\" must be used into design format files only");
}

void DtaScript::parseSetInputLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"setInputLocation\" must be used into design format files only");
}

void DtaScript::parseAllFloatingLocations(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"allFloatingLocations\" must be used into pattern files only");
}

void DtaScript::parseAttachOutputToSocket(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"attachOutputToSocket\" must be used into pattern files only");
}

void DtaScript::parseDetachOutputFromSocket(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"detachOutputFromSocket\" must be used into pattern files only");
}

void DtaScript::parseIncrementIndentLevel(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"incrementIndentLevel\" must be used into pattern files only");
}

void DtaScript::parseInsertText(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"insertText\" must be used into pattern files only");
}

void DtaScript::parseInsertTextOnce(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"insertTextOnce\" must be used into pattern files only");
}

void DtaScript::parseInsertTextToFloatingLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"insertTextToFloatingLocation\" must be used into pattern files only");
}

void DtaScript::parseInsertTextOnceToFloatingLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"insertTextOnceToFloatingLocation\" must be used into pattern files only");
}

void DtaScript::parseOverwritePortion(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"overwritePortion\" must be used into pattern files only");
}

void DtaScript::parsePopulateProtectedArea(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"populateProtectedArea\" must be used into pattern files only");
}

void DtaScript::parseResizeOutputStream(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"resizeOutputStream\" must be used into pattern files only");
}

void DtaScript::parseSetFloatingLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"setFloatingLocation\" must be used into pattern files only");
}

void DtaScript::parseSetOutputLocation(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"setOutputLocation\" must be used into pattern files only");
}

void DtaScript::parseSetProtectedArea(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"setProtectedArea\" must be used into pattern files only");
}

void DtaScript::parseWriteBytes(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"writeBytes\" must be used into pattern files only");
}

void DtaScript::parseWriteText(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"writeText\" must be used into pattern files only");
}

void DtaScript::parseWriteTextOnce(GrfBlock& /*block*/, ScpStream& script, ExprScriptVariable* /*pMethodCaller*/) {
	throw UtlException(script, "command \"writeTextOnce\" must be used into pattern files only");
}

void DtaScript::parseCloseSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfCloseSocket* pCloseSocket = new GrfCloseSocket;
	if (requiresParsingInformation()) pCloseSocket->setParsingInformation(getFilenamePtr(), script);
	block.add(pCloseSocket);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pCloseSocket->setSocket(parseExpression(block, script));
	else pCloseSocket->setSocket(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

//##end##"parsing"

	void DtaScript::parseNop(GrfBlock& block, ScpStream& script) {
		GrfNop* pNop = new GrfNop;
		if (requiresParsingInformation()) pNop->setParsingInformation(getFilenamePtr(), script);
		block.add(pNop);
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
		script.skipEmpty();
		pNop->setExpression(parseExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaScript::parseLocalVariable(GrfBlock& block, ScpStream& script) {
		EXPRESSION_TYPE exprType = parseClassType(block, script, NODE_EXPRTYPE);
		do {
			GrfLocalVariable* pLocalVariable = new GrfLocalVariable;
			if (requiresParsingInformation()) pLocalVariable->setParsingInformation(getFilenamePtr(), script);
			block.add(pLocalVariable);
			script.skipEmpty();
			std::string sVariable;
			if (!script.readIdentifier(sVariable)) throw UtlException(script, "syntax error: name of the local variable expected");
			if ((sVariable == "project") || (sVariable == "null") || (sVariable == "this")) throw UtlException(script, "'" + sVariable + "' is a reserved variable, and cannot be redeclared");
			ExprScriptVariable* pVarExpr = new ExprScriptVariable(sVariable.c_str());
			pLocalVariable->setLocalVariable(pVarExpr, exprType);
			script.skipEmpty();
			if (script.isEqualTo('=')) {
				script.skipEmpty();
				pLocalVariable->setValue(parseAssignmentExpression(block, script));
				script.skipEmpty();
			}
		} while (script.isEqualTo(','));
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaScript::parseLocalReference(GrfBlock& block, ScpStream& script) {
		EXPRESSION_TYPE exprType = parseClassType(block, script, REFERENCE_EXPRTYPE);
		do {
			GrfLocalReference* pLocalReference = new GrfLocalReference;
			if (requiresParsingInformation()) pLocalReference->setParsingInformation(getFilenamePtr(), script);
			block.add(pLocalReference);
			script.skipEmpty();
			std::string sVariable;
			if (!script.readIdentifier(sVariable)) throw UtlException(script, "syntax error: name of the local variable expected");
			if ((sVariable == "project") || (sVariable == "null") || (sVariable == "this")) throw UtlException(script, "'" + sVariable + "' is a reserved variable, and cannot be redeclared");
			ExprScriptVariable* pVarExpr = new ExprScriptVariable(sVariable.c_str());
			pLocalReference->setLocalVariable(pVarExpr, exprType);
			script.skipEmpty();
			if (!script.isEqualTo('=')) throw UtlException(script, "syntax error: '=' expected");
			script.skipEmpty();
			pLocalReference->setReference(parseVariableExpression(block, script));
			script.skipEmpty();
		} while (script.isEqualTo(','));
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaScript::parseGlobalVariable(GrfBlock& block, ScpStream& script) {
		EXPRESSION_TYPE exprType = parseClassType(block, script, NODE_EXPRTYPE);
		do {
			GrfGlobalVariable* pGlobalVariable = new GrfGlobalVariable;
			if (requiresParsingInformation()) pGlobalVariable->setParsingInformation(getFilenamePtr(), script);
			block.add(pGlobalVariable);
			script.skipEmpty();
			std::string sVariable;
			if (!script.readIdentifier(sVariable)) throw UtlException(script, "syntax error: name of the global variable expected");
			if ((sVariable == "project") || (sVariable == "null") || (sVariable == "this")) throw UtlException(script, "'" + sVariable + "' is a reserved variable, and cannot be redeclared");
			pGlobalVariable->setVariable(sVariable, exprType);
			script.skipEmpty();
			if (script.isEqualTo('=')) {
				script.skipEmpty();
				pGlobalVariable->setValue(parseAssignmentExpression(block, script));
				script.skipEmpty();
			}
		} while (script.isEqualTo(','));
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaScript::parseSetAssignment(GrfBlock& block, ScpStream& script) {
		GrfSetAssignment* pAssignment = new GrfSetAssignment;
		if (requiresParsingInformation()) pAssignment->setParsingInformation(getFilenamePtr(), script);
		block.add(pAssignment);
		pAssignment->setVariable(parseVariableExpression(block, script));
		script.skipEmpty();
		bool bConcat = script.isEqualTo('+');
		if (!script.isEqualTo('=')) throw UtlException(script, "syntax error: assignment expected ('+=' or '=' operator)");
		script.skipEmpty();
		pAssignment->setValue(parseAssignmentExpression(block, script), bConcat);
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaScript::parseInsertAssignment(GrfBlock& block, ScpStream& script) {
		GrfInsertAssignment* pAssignment = new GrfInsertAssignment;
		if (requiresParsingInformation()) pAssignment->setParsingInformation(getFilenamePtr(), script);
		block.add(pAssignment);
		pAssignment->setVariable(parseVariableExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo(';')) {
			bool bConcat = script.isEqualTo('+');
			if (!script.isEqualTo('=')) throw UtlException(script, "syntax error: assignment expected ('+=' or '=' operator)");
			script.skipEmpty();
			pAssignment->setValue(parseAssignmentExpression(block, script), bConcat);
			script.skipEmpty();
			if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
		}
	}

	void DtaScript::parseSetAll(GrfBlock& block, ScpStream& script) {
		GrfSetAll* pAffectation = new GrfSetAll;
		if (requiresParsingInformation()) pAffectation->setParsingInformation(getFilenamePtr(), script);
		block.add(pAffectation);
		pAffectation->setVariable(parseVariableExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo('=')) throw UtlException(script, "syntax error: assignation expected ('=' operator)");
		script.skipEmpty();
		pAffectation->setSource(parseVariableExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaScript::parseMerge(GrfBlock& block, ScpStream& script) {
		GrfMerge* pAffectation = new GrfMerge;
		if (requiresParsingInformation()) pAffectation->setParsingInformation(getFilenamePtr(), script);
		block.add(pAffectation);
		pAffectation->setVariable(parseVariableExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo('=')) throw UtlException(script, "syntax error: assignation expected ('=' operator)");
		script.skipEmpty();
		pAffectation->setSource(parseVariableExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaScript::parsePushItem(GrfBlock& block, ScpStream& script) {
		GrfPushItem* pAffectation = new GrfPushItem;
		if (requiresParsingInformation()) pAffectation->setParsingInformation(getFilenamePtr(), script);
		block.add(pAffectation);
		pAffectation->setVariable(parseVariableExpression(block, script));
		script.skipEmpty();
		if (script.isEqualTo('=')) {
			script.skipEmpty();
			pAffectation->setValue(parseExpression(block, script));
			script.skipEmpty();
		}
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaScript::parseReference(GrfBlock& block, ScpStream& script) {
		GrfReference* pReference = new GrfReference;
		if (requiresParsingInformation()) pReference->setParsingInformation(getFilenamePtr(), script);
		block.add(pReference);
	/*	std::string sWord;
		skipEmpty(script);
		if (!readIdentifier(script, sWord)) throw UtlException(script, "syntax error: variable expected");
		pReference->setVariable(sWord.c_str());
	*/
		pReference->setVariable(parseVariableExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo('=')) throw UtlException(script, "syntax error: '=' expected");
		script.skipEmpty();
		pReference->setReference(parseVariableExpression(block, script));
		script.skipEmpty();
		if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
	}

	void DtaScript::parseSyntax(GrfBlock& block, ScpStream& script) {
		script.skipEmpty();
		std::string sParsingMode;
		std::string sParsingScript;
		if (script.readIdentifier(sParsingMode)) {
			script.skipLineBlanks();
			if (script.isEqualTo(':')) {
				script.skipEmpty();
				if (!script.readString(sParsingScript) || sParsingScript.empty()) {
					throw UtlException(script, "directive '#syntax', BNF script file expected after ':'");
				}
			}
		} else if (!script.readString(sParsingScript) || sParsingScript.empty()) {
			throw UtlException(script, "syntax error on directive '#syntax': parsing mode or BNF script file expected");
		}
		if (!sParsingMode.empty()) {
			std::map<std::string, std::string>::const_iterator cursor = _mapOfSyntaxModes.find(sParsingMode);
			if (cursor != _mapOfSyntaxModes.end()) {
				if (sParsingScript.empty()) sParsingScript = cursor->second;
				else if (sParsingScript != cursor->second) {
					throw UtlException("on directive '#syntax': the BNF script file attached to the parsing mode '" + sParsingMode + "' cannot change");
				}
			} else {
				if (sParsingScript.empty()) throw UtlException("on directive '#syntax': no BNF script file attached to the parsing mode '" + sParsingMode + "'");
				_mapOfSyntaxModes[sParsingMode] = sParsingScript;
			}
		}
		if (!script.findString("\n")) throw UtlException(script, "syntax error on directive '#syntax': expected end of file");
		std::string sText;
		std::string sLine;
		while (script.readLine(sLine)) {
			if (strncmp(sLine.c_str(), "#end", 4) == 0) {
				register int iIndex = 4;
				register char a = sLine[4];
				while ((a == ' ') || (a == '\t')) {
					iIndex++;
					a = sLine[iIndex];
				}
				if ((iIndex > 4) && (strncmp(sLine.c_str() + iIndex, "syntax", 6) == 0)) break;
			}
			sText += sLine + CGRuntime::endl();
		}
		GrfParseStringAsBNF* pParseStringAsBNF = new GrfParseStringAsBNF;
		if (requiresParsingInformation()) pParseStringAsBNF->setParsingInformation(getFilenamePtr(), script);
		block.add(pParseStringAsBNF);
		pParseStringAsBNF->setBNFFileName(new ExprScriptConstant(sParsingScript.c_str()));
		pParseStringAsBNF->setThis(new ExprScriptVariable("this"));
		pParseStringAsBNF->setContent(new ExprScriptConstant(sText.c_str()));
	}

	SEQUENCE_INTERRUPTION_LIST DtaScript::execute(DtaScriptVariable& thisContext) {
		SEQUENCE_INTERRUPTION_LIST result;
		CGThisModifier thisModifier(&thisContext);
		DtaScriptVariable stackScript(&thisContext, "##stack## script");
		if (GrfCommand::getCurrentExecutionContext() != NULL) {
			try {
				GrfCommand::getCurrentExecutionContext()->handleBeforeScriptExecutionCBK(&_graph, stackScript);
				result = _graph.execute(stackScript);
			} catch(std::exception&) {
				GrfCommand::getCurrentExecutionContext()->handleAfterScriptExecutionCBK(&_graph, stackScript);
				throw;
			}
			GrfCommand::getCurrentExecutionContext()->handleAfterScriptExecutionCBK(&_graph, stackScript);
		} else {
			DtaScript* pOldScript = DtaProject::getInstance().getScript();
			try {
				DtaProject::getInstance().setScript(this);
				result = _graph.execute(stackScript);
				DtaProject::getInstance().setScript(pOldScript);
			} catch(std::exception&) {
				DtaProject::getInstance().setScript(pOldScript);
				throw;
			}
		}
		return result;
	}

	void DtaScript::compileCpp(CppCompilerEnvironment& theCompilerEnvironment, const std::string& sScriptFilename) const {
		if (theCompilerEnvironment.pushFilename(sScriptFilename)) {
			try {
				std::string sOldIndentation = theCompilerEnvironment.getIndentation();
				theCompilerEnvironment.setIndentation("");
				std::string sIdentifier = convertFilenameAsIdentifier(theCompilerEnvironment.getRadical());
				theCompilerEnvironment.getHeader() << "#ifndef _" << sIdentifier << "_h_";
				theCompilerEnvironment.getHeader().endl();
				theCompilerEnvironment.getHeader() << "#define _" << sIdentifier << "_h_";
				theCompilerEnvironment.getHeader().endl();
				theCompilerEnvironment.getHeader().endl();
				compileCppHeaderIncludes(theCompilerEnvironment);
				theCompilerEnvironment.getHeader().endl();
				theCompilerEnvironment.getHeader().endl();
				theCompilerEnvironment.getHeader() << "class " << sIdentifier << " {";
				theCompilerEnvironment.getHeader().endl();
				theCompilerEnvironment.getHeader() << "\tpublic:";
				theCompilerEnvironment.getHeader().endl();

				CW_BODY_STREAM << "#ifdef WIN32";CW_BODY_ENDL;
				CW_BODY_STREAM << "#pragma warning(disable: 4786)";CW_BODY_ENDL;
				CW_BODY_STREAM << "#endif";CW_BODY_ENDL;
				CW_BODY_ENDL;
				CW_BODY_STREAM << "#include <exception>";CW_BODY_ENDL;
				CW_BODY_STREAM << "#include <iostream>";CW_BODY_ENDL;
				CW_BODY_STREAM << "#include <map>";CW_BODY_ENDL;
				CW_BODY_STREAM << "#include <set>";CW_BODY_ENDL;
				CW_BODY_STREAM << "#include <time.h>";CW_BODY_ENDL;
				CW_BODY_STREAM << "#include \"UtlException.h\"";CW_BODY_ENDL;
				CW_BODY_STREAM << "#include \"CGRuntime.h\"";CW_BODY_ENDL;
				CW_BODY_ENDL;
				if (hasTargetLanguage()) {
					CW_BODY_STREAM << "// The two following preprocessor definitions delimit the " << getTargetLanguage() << " code";CW_BODY_ENDL;
					CW_BODY_STREAM << "// you have written in the original CodeWorker script. They do nothing but";CW_BODY_ENDL;
					CW_BODY_STREAM << "// to inform you about the position of this piece of code in the script:";CW_BODY_ENDL;
					CW_BODY_STREAM << "//     - lang: target language (certainly " << getTargetLanguage() << " here),";CW_BODY_ENDL;
					CW_BODY_STREAM << "//     - file: CodeWorker script file the piece of code is coming from,";CW_BODY_ENDL;
					CW_BODY_STREAM << "//     - line: line number of the piece of code in the script,";CW_BODY_ENDL;
					CW_BODY_STREAM << "#define BEGIN_TARGET_LANGUAGE_CODE(lang, file, line)";CW_BODY_ENDL;
					CW_BODY_STREAM << "#define END_TARGET_LANGUAGE_CODE(lang)";CW_BODY_ENDL;
					CW_BODY_STREAM << "// The next preprocessor definitions indicate a syntactic entity,";CW_BODY_ENDL;
					CW_BODY_STREAM << "// defined in the target language:";CW_BODY_ENDL;
					CW_BODY_STREAM << "#define TARGET_LANGUAGE_TYPE_SPECIFIER(type_specifier) type_specifier";CW_BODY_ENDL;
					CW_BODY_STREAM << "#define TARGET_LANGUAGE_VARIABLE(var) var";CW_BODY_ENDL;
					CW_BODY_STREAM << "#define TARGET_LANGUAGE_EXPRESSION(expr) expr";CW_BODY_ENDL;
					CW_BODY_ENDL;
				}
				CW_BODY_STREAM << "#include \"" << theCompilerEnvironment.getRadical() << ".h\"";CW_BODY_ENDL;
				CW_BODY_ENDL;
				CW_BODY_STREAM << "using namespace CodeWorker;";CW_BODY_ENDL;
				CW_BODY_ENDL;
				int iInsertAreaLocation = CW_BODY_STREAM.getOutputLocation();
				CW_BODY_ENDL;
				CW_BODY_STREAM.setFloatingLocation("INSERT AREA", iInsertAreaLocation);
				compileCppFunctions(theCompilerEnvironment);
				CW_BODY_STREAM << "Execute" << sIdentifier << "& Execute" << sIdentifier << "::instance() {";CW_BODY_ENDL;
				CW_BODY_STREAM << "\tstatic Execute" << sIdentifier << " theInstance;";CW_BODY_ENDL;
				CW_BODY_STREAM << "\treturn theInstance;";CW_BODY_ENDL;
				CW_BODY_STREAM << "}";CW_BODY_ENDL;
				CW_BODY_ENDL;
				CW_BODY_STREAM << "void Execute" << sIdentifier << "::run() ";
				int iGlobalInitializationPosition = CW_BODY_STREAM.getOutputLocation();
				CW_BODY_STREAM.setFloatingLocation("GLOBAL INIT", iGlobalInitializationPosition);
				_graph.compileCpp(theCompilerEnvironment);
				theCompilerEnvironment.getHeader() << "};";
				theCompilerEnvironment.getHeader().endl();
				if (theCompilerEnvironment.getIncludeParentScript().empty()) {
					// dynamic packages?
					{
						const std::map<std::string, DynPackage*>& allPackages = DynPackage::allPackages();
						if (!allPackages.empty()) {
							ScpStream* pOwner = NULL;
							iGlobalInitializationPosition = CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner);
							CW_BODY_STREAM.insertText(ScpStream::ENDL, iGlobalInitializationPosition);
							for (std::map<std::string, DynPackage*>::const_iterator i = allPackages.begin(); i != allPackages.end(); ++i) {
								CW_BODY_STREAM.insertTextOnce("#include \"" + i->first + ".h\"" + ScpStream::ENDL, iGlobalInitializationPosition);
							}
							CW_BODY_STREAM.insertText("// header files for all dynamic packages" + ScpStream::ENDL, iGlobalInitializationPosition);
						}
					}
					// global variables?
					const std::set<std::string>& globalVars = theCompilerEnvironment.getGlobalVariables();
					ScpStream* pOwner;
					if (!globalVars.empty()) {
						iGlobalInitializationPosition = CW_BODY_STREAM.getFloatingLocation("GLOBAL INIT", pOwner);
						while (CW_BODY_STREAM.readBuffer()[iGlobalInitializationPosition] != '\n') {
							iGlobalInitializationPosition++;
						}
						CW_BODY_STREAM.setFloatingLocation("GLOBAL INIT", iGlobalInitializationPosition + 1);
						{
							theCompilerEnvironment.getHeader() << "// global variables:" << CGRuntime::endl();
							for (std::set<std::string>::const_iterator i = globalVars.begin(); i != globalVars.end(); ++i) {
								theCompilerEnvironment.getHeader() << "extern CodeWorker::CppParsingTree_global " << *i << ";" << CGRuntime::endl();
							}
							theCompilerEnvironment.getHeader().endl();
							theCompilerEnvironment.getHeader().endl();
						}
						CW_BODY_STREAM.insertText("// global variables:" + CGRuntime::endl(), CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner));
						for (std::set<std::string>::const_iterator i = globalVars.begin(); i != globalVars.end(); ++i) {
							CW_BODY_STREAM.insertText("CppParsingTree_global " + *i + ";" + CGRuntime::endl(), CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner));
						}
					}

					CW_BODY_ENDL;
					CW_BODY_ENDL;
					if (DtaProject::getInstance().getReadonlyHook() != NULL) {
						DtaProject::getInstance().getReadonlyHook()->compileCpp(theCompilerEnvironment);
					}
					if (DtaProject::getInstance().getWritefileHook() != NULL) {
						DtaProject::getInstance().getWritefileHook()->compileCpp(theCompilerEnvironment);
					}
					CW_BODY_STREAM << "int main(int iNargs, char** tsArgs) {";CW_BODY_ENDL;
					CW_BODY_STREAM << "\tint iReturnCode = -1;";CW_BODY_ENDL;
					CW_BODY_STREAM << "\ttry {";CW_BODY_ENDL;
					{
						// initialization of global variables
						for (std::set<std::string>::const_iterator i = globalVars.begin(); i != globalVars.end(); ++i) {
							CW_BODY_STREAM.insertText("\t" + *i + ".initialize(\"" + *i + "\");" + CGRuntime::endl(), CW_BODY_STREAM.getFloatingLocation("GLOBAL INIT", pOwner));
						}
					}
					if (DtaProject::getInstance().getReadonlyHook() != NULL) {
						CW_BODY_STREAM << "\t\tCGRuntime::registerReadonlyHook(" << DtaProject::getInstance().getReadonlyHook()->getFunctionName() << ");";CW_BODY_ENDL;
					}
					if (DtaProject::getInstance().getWritefileHook() != NULL) {
						CW_BODY_STREAM << "\t\tCGRuntime::registerWritefileHook(" << DtaProject::getInstance().getWritefileHook()->getFunctionName() << ");";CW_BODY_ENDL;
					}
					const std::list<std::string>& listOfModules = theCompilerEnvironment.getProjectModules();
					std::list<std::string>::const_iterator i;
					for (i = listOfModules.begin(); i != listOfModules.end(); i++) {
						CW_BODY_STREAM << "\t\tCGRuntime::registerScript(\"" << CppCompilerEnvironment::getRadical(*i) << "\", &Execute" << convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(*i)) << "::instance());";CW_BODY_ENDL;
					}
					{
						const std::map<std::string, DynPackage*>& allPackages = DynPackage::allPackages();
						for (std::map<std::string, DynPackage*>::const_iterator i = allPackages.begin(); i != allPackages.end(); ++i) {
							CW_BODY_STREAM << "\t\t" << i->first << " initializeDynamicPackage" << i->first << ";";CW_BODY_ENDL;
						}
					}
					CW_BODY_STREAM << "\t\tiReturnCode = CGRuntime::entryPoint(iNargs, tsArgs, &Execute" << sIdentifier <<"::instance());";
					CW_BODY_ENDL;
					CW_BODY_STREAM << "\t} catch(UtlExitException& exit) {";CW_BODY_ENDL;
					CW_BODY_STREAM << "\t\tiReturnCode = exit.getCode();";CW_BODY_ENDL;
					CW_BODY_STREAM << "\t} catch(std::exception& exception) {";CW_BODY_ENDL;
					CW_BODY_STREAM << "\t\tCGRuntime::traceLine(exception.what());";CW_BODY_ENDL;
					CW_BODY_STREAM << "\t}";CW_BODY_ENDL;
					CW_BODY_STREAM << "\treturn iReturnCode;";CW_BODY_ENDL;
					CW_BODY_STREAM << "}";CW_BODY_ENDL;
					theCompilerEnvironment.getMainHeader().endl();
					for (i = listOfModules.begin(); i != listOfModules.end(); i++) {
						std::string sIdentifier = convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(*i));
						theCompilerEnvironment.getMainHeader() << "class Execute" << sIdentifier << " : public CodeWorker::EXECUTE_FUNCTION {";
						theCompilerEnvironment.getMainHeader().endl();
						theCompilerEnvironment.getMainHeader() << "\tpublic:";theCompilerEnvironment.getMainHeader().endl();
						theCompilerEnvironment.getMainHeader() << "\t\tstatic Execute" << sIdentifier << "& instance();";theCompilerEnvironment.getMainHeader().endl();
						theCompilerEnvironment.getMainHeader() << "\t\tvirtual void run();";theCompilerEnvironment.getMainHeader().endl();
						theCompilerEnvironment.getMainHeader() << "};";theCompilerEnvironment.getMainHeader().endl();
						theCompilerEnvironment.getMainHeader().endl();
					}
				}

				theCompilerEnvironment.getHeader().endl();
				theCompilerEnvironment.getHeader() << "#endif";
				theCompilerEnvironment.getHeader().endl();

				theCompilerEnvironment.setIndentation(sOldIndentation);
			} catch(UtlException& exception) {
				theCompilerEnvironment.catchFilename(exception);
			}
			theCompilerEnvironment.popFilename();
		}
	}

	void DtaScript::compileCppHeaderIncludes(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (!theCompilerEnvironment.getIncludeParentScript().empty()) {
			std::string sCppParent = theCompilerEnvironment.getIncludeParentScript();
			theCompilerEnvironment.getHeader() << "#include \"" << CppCompilerEnvironment::getRelativePath(sCppParent) << ".h\"";
			theCompilerEnvironment.getHeader().endl();
		} else {
			theCompilerEnvironment.getHeader() << "#include <string>";
			theCompilerEnvironment.getHeader().endl();
			theCompilerEnvironment.getHeader() << "#include \"CppParsingTree.h\"";
			theCompilerEnvironment.getHeader().endl();
			theCompilerEnvironment.getHeader() << "#include \"CGExternalHandling.h\"";
			theCompilerEnvironment.getHeader().endl();
		}
	}

	void DtaScript::compileCppFunctions(CppCompilerEnvironment& theCompilerEnvironment) const {
		_graph.compileCppFunctions(theCompilerEnvironment);
	}

	std::string DtaScript::convertFilenameAsIdentifier(const std::string& sFilename) {
		std::string sIdentifier;
		std::string::size_type iIndex = sFilename.find(".");
		if (iIndex == std::string::npos) sIdentifier = sFilename;
		else sIdentifier = sFilename.substr(0, iIndex);
		for (std::string::size_type i = 0; i < sIdentifier.size(); i++) {
			char a = sIdentifier[i];
			if (((a < 'a') || (a > 'z')) && ((a < 'A') || (a > 'Z')) && ((a < '0') || (a > '9')) && (a != '_')) {
				if (a == '+') a = 'p';
				else a = '_';
				sIdentifier[i] = a;
			}
		}
		return sIdentifier;
	}

	bool DtaScript::equalsIgnoringGenerationHeader(ScpStream& theInputStream, ScpStream& theOutputStream, int& iPosition) {
		std::string sGenerator;
		std::string sVersion;
		std::string sDate;
		std::string sComment1;
		CGRuntime::extractGenerationHeader(theInputStream, sGenerator, sVersion, sDate, sComment1);
		std::string sComment2;
		CGRuntime::extractGenerationHeader(theOutputStream, sGenerator, sVersion, sDate, sComment2);
		if (sComment1 != sComment2) {
			iPosition = theInputStream.getInputLocation();
			return false;
		}
		return theInputStream.equalsFromInputLocations(theOutputStream, iPosition);
	}

	int DtaScript::computeReferenceMagicNumber(ScpStream& script, const char* tcFilename, const std::string& sKey) {
		int iChar;
		int iOldChar = '\0';
		int iMagicNumber = 0;
		for(;;) {
			iChar = script.readChar();
			if (iChar == -1)  throw UtlException(script, "syntax error on '#reference' area: '#end " + sKey + "' expected before end of file");
			if ((iChar == '#') && (iOldChar == '\n')) {
				std::string sDirective;
				if (!script.readIdentifier(sDirective)) throw UtlException(script, "preprocessor directive expected after '#' symbol");
				if (sDirective == "reference") {
					script.skipLineBlanks();
					std::string sEmbeddedKey;
					if (!script.readIdentifier(sEmbeddedKey)) throw UtlException(script, "syntax error on '#reference': identifier expected");
					if (!script.findString("\n")) throw UtlException(script, "syntax error on '#reference " + sEmbeddedKey + "': end of line expected");
					int iEmbeddedMagicNumber = computeReferenceMagicNumber(script, tcFilename, sKey);
					iMagicNumber = (31*iMagicNumber + iEmbeddedMagicNumber) & 0x03FFFFFF;
				} else if (sDirective == "end") {
					script.skipLineBlanks();
					if (script.isEqualToIdentifier(sKey.c_str())) {
						if (!script.findString("\n")) throw UtlException(script, "syntax error on '#end " + sKey + "': end of line expected");
						break;
					}
				}
			} else {
				iMagicNumber = (31*iMagicNumber + iChar) & 0x03FFFFFF;
			}
			iOldChar = iChar;
		}
		std::string sFileName = tcFilename;
		std::string::size_type iIndex = sFileName.find_last_of("/\\");
		if (iIndex != std::string::npos) sFileName = sFileName.substr(iIndex + 1);
		_mapOfReferenceMagicNumbers[sKey + ":" + sFileName] = iMagicNumber;
		return iMagicNumber;
	}

	int DtaScript::computeReferenceMagicNumber(ScpStream& theStream, const std::string& sKey) {
		int iChar;
		int iOldChar = '\0';
		int iMagicNumber = 0;
		for(;;) {
			iChar = theStream.readChar();
			if (iChar == -1)  throw UtlException(theStream, "syntax error on '#reference' area: '#end " + sKey + "' expected before end of file");
			if ((iChar == '#') && (iOldChar == '\n')) {
				std::string sDirective;
				if (!theStream.readIdentifier(sDirective)) throw UtlException(theStream, "preprocessor directive expected after '#' symbol");
				if (sDirective == "reference") {
					theStream.skipLineBlanks();
					std::string sEmbeddedKey;
					if (!theStream.readIdentifier(sEmbeddedKey)) throw UtlException(theStream, "syntax error on '#reference': identifier expected");
					if (!theStream.findString("\n")) throw UtlException(theStream, "syntax error on '#reference " + sEmbeddedKey + "': end of line expected");
					int iEmbeddedMagicNumber = computeReferenceMagicNumber(theStream, sKey);
					iMagicNumber = (31*iMagicNumber + iEmbeddedMagicNumber) & 0x03FFFFFF;
				} else if (sDirective == "end") {
					theStream.skipLineBlanks();
					if (theStream.isEqualToIdentifier(sKey.c_str())) {
						if (!theStream.findString("\n")) throw UtlException(theStream, "syntax error on '#end " + sKey + "': end of line expected");
						break;
					}
				}
			} else {
				iMagicNumber = (31*iMagicNumber + iChar) & 0x03FFFFFF;
			}
			iOldChar = iChar;
		}
		std::string sFileName = theStream.getFilename();
		std::string::size_type iIndex = sFileName.find_last_of("/\\");
		if (iIndex != std::string::npos) sFileName = sFileName.substr(iIndex + 1);
		_mapOfReferenceMagicNumbers[sKey + ":" + sFileName] = iMagicNumber;
		return iMagicNumber;
	}

	bool DtaScript::extractReferenceMagicNumber(const std::string& sReferenceFile, const std::string& sKey, int& iMagicNumber) {
		std::auto_ptr<ScpStream> pScriptFile(new ScpStream(sReferenceFile, ScpStream::IN | ScpStream::PATH));
		while (pScriptFile->findString("#reference")) {
			int iLocation = pScriptFile->getInputLocation();
			bool bToHandle = (iLocation <= 10);
			if (!bToHandle) {
				pScriptFile->setInputLocation(iLocation - 11);
				bToHandle = (pScriptFile->readChar() == (int) '\n');
				pScriptFile->setInputLocation(iLocation);
			}
			if (bToHandle) {
				pScriptFile->skipLineBlanks();
				if (pScriptFile->isEqualToIdentifier(sKey.c_str())) {
					if (!pScriptFile->findString("\n")) return false;
					iMagicNumber = computeReferenceMagicNumber(*pScriptFile, sKey);
					return true;
				}
			}
		}
		return false;
	}
}
