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

#include "UtlException.h"
#include "ScpStream.h"

#include "CGRuntime.h"
#include "Workspace.h"
#include "DtaScriptVariable.h"
#include "DtaProject.h"
#include "DtaConsoleScript.h"

#include <map>

namespace CodeWorker {
	DtaConsoleScript::~DtaConsoleScript() {}

	bool DtaConsoleScript::betweenCommands(ScpStream& script, GrfBlock&) {
		script.skipEmpty();
		if (script.isEqualTo('@') && (_sCursor.size() > 1)) waitForCommand(script);
		return false;
	}

	void DtaConsoleScript::beforeParsingABlock(ScpStream&, GrfBlock&) {
		_sCursor += ">";
	}

	void DtaConsoleScript::afterParsingABlock(ScpStream&, GrfBlock&) {
		_sCursor = _sCursor.substr(1);
	}

	void DtaConsoleScript::handleNotAWordCommand(ScpStream& script, GrfBlock& block) {
		if (script.isEqualTo('?')) Workspace::displayHelp();
		else DtaScript::handleNotAWordCommand(script, block);
	}

	void DtaConsoleScript::handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block) {
		if ((sCommand == "exit") || (sCommand == "quit")) throw "exit";
		else if (sCommand == "help") {
			Workspace::displayHelp();
			if (script.skipEmpty() && !script.isEqualTo('@')) {
				if (!script.isEqualTo(';')) {
					throw UtlException(script, "';' expected");
				}
			}
		} else DtaScript::handleUnknownCommand(sCommand, script, block);
	}

	void DtaConsoleScript::waitForCommand(ScpStream& stream) {
		std::string sText;
		CGRuntime::traceText(_sCursor + " ");
		int iPos = stream.getInputLocation();
		bool bContinue = true;
		do {
			sText = CGRuntime::inputLine(true, "");
			stream << sText << CGRuntime::endl();
			for (int iIndex = sText.size() - 1; iIndex >= 0; iIndex--) {
				if ((sText[iIndex] < '\0') || (sText[iIndex] > ' ')) {
					bContinue = (sText[iIndex] != ';') && (sText[iIndex] != '{') && (sText[iIndex] != '}');
					break;
				}
			}
			if (bContinue) {
				stream.skipEmpty();
				bContinue = !(stream.isEqualToIdentifier("help") || stream.isEqualToIdentifier("quit") || stream.isEqualToIdentifier("exit"));
				stream.setInputLocation(iPos);
			}
		} while (bContinue);
		stream << "@";
	}

	void DtaConsoleScript::traceEngine() const {
		CGRuntime::traceLine("Console script:");
		traceInternalEngine();
	}

	SEQUENCE_INTERRUPTION_LIST DtaConsoleScript::execute(DtaScriptVariable& thisContext) {
		CGRuntime::traceLine("Console mode. Type 'help' for more information.");
		bool bContinue = true;
		GrfBlock* pTemporaryBlock = new GrfBlock(&getBlock());
		DtaProject::getInstance().prepareExecution();
		while (bContinue) {
			_sCursor = ">";
			try {
				ScpStream stream;
				waitForCommand(stream);
				parseScript(stream, *pTemporaryBlock);
				pTemporaryBlock->execute(CGRuntime::getThisInternalNode());
				pTemporaryBlock->moveFunctions(getBlock());
				pTemporaryBlock->moveClasses(getBlock());
			} catch(UtlException& exception) {
				CGRuntime::traceLine(exception.getMessage());
				CGRuntime::traceText(exception.getTraceStack());
			} catch(const char* sExit) {
				if (strcmp(sExit, "exit") == 0) bContinue = false;
				else CGRuntime::traceLine(sExit);
			}
			pTemporaryBlock->clear();
		}
		return EXIT_INTERRUPTION;
	}
}
