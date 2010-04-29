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

#include <fstream>

#include "UtlException.h"
#include "CppCompilerEnvironment.h"

#include "CGRuntime.h"
#include "DtaScriptVariable.h"
#include "DtaProject.h"
#include "DtaBNFScript.h"
#include "ExprScriptVariable.h"
#include "ExprScriptExpression.h"
#include "UtlString.h"
#include "GrfDebugExecution.h"

#define printDebugLine traceLine
#define printDebugText traceText
#define debugCommand inputLine

namespace CodeWorker {
	DtaBreakpoint::~DtaBreakpoint() {
		delete _pCondition;
		delete _pAction;
	}

	void DtaBreakpoint::setCondition(const std::string& sCondition) {
		_sCondition = sCondition;
		delete _pCondition;
		_pCondition = NULL;
	}

	void DtaBreakpoint::setAction(const std::string& sAction) {
		_sAction = sAction;
		delete _pAction;
		_pAction = NULL;
	}

	bool DtaBreakpoint::executeCondition(GrfCommand* pCommand, DtaScriptVariable& visibility) {
		if (_sCondition.empty()) return true;
		if (_pCondition == NULL) {
			GrfBlock* pBlock = pCommand->getParent();
			ScpStream stream(_sCondition);
			try {
				_pCondition = pCommand->getParent()->getScript()->parseExpression(*pBlock, stream);
			} catch(UtlException& exception) {
				CGRuntime::printDebugLine("in 'when' clause of a breakpoint: " + _sCondition);
				CGRuntime::printDebugLine(exception.getMessage());
				CGRuntime::printDebugText(exception.getTraceStack());
				return true;
			}
			if (stream.skipEmpty()) {
				if (stream.peekChar() != '{') {
					delete _pCondition;
					_pCondition = NULL;
					CGRuntime::printDebugLine("syntax error at the end of the 'when' clause of a breakpoint: " + _sCondition);
					return true;
				}
				setAction(stream.readBuffer() + stream.getInputLocation());
			}
		}
		std::string sResult = _pCondition->getValue(visibility);
		return !sResult.empty();
	}

	void DtaBreakpoint::executeAction(GrfCommand* pCommand, DtaScriptVariable& visibility) {
		if (_sAction.empty()) return;
		if (_pAction == NULL) {
			ScpStream stream(_sAction);
			_pAction = new GrfBlock(pCommand->getParent());
			try {
				pCommand->getParent()->getScript()->parseBlock(stream, *_pAction);
			} catch(UtlException& exception) {
				delete _pAction;
				_pAction = NULL;
				CGRuntime::printDebugLine("in action of a breakpoint: " + _sAction);
				CGRuntime::printDebugLine(exception.getMessage());
				CGRuntime::printDebugText(exception.getTraceStack());
				return;
			}
		}
		_pAction->execute(visibility);
	}


	class DtaWatchNodeListener : public DtaNodeListener {
	private:
		GrfDebugExecution* _pDebugContext;
		bool _bDeleted;
		DtaBreakpoint* _pBP;
		std::string _sKey;

	public:
		inline DtaWatchNodeListener(const std::string& sKey, GrfDebugExecution* pDebugContext) : _sKey(sKey), _pDebugContext(pDebugContext), _bDeleted(false), _pBP(NULL) {}
		virtual ~DtaWatchNodeListener() {
			_pDebugContext->removeWatchNodeListener(this);
			delete _pBP;
		}

		inline const std::string& getKey() const { return  _sKey; }
		inline std::string getCondition() const { return (_pBP == NULL) ? "" : _pBP->getCondition(); }

		void setCondition(const std::string& sCondition) {
			if (_pBP == NULL) _pBP = new DtaBreakpoint;
			_pBP->setCondition(sCondition);
		}

		void setAction(const std::string& sAction) {
			if (_pBP == NULL) _pBP = new DtaBreakpoint;
			_pBP->setAction(sAction);
		}

		virtual void onSetValue(const char* tcValue) { onHasChanged(); }
		virtual void onConcatValue(const char* tcValue) { onHasChanged(); }
		virtual void onSetReference(DtaScriptVariable* pReference) { onHasChanged(); }
		virtual void onSetExternal(ExternalValueNode* pExternalValue) { onHasChanged(); }
		virtual void onSetIterator(DtaArrayIterator** pIteratorData) { onHasChanged(); }
		virtual void onAddAttribute(DtaScriptVariable* pAttribute) { onHasChanged(); }
		virtual void onRemoveAttribute(DtaScriptVariable* pAttribute) { onHasChanged(); }
		virtual void onArrayHasChanged() { onHasChanged(); }
		virtual void onClearContent() { onHasChanged(); }
		virtual void onDelete() { _bDeleted = true; }

	private:
		void onHasChanged() {
			if (!_bDeleted) {
				CGRuntime::printDebugLine("debug: the variable '" + getNode()->getCompleteName() + "' has changed");
				if (_pBP == NULL) _pDebugContext->_bStop = true;
				else _pDebugContext->_pConditionalStop = _pBP;
			}
		}
	};

	class GrfDebugActivation {
	private:
		static GrfDebugExecution* _pActiveDebugger;
		GrfDebugExecution* _pPreviousActiveDebugger;

	public:
		GrfDebugActivation(GrfDebugExecution* pActiveDebugger) {
			_pPreviousActiveDebugger = _pActiveDebugger;
			_pActiveDebugger = pActiveDebugger;
		}
		~GrfDebugActivation() {
			_pActiveDebugger = _pPreviousActiveDebugger;
		}

		static void unactivateCurrentDebugger() { _pActiveDebugger = NULL; }
		static bool isActiveDebugger(GrfDebugExecution* pDebugger) { return (_pActiveDebugger == pDebugger); }
	};


	GrfDebugExecution* GrfDebugActivation::_pActiveDebugger = NULL;


	GrfDebugExecution::~GrfDebugExecution() {
		clearBreakpoints();
		clearWatchpoints();
	}

	void GrfDebugExecution::clearBreakpoints() {
		for (std::map<std::string, std::map<int, DtaBreakpoint*> >::iterator i = _listOfBreakpoints.begin(); i != _listOfBreakpoints.end(); ++i) {
			for (std::map<int, DtaBreakpoint*>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
				delete j->second;
			}
		}
		_listOfBreakpoints.clear();
	}

	void GrfDebugExecution::clearWatchpoints() {
		while (!_watchListeners.empty()) {
			DtaWatchNodeListener* pListener = *(_watchListeners.begin());
			_watchListeners.pop_front();
			pListener->getNode()->removeListener(pListener);
		}
	}

	void GrfDebugExecution::clearWatchpoint(const std::string& sKey) {
		for (std::list<DtaWatchNodeListener*>::iterator i = _watchListeners.begin(); i != _watchListeners.end(); ++i) {
			if ((*i)->getKey() == sKey) {
				DtaWatchNodeListener* pListener = *i;
				_watchListeners.remove(pListener);
				pListener->getNode()->removeListener(pListener);
				break;
			}
		}
	}

	bool GrfDebugExecution::executeCommand(GrfCommand* pCommand, DtaScriptVariable& visibility, ScpStream& stream) {
		if (!stream.skipEmpty()) return false;
		bool bContinue = false;
		std::string sIdentifier;
		int iChar = stream.readChar();
		while (((iChar >= (int) 'a') && (iChar <= (int) 'z')) || ((iChar >= (int) 'A') && (iChar <= (int) 'Z'))) {
			if ((iChar >= (int) 'A') && (iChar <= (int) 'Z')) iChar -= (int) ' ';
			sIdentifier += (char) iChar;
			iChar = stream.readChar();
		}
		if (iChar > 0) stream.goBack();
		if (sIdentifier.empty()) {
			if (stream.isEqualTo('?')) {
				help();
			} else if (stream.peekChar() == '{') {
				try {
					GrfBlock action(pCommand->getParent());
					DtaScript* pScript = pCommand->getParent()->getScript();
					DtaPatternScript* pPatternScript = dynamic_cast<DtaPatternScript*>(pScript);
					if (pPatternScript != NULL) {
						pPatternScript->setExecMode(true);
						DtaBNFScript* pBNFScript = dynamic_cast<DtaBNFScript*>(pScript);
						if (pBNFScript != NULL) {
							pBNFScript->setBNFMode(false);
						}
					}
					pScript->parseBlock(stream, action);
					action.execute(visibility);
				} catch(UtlException& exception) {
					CGRuntime::printDebugLine(exception.getMessage());
					CGRuntime::printDebugText(exception.getTraceStack());
					return false;
				}
			} else if (stream.skipEmpty()) {
				CGRuntime::printDebugLine("debug: syntax error");
				return false;
			}
		} else if ((sIdentifier == "h") || (sIdentifier == "help")) {
			help();
		} else if ((sIdentifier == "q") || (sIdentifier == "quit")) {
			GrfDebugActivation::unactivateCurrentDebugger();
			bContinue = true;
		} else if ((sIdentifier == "r") || (sIdentifier == "run")) {
			_bStop = false;
			_bRun  = true;
			_pNext = NULL;
			bContinue = true;
		} else if ((sIdentifier == "s") || (sIdentifier == "step")) {
			stream.skipEmpty();
			if (stream.readInt(_iIterations)) {
				if (_iIterations <= 0) {
					CGRuntime::printDebugLine("debug: 'step' cannot require a negative number of iterations");
					return false;
				}
			} else _iIterations = 1;
			_bStop = true;
			_bRun  = false;
			_pNext = NULL;
			bContinue = true;
		} else if ((sIdentifier == "n") || (sIdentifier == "next")) {
			stream.skipEmpty();
			if (stream.readInt(_iIterations)) {
				if (_iIterations <= 0) {
					CGRuntime::printDebugLine("debug: 'next' cannot require a negative number of iterations");
					return false;
				}
			} else _iIterations = 1;
			_bStop = false;
			_bRun  = false;
			_pNext = pCommand;
			bContinue = true;
		} else if ((sIdentifier == "b") || (sIdentifier == "breakpoint")) {
			stream.skipEmpty();
			std::string sFilename;
			if (!stream.readString(sFilename)) {
				if (stream.isEqualToIdentifier("list") || !stream.skipEmpty()) {
					std::string sCurrentDirectory = CGRuntime::getCurrentDirectory();
					int iRank = 1;
					for (std::map<std::string, std::map<int, DtaBreakpoint*> >::const_iterator i = _listOfBreakpoints.begin(); i != _listOfBreakpoints.end(); ++i) {
						for (std::map<int, DtaBreakpoint*>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
							std::string sFilename = i->first;
							if (sFilename.size() > sCurrentDirectory.size()) {
								std::string::size_type iSize = sCurrentDirectory.size();
								std::string sPath = sFilename.substr(0, iSize);
								if (stricmp(sPath.c_str(), sCurrentDirectory.c_str()) == 0) {
									if ((sFilename[iSize] == '/') || (sFilename[iSize] == '\\')) ++iSize;
									sFilename = sFilename.substr(iSize);
								}
							}
							char tcRank[64];
							sprintf(tcRank, "%4d: \"", iRank++);
							char tcNumber[64];
							sprintf(tcNumber, "\" breakpoint at %d", j->first);
							CGRuntime::printDebugLine(tcRank + sFilename + tcNumber);
							if (!j->second->getCondition().empty()) {
								CGRuntime::printDebugLine("\t\twhen " + j->second->getCondition());
							}
						}
					}
					goto finallyExecuteCommand;
				}
				GrfCommand* pCommand = getLastCommand();
				if (pCommand == NULL) {
					CGRuntime::printDebugLine("debug: filename of breakpoint expected");
					return false;
				}
				sFilename = getParsingFilePtr(pCommand);
			} else {
				stream.skipEmpty();
			}
			std::string sIdentifier;
			if ((!stream.readIdentifier(sIdentifier)) || (sIdentifier != "at")) {
				CGRuntime::printDebugLine("debug: 'at' expected for giving line number");
				return false;
			}
			stream.skipEmpty();
			int iLineNumber;
			if ((!stream.readInt(iLineNumber)) || (iLineNumber <= 0)) {
				CGRuntime::printDebugLine("debug: positive line number expected after 'at'");
				return false;
			}
			stream.skipEmpty();
			DtaBreakpoint* pBP = registerBreakpoint(visibility, sFilename.c_str(), iLineNumber);
			if (stream.isEqualToIdentifier("when")) {
				std::string sCondition = stream.readBuffer() + stream.getInputLocation();
				pBP->setCondition(sCondition);
			} else if (stream.peekChar() == '{') {
				std::string sAction = stream.readBuffer() + stream.getInputLocation();
				if (!sAction.empty()) pBP->setAction(sAction);
			}
		} else if ((sIdentifier == "c") || (sIdentifier == "clear")) {
			stream.skipEmpty();
			std::string sFilename;
			int iRank;
			if (stream.readString(sFilename)) {
				stream.skipEmpty();
				if (stream.isEqualToIdentifier("at")) {
					stream.skipEmpty();
					int iLineNumber;
					if ((!stream.readInt(iLineNumber)) || (iLineNumber <= 0)) {
						CGRuntime::printDebugLine("debug: positive line number expected after 'at'");
						return false;
					}
					clearBreakpoint(visibility, sFilename.c_str(), iLineNumber);
				} else {
					clearBreakpoint(visibility, sFilename.c_str(), -1);
				}
			} else if (stream.isEqualToIdentifier("all")) {
				clearBreakpoint(visibility, NULL, -1);
				clearWatchpoints();
			} else if (stream.readInt(iRank)) {
				for (std::map<std::string, std::map<int, DtaBreakpoint*> >::const_iterator i = _listOfBreakpoints.begin(); i != _listOfBreakpoints.end(); ++i) {
					for (std::map<int, DtaBreakpoint*>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
						--iRank;
						if (iRank == 0) {
							clearBreakpoint(visibility, i->first.c_str(), j->first);
							goto finallyExecuteCommand;
						}
					}
				}
				CGRuntime::printDebugLine("debug: bad breakpoint rank; please check its value by typing 'b list'");
				return false;
			} else {
				try {
					GrfBlock* pBlock = pCommand->getParent();
					std::auto_ptr<ExprScriptVariable> pVarExpr(pCommand->getParent()->getScript()->parseVariableExpression(*pBlock, stream));
					std::string sKey = pVarExpr->toString();
					clearWatchpoint(sKey);
				} catch(UtlException& exception) {
					CGRuntime::printDebugLine(exception.getMessage());
					CGRuntime::printDebugText(exception.getTraceStack());
					return false;
				}
			}
		} else if ((sIdentifier == "d") || (sIdentifier == "display")) {
			stream.skipEmpty();
			int iDisplaySize;
			if (stream.readInt(iDisplaySize) && (iDisplaySize >= 0)) _iDisplaySize = iDisplaySize;
			display(pCommand, visibility, _iDisplaySize);
		} else if ((sIdentifier == "fl") || (sIdentifier == "fs") || (sIdentifier == "file")) {
			stream.skipEmpty();
			if (sIdentifier == "file") {
				if (!stream.readIdentifier(sIdentifier)) {
					CGRuntime::printDebugLine("debug: 'file load' or 'file save' expected");
					return false;
				}
				if (sIdentifier == "load") sIdentifier = "fl";
				else if (sIdentifier == "save") sIdentifier = "fs";
				else {
					CGRuntime::printDebugLine("debug: 'file load' or 'file save' expected");
					return false;
				}
				stream.skipEmpty();
			}
			std::string sFile;
			if (!stream.readString(sFile)) {
				CGRuntime::printDebugLine("debug: file name between double quotes expected");
				return false;
			}
			if (!_bReadingHistory) {
				if (sIdentifier == "fl") {
					try {
						ScpStream file(sFile, ScpStream::IN | ScpStream::PATH);
						_history << file.readBuffer();
					} catch(std::exception& exception) {
						CGRuntime::printDebugLine("debug: " + std::string(exception.what()));
						return false;
					}
					_bReadingHistory = true;
				} else {
					_history.saveIntoFile(sFile, true);
				}
			}
			return false;
		} else if ((sIdentifier == "l") || (sIdentifier == "local")) {
			DtaScriptVariableList* pAttributes = visibility.getAttributes();
			while (pAttributes != NULL) {
				CGRuntime::printDebugLine(pAttributes->getNode()->getName());
				pAttributes = pAttributes->getNext();
			}
		} else if ((sIdentifier == "o") || (sIdentifier == "object")) {
			DtaScript* pScript = pCommand->getParent()->getScript();
			ExprScriptVariable* pVariableExpr = pScript->parseVariableExpression(*pCommand->getParent(), stream);
			stream.skipEmpty();
			int iDepth;
			if (!stream.readInt(iDepth)) iDepth = 0;
			if (pVariableExpr == NULL) CGRuntime::printDebugLine("debug: syntax error on variable");
			else {
				DtaScriptVariable* pVariable = visibility.getExistingVariable(*pVariableExpr);
				if (pVariable == NULL) {
					CGRuntime::printDebugLine("(null)");
				} else {
					pVariable->traceObject(iDepth);
				}
				delete pVariableExpr;
			}
		} else if (sIdentifier == "stack") {
			traceStack(pCommand, visibility);
		} else if ((sIdentifier == "t") || (sIdentifier == "trace")) {
			std::auto_ptr<DtaScript> pScript(new DtaScript(NULL));
			std::auto_ptr<ExprScriptExpression> pExpr(pScript->parseExpression(*pCommand->getParent(), stream));
			if (pExpr.get() == NULL) CGRuntime::printDebugLine("debug: syntax error on expression");
			else {
				std::string sValue = pExpr->getValue(visibility);
				CGRuntime::printDebugLine(sValue);
			}
		} else if ((sIdentifier == "w") || (sIdentifier == "watch")) {
			if (stream.skipEmpty()) {
				for (std::list<DtaWatchNodeListener*>::const_iterator i = _watchListeners.begin(); i != _watchListeners.end(); ++i) {
					DtaWatchNodeListener* pListener = *i;
					CGRuntime::printDebugText("\t" + pListener->getKey());
					std::string sCondition = pListener->getCondition();
					if (sCondition.empty()) {
						CGRuntime::printDebugLine("");
					} else {
						CGRuntime::printDebugLine(" when " + sCondition);
					}
				}
			} else {
				try {
					GrfBlock* pBlock = pCommand->getParent();
					std::auto_ptr<ExprScriptVariable> pVarExpr(pCommand->getParent()->getScript()->parseVariableExpression(*pBlock, stream));
					DtaScriptVariable* pVariable = visibility.getExistingVariable(*pVarExpr);
					if (pVariable == NULL) {
						CGRuntime::printDebugLine("debug: variable not found in the scope");
						return false;
					}
					std::string sKey = pVarExpr->toString();
					clearWatchpoint(sKey);
					DtaWatchNodeListener* pListener = new DtaWatchNodeListener(sKey, this);
					_watchListeners.push_back(pListener);
					pVariable->addListener(pListener);
					stream.skipEmpty();
					if (stream.isEqualToIdentifier("when")) {
						std::string sCondition = stream.readBuffer() + stream.getInputLocation();
						pListener->setCondition(sCondition);
					} else if (stream.peekChar() == '{') {
						std::string sAction = stream.readBuffer() + stream.getInputLocation();
						if (!sAction.empty()) pListener->setAction(sAction);
					}
				} catch(UtlException& exception) {
					CGRuntime::printDebugLine(exception.getMessage());
					CGRuntime::printDebugText(exception.getTraceStack());
					return false;
				}
			}
		} else {
			CGRuntime::printDebugLine("debug: unknown command '" + sIdentifier + "'");
			return false;
		}
finallyExecuteCommand:
		if (!_bReadingHistory) _history << stream.readBuffer() << '\n';
		return bContinue;
	}

	void GrfDebugExecution::help() {
		CGRuntime::printDebugLine("Debugger");
		CGRuntime::printDebugLine("  Commands available:");
		CGRuntime::printDebugLine("    - 'b' [<filename>] 'at' <line> [<when>] [<action>] (or 'breakpoint'):");
		CGRuntime::printDebugLine("        sets a breakpoint into file <filename> at line <line>");
		CGRuntime::printDebugLine("        - <when> ::= 'when' <boolean_expression>");
		CGRuntime::printDebugLine("            conditional breakpoint; the expression is in CW script");
		CGRuntime::printDebugLine("        - <action> ::= '{' <instructions> '}'");
		CGRuntime::printDebugLine("            a piece of CW script to execute when activating the bp");
		CGRuntime::printDebugLine("    - 'b' ['list'] (or 'breakpoint'):");
		CGRuntime::printDebugLine("        displays all breakpoints and their condition, if any");
		CGRuntime::printDebugLine("    - 'c' [<filename> ['at' <line>]] (or 'clear'):");
		CGRuntime::printDebugLine("        1. clears a breakpoint into file <filename> at line <line>");
		CGRuntime::printDebugLine("        2. or clears all breakpoints into file <filename>");
		CGRuntime::printDebugLine("    - 'c' <rank> (or 'clear'):");
		CGRuntime::printDebugLine("        clears the breakpoint designated by its line number in the list");
		CGRuntime::printDebugLine("    - 'c' <variable> (or 'clear'):");
		CGRuntime::printDebugLine("        clears the watchpoint on <variable> if any");
		CGRuntime::printDebugLine("    - 'c' 'all' (or 'clear all'):");
		CGRuntime::printDebugLine("        clears all breakpoints and all watchpoints");
		CGRuntime::printDebugLine("    - 'd' [<size>] or 'display' [<size>]:");
		CGRuntime::printDebugLine("        current line is displayed with <size> lines above and below");
		CGRuntime::printDebugLine("        If size isn't specified, the last known is taken");
		CGRuntime::printDebugLine("    - 'fl' \"<filename>\" (or 'file load'):");
		CGRuntime::printDebugLine("        loads a file containing debugger commands to execute");
		CGRuntime::printDebugLine("    - 'fs' \"<filename>\" (or 'file save'):");
		CGRuntime::printDebugLine("        saves the debugger commands executed up to now to a file");
		CGRuntime::printDebugLine("    - 'h' or 'help' or '?': help");
		CGRuntime::printDebugLine("    - 'l' or 'local': displays local variables on the stack");
		CGRuntime::printDebugLine("    - 'n' or 'next': next");
		CGRuntime::printDebugLine("    - 'o' or 'object' <variable> [<depth>]:");
		CGRuntime::printDebugLine("        displays the content of a variable, eventually up to a given depth");
		CGRuntime::printDebugLine("    - 'q' or 'quit' quits this debugging session");
		CGRuntime::printDebugLine("    - 'r' or 'run': run up to next breakpoint");
		CGRuntime::printDebugLine("    - 's' or 'step': go step by step");
		CGRuntime::printDebugLine("    - 'stack': displays stack call");
		CGRuntime::printDebugLine("    - 't' or 'trace' <expression>: displays result of an expression");
		CGRuntime::printDebugLine("    - 'w' <variable> [<when>] [<action>] (or 'watch'):");
		CGRuntime::printDebugLine("        the controlling sequence stops when <variable> changes");
		CGRuntime::printDebugLine("        - <when> ::= 'when' <boolean_expression>");
		CGRuntime::printDebugLine("            conditional watchpoint; the expression is in CW script");
		CGRuntime::printDebugLine("        - <action> ::= '{' <instructions> '}'");
		CGRuntime::printDebugLine("            a piece of CW script to execute at the watchpoint");
		CGRuntime::printDebugLine("    - 'w' or 'watch':");
		CGRuntime::printDebugLine("        displays all watchpoints and their condition, if any");
		CGRuntime::printDebugLine("    - '{' <instructions> '}': a piece of CW script to execute immediatly");
		CGRuntime::printDebugLine("    - '//' or '/*'...'*/': comment");
	}

	std::string GrfDebugExecution::display(GrfCommand* pCommand, DtaScriptVariable& visibility, int iSize, bool bEchoOn) {
		std::string sDisplayedText;
		std::string sCompleteFileName;
		if (CGRuntime::getInputStream() != NULL) {
			sDisplayedText = "parsed file is \"" + CGRuntime::getInputStream()->getFilename() + "\":";
			char sLocation[80];
			sprintf(sLocation, "%d,%d\n", CGRuntime::countInputLines(), CGRuntime::countInputCols());
			sDisplayedText += sLocation;
		}
		const char* tcParsingFile = getParsingFilePtr(pCommand);
		if (tcParsingFile == NULL) {
			sDisplayedText += "no debug information on the current command for displaying";
		} else {
			std::ifstream* pFile = openInputFileFromIncludePath(tcParsingFile, sCompleteFileName);
			if (pFile == NULL) {
				sDisplayedText  += "debug: unable to open file \"";
				sDisplayedText += getParsingFilePtr(pCommand);
				sDisplayedText += "\" for displaying\n";
			} else {
				setLocation(*pFile, getFileLocation(pCommand));
				setLocationAtXLines(*pFile, -iSize);
				for (int i = 0; i < 2*iSize + 1; i++) {
					std::string sLine;
					int iLocation = getLineCount(*pFile);
					if (readLine(*pFile, sLine)) {
						char sLineText[512];
						if (iSize > 0) sprintf(sLineText, "%d: ", iLocation);
						else {
							const char* u = strrchr(getParsingFilePtr(pCommand), '/');
							if (u == NULL) {
								u = strrchr(getParsingFilePtr(pCommand), '\\');
								if (u == NULL) u = getParsingFilePtr(pCommand);
								else ++u;
							} else {
								const char* v = strrchr(u, '\\');
								if (v != NULL) u = v;
								u++;
							}
							sprintf(sLineText, "\"%s\" at %d: ", u, iLocation);
						}
						sLine = sLineText + sLine + "\n";
						if (i == iSize) {
							char* u = sLineText;
							while (*u != '\0') *u++ = ' ';
							sLine += sLineText;
							int iPrecedent = getLocation(*pFile);
							setLocation(*pFile, getFileLocation(pCommand));
							std::string sHeader;
							getColEmptyHeader(*pFile, sHeader);
							setLocation(*pFile, iPrecedent);
							sLine += sHeader + "^\n";
						}
						sDisplayedText += sLine;
					}
				}
				pFile->close();
				delete pFile;
			}
		}
		if (bEchoOn && !sDisplayedText.empty()) CGRuntime::printDebugText(sDisplayedText);
		return sDisplayedText;
	}

	void GrfDebugExecution::traceStack(GrfCommand* pCommand, DtaScriptVariable& visibility) {
		display(pCommand, visibility, 0);
		std::string sPreviousText;
		for (std::list<GrfCommand*>::iterator i = _stack.begin(); i != _stack.end(); i++) {
			if (*i != NULL) {
				std::string sText = display(*i, visibility, 0, false);
				if (sPreviousText != sText) CGRuntime::printDebugText(sText);
				sPreviousText = sText;
			}
		}
	}

	DtaBreakpoint* GrfDebugExecution::registerBreakpoint(DtaScriptVariable& visibility, const char* sFilename, int iLineNumber) {
		std::string sCompleteFileName;
		if (_listOfBreakpoints.find(sFilename) == _listOfBreakpoints.end()) {
			std::ifstream* pFile = openInputFileFromIncludePath(sFilename, sCompleteFileName);
			if (pFile == NULL) {
				CGRuntime::printDebugLine("debug: unable to open file \"" + std::string(sFilename) + "\" for breakpoint");
				return NULL;
			}
			pFile->close();
			delete pFile;
		} else {
			sCompleteFileName = sFilename;
		}
		sCompleteFileName = CGRuntime::canonizePath(sCompleteFileName);
		std::map<int, DtaBreakpoint*>& bpList = _listOfBreakpoints[sCompleteFileName];
		if (bpList.find(iLineNumber) == bpList.end()) {
			std::ifstream* pFile = openInputFileFromIncludePath(sFilename, sCompleteFileName);
			if (pFile == NULL) {
				CGRuntime::printDebugLine("debug: unable to open file \"" + std::string(sFilename) + "\" for breakpoint");
				return NULL;
			}
			setLocationAtXLines(*pFile, iLineNumber - 1);
			int iBeginning = getLocation(*pFile);
			setLocationAtXLines(*pFile, 1);
			int iEnd = getLocation(*pFile);
			pFile->close();
			delete pFile;
			if (iBeginning >= iEnd) {
				char tcNumber[32];
				sprintf(tcNumber, "%d", iLineNumber);
				CGRuntime::printDebugLine("debug: unable to set a breakpoint at line " + std::string(tcNumber));
				return NULL;
			}
			DtaBreakpoint* bp = new DtaBreakpoint;
			bpList[iLineNumber] = bp;
			bp->_iBeginning = iBeginning;
			bp->_iEnd = iEnd;
			return bp;
		}
		return bpList[iLineNumber];
	}

	void GrfDebugExecution::clearBreakpoint(DtaScriptVariable& /*visibility*/, const char* sFilename, int iLineNumber) {
		if ((sFilename == NULL) || (*sFilename == '\0')) {
			clearBreakpoints();
		} else {
			if (_listOfBreakpoints.find(sFilename) == _listOfBreakpoints.end()) {
				CGRuntime::printDebugLine("debug: no breakpoint on file \"" + std::string(sFilename) + "\"");
			} else if (iLineNumber <= 0) {
				std::map<int, DtaBreakpoint*> bpList;
				_listOfBreakpoints[sFilename] = bpList;
			} else {
				std::map<int, DtaBreakpoint*>& bpList = _listOfBreakpoints[sFilename];
				if (bpList.find(iLineNumber) == bpList.end()) {
					char tcNumber[32];
					sprintf(tcNumber, "%d", iLineNumber);
					CGRuntime::printDebugLine("debug: no breakpoint at line " + std::string(tcNumber) + " into file \"" + std::string(sFilename) + "\"");
				} else {
					delete bpList[iLineNumber];
					bpList.erase(iLineNumber);
				}
			}
		}
	}

	DtaBreakpoint* GrfDebugExecution::stopOnBreakpoint(GrfCommand* pCommand, DtaScriptVariable& visibility) {
		const char* sFilename = getParsingFilePtr(pCommand);
		int iCharLocation = getFileLocation(pCommand);
		std::map<std::string, std::map<int, DtaBreakpoint*> >::const_iterator cursor = _listOfBreakpoints.find(sFilename);
		if (cursor == _listOfBreakpoints.end()) return NULL;
		const std::map<int, DtaBreakpoint*>& bpList = cursor->second;
		for (std::map<int, DtaBreakpoint*>::const_iterator i = bpList.begin(); i != bpList.end(); i++) {
			if (i->second->_iEnd > iCharLocation) {
				if (iCharLocation < i->second->_iBeginning) return NULL;
				if (i->second->executeCondition(pCommand, visibility)) return i->second;
				break;
			}
		}
		return NULL;
	}

	void GrfDebugExecution::handleBeforeExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& visibility) {
		if (!GrfDebugActivation::isActiveDebugger(this)) return;
		if (getParsingFilePtr(pCommand) != NULL) {
			GrfExecutionContext::handleBeforeExecutionCBK(pCommand, visibility);
			DtaBreakpoint* pBP;
			if (_pConditionalStop != NULL) {
				if (_pConditionalStop->executeCondition(pCommand, visibility)) pBP = _pConditionalStop;
				else pBP = stopOnBreakpoint(pCommand, visibility);
				_pConditionalStop = NULL;
			} else pBP = stopOnBreakpoint(pCommand, visibility);
			if (pBP != NULL) {
				_iIterations = 0;
				pBP->executeAction(pCommand, visibility);
			} else if (_iIterations > 0) _iIterations--;
			if (_bStop || (pBP != NULL)) {
				if (_iIterations > 0) {
					if (_pNext != NULL) {
						_pNext = pCommand;
						_bStop = false;
					}
				} else {
					_pNext = NULL;
					std::string sText;
					bool bContinue;
					display(pCommand, visibility, 0);
					do {
						if (_bReadingHistory) {
							if (_history.skipEmpty() && _history.readLine(sText)) {
								CGRuntime::printDebugLine("<history> " + _sCursor + sText);
							} else {
								_bReadingHistory = false;
								sText = CGRuntime::debugCommand(true, _sCursor);
							}
						} else {
							sText = CGRuntime::debugCommand(true, _sCursor);
						}
						ScpStream stream;
						stream << sText;
						bContinue = !executeCommand(pCommand, visibility, stream);
					} while (bContinue);
				}
			}
		}
	}

	void GrfDebugExecution::handleAfterExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/) {
		if (_pNext == pCommand) {
			_bStop = true;
		}
	}

	void GrfDebugExecution::handleAfterExceptionCBK(GrfCommand* pCommand, DtaScriptVariable& visibility, UtlException& exception) {
		if (!GrfDebugActivation::isActiveDebugger(this)) return;
		GrfDebugActivation::unactivateCurrentDebugger();
		if (getParsingFilePtr(pCommand) != NULL) {
			std::string sText;
			bool bContinue;
			CGRuntime::printDebugLine(exception.getMessage());
			CGRuntime::printDebugLine(exception.getTraceStack());
			do {
				if (_bReadingHistory) {
					if (_history.skipEmpty() && _history.readLine(sText)) {
						CGRuntime::printDebugLine("<history> " + _sCursor + sText);
					} else {
						_bReadingHistory = false;
						sText = CGRuntime::debugCommand(true, _sCursor);
					}
				} else {
					sText = CGRuntime::debugCommand(true, _sCursor);
				}
				ScpStream stream(sText);
				bContinue = !executeCommand(pCommand, visibility, stream);
			} while (bContinue);
		}
	}

	SEQUENCE_INTERRUPTION_LIST GrfDebugExecution::openSession(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		int iDepth = 1;
		GrfExecutionContext* pContext = getLastExecutionContext();
		while (pContext != NULL) {
			pContext = pContext->getLastExecutionContext();
			iDepth++;
		}
		char tcDepth[16];
		sprintf(tcDepth, "%d", iDepth);
		CGRuntime::printDebugLine("-- debug session 'D" + std::string(tcDepth) + "' --");
		GrfDebugActivation debugActivation(this);
		char sBuffer[16];
		sprintf(sBuffer, "D%d> ", iDepth);
		_sCursor = sBuffer;
		try {
			clearBreakpoints();
			_pConditionalStop = NULL;
			result = GrfBlock::executeInternal(visibility);
		} catch(UtlException&/* exception*/) {
			CGRuntime::printDebugLine("-- debug session 'D" + std::string(tcDepth) + "' interrupted by an exception --");
			throw/* UtlException(exception)*/;
		}
		CGRuntime::printDebugLine("-- end of debug session 'D" + std::string(tcDepth) + "' --");
		return result;
	}

	void GrfDebugExecution::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "// DEBUG execution required into the source code generation script: not translated";
		CW_BODY_ENDL;
	}
}
