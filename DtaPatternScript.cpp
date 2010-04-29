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

#include <stdlib.h>

#include "UtlException.h"
#include "UtlDate.h"
#include "ScpStream.h"

#include "ExprScriptVariable.h"
#include "DtaScriptVariable.h"
#include "DtaProject.h"
#include "GrfReadonlyHook.h"
#include "GrfNop.h"
#include "GrfSetAssignment.h"
#include "CGRuntime.h"

//##markup##"includes"
//##begin##"includes"
#include "GrfAllFloatingLocations.h"
#include "GrfAttachOutputToSocket.h"
#include "GrfDetachOutputFromSocket.h"
#include "GrfIncrementIndentLevel.h"
#include "GrfInsertText.h"
#include "GrfInsertTextOnce.h"
#include "GrfInsertTextToFloatingLocation.h"
#include "GrfInsertTextOnceToFloatingLocation.h"
#include "GrfOverwritePortion.h"
#include "GrfPopulateProtectedArea.h"
#include "GrfResizeOutputStream.h"
#include "GrfSetFloatingLocation.h"
#include "GrfSetOutputLocation.h"
#include "GrfSetProtectedArea.h"
#include "GrfWriteBytes.h"
#include "GrfWriteText.h"
#include "GrfWriteTextOnce.h"
//##end##"includes"

#include "GrfWritefileHook.h"
#include "GrfText.h"
#include "GrfGeneratedFile.h"
#include "GrfAppendedFile.h"
#include "GrfGeneratedString.h"
#include "DtaSharpTagsHandler.h"
#include "DtaProtectedAreasBag.h"
#include "GrfJointPoint.h"
#include "GrfJointPointCall.h"
#include "GrfAspectAdvice.h"
#include "DtaPatternScript.h"

namespace CodeWorker {
	class GrfAspectAdvice;

	class DtaAspectAdvices {
	private:
		std::vector<GrfAspectAdvice*> _before;
		std::vector<GrfAspectAdvice*> _around;
		std::vector<GrfAspectAdvice*> _after;
		std::vector<GrfAspectAdvice*> _beforeIteration;
		std::vector<GrfAspectAdvice*> _aroundIteration;
		std::vector<GrfAspectAdvice*> _afterIteration;

		friend class DtaOutputFile;

	public:
		inline DtaAspectAdvices() {}

		inline const std::vector<GrfAspectAdvice*>& before() const { return _before; }
		inline const std::vector<GrfAspectAdvice*>& around() const { return _around; }
		inline const std::vector<GrfAspectAdvice*>& after() const { return _after; }
		inline const std::vector<GrfAspectAdvice*>& beforeIteration() const { return _beforeIteration; }
		inline const std::vector<GrfAspectAdvice*>& aroundIteration() const { return _aroundIteration; }
		inline const std::vector<GrfAspectAdvice*>& afterIteration() const {return _afterIteration; }
	};


	int DtaOutputFile::_iNumberOfTemporaryFiles = 0;

	DtaOutputFile::DtaOutputFile(DtaPatternScript* pPatternScript) : _pInputStream(NULL), _pOutputStream(NULL), _pPatternScript(pPatternScript), _pOldOutputFile(NULL), _pOutputCoverage(NULL), _pAspectAdvices(NULL) {
		_pProtectedAreasBag = new DtaProtectedAreasBag;
		if (_pPatternScript != NULL) {
			_pOldOutputFile = _pPatternScript->_pOutputFile;
			_pPatternScript->_pOutputFile = this;
		}
		if (!DtaProject::getInstance().getSpeed()) {
			const char* sTemp = getenv("TEMP");
			if (sTemp == NULL) sTemp = getenv("TMP");
			if (sTemp != NULL) _sTemporaryDirectory = sTemp;
		}
	}

	DtaOutputFile::~DtaOutputFile() {
		if (_pPatternScript != NULL) {
			_pPatternScript->_pOutputFile = _pOldOutputFile;
		}
		delete _pProtectedAreasBag;
		delete _pAspectAdvices;
	}

	void DtaOutputFile::insertAspectAdvice(GrfAspectAdvice* pAdvice) {
		if (_pAspectAdvices == NULL) _pAspectAdvices = new DtaAspectAdvices;
		switch(pAdvice->getType()) {
			case GrfAspectAdvice::ADVICE_BEFORE: _pAspectAdvices->_before.push_back(pAdvice);break;
			case GrfAspectAdvice::ADVICE_AROUND: _pAspectAdvices->_around.push_back(pAdvice);break;
			case GrfAspectAdvice::ADVICE_AFTER: _pAspectAdvices->_after.push_back(pAdvice);break;
			case GrfAspectAdvice::ADVICE_BEFORE_ITERATION: _pAspectAdvices->_beforeIteration.push_back(pAdvice);break;
			case GrfAspectAdvice::ADVICE_AROUND_ITERATION: _pAspectAdvices->_aroundIteration.push_back(pAdvice);break;
			case GrfAspectAdvice::ADVICE_AFTER_ITERATION: _pAspectAdvices->_afterIteration.push_back(pAdvice);break;
		}
	}

	void DtaOutputFile::prepareCoverage(DtaScriptVariable& thisContext, int iTargetLocation) {
		if (_pPatternScript != NULL) {
			if (_pPatternScript->_pCoverageTree != NULL) {
				_pOutputCoverage = thisContext.getVariable(*_pPatternScript->_pCoverageTree);
				_pOutputCoverage->setValue(iTargetLocation);
			}
		}
	}

	GrfText* DtaOutputFile::createText(ScpStream& stream, const char* sText) const {
		GrfText* pText = new GrfText(&_pOutputCoverage, CGRuntime::_pOutputStream, sText);
		if ((_pPatternScript != NULL) && _pPatternScript->requiresParsingInformation()) {
			pText->setParsingInformation(_pPatternScript->getFilenamePtr(), stream);
		}
		return pText;
	}

	void DtaOutputFile::captureOutputFile(const char* tcFile) {
		if ((tcFile == NULL) || (_pPatternScript == NULL) || (_pPatternScript->getFilenamePtr() == NULL)) return;
		DtaProject::getInstance().captureOutputFile(tcFile, _pPatternScript->getFilenamePtr());
	}

	ScpStream* DtaOutputFile::openGenerate(bool bAutomatic, const char* sFile, ScpStream*& pOldOutputStream) {
		openAndloadProtectedCode(sFile);
		_pOutputStream->setParentStream(CGRuntime::_pOutputStream);
		if (bAutomatic) {
			pOldOutputStream = CGRuntime::_pOutputStream;
			CGRuntime::_pOutputStream = _pOutputStream;
			if (_pPatternScript != 0) CGRuntime::_listOfPatternScripts.push_front(_pPatternScript);
		}
		DtaProject& theProject = DtaProject::getInstance();
		if (!theProject.getGenerationHeader().empty()) {
			UtlDate today;
			(*_pOutputStream) << theProject.getCommentBegin() << "##generation header##";
			(*_pOutputStream) << CGRuntime::getApplicationName() << "##" << CGRuntime::getVersionNumber();
			(*_pOutputStream) << "##" << today.getFormattedDate("%d%b%Y %H:%M:%S") << "##\"";
			if (_pPatternScript->getFilenamePtr() != NULL) {
				(*_pOutputStream) << _pPatternScript->getFilenamePtr();
			}
			(*_pOutputStream) << "\"##";
			std::string sCommentEnd = theProject.getCommentEnd();
			if ((DtaProject::getInstance().getTextMode() == DtaProject::DOS_MODE) && (sCommentEnd == "\n")) sCommentEnd = "\r\n";
			else if (sCommentEnd.find('\n') == std::string::npos) sCommentEnd += CGRuntime::endl();
			std::string sHeader = theProject.getGenerationHeader();
			if (sHeader.find('\n') != std::string::npos) {
				sHeader = CGRuntime::replaceString("\r", "", sHeader);
				if (sHeader[sHeader.size() - 1] == '\n') sHeader = sHeader.substr(0, sHeader.size() - 1);
				(*_pOutputStream) << sCommentEnd << theProject.getCommentBegin() << "##header start##" << sCommentEnd;
				sHeader = theProject.getCommentBegin() + CGRuntime::replaceString("\n", sCommentEnd + theProject.getCommentBegin(), sHeader) + sCommentEnd;
				(*_pOutputStream) << sHeader;
				(*_pOutputStream) << theProject.getCommentBegin() << "##header end##" << sCommentEnd;
			} else {
				_pOutputStream->writeString(sHeader);
				(*_pOutputStream) << sCommentEnd;
			}
		}
		_iNumberOfTemporaryFiles++;
		return _pOutputStream;
	}

	ScpStream* DtaOutputFile::openAppend(bool bAutomatic, const char* sFile, ScpStream*& pOldOutputStream) {
		_pInputStream = ScpStream::openInputFile(sFile);
		_pOutputStream = new ScpStream;
		if (sFile != NULL) _pOutputStream->setFilename(sFile);
		if (_pInputStream != NULL) _pOutputStream->copy(*_pInputStream, 0);
		if (_pPatternScript != 0) _pPatternScript->allocateLocalParameters();
		if (bAutomatic) {
			pOldOutputStream = CGRuntime::_pOutputStream;
			CGRuntime::_pOutputStream = _pOutputStream;
			if (_pPatternScript != 0) CGRuntime::_listOfPatternScripts.push_front(_pPatternScript);
		}
		_iNumberOfTemporaryFiles++;
		return _pOutputStream;
	}

	std::string DtaOutputFile::catchGenerateExecution(bool bAutomatic, ScpStream* pOldOutputStream, UtlException* pCatchedException) {
		std::string sMessage;
		if (bAutomatic) {
			CGRuntime::_pOutputStream = pOldOutputStream;
			CGRuntime::_listOfPatternScripts.pop_front();
		}
		saveProtectedCodeAndClose();
		if (_pOutputStream != NULL) {
			delete _pOutputStream;
			_pOutputStream = NULL;
		}
		if (_pInputStream != NULL) {
			delete _pInputStream;
			_pInputStream = NULL;
		}
		_iNumberOfTemporaryFiles--;
		if (pCatchedException != NULL) {
			throw UtlException(pCatchedException->getTraceStack(), pCatchedException->getMessage());
		}
		return sMessage;
	}

	std::string DtaOutputFile::closeGenerate(bool bAutomatic, const char* sFile, ScpStream* pOldOutputStream) {
		std::string sOutputString;
		try {
			saveProtectedCodeAndClose();
			copyFromTemporaryFile(sFile);
			if (_pOutputStream != NULL) {
				if (sFile == NULL) sOutputString = _pOutputStream->readBuffer();
				_pOutputStream->close();
				delete _pOutputStream;
				_pOutputStream = NULL;
			}
			if (_pInputStream != NULL) {
				delete _pInputStream;
				_pInputStream = NULL;
			}
			_iNumberOfTemporaryFiles--;
		} catch(std::exception&) {
			if (bAutomatic) {
				CGRuntime::_pOutputStream = pOldOutputStream;
				CGRuntime::_listOfPatternScripts.pop_front();
			}
			if (_pOutputStream != NULL) {
				delete _pOutputStream;
				_pOutputStream = NULL;
			}
			if (_pInputStream != NULL) {
				delete _pInputStream;
				_pInputStream = NULL;
			}
			_iNumberOfTemporaryFiles--;
			throw;
		}
		if (bAutomatic) {
			CGRuntime::_pOutputStream = pOldOutputStream;
			CGRuntime::_listOfPatternScripts.pop_front();
		}
		return sOutputString;
	}

	// Loading all protected areas
	void DtaOutputFile::openAndloadProtectedCode(const char* sFile) {
		if (sFile == NULL) _pInputStream = new ScpStream;
		else {
			_pInputStream = ScpStream::openInputFile(sFile);
			if (_pInputStream != NULL) {
				try {
					getProtectedAreasBag().recoverProtectedCodes(*_pInputStream);
				} catch(UtlException& e) {
					std::string sMessage = sFile;
					sMessage += ", ";
					sMessage += e.getMessage();
					_pInputStream->close();
					delete _pInputStream;
					_pInputStream = NULL;
					throw UtlException(e.getTraceStack(), sMessage);
				} catch(std::exception&) {
					_pInputStream->close();
					delete _pInputStream;
					_pInputStream = NULL;
					throw;
				}
				_pInputStream->setInputLocation(0);
			}
		}
		_pOutputStream = new ScpStream;
		if (sFile != NULL) _pOutputStream->setFilename(sFile);
		if (_pPatternScript != 0) _pPatternScript->allocateLocalParameters();
	}

	// writing of protected areas that have been forgotten
	void DtaOutputFile::saveProtectedCode() {
		std::string sCommentEnd;
		if ((DtaProject::getInstance().getTextMode() == DtaProject::DOS_MODE) && (DtaProject::getInstance().getCommentEnd() == "\n")) sCommentEnd = "\r\n";
		else sCommentEnd = DtaProject::getInstance().getCommentEnd();
		bool bAtLeastOneFloatingCode = false;
		const std::map<std::string, DtaProtectedArea*>& listOfCodes = getProtectedAreasBag().getProtectedAreas();
		for (std::map<std::string, DtaProtectedArea*>::const_iterator i = listOfCodes.begin(); i != listOfCodes.end(); i++) {
			if (!i->second->isAlreadyGenerated()) {
				if (!bAtLeastOneFloatingCode) {
					bAtLeastOneFloatingCode = true;
					(*_pOutputStream) << DtaProject::getInstance().getCommentBegin();
					(*_pOutputStream) << "*********************************************************************" << sCommentEnd;
					(*_pOutputStream) << DtaProject::getInstance().getCommentBegin();
					(*_pOutputStream) << " Please find below the protected areas that the template-based script" << sCommentEnd;
					(*_pOutputStream) << DtaProject::getInstance().getCommentBegin();
					(*_pOutputStream) << " leading the generation hasn't recognized." << sCommentEnd;
					(*_pOutputStream) << DtaProject::getInstance().getCommentBegin();
					(*_pOutputStream) << "*********************************************************************" << sCommentEnd;
				}
				setProtectedArea(i->first);
			}
		}
		getProtectedAreasBag().clearAll();
	}

	void DtaOutputFile::saveProtectedCodeAndClose() {
		if (_pOutputStream != NULL) saveProtectedCode();
		else getProtectedAreasBag().clearAll();
		if (_pPatternScript != 0) {
			_pPatternScript->_bExecMode = false;
			_pPatternScript->freeLocalParameters();
		}
	}

	// Updating the old version of the file
	void DtaOutputFile::copyFromTemporaryFile(const char* sFile) {
		if (sFile == NULL) return;
		if (_pInputStream != NULL) {
			int iPosition;
			if (!DtaScript::equalsIgnoringGenerationHeader(*_pInputStream, *_pOutputStream, iPosition)) {
				if (!_sTemporaryDirectory.empty()) {
					char tcTemporaryFile[1024];
					sprintf(tcTemporaryFile, "%s/~generator%d.tmp", _sTemporaryDirectory.c_str(), _iNumberOfTemporaryFiles);
					_pOutputStream->saveIntoFile(tcTemporaryFile, false);
				}
				GrfWritefileHook* pHook = DtaProject::getInstance().getWritefileHook();
				if (pHook != NULL) {
					std::string sSuccess;
					try {
						sSuccess = pHook->executeHook(CGRuntime::getThisInternalNode(), sFile, iPosition, false);
					} catch(UtlException& exc) {
						throw UtlException(std::string("FATAL ERROR! Before writing file \"") + sFile + "\", a call to 'writefileHook' has raised the following error:" + CGRuntime::endl() + "\"" + exc.getMessage() + "\"");
					}
					if (sSuccess.empty()) return;
				}
				std::auto_ptr<ScpStream> pFile(ScpStream::createFile(sFile));
				if (pFile.get() == NULL) {
					std::auto_ptr<std::ifstream> pReadonlyFile(ScpStream::openSTLInputFile(sFile));
					if (pReadonlyFile.get() != NULL) {
						pReadonlyFile->close();
						GrfReadonlyHook* pHook = DtaProject::getInstance().getReadonlyHook();
						if (pHook != NULL) {
							try {
								pHook->executeHook(CGRuntime::getThisInternalNode(), sFile);
							} catch(UtlException& exc) {
								throw UtlException(std::string("FATAL ERROR! File \"") + sFile + "\" is read-only and call to the hook has launched the following error:\n\"" + exc.getMessage() + "\"");
							}
							std::auto_ptr<ScpStream> pFileAgain(ScpStream::createFile(sFile));
							pFile = pFileAgain;
							if (pFile.get() == NULL) {
								throw UtlException(std::string("FATAL ERROR! File \"") + sFile + "\" is read-only and call to the hook didn't change its state.");
							}
						} else {
							throw UtlException(std::string("FATAL ERROR! File \"") + sFile + "\" is read-only and no hook has been implemented for changing its state (see 'readonlyHook' instruction).");
						}
					} else {
						throw UtlException(std::string("Unable to open file \"") + sFile + "\" for writing, but should be ?!");
					}
				}
				try {
					_pOutputStream->saveIntoFile(sFile, false);
				} catch(...) {
					char tcTemporaryFile[1024];
					if (_sTemporaryDirectory.empty()) {
						const char* sTemp = getenv("TEMP");
						if (sTemp == NULL) sTemp = getenv("TMP");
						sprintf(tcTemporaryFile, "%s/~generator%d.tmp", sTemp, _iNumberOfTemporaryFiles);
						_pOutputStream->saveIntoFile(tcTemporaryFile, false);
					} else {
						sprintf(tcTemporaryFile, "%s/~generator%d.tmp", _sTemporaryDirectory.c_str(), _iNumberOfTemporaryFiles);
					}
					throw UtlException("DANGER! File error while replacing old version of file \"" + std::string(sFile) + "\" by the new one. Its content may be corrupted. If so, recover it by temporary file \"" + std::string(tcTemporaryFile) + "\".");
				}
			}
		} else {
			GrfWritefileHook* pHook = DtaProject::getInstance().getWritefileHook();
			if (pHook != NULL) {
				std::string sSuccess;
				try {
					sSuccess = pHook->executeHook(CGRuntime::getThisInternalNode(), sFile, 0, true);
				} catch(UtlException& exc) {
					throw UtlException(std::string("FATAL ERROR! Before creating file \"") + sFile + "\", a call to 'writefileHook' has raised the following error:" + CGRuntime::endl() + "\"" + exc.getMessage() + "\"");
				}
				if (sSuccess.empty()) return;
			}
			_pOutputStream->saveIntoFile(sFile, true);
		}
	}

	bool DtaOutputFile::createCoverageTreeForFinalInfo(const std::string& sAbsolutePath) {
		if ((_pPatternScript->_pCoverageTree == NULL) && ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_TEMPLATE_COVERAGE_FOR_EVERYBODY) != 0)) {
			// create a global variable expression for coverage recording
			// when final info requires coverage on generated files;
			// the variable expression points to an item whose key is the name
			// of the generated file.
			if (DtaProject::getInstance().getGlobalVariable("_FINAL_INFO_TEMPLATE_COVERAGE") == NULL) {
				DtaProject::getInstance().setGlobalVariable("_FINAL_INFO_TEMPLATE_COVERAGE");
			}
			_pPatternScript->_pCoverageTree = new ExprScriptVariable("_FINAL_INFO_TEMPLATE_COVERAGE");
			_pPatternScript->_pCoverageTree->setArrayKey(new ExprScriptConstant(sAbsolutePath.c_str()));
			return true;
		}
		return false;
	}

	void DtaOutputFile::restoreCoverageTree(DtaScriptVariable& thisContext, bool bCreateCoverageTreeForFinalInfo, const std::string& sAbsolutePath) {
		if (_pPatternScript->_pCoverageTree != NULL) {
			if (bCreateCoverageTreeForFinalInfo) {
				delete _pPatternScript->_pCoverageTree;
				_pPatternScript->_pCoverageTree = NULL;
			} else if ((DtaProject::getInstance().getFinalInfoFlag() & DtaProject::FINAL_INFO_TEMPLATE_COVERAGE) != 0) {
				//yes, the coverage tree must be copied for 'final info'
				DtaScriptVariable* pOutputCoverage = thisContext.getVariable(*_pPatternScript->_pCoverageTree);
				if (pOutputCoverage != NULL) {
					DtaScriptVariable* pGlobal = DtaProject::getInstance().getGlobalVariable("_FINAL_INFO_TEMPLATE_COVERAGE");
					if (pGlobal == NULL) {
						pGlobal = DtaProject::getInstance().setGlobalVariable("_FINAL_INFO_TEMPLATE_COVERAGE");
					}
					DtaScriptVariable* pStorage = pGlobal->addElement(sAbsolutePath);
					pStorage->copyAll(*pOutputCoverage);
				}
			}
		}
	}

	SEQUENCE_INTERRUPTION_LIST DtaOutputFile::generate(const char* tcFile, DtaScriptVariable& thisContext) {
		if (_pPatternScript == 0) throw UtlException("internal error in 'DtaOutputFile::generate()': the pattern script is null");
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		std::string sAbsolutePath;
		if (tcFile != NULL) {
			if (ScpStream::existInputFileFromIncludePath(tcFile, sAbsolutePath)) {
				if (!ScpStream::existVirtualFile(tcFile)) {
					sAbsolutePath = CGRuntime::canonizePath(sAbsolutePath);
				}
			} else {
				sAbsolutePath = CGRuntime::canonizePath(tcFile);
			}
		}
		bool bCreateCoverageTreeForFinalInfo = createCoverageTreeForFinalInfo(sAbsolutePath);
		ScpStream* pOldOutputStream;
		openGenerate(true, tcFile, pOldOutputStream);
		try {
			result = _pPatternScript->execute(thisContext);
			if (!sAbsolutePath.empty()) captureOutputFile(sAbsolutePath.c_str());
		} catch(UtlException& exception) {
			catchGenerateExecution(true, pOldOutputStream, &exception);
		} catch(std::exception&) {
			catchGenerateExecution(true, pOldOutputStream, 0);
			// restore coverage tree in error mode: minimal intervention
			if ((_pPatternScript->_pCoverageTree != NULL) && bCreateCoverageTreeForFinalInfo) {
				delete _pPatternScript->_pCoverageTree;
				_pPatternScript->_pCoverageTree = NULL;
			}
			throw;
		}
		closeGenerate(true, tcFile, pOldOutputStream);
		restoreCoverageTree(thisContext, bCreateCoverageTreeForFinalInfo, sAbsolutePath);
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST DtaOutputFile::generateString(std::string& sOutput, DtaScriptVariable& thisContext) {
		if (_pPatternScript == 0) throw UtlException("internal error in 'DtaOutputFile::generateString()': the pattern script is null");
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		ScpStream* pOldOutputStream;
		openGenerate(true, NULL, pOldOutputStream);
		(*_pInputStream) << sOutput;
		try {
			result = _pPatternScript->execute(thisContext);
			switch(result) {
				case CONTINUE_INTERRUPTION:
				case BREAK_INTERRUPTION:
				case RETURN_INTERRUPTION:
					result = NO_INTERRUPTION;
					break;
			}
		} catch(UtlException& exception) {
			catchGenerateExecution(true, pOldOutputStream, &exception);
		} catch(std::exception&) {
			catchGenerateExecution(true, pOldOutputStream, 0);
			throw;
		}
		sOutput = closeGenerate(true, NULL, pOldOutputStream);
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST DtaOutputFile::expand(const char* tcFile, DtaScriptVariable& thisContext) {
		if (_pPatternScript == 0) throw UtlException("internal error in 'DtaOutputFile::expand()': the pattern script is null");
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		_pInputStream = ScpStream::openInputFile(tcFile);
		if (_pInputStream != NULL) {
			std::string sAbsolutePath;
			if (tcFile != NULL) {
				if (ScpStream::existInputFileFromIncludePath(tcFile, sAbsolutePath)) {
					if (!ScpStream::existVirtualFile(tcFile)) {
						sAbsolutePath = CGRuntime::canonizePath(sAbsolutePath);
					}
				} else {
					sAbsolutePath = CGRuntime::canonizePath(tcFile);
				}
			}
			bool bCreateCoverageTreeForFinalInfo = createCoverageTreeForFinalInfo(sAbsolutePath);
			_pOutputStream = new ScpStream;
			if (tcFile != NULL) _pOutputStream->setFilename(tcFile);
			ScpStream* pOldOutputStream = CGRuntime::_pOutputStream;
			CGRuntime::_pOutputStream = _pOutputStream;
			CGRuntime::_listOfPatternScripts.push_front(_pPatternScript);
			int iEnd = 0;
			_pPatternScript->allocateLocalParameters();
			_iNumberOfTemporaryFiles++;
			if (!DtaProject::getInstance().getCommentBegin().empty()) {
				DtaSharpTagsHandler tagsHandler(_pInputStream);
				while (tagsHandler.findExpansionMarkup()) {
					try {
						// extracting the script/data blocks
						std::string sScript;
						std::string sData;
						while ((sScript.empty() && tagsHandler.readMarkupScript(sScript)) ||
								(sData.empty() && tagsHandler.readMarkupData(sData))) {
								// '##script##' or '##data##'
						}
						DtaProject::getInstance().setMarkupValue(sData);

						// copy of the source up to the last script/data block or announcement
						int iBegin = _pInputStream->getInputLocation();
						_pOutputStream->copy(*_pInputStream, iEnd, iBegin - iEnd);
						// extracting all protected areas, included between the current
						// position and the end of the markup definition
						getProtectedAreasBag().recoverMarker(tagsHandler);
						iEnd = _pInputStream->getInputLocation();
						std::auto_ptr<ScpStream> pOutputStream(_pOutputStream);
						_pOutputStream = new ScpStream;
						if (tcFile != NULL) _pOutputStream->setFilename(tcFile);
						_pOutputStream->setParentStream(pOutputStream.get());
						CGRuntime::_pOutputStream = _pOutputStream;
						tagsHandler.writeMarkupBegin(*pOutputStream);
						result = _pPatternScript->executeExpansion(sScript, thisContext, pOutputStream->getOutputLocation());
						saveProtectedCode();
						(*pOutputStream) << (*_pOutputStream);
						tagsHandler.writeMarkupEnd(*pOutputStream);
						delete _pOutputStream;
						_pOutputStream = pOutputStream.release();
						CGRuntime::_pOutputStream = NULL;
						_pInputStream->setInputLocation(iEnd);
					} catch(UtlException& e) {
						_pPatternScript->freeLocalParameters();
						_pInputStream->close();
						delete _pInputStream;
						_pInputStream = NULL;
						if (_pOutputStream != NULL) {
							delete _pOutputStream;
							_pOutputStream = NULL;
							CGRuntime::_pOutputStream = pOldOutputStream;
							CGRuntime::_listOfPatternScripts.pop_front();
						}
						_iNumberOfTemporaryFiles--;
						std::string sException = e.getMessage();
						std::string sMessage = "\"";
						if (_pPatternScript->getFilenamePtr() == NULL) sMessage += "(unknown)";
						else sMessage += _pPatternScript->getFilenamePtr();
						sMessage += "\":" + CGRuntime::endl() + sException;
						// restore coverage tree in error mode: minimal intervention
						if ((_pPatternScript->_pCoverageTree != NULL) && bCreateCoverageTreeForFinalInfo) {
							delete _pPatternScript->_pCoverageTree;
							_pPatternScript->_pCoverageTree = NULL;
						}
						throw UtlException(e.getTraceStack(), sMessage);
					} catch(std::exception&) {
						_pPatternScript->freeLocalParameters();
						_pInputStream->close();
						delete _pInputStream;
						_pInputStream = NULL;
						if (_pOutputStream != NULL) {
							delete _pOutputStream;
							_pOutputStream = NULL;
							CGRuntime::_pOutputStream = pOldOutputStream;
							CGRuntime::_listOfPatternScripts.pop_front();
						}
						_iNumberOfTemporaryFiles--;
						// restore coverage tree in error mode: minimal intervention
						if ((_pPatternScript->_pCoverageTree != NULL) && bCreateCoverageTreeForFinalInfo) {
							delete _pPatternScript->_pCoverageTree;
							_pPatternScript->_pCoverageTree = NULL;
						}
						throw;
					}
				}
			}
			_pPatternScript->freeLocalParameters();
			_pOutputStream->copy(*_pInputStream, iEnd);
			if (!sAbsolutePath.empty()) {
				copyFromTemporaryFile(sAbsolutePath.c_str());
			} else {
				copyFromTemporaryFile(tcFile);
			}
			_iNumberOfTemporaryFiles--;
			_pOutputStream->close();
			delete _pOutputStream;
			_pOutputStream = NULL;
			CGRuntime::_pOutputStream = pOldOutputStream;
			CGRuntime::_listOfPatternScripts.pop_front();
			restoreCoverageTree(thisContext, bCreateCoverageTreeForFinalInfo, sAbsolutePath);
			if (!sAbsolutePath.empty()) captureOutputFile(sAbsolutePath.c_str());
		} else throw UtlException("unable to open marked file \"" + std::string(tcFile) + "\"");
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	// Building a protected area at the current location of the output file //
	//////////////////////////////////////////////////////////////////////////
	//
	// Called by 'setProtectedArea' procedure.
	//////////////////////////////////////////////////////////////////////////

	std::string DtaOutputFile::getProtectedAreaIndentation() const {
		std::string sIndentation;
		int iInputPosition = _pOutputStream->getInputLocation();
		int iCurrent = _pOutputStream->getOutputLocation();
		while (iCurrent > 0) {
			iCurrent--;
			_pOutputStream->setInputLocation(iCurrent);
			int iChar = _pOutputStream->readChar();
			if (iChar == '\t') sIndentation = std::string("\t") + sIndentation;
			else if (iChar == '\r') {
				break;//sIndentation = std::string("\r") + sIndentation;
			} else if ((iChar == '\r') || (iChar == '\n')) break;
			else sIndentation = std::string(" ") + sIndentation;
		}
		_pOutputStream->setInputLocation(iInputPosition);
		return sIndentation;
	}

	void DtaOutputFile::populateProtectedArea(const std::string& sProtectedAreaName, const std::string& sContent) {
		getProtectedAreasBag().setProtection(sProtectedAreaName.c_str(), sContent.c_str());
		setProtectedArea(sProtectedAreaName);
	}

	void DtaOutputFile::setProtectedArea(const std::string& sProtectedAreaName) {
		std::string sIndentation = getProtectedAreaIndentation();
		const DtaProtectedArea& myProtectedArea = getProtectedAreasBag().registerNewProtection(sProtectedAreaName.c_str());
		DtaSharpTagsHandler tagsHandler(NULL);
		tagsHandler.writeProtectBegin(*_pOutputStream, sProtectedAreaName, myProtectedArea.getDefine());
		if (!myProtectedArea.getText().empty()) {
			if (!myProtectedArea.getDefine().empty()) writeTargetedArea(myProtectedArea.getText(), myProtectedArea.getDefine());
			else writeStandardArea(myProtectedArea.getText());
		}
		tagsHandler.writeProtectEnd(*_pOutputStream, sProtectedAreaName, myProtectedArea.getDefine());
	}

	bool DtaOutputFile::removeProtectedArea(const std::string& sProtectedAreaName) {
		return getProtectedAreasBag().removeProtection(sProtectedAreaName.c_str());
	}

	void DtaOutputFile::writeStandardArea(const std::string& sAreaCode) {
		DtaProject& myProject = DtaProject::getInstance();
		std::string sCommentBegin = myProject.getCommentBegin();
		std::string sCommentEnd = myProject.getCommentEnd();
		std::string::size_type iNextIndex = sAreaCode.find(sCommentBegin + "##");
		if (iNextIndex < 0) {
			(*_pOutputStream) << sAreaCode;
		} else {
			std::string::size_type iPreviousIndex = 0;
			do {
				std::string::size_type iEndIndex;
				bool bChange;
				const char* sCommand = sAreaCode.c_str() + (iNextIndex + 2/*sizeof("##")*/ + sCommentBegin.size());
				if (strncmp(sCommand, "ifdef ", 6) == 0) {
					int iIndex = iNextIndex + 8 + sCommentBegin.size();
					iEndIndex = sAreaCode.find("##", iIndex);
					std::string sTarget = sAreaCode.substr(iIndex, iEndIndex - iIndex);
					bChange = (!myProject.getDefineTarget(sTarget).empty());
				} else if (strncmp(sCommand, "ifndef ", 7) == 0) {
					int iIndex = iNextIndex + 9 + sCommentBegin.size();
					iEndIndex = sAreaCode.find("##", iIndex);
					std::string sTarget = sAreaCode.substr(iIndex, iEndIndex - iIndex);
					bChange = (myProject.getDefineTarget(sTarget).empty());
				} else if (strncmp(sCommand, "temporary", 9) == 0) {
					int iIndex = iNextIndex + 11 + sCommentBegin.size();
					iEndIndex = sAreaCode.find(sCommentBegin + "##temporary", iIndex);
					iEndIndex = sAreaCode.find(sCommentEnd, iEndIndex);
					std::string sText = sAreaCode.substr(iPreviousIndex, iNextIndex - iPreviousIndex);
					(*_pOutputStream) << sText;
					iPreviousIndex = iEndIndex + sCommentEnd.size();
					bChange = false;
				} else {
					bChange = false;
					iEndIndex = sAreaCode.find(sCommentEnd, iNextIndex + 2/*sizeof("##")*/ + sCommentBegin.size());
				}
				if (bChange) {
					iEndIndex += 2/*sizeof("##")*/;
					std::string sText = sAreaCode.substr(iPreviousIndex, iEndIndex - iPreviousIndex);
					(*_pOutputStream) << sText;
					iPreviousIndex = iEndIndex;
					iEndIndex = sAreaCode.find(sCommentEnd, iPreviousIndex);
					sText = sAreaCode.substr(iPreviousIndex, iEndIndex + sCommentEnd.size() - iPreviousIndex);
					(*_pOutputStream) << sText;
					(*_pOutputStream) << sCommentBegin + "##temporary BEGIN##" << sCommentEnd;
					(*_pOutputStream) << sText;
					(*_pOutputStream) << sCommentBegin + "##temporary END##" << sCommentEnd;
					iPreviousIndex = iEndIndex + sCommentEnd.size();
				}
				iNextIndex = sAreaCode.find(sCommentBegin + "##", iEndIndex);
			} while (iNextIndex != std::string::npos);
			if (iPreviousIndex < sAreaCode.size()) {
				std::string sText = sAreaCode.substr(iPreviousIndex);
				(*_pOutputStream) << sText;
			}
		}
	}

	void DtaOutputFile::writeTargetedArea(const std::string& sAreaCode, const std::string& sAreaTarget) {
		DtaProject& myProject = DtaProject::getInstance();
		if ((sAreaTarget[0] == '!' && myProject.getDefineTarget(sAreaTarget.substr(1)).empty()) ||
			(sAreaTarget[0] != '!' && !myProject.getDefineTarget(sAreaTarget).empty())) {
			writeStandardArea(sAreaCode);
		} else {
			std::string sCommentBegin = myProject.getCommentBegin();
			std::string sCommentEnd = myProject.getCommentEnd();
			std::string::size_type iPreviousIndex = 0;
			std::string::size_type iNextIndex = 0;
			do {
				if (strncmp(sAreaCode.c_str() + iNextIndex, sCommentBegin.c_str(), sCommentBegin.size()) != 0) {
					if (iNextIndex != iPreviousIndex) {
						std::string sText = sAreaCode.substr(iPreviousIndex, iNextIndex - iPreviousIndex);
						(*_pOutputStream) << sText;
					}
					(*_pOutputStream) << sCommentBegin << "##ifdef " << sAreaTarget << "##";
					iPreviousIndex = iNextIndex;
				}
				iNextIndex = sAreaCode.find(sCommentEnd, iNextIndex);
				iNextIndex += sCommentEnd.size();
			} while (iNextIndex < sAreaCode.size());
			if (iPreviousIndex < sAreaCode.size()) {
				std::string sText = sAreaCode.substr(iPreviousIndex);
				(*_pOutputStream) << sText;
			}
		}
	}

	std::list<std::string> DtaOutputFile::getProtectionKeys() const {
		return _pProtectedAreasBag->getProtectionKeys();
	}

	std::list<std::string> DtaOutputFile::remainingProtectionKeys() const {
		return _pProtectedAreasBag->remainingProtectionKeys();
	}

	bool DtaOutputFile::weaveBeforeIteration(GrfJointPoint& jointPoint, DtaScriptVariable& context) {
		bool bSuccess = false;
		if (_pAspectAdvices != NULL) {
			for (std::vector<GrfAspectAdvice*>::const_iterator i = _pAspectAdvices->beforeIteration().begin(); i != _pAspectAdvices->beforeIteration().end(); ++i) {
				if ((*i)->matchPointcut(jointPoint, context)) {
					bSuccess = true;
					(*i)->execute(context);
				}
			}
		}
		return bSuccess;
	}

	bool DtaOutputFile::weaveAroundIteration(GrfJointPoint& jointPoint, DtaScriptVariable& context) {
		bool bSuccess = false;
		if (_pAspectAdvices != NULL) {
			for (std::vector<GrfAspectAdvice*>::const_iterator i = _pAspectAdvices->aroundIteration().begin(); i != _pAspectAdvices->aroundIteration().end(); ++i) {
				if ((*i)->matchPointcut(jointPoint, context)) {
					bSuccess = true;
					(*i)->execute(context);
				}
			}
		}
		return bSuccess;
	}

	bool DtaOutputFile::weaveAfterIteration(GrfJointPoint& jointPoint, DtaScriptVariable& context) {
		bool bSuccess = false;
		if (_pAspectAdvices != NULL) {
			for (std::vector<GrfAspectAdvice*>::const_iterator i = _pAspectAdvices->afterIteration().begin(); i != _pAspectAdvices->afterIteration().end(); ++i) {
				if ((*i)->matchPointcut(jointPoint, context)) {
					bSuccess = true;
					(*i)->execute(context);
				}
			}
		}
		return bSuccess;
	}

	bool DtaOutputFile::weaveBefore(GrfJointPoint& jointPoint, DtaScriptVariable& context) {
		bool bSuccess = false;
		if (_pAspectAdvices != NULL) {
			for (std::vector<GrfAspectAdvice*>::const_iterator i = _pAspectAdvices->before().begin(); i != _pAspectAdvices->before().end(); ++i) {
				if ((*i)->matchPointcut(jointPoint, context)) {
					bSuccess = true;
					(*i)->execute(context);
				}
			}
		}
		return bSuccess;
	}

	bool DtaOutputFile::weaveAround(GrfJointPoint& jointPoint, DtaScriptVariable& context){
		bool bSuccess = false;
		if (_pAspectAdvices != NULL) {
			for (std::vector<GrfAspectAdvice*>::const_iterator i = _pAspectAdvices->around().begin(); i != _pAspectAdvices->around().end(); ++i) {
				if ((*i)->matchPointcut(jointPoint, context)) {
					bSuccess = true;
					(*i)->execute(context);
					break; // leave after encountering a 'around' for the first time
				}
			}
		}
		return bSuccess;
	}

	bool DtaOutputFile::weaveAfter(GrfJointPoint& jointPoint, DtaScriptVariable& context) {
		bool bSuccess = false;
		if (_pAspectAdvices != NULL) {
			for (std::vector<GrfAspectAdvice*>::const_iterator i = _pAspectAdvices->after().begin(); i != _pAspectAdvices->after().end(); ++i) {
				if ((*i)->matchPointcut(jointPoint, context)) {
					bSuccess = true;
					(*i)->execute(context);
				}
			}
		}
		return bSuccess;
	}


	//////////////////////////////////////////////////////////////////////////
	//                          A pattern script                            //
	//////////////////////////////////////////////////////////////////////////

	DtaPatternScript::DtaPatternScript(bool bExecMode) : DtaScript(NULL), _bExecMode(bExecMode), _executeFunction(NULL), _iExecStartLocation(-1), _pOutputFile(NULL), _pCoverageTree(NULL) {
		_pOutputFile = new DtaOutputFile(this);
		freeLocalParameters();
	}

	DtaPatternScript::DtaPatternScript(GrfBlock* pParentBlock, bool bExecMode) : DtaScript(pParentBlock), _bExecMode(bExecMode), _executeFunction(NULL), _iExecStartLocation(-1), _pOutputFile(NULL), _pCoverageTree(NULL) {
		_pOutputFile = new DtaOutputFile(this);
		freeLocalParameters();
	}

	DtaPatternScript::DtaPatternScript(EXECUTE_FUNCTION* executeFunction, bool bExecMode) : DtaScript(NULL), _bExecMode(bExecMode), _executeFunction(executeFunction), _iExecStartLocation(-1), _pOutputFile(NULL), _pCoverageTree(NULL) {
		_pOutputFile = new DtaOutputFile(this);
		freeLocalParameters();
	}

	DtaPatternScript::~DtaPatternScript() {
		delete _pOutputFile;
		delete _pCoverageTree;
	}

	DtaScriptFactory::SCRIPT_TYPE DtaPatternScript::getType() const { return DtaScriptFactory::PATTERN_SCRIPT; }
	bool DtaPatternScript::isAGenerateScript() const { return true; }

	void DtaPatternScript::traceEngine() const {
		if (getFilenamePtr() == NULL) CGRuntime::traceLine("template-based script (no filename):");
		else CGRuntime::traceLine("template-based script \"" + std::string(getFilenamePtr()) + "\":");
		traceInternalEngine();
	}

	DtaBNFScript& DtaPatternScript::getAlienParser() const {
		DtaBNFScript* pScript = DtaProject::getInstance().getTemplateAlienParser();
		if (pScript == NULL) throw UtlException("compiling a template script in " + _sTargetLanguage + " requires \"($CODEWORKER_HOME)/" + _sTargetLanguage + "/default-scripts/" + _sTargetLanguage + "TemplateScript.cwp\"");
		return *pScript;
	}


	/////////////////////////////////////////////////////////
	// Processing a 'generate' or 'expand' text generation //
	/////////////////////////////////////////////////////////
	SEQUENCE_INTERRUPTION_LIST DtaPatternScript::execute(DtaScriptVariable& thisContext) {
		if (_pCoverageTree != NULL) {
			CGThisModifier thisModifier(&thisContext);
			_pOutputFile->prepareCoverage(thisContext, 0);
		}
		return executeLight(thisContext);
	}

	SEQUENCE_INTERRUPTION_LIST DtaPatternScript::executeLight(DtaScriptVariable& thisContext) {
		if (_executeFunction == NULL) return DtaScript::execute(thisContext);
		CGThisModifier thisModifier(&thisContext);
		_executeFunction->run();
		return NO_INTERRUPTION;
	}

	SEQUENCE_INTERRUPTION_LIST DtaPatternScript::executeExpansion(const std::string& sScriptContent, DtaScriptVariable& thisContext, int iTargetLocation) {
		if (sScriptContent.empty()) {
			if (_pCoverageTree != NULL) {
				CGThisModifier thisModifier(&thisContext);
				_pOutputFile->prepareCoverage(thisContext, iTargetLocation);
			}
			return executeLight(thisContext);
		}
		ScpStream theStream;
		theStream << sScriptContent;
		DtaPatternScript theEmbeddedScript(&_graph, true);
		try {
			theEmbeddedScript.parseStream(theStream);
		} catch(UtlException& exception) {
			std::string sMessage = exception.getMessage() + CGRuntime::endl() + "... while expanding \"" + CGRuntime::composeCLikeString(DtaProject::getInstance().getMarkupKey()) + "\", in embedded script:" + CGRuntime::endl() + sScriptContent + CGRuntime::endl();
			throw UtlException(exception.getTraceStack(), sMessage);
		}
		return theEmbeddedScript.executeLight(thisContext);
	}


	////////////////////////////////////////////////////////////////////////////
	// Parsing of specific functions and procedure of 'generate' and 'expand' //
	////////////////////////////////////////////////////////////////////////////

	bool DtaPatternScript::betweenCommands(ScpStream& script, GrfBlock& block) {
		bool bNewCommand = false;
		if (_bExecMode) {
			script.skipEmpty();
			if (script.isEqualTo('@') || script.isEqualTo("%>")) _bExecMode = false;
		}
		if (!_bExecMode) {
			char sBuffer[16384];
			int iIndex = 0;
			int iChar = script.readChar();
			while (iChar > 0) {
				if (iChar == (int) '@') {
					_bExecMode = true;
					break;
				} else if (iChar == (int) '\\') {
					iChar = script.readChar();
					if ((iChar == (int) '@') || (iChar == (int) '<') || (iChar == (int) '%')) sBuffer[iIndex++] = (char) iChar;
					else {
						sBuffer[iIndex++] = '\\';
						if (iChar > 0) sBuffer[iIndex++] = (char) iChar;
					}
				} else if (iChar == (int) '<') {
					iChar = script.readChar();
					if (iChar == (int) '%') {
						_bExecMode = true;
						break;
					}
					script.goBack();
					sBuffer[iIndex++] = '<';
				} else {
					sBuffer[iIndex++] = (char) iChar;
				}
				if (iIndex >= 16384 - 2) {
					sBuffer[iIndex] = '\0';
					iIndex = 0;
					bNewCommand = true;
					block.add(_pOutputFile->createText(script, sBuffer));
				}
				iChar = script.readChar();
			}
			if (iIndex > 0) {
				sBuffer[iIndex] = '\0';
				bNewCommand = true;
				block.add(_pOutputFile->createText(script, sBuffer));
			}
			if (_bExecMode) {
				script.skipEmpty();
				_iExecStartLocation = script.getInputLocation();
				int a = script.peekChar();
				if ((a == (int) '$') || (a == (int) '(') || (a == (int) '"') || (a == (int) '\'')) {
					std::auto_ptr<ExprScriptExpression> pExpr(parseExpression(block, script));
					script.skipEmpty();
					if (!handleEmbeddedExpression(script, block, pExpr)) {
						throw UtlException(script, "expression (closed by '@' or '%>') or instruction expected behind a '@' or '<%'");
					}
				}
			}
		}
		return bNewCommand;
	}

	bool DtaPatternScript::handleEmbeddedExpression(ScpStream& script, GrfBlock& block, std::auto_ptr<ExprScriptExpression>& pExpr) {
		bool bEmbeddedExpression = script.isEqualTo('@');
		if (!bEmbeddedExpression) {
			bEmbeddedExpression = script.isEqualTo("%>");
			if (bEmbeddedExpression) script.goBack();
			else bEmbeddedExpression = !script.skipEmpty();
		}
		if (bEmbeddedExpression) {
			script.goBack();
			GrfWriteText* pWriteText = new GrfWriteText;
			pWriteText->setText(pExpr.release());
			if (requiresParsingInformation()) pWriteText->setParsingInformation(getFilenamePtr(), script);
			block.add(pWriteText);
			pWriteText->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
		}
		return bEmbeddedExpression;
	}

	void DtaPatternScript::handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block) {
		if (sCommand == "jointpoint") {
			parseJointPoint(block, script);
			return;
		} else if (sCommand == "advice") {
			parseAdvice(block, script);
			return;
		} else if (script.getInputLocation() - (int) sCommand.size() == _iExecStartLocation) {
			script.setInputLocation(_iExecStartLocation);
			std::auto_ptr<ExprScriptExpression> pExpr(parseKeyTemplateExpression(block, script));
			script.skipEmpty();
			if (script.isEqualTo(';')) {
				if (!pExpr->isAFunctionExpression()) {
					script.setInputLocation(_iExecStartLocation);
					throw UtlException(script, "unknown command or procedure or function \"" + sCommand + "\"");
				}
				GrfNop* pNop = new GrfNop;
				if (requiresParsingInformation()) pNop->setParsingInformation(getFilenamePtr(), script);
				block.add(pNop);
				pNop->setExpression(pExpr.release());
				return;
			} else if (script.isEqualTo('+') || script.isEqualTo('=')) {
				script.goBack();
				GrfSetAssignment* pAssignment = new GrfSetAssignment;
				if (requiresParsingInformation()) pAssignment->setParsingInformation(getFilenamePtr(), script);
				block.add(pAssignment);
				ExprScriptVariable* pVariable = dynamic_cast<ExprScriptVariable*>(pExpr.get());
				if (pVariable == NULL) throw UtlException(script, "variable expected on the left side of an assignment operator");
				pAssignment->setVariable(pVariable);
				pExpr.release();
				bool bConcat = script.isEqualTo('+');
				if (!script.isEqualTo('=')) throw UtlException(script, "syntax error: assignment expected ('+=' or '=' operator)");
				script.skipEmpty();
				pAssignment->setValue(parseExpression(block, script), bConcat);
				script.skipEmpty();
				if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
				return;
			} else {
				if (handleEmbeddedExpression(script, block, pExpr)) return;
			}
			script.setInputLocation(_iExecStartLocation);
		}
		DtaScript::handleUnknownCommand(sCommand, script, block);
	}

	void DtaPatternScript::handleNotAWordCommand(ScpStream& script, GrfBlock& block) {
		if (_bExecMode && (script.isEqualTo('@') || script.isEqualTo("%>"))) {
			_bExecMode = false;
			betweenCommands(script, block);
		} else {
			DtaScript::handleNotAWordCommand(script, block);
		}
	}

	void DtaPatternScript::parsePreprocessorDirective(const std::string& sDirective, ScpStream& script, GrfBlock& block) {
		if (sDirective == "#coverage") {
			script.skipEmpty();
			if (!script.isEqualTo('(')) throw UtlException(script, "syntax error, '(' expected");
			script.skipEmpty();
			if (_pCoverageTree != NULL) {
				delete _pCoverageTree;
				_pCoverageTree = NULL;
			}
			_pCoverageTree = parseVariableExpression(block, script);
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error, ')' expected");
		} else if (sDirective == "#jointpoint") {
			GrfJointPointCall* pCall = new GrfJointPointCall;
			if (requiresParsingInformation()) pCall->setParsingInformation(getFilenamePtr(), script);
			block.add(pCall);
			script.skipEmpty();
			if (script.isEqualTo('(')) {
				script.skipEmpty();
				pCall->setContext(parseVariableExpression(block, script));
				script.skipEmpty();
				if (!script.isEqualTo(')')) throw UtlException(script, "syntax error, ')' expected");
			}
		} else {
			DtaScript::parsePreprocessorDirective(sDirective, script, block);
		}
	}

//##markup##"parsing"
//##begin##"parsing"
void DtaPatternScript::parseAllFloatingLocations(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'allFloatingLocations' is available on pattern scripts only");
	GrfAllFloatingLocations* pAllFloatingLocations = new GrfAllFloatingLocations;
	if (requiresParsingInformation()) pAllFloatingLocations->setParsingInformation(getFilenamePtr(), script);
	block.add(pAllFloatingLocations);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pAllFloatingLocations->setList(parseVariableExpression(block, script));
	else pAllFloatingLocations->setList(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaPatternScript::parseAttachOutputToSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'attachOutputToSocket' is available on pattern scripts only");
	GrfAttachOutputToSocket* pAttachOutputToSocket = new GrfAttachOutputToSocket;
	if (requiresParsingInformation()) pAttachOutputToSocket->setParsingInformation(getFilenamePtr(), script);
	block.add(pAttachOutputToSocket);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pAttachOutputToSocket->setSocket(parseExpression(block, script));
	else pAttachOutputToSocket->setSocket(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaPatternScript::parseDetachOutputFromSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'detachOutputFromSocket' is available on pattern scripts only");
	GrfDetachOutputFromSocket* pDetachOutputFromSocket = new GrfDetachOutputFromSocket;
	if (requiresParsingInformation()) pDetachOutputFromSocket->setParsingInformation(getFilenamePtr(), script);
	block.add(pDetachOutputFromSocket);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pDetachOutputFromSocket->setSocket(parseExpression(block, script));
	else pDetachOutputFromSocket->setSocket(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaPatternScript::parseIncrementIndentLevel(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'incrementIndentLevel' is available on pattern scripts only");
	GrfIncrementIndentLevel* pIncrementIndentLevel = new GrfIncrementIndentLevel;
	if (requiresParsingInformation()) pIncrementIndentLevel->setParsingInformation(getFilenamePtr(), script);
	block.add(pIncrementIndentLevel);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	do {
		if (pMethodCaller == NULL) { if (script.peekChar() != ')') pIncrementIndentLevel->setLevel(parseExpression(block, script)); }
		else pIncrementIndentLevel->setLevel(pMethodCaller);
	} while (false);
	pIncrementIndentLevel->populateDefaultParameters();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaPatternScript::parseInsertText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'insertText' is available on pattern scripts only");
	GrfInsertText* pInsertText = new GrfInsertText;
	if (requiresParsingInformation()) pInsertText->setParsingInformation(getFilenamePtr(), script);
	block.add(pInsertText);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pInsertText->setLocation(parseExpression(block, script));
	else pInsertText->setLocation(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pInsertText->setText(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parseInsertText"
	pInsertText->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parseInsertText"
}

void DtaPatternScript::parseInsertTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'insertTextOnce' is available on pattern scripts only");
	GrfInsertTextOnce* pInsertTextOnce = new GrfInsertTextOnce;
	if (requiresParsingInformation()) pInsertTextOnce->setParsingInformation(getFilenamePtr(), script);
	block.add(pInsertTextOnce);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pInsertTextOnce->setLocation(parseExpression(block, script));
	else pInsertTextOnce->setLocation(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pInsertTextOnce->setText(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parseInsertTextOnce"
	pInsertTextOnce->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parseInsertTextOnce"
}

void DtaPatternScript::parseInsertTextToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'insertTextToFloatingLocation' is available on pattern scripts only");
	GrfInsertTextToFloatingLocation* pInsertTextToFloatingLocation = new GrfInsertTextToFloatingLocation;
	if (requiresParsingInformation()) pInsertTextToFloatingLocation->setParsingInformation(getFilenamePtr(), script);
	block.add(pInsertTextToFloatingLocation);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pInsertTextToFloatingLocation->setLocation(parseExpression(block, script));
	else pInsertTextToFloatingLocation->setLocation(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pInsertTextToFloatingLocation->setText(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parseInsertTextToFloatingLocation"
	pInsertTextToFloatingLocation->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parseInsertTextToFloatingLocation"
}

void DtaPatternScript::parseInsertTextOnceToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'insertTextOnceToFloatingLocation' is available on pattern scripts only");
	GrfInsertTextOnceToFloatingLocation* pInsertTextOnceToFloatingLocation = new GrfInsertTextOnceToFloatingLocation;
	if (requiresParsingInformation()) pInsertTextOnceToFloatingLocation->setParsingInformation(getFilenamePtr(), script);
	block.add(pInsertTextOnceToFloatingLocation);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pInsertTextOnceToFloatingLocation->setLocation(parseExpression(block, script));
	else pInsertTextOnceToFloatingLocation->setLocation(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pInsertTextOnceToFloatingLocation->setText(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parseInsertTextOnceToFloatingLocation"
	pInsertTextOnceToFloatingLocation->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parseInsertTextOnceToFloatingLocation"
}

void DtaPatternScript::parseOverwritePortion(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'overwritePortion' is available on pattern scripts only");
	GrfOverwritePortion* pOverwritePortion = new GrfOverwritePortion;
	if (requiresParsingInformation()) pOverwritePortion->setParsingInformation(getFilenamePtr(), script);
	block.add(pOverwritePortion);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pOverwritePortion->setLocation(parseExpression(block, script));
	else pOverwritePortion->setLocation(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pOverwritePortion->setText(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pOverwritePortion->setSize(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parseOverwritePortion"
	pOverwritePortion->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parseOverwritePortion"
}

void DtaPatternScript::parsePopulateProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'populateProtectedArea' is available on pattern scripts only");
	GrfPopulateProtectedArea* pPopulateProtectedArea = new GrfPopulateProtectedArea;
	if (requiresParsingInformation()) pPopulateProtectedArea->setParsingInformation(getFilenamePtr(), script);
	block.add(pPopulateProtectedArea);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pPopulateProtectedArea->setProtectedAreaName(parseExpression(block, script));
	else pPopulateProtectedArea->setProtectedAreaName(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pPopulateProtectedArea->setContent(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parsePopulateProtectedArea"
	pPopulateProtectedArea->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parsePopulateProtectedArea"
}

void DtaPatternScript::parseResizeOutputStream(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'resizeOutputStream' is available on pattern scripts only");
	GrfResizeOutputStream* pResizeOutputStream = new GrfResizeOutputStream;
	if (requiresParsingInformation()) pResizeOutputStream->setParsingInformation(getFilenamePtr(), script);
	block.add(pResizeOutputStream);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pResizeOutputStream->setNewSize(parseExpression(block, script));
	else pResizeOutputStream->setNewSize(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaPatternScript::parseSetFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'setFloatingLocation' is available on pattern scripts only");
	GrfSetFloatingLocation* pSetFloatingLocation = new GrfSetFloatingLocation;
	if (requiresParsingInformation()) pSetFloatingLocation->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetFloatingLocation);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetFloatingLocation->setKey(parseExpression(block, script));
	else pSetFloatingLocation->setKey(pMethodCaller);
	if ((pMethodCaller == NULL) && !script.isEqualTo(',')) throw UtlException(script, "syntax error: ',' expected");
	script.skipEmpty();
	pSetFloatingLocation->setLocation(parseExpression(block, script));
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaPatternScript::parseSetOutputLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'setOutputLocation' is available on pattern scripts only");
	GrfSetOutputLocation* pSetOutputLocation = new GrfSetOutputLocation;
	if (requiresParsingInformation()) pSetOutputLocation->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetOutputLocation);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetOutputLocation->setLocation(parseExpression(block, script));
	else pSetOutputLocation->setLocation(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaPatternScript::parseSetProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'setProtectedArea' is available on pattern scripts only");
	GrfSetProtectedArea* pSetProtectedArea = new GrfSetProtectedArea;
	if (requiresParsingInformation()) pSetProtectedArea->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetProtectedArea);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetProtectedArea->setProtectedAreaName(parseExpression(block, script));
	else pSetProtectedArea->setProtectedAreaName(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parseSetProtectedArea"
	pSetProtectedArea->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parseSetProtectedArea"
}

void DtaPatternScript::parseWriteBytes(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'writeBytes' is available on pattern scripts only");
	GrfWriteBytes* pWriteBytes = new GrfWriteBytes;
	if (requiresParsingInformation()) pWriteBytes->setParsingInformation(getFilenamePtr(), script);
	block.add(pWriteBytes);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pWriteBytes->setBytes(parseExpression(block, script));
	else pWriteBytes->setBytes(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parseWriteBytes"
	pWriteBytes->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parseWriteBytes"
}

void DtaPatternScript::parseWriteText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'writeText' is available on pattern scripts only");
	GrfWriteText* pWriteText = new GrfWriteText;
	if (requiresParsingInformation()) pWriteText->setParsingInformation(getFilenamePtr(), script);
	block.add(pWriteText);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pWriteText->setText(parseExpression(block, script));
	else pWriteText->setText(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parseWriteText"
	pWriteText->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parseWriteText"
}

void DtaPatternScript::parseWriteTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	if (!isAGenerateScript()) throw UtlException(script, "command 'writeTextOnce' is available on pattern scripts only");
	GrfWriteTextOnce* pWriteTextOnce = new GrfWriteTextOnce;
	if (requiresParsingInformation()) pWriteTextOnce->setParsingInformation(getFilenamePtr(), script);
	block.add(pWriteTextOnce);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pWriteTextOnce->setText(parseExpression(block, script));
	else pWriteTextOnce->setText(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
//##protect##"parseWriteTextOnce"
	pWriteTextOnce->prepareCoverage(_pOutputFile->getOutputCoveragePtr());
//##protect##"parseWriteTextOnce"
}

//##end##"parsing"

	void DtaPatternScript::parseJointPoint(GrfBlock& block, ScpStream& script) {
		script.skipEmpty();
		bool bIterate = script.isEqualToIdentifier("iterate");
		script.skipEmpty();
		std::string sName;
		if (!script.readIdentifier(sName)) throw UtlException(script, "syntax error: joint point identifier expected");
		GrfJointPoint* pJointPoint = new GrfJointPoint(this, &block, sName);
		if (requiresParsingInformation()) pJointPoint->setParsingInformation(getFilenamePtr(), script);
		block.add(pJointPoint);
		pJointPoint->isIterate(bIterate);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pJointPoint->setContext(parseVariableExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
			script.skipEmpty();
		}
		if (!script.isEqualTo(';')) {
			parseBlock(script, *pJointPoint);
		}
	}

	void DtaPatternScript::parseAdvice(GrfBlock& block, ScpStream& script) {
		script.skipEmpty();
		std::string sType;
		if (!script.readIdentifier(sType)) throw UtlException(script, "syntax error: advice type expected");
		GrfAspectAdvice::ADVICE_TYPE theType;
		if (sType == "before") theType = GrfAspectAdvice::ADVICE_BEFORE;
		else if (sType == "before_iteration") theType = GrfAspectAdvice::ADVICE_BEFORE_ITERATION;
		else if (sType == "around") theType = GrfAspectAdvice::ADVICE_AROUND;
		else if (sType == "around_iteration") theType = GrfAspectAdvice::ADVICE_AROUND_ITERATION;
		else if (sType == "after_iteration") theType = GrfAspectAdvice::ADVICE_AFTER_ITERATION;
		else if (sType == "after") theType = GrfAspectAdvice::ADVICE_AFTER;
		script.skipEmpty();
		std::string sJointPointName;
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			if (!script.readIdentifier(sJointPointName)) throw UtlException(script, "syntax error: joint point identifier expected");
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
			script.skipEmpty();
		}
		GrfAspectAdvice* pAdvice = new GrfAspectAdvice(this, &block, theType, sJointPointName);
		if (requiresParsingInformation()) pAdvice->setParsingInformation(getFilenamePtr(), script);
		_pOutputFile->insertAspectAdvice(pAdvice);
		if (!script.isEqualTo(':')) throw UtlException(script, "syntax error: ':' expected");
		script.skipEmpty();
		block.addLocalVariable("jointpoint", VALUE_EXPRTYPE);
		block.addLocalVariable("context", REFERENCE_EXPRTYPE);
		pAdvice->setPointcut(parseExpression(block, script));
		script.skipEmpty();
		parseBlock(script, *pAdvice);
	}

	void DtaPatternScript::parseGeneratedFile(GrfBlock& block, ScpStream& script) {
		if (!isAGenerateScript()) throw UtlException(script, "statement modifier 'generated_file' is available on pattern scripts only");
		GrfGeneratedFile* pGeneratedFileBlock = new GrfGeneratedFile(this, &block);
		if (requiresParsingInformation()) pGeneratedFileBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pGeneratedFileBlock);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pGeneratedFileBlock->setOutputFile(parseExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		}
		parseBlock(script, *pGeneratedFileBlock);
	}

	void DtaPatternScript::parseGeneratedString(GrfBlock& block, ScpStream& script) {
		if (!isAGenerateScript()) throw UtlException(script, "statement modifier 'generated_string' is available on pattern scripts only");
		GrfGeneratedString* pGeneratedStringBlock = new GrfGeneratedString(this, &block);
		if (requiresParsingInformation()) pGeneratedStringBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pGeneratedStringBlock);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pGeneratedStringBlock->setOutputString(parseVariableExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected after the variable");
		}
		parseBlock(script, *pGeneratedStringBlock);
	}

	void DtaPatternScript::parseAppendedFile(GrfBlock& block, ScpStream& script) {
		if (!isAGenerateScript()) throw UtlException(script, "statement modifier 'Append_file' is available on pattern scripts only");
		GrfAppendedFile* pAppendedFileBlock = new GrfAppendedFile(this, &block);
		if (requiresParsingInformation()) pAppendedFileBlock->setParsingInformation(getFilenamePtr(), script);
		block.add(pAppendedFileBlock);
		script.skipEmpty();
		if (script.isEqualTo('(')) {
			script.skipEmpty();
			pAppendedFileBlock->setOutputFile(parseExpression(block, script));
			script.skipEmpty();
			if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
		}
		parseBlock(script, *pAppendedFileBlock);
	}
}
