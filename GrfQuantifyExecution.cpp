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
#pragma warning(disable: 4503)
#pragma warning (disable : 4786)
#endif

#include <math.h>
#include <fstream>

#include "ScpStream.h"
#include "UtlException.h"
#include "UtlTimer.h"

#ifndef WIN32
#	include "UtlString.h" // for Debian/gcc 2.95.4
#endif

#include "CGRuntime.h"
#include "DtaScript.h"
#include "DtaProject.h"
#include "ExprScriptFunction.h"
#include "GrfFunction.h"
#include "BNFClauseCall.h"
#include "GrfQuantifyExecution.h"

namespace CodeWorker {
	bool DtaQuantifyFunction::operator < (const DtaQuantifyFunction& function) const {
		return (_sName < function._sName);
	}

	bool DtaQuantifyFunction::operator > (const DtaQuantifyFunction& function) const {
		return (_sName > function._sName);
	}

	bool DtaQuantifyFunction::operator ==(const DtaQuantifyFunction& function) const {
		return (_sName == function._sName) && (_sFile == function._sFile) && (_iLocation == function._iLocation);
	}

	bool DtaQuantifyFunction::operator !=(const DtaQuantifyFunction& function) const {
		return !((*this) == function);
	}


	GrfQuantifyExecution::~GrfQuantifyExecution() {
		delete _pFilename;
	}

	void GrfQuantifyExecution::handleBeforeExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/) {
		incrementCounter(pCommand);
	}

	void GrfQuantifyExecution::handleAfterExecutionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/) {}
	void GrfQuantifyExecution::handleAfterExceptionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/, UtlException& /*exception*/) {}

	void GrfQuantifyExecution::handleStartingFunction(GrfFunction* pFunction) {
		GrfExecutionContext::handleStartingFunction(pFunction);
		pFunction->startTimer();
	}

	void GrfQuantifyExecution::handleEndingFunction(GrfFunction* pFunction) {
		pFunction->stopTimer();
		GrfExecutionContext::handleEndingFunction(pFunction);
	}

	void GrfQuantifyExecution::handleStartingBNFClause(BNFClauseCall* pBNFClause) {
		GrfExecutionContext::handleStartingBNFClause(pBNFClause);
		pBNFClause->startTimer();
	}

	void GrfQuantifyExecution::handleEndingBNFClause(BNFClauseCall* pBNFClause) {
		pBNFClause->stopTimer();
		GrfExecutionContext::handleEndingBNFClause(pBNFClause);
	}

	SEQUENCE_INTERRUPTION_LIST GrfQuantifyExecution::openSession(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		CGRuntime::traceLine("-- quantify session --");
		std::string sFilename;
		try {
			_listOfPredefinedFunctions = std::map<std::string, int>();
			_listOfUserFunctions = std::map<std::string, std::map<std::string, DtaQuantifyFunction> >();
			ExprScriptFunction::clearCounters();
			_iCoveredCode = 0;
			_iTotalCode = 0;
			if (_pFilename != NULL) {
				sFilename = _pFilename->getValue(visibility);
				if (sFilename.size() < 5) throw UtlException("HTML file must have extension '.htm' or '.html'");
				std::string sExtension = sFilename.substr(sFilename.size() - 5);
				if ((stricmp(sExtension.c_str(), ".html") != 0) && (stricmp(sExtension.c_str() + 1, ".htm") != 0)) throw UtlException("HTML file must have extension '.htm' or '.html'");
			}
			UtlTimer myTimer;
			myTimer.start();
			result = GrfBlock::executeInternal(visibility);
			myTimer.stop();
			char tcMessage[80];
			sprintf(tcMessage, "quantify execution time = %dms", myTimer.getTimeInMillis());
			CGRuntime::traceLine(tcMessage);
		} catch(UtlException&/* exception*/) {
			CGRuntime::traceLine("-- quantify session interrupted by an exception --");
			throw/* UtlException(exception)*/;
		}
		applyRecursively(recoverData);
		if (sFilename.empty()) displayResults(visibility);
		else generateHTMLFile(sFilename.c_str(), visibility);
		CGRuntime::traceLine("-- end of quantify session --");
		return result;
	}

	void GrfQuantifyExecution::handleBeforeScriptExecutionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/) {}

	void GrfQuantifyExecution::handleAfterScriptExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/) {
		pCommand->applyRecursively(recoverData);
	}

	void GrfQuantifyExecution::recoverData(GrfCommand* pCommand) {
		if (pCommand->getFunctionName() != NULL) {
			if (pCommand->isAPredefinedFunction()) getCurrentQuantify()->registerPredefinedFunction(pCommand);
			else getCurrentQuantify()->registerUserFunction((GrfFunction*) pCommand);
		}
		getCurrentQuantify()->registerCode(pCommand);
	}

	void GrfQuantifyExecution::registerCode(GrfCommand* pCommand) {
		if (getParsingFilePtr(pCommand) != NULL) {
			_iTotalCode++;
			if (getCounter(pCommand) > 0) _iCoveredCode++;
			_coveredLines[getParsingFilePtr(pCommand)][getFileLocation(pCommand)] = getCounter(pCommand);
		}
	}

	void GrfQuantifyExecution::registerUserFunction(GrfFunction* pFunction) {
		if (getParsingFilePtr(pFunction) != NULL) {
			std::map<std::string, std::map<std::string, DtaQuantifyFunction> >::iterator iterateFile = _listOfUserFunctions.find(getParsingFilePtr(pFunction));
			if (iterateFile != _listOfUserFunctions.end()) {
				std::map<std::string, DtaQuantifyFunction>::iterator iterateName = iterateFile->second.find(pFunction->getFunctionName());
				if (iterateName != iterateFile->second.end()) {
					iterateName->second._iCounter = getCounter(pFunction);
					iterateName->second._iTimeInMillis = getTimeInMillis(pFunction);
				} else {
					DtaQuantifyFunction& myFunction = iterateFile->second[pFunction->getFunctionName()];
					myFunction._sName = pFunction->getFunctionName();
					myFunction._sFile = getParsingFilePtr(pFunction);
					myFunction._iLocation = getFileLocation(pFunction);
					myFunction._iCounter = getCounter(pFunction);
					myFunction._iTimeInMillis = getTimeInMillis(pFunction);
				}
			} else {
				DtaQuantifyFunction& myFunction = _listOfUserFunctions[getParsingFilePtr(pFunction)][pFunction->getFunctionName()];
				myFunction._sName = pFunction->getFunctionName();
				myFunction._sFile = getParsingFilePtr(pFunction);
				myFunction._iLocation = getFileLocation(pFunction);
				myFunction._iCounter = getCounter(pFunction);
				myFunction._iTimeInMillis = getTimeInMillis(pFunction);
			}
		}
	}

	void GrfQuantifyExecution::registerPredefinedFunction(GrfCommand* pCommand) {
		_listOfPredefinedFunctions[pCommand->getFunctionName()] = _listOfPredefinedFunctions[pCommand->getFunctionName()] + getCounter(pCommand);
	}

	void GrfQuantifyExecution::displayResults(DtaScriptVariable& visibility) {
		if (!_listOfUserFunctions.empty()) {
			CGRuntime::traceLine("User defined functions:");
			for (std::map<std::string, std::map<std::string, DtaQuantifyFunction> >::iterator i = _listOfUserFunctions.begin(); i != _listOfUserFunctions.end(); i++) {
				std::string sCompleteFileName;
				std::ifstream* pFile = openInputFileFromIncludePath(i->first.c_str(), sCompleteFileName);
				for (std::map<std::string, DtaQuantifyFunction>::iterator j = i->second.begin(); j != i->second.end(); j++) {
					CGRuntime::traceText("  " + j->second._sName + "(...) file \"" + j->second._sFile);
					if (pFile != NULL) {
						setLocation(*pFile, j->second._iLocation);
						char tcMessage[40];
						sprintf(tcMessage, "\" at %d", getLineCount(*pFile));
						CGRuntime::traceText(tcMessage);
					}
					char tcMessage[80];
					sprintf(tcMessage, ": %d occurences in %dms", j->second._iCounter, j->second._iTimeInMillis);
					CGRuntime::traceLine(tcMessage);
				}
				if (pFile != NULL) {
					pFile->close();
					delete pFile;
				}
			}
		}
		CGRuntime::traceLine("Predefined functions:");
		for (std::map<std::string, DtaFunctionInfo>::iterator i = ExprScriptFunction::getFunctionRegister().begin(); i != ExprScriptFunction::getFunctionRegister().end(); i++) {
			if (*(i->second.pCounter) > 0) {
				CGRuntime::traceText("  " + i->first + "(...): ");
				char tcMessage[40];
				sprintf(tcMessage, "%d occurrences", *(i->second.pCounter));
				CGRuntime::traceLine(tcMessage);
			}
		}
		if (!_listOfPredefinedFunctions.empty()) {
			CGRuntime::traceLine("Procedures:");
			for (std::map<std::string, int>::iterator i = _listOfPredefinedFunctions.begin(); i != _listOfPredefinedFunctions.end(); i++) {
				CGRuntime::traceText("  " + i->first + "(...): ");
				char tcMessage[40];
				sprintf(tcMessage, "%d occurrences", i->second);
				CGRuntime::traceLine(tcMessage);
			}
		}
		double dCoveredCode = getCoveredCode();
		double dTotalCode = getTotalCode();
		double dPercentCovered = floor(100.0 * dCoveredCode / dTotalCode);
		char tcMessage[80];
		sprintf(tcMessage, "Covered source code: %d%%", (int) dPercentCovered);
		CGRuntime::traceLine(tcMessage);
	}

	void GrfQuantifyExecution::generateHTMLFile(const char* sFilename, DtaScriptVariable& visibility) {
		static const char* tColorArray[6][6]={{"#ff0000", "#0000ff", "#0000ff", "#0000ff", "#0000ff", "#0000ff"},
										{"#0000ff", "#ff0000", "#ffff00", "#ffff00", "#00ff00",	"#00ff00"},
										{"#0000ff", "#ffff00", "#ff0000", "#ff8000", "#ffff00", "#ffff00"},
										{"#0000ff", "#ffff00", "#ff8000", "#ff0000", "#ff8000", "#ff8000"},
										{"#0000ff", "#00ff00", "#ffff00", "#ff8000", "#ff0000", "#ff00ff"},
										{"#0000ff", "#00ff00", "#ffff00", "#ff8000", "#ff00ff", "#ff0000"}};
		if ((sFilename != NULL) && (sFilename[0] != '\0')) {
			std::fstream* pHTMLFile = DtaScript::openOutputFile(sFilename, true);
			CGRuntime::traceLine("Generating HTML file \"" + std::string(sFilename) + "\"");
			(*pHTMLFile) << "<HTML>" << std::endl;
			(*pHTMLFile) << "<HEAD>" << std::endl;
			(*pHTMLFile) << "<TITLE>Covered source code</TITLE>" << std::endl;
			(*pHTMLFile) << "</HEAD>" << std::endl;
			(*pHTMLFile) << "<BODY>" << std::endl;
			// generates predefined functions occurences
			bool bInitialized = false;
			for (std::map<std::string, DtaFunctionInfo>::iterator i1 = ExprScriptFunction::getFunctionRegister().begin(); i1 != ExprScriptFunction::getFunctionRegister().end(); i1++) {
				if (*(i1->second.pCounter) > 0) {
					if (!bInitialized) {
						bInitialized = true;
						(*pHTMLFile) << "<H1>Predefined functions:</H1>" << std::endl;
						(*pHTMLFile) << "<TABLE BORDER CELLSPACING=1 CELLPADDING=7 WIDTH=360>" << std::endl;
						(*pHTMLFile) << "  <TR>" << std::endl;
						(*pHTMLFile) << "    <TD WIDTH=\"69%\" VALIGN=\"TOP\"><B><FONT SIZE=3><P ALIGN=\"CENTER\">Function name</B></FONT></TD>" << std::endl;
						(*pHTMLFile) << "    <TD WIDTH=\"31%\" VALIGN=\"TOP\"><B><FONT SIZE=3><P ALIGN=\"CENTER\">Occurence</B></FONT></TD>" << std::endl;
						(*pHTMLFile) << "  </TR>" << std::endl;
					}
					(*pHTMLFile) << "  <TR>" << std::endl;
					(*pHTMLFile) << "    <TD WIDTH=\"69%\" VALIGN=\"TOP\"><B><FONT SIZE=2><P>" << i1->first << "(...) " << "</B></FONT></TD>" << std::endl;
					(*pHTMLFile) << "    <TD WIDTH=\"31%\" VALIGN=\"TOP\"><FONT SIZE=2><P>" << *(i1->second.pCounter) << "</FONT></TD>" << std::endl;
					(*pHTMLFile) << "  </TR>" << std::endl;
				}
			}
			if (bInitialized) (*pHTMLFile) << "</TABLE>" << std::endl;
			// generates instructions occurrences
			if (!_listOfPredefinedFunctions.empty()) {
				(*pHTMLFile) << "<H1>Procedures:</H1>" << std::endl;
				(*pHTMLFile) << "<TABLE BORDER CELLSPACING=1 CELLPADDING=7 WIDTH=360>" << std::endl;
				(*pHTMLFile) << "  <TR>" << std::endl;
				(*pHTMLFile) << "    <TD WIDTH=\"69%\" VALIGN=\"TOP\"><B><FONT SIZE=3><P ALIGN=\"CENTER\">Instruction name</B></FONT></TD>" << std::endl;
				(*pHTMLFile) << "    <TD WIDTH=\"31%\" VALIGN=\"TOP\"><B><FONT SIZE=3><P ALIGN=\"CENTER\">Occurence</B></FONT></TD>" << std::endl;
				(*pHTMLFile) << "  </TR>" << std::endl;
				for (std::map<std::string, int>::iterator i2 = _listOfPredefinedFunctions.begin(); i2 != _listOfPredefinedFunctions.end(); i2++) {
					(*pHTMLFile) << "  <TR>" << std::endl;
					(*pHTMLFile) << "    <TD WIDTH=\"69%\" VALIGN=\"TOP\"><B><FONT SIZE=2><P>" << i2->first << "(...) " << "</B></FONT></TD>" << std::endl;
					(*pHTMLFile) << "    <TD WIDTH=\"31%\" VALIGN=\"TOP\"><FONT SIZE=2><P>" << i2->second << "</FONT></TD>" << std::endl;
					(*pHTMLFile) << "  </TR>" << std::endl;
				}
				(*pHTMLFile) << "</TABLE>" << std::endl;
			}
			// generates by file
			for (std::map<std::string, std::map<int, int> >::const_iterator i = _coveredLines.begin(); i != _coveredLines.end(); i++) {
				std::string sCompleteFileName;
				std::ifstream* pFile = openInputFileFromIncludePath(i->first.c_str(), sCompleteFileName);
				if (pFile != NULL) {
					(*pHTMLFile) << "<H1>File \"" << i->first << "\":</H1>" << std::endl;
					// generates predefined functions
					bInitialized = false;
					std::map<std::string, std::map<std::string, DtaQuantifyFunction> >::iterator i3 = _listOfUserFunctions.find(i->first);
					if (i3 != _listOfUserFunctions.end()) {
						for (std::map<std::string, DtaQuantifyFunction>::iterator i4 = i3->second.begin(); i4 != i3->second.end(); i4++) {
							if (!bInitialized) {
								bInitialized = true;
								(*pHTMLFile) << "<TABLE BORDER CELLSPACING=1 CELLPADDING=7 WIDTH=480>" << std::endl;
								(*pHTMLFile) << "  <TR>" << std::endl;
								(*pHTMLFile) << "    <TD WIDTH=\"60%\" VALIGN=\"TOP\"><B><FONT SIZE=3><P ALIGN=\"CENTER\">User defined function</B></FONT></TD>" << std::endl;
								(*pHTMLFile) << "    <TD WIDTH=\"20%\" VALIGN=\"TOP\"><B><FONT SIZE=3><P ALIGN=\"CENTER\">Occurence</B></FONT></TD>" << std::endl;
								(*pHTMLFile) << "    <TD WIDTH=\"20%\" VALIGN=\"TOP\"><B><FONT SIZE=3><P ALIGN=\"CENTER\">Time in ms</B></FONT></TD>" << std::endl;
								(*pHTMLFile) << "  </TR>" << std::endl;
							}
							setLocation(*pFile, i4->second._iLocation);
							(*pHTMLFile) << "  <TR>" << std::endl;
							(*pHTMLFile) << "    <TD WIDTH=\"60%\" VALIGN=\"TOP\"><B><FONT SIZE=2><P>" << i4->second._sName << "(...) at " << getLineCount(*pFile) << "</B></FONT></TD>" << std::endl;
							(*pHTMLFile) << "    <TD WIDTH=\"20%\" VALIGN=\"TOP\"><FONT SIZE=2><P>" << i4->second._iCounter << "</FONT></TD>" << std::endl;
							(*pHTMLFile) << "    <TD WIDTH=\"20%\" VALIGN=\"TOP\"><FONT SIZE=2><P>" << i4->second._iTimeInMillis << "</FONT></TD>" << std::endl;
							(*pHTMLFile) << "  </TR>" << std::endl;
						}
					}
					if (bInitialized) {
						(*pHTMLFile) << "</TABLE>" << std::endl;
						setLocation(*pFile, 0);
					}
					// generates colored source
					std::map<int, int>::const_iterator j;
					int iMaxCounter = 0;
					for (j = i->second.begin(); j != i->second.end(); j++) if (j->second > iMaxCounter) iMaxCounter = j->second;
					double dShare = ((double) iMaxCounter) / 6.0;
					std::string sEmptyFormat;
					char sSizeFormat[10];
					sprintf(sSizeFormat, "%d", iMaxCounter);
					int iSizeFormat = strlen(sSizeFormat);
					for (int k = 0; k < iSizeFormat; k++) sEmptyFormat += "&nbsp;";
					std::string sLine;
					int iPrecLocation = 0;
					j = i->second.begin();
					while (readLine(*pFile, sLine)) {
						int iNextLocation = getLocation(*pFile);
						while ((j->first < iPrecLocation) && (j != i->second.end())) j++;
						int iNbLines = iMaxCounter + 1;
						while ((j->first < iNextLocation) && (j != i->second.end())) {
							if (iNbLines > j->second) iNbLines = j->second;
							j++;
						}
						(*pHTMLFile) << "<FONT FACE=\"Courier New\" SIZE=2><BR>";
						if ((iNbLines > iMaxCounter) || (iNbLines == 0)) (*pHTMLFile) << "&nbsp;&nbsp;" << sEmptyFormat;
						else {
							char sLineCount[16];
							sprintf(sLineCount, "%d", iNbLines);
							(*pHTMLFile) << "[" << (sEmptyFormat.c_str() + 6*strlen(sLineCount)) << sLineCount << "]";
						}
						(*pHTMLFile) << "</FONT>";
						if (iNbLines <= iMaxCounter) {
							(*pHTMLFile) << "<B><FONT SIZE=2 COLOR=\"";
							if (iNbLines == 0) (*pHTMLFile) << "#808080" << "\">";
							else if (dShare < 1.0) (*pHTMLFile) << tColorArray[iMaxCounter - 1][iNbLines] << "\">";
							else {
								int iIndex = (int) (((double) (iNbLines - 1)) / dShare);
								(*pHTMLFile) << tColorArray[5][iIndex] << "\">";
							}
							int iFirstChar = 0;
							do {
								if (sLine[iFirstChar] == ' ') (*pHTMLFile) << "&nbsp;";
								else if (sLine[iFirstChar] == '\t') (*pHTMLFile) << "&nbsp;&nbsp;&nbsp;&nbsp;";
								else break;
								iFirstChar++;
							} while (true);
							(*pHTMLFile) << (sLine.c_str() + iFirstChar) << "</B></FONT>" << std::endl;
						} else {
							int iFirstChar = 0;
							do {
								if (sLine[iFirstChar] == ' ') (*pHTMLFile) << "&nbsp;";
								else if (sLine[iFirstChar] == '\t') (*pHTMLFile) << "&nbsp;&nbsp;&nbsp;&nbsp;";
								else break;
								iFirstChar++;
							} while (true);
							(*pHTMLFile) << "<FONT SIZE=2>" << (sLine.c_str() + iFirstChar) << "</FONT>" << std::endl;
						}
						iPrecLocation = iNextLocation;
					}
					pFile->close();
					delete pFile;
				}
			}
			double dPercentCovered = floor(100.0 * ((double) getCoveredCode()) / ((double) getTotalCode()));
			(*pHTMLFile) << "<H1>Covered source code: <FONT COLOR=\"#ff0000\">" << dPercentCovered << "%</FONT></H1>" << std::endl;
			(*pHTMLFile) << "</BODY>" << std::endl;
			(*pHTMLFile) << "</HTML>" << std::endl;
			pHTMLFile->close();
			delete pHTMLFile;
		}
	}
}
