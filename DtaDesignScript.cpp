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
#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "GrfSetInputLocation.h"
#include "GrfAttachInputToSocket.h"
#include "GrfDetachInputFromSocket.h"
#include "GrfGoBack.h"
#include "DtaProtectedAreasBag.h"
#include "CGRuntime.h"
#include "DtaDesignScript.h"

namespace CodeWorker {
	DtaDesignScript::DtaDesignScript(/*DtaScriptVariable* pVisibility, */GrfBlock* pParentBlock) : DtaScript(/*pVisibility, */pParentBlock), _pProtectedAreasBag(NULL) {
	}

	DtaDesignScript::~DtaDesignScript() {
		if (_pProtectedAreasBag != NULL) delete _pProtectedAreasBag;
	}

	DtaProtectedAreasBag& DtaDesignScript::getProtectedAreasBag() {
		if (_pProtectedAreasBag == NULL) {
			_pProtectedAreasBag = new DtaProtectedAreasBag;
			_pProtectedAreasBag->recoverProtectedCodes(*CGRuntime::_pInputStream);
		}
		return *_pProtectedAreasBag;
	}

	DtaScriptFactory::SCRIPT_TYPE DtaDesignScript::getType() const { return DtaScriptFactory::FREE_SCRIPT; }
	bool DtaDesignScript::isAParseScript() const { return true; }

	void DtaDesignScript::traceEngine() const {
		if (getFilenamePtr() == NULL) CGRuntime::traceLine("free-parsing script (no filename):");
		else CGRuntime::traceLine("free-parsing script \"" + std::string(getFilenamePtr()) + "\":");
		traceInternalEngine();
	}

	void DtaDesignScript::handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block) {
		DtaScript::handleUnknownCommand(sCommand, script, block);
	}

//##markup##"parsing"
//##begin##"parsing"
void DtaDesignScript::parseAttachInputToSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfAttachInputToSocket* pAttachInputToSocket = new GrfAttachInputToSocket;
	if (requiresParsingInformation()) pAttachInputToSocket->setParsingInformation(getFilenamePtr(), script);
	block.add(pAttachInputToSocket);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pAttachInputToSocket->setSocket(parseExpression(block, script));
	else pAttachInputToSocket->setSocket(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaDesignScript::parseDetachInputFromSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfDetachInputFromSocket* pDetachInputFromSocket = new GrfDetachInputFromSocket;
	if (requiresParsingInformation()) pDetachInputFromSocket->setParsingInformation(getFilenamePtr(), script);
	block.add(pDetachInputFromSocket);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pDetachInputFromSocket->setSocket(parseExpression(block, script));
	else pDetachInputFromSocket->setSocket(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaDesignScript::parseGoBack(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfGoBack* pGoBack = new GrfGoBack;
	if (requiresParsingInformation()) pGoBack->setParsingInformation(getFilenamePtr(), script);
	block.add(pGoBack);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

void DtaDesignScript::parseSetInputLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller) {
	GrfSetInputLocation* pSetInputLocation = new GrfSetInputLocation;
	if (requiresParsingInformation()) pSetInputLocation->setParsingInformation(getFilenamePtr(), script);
	block.add(pSetInputLocation);
	script.skipEmpty();
	if (!script.isEqualTo('(')) throw UtlException(script, "syntax error: '(' expected");
	script.skipEmpty();
	if (pMethodCaller == NULL) pSetInputLocation->setLocation(parseExpression(block, script));
	else pSetInputLocation->setLocation(pMethodCaller);
	if (!script.isEqualTo(')')) throw UtlException(script, "syntax error: ')' expected");
	script.skipEmpty();
	if (!script.isEqualTo(';')) throw UtlException(script, "syntax error: ';' expected");
}

//##end##"parsing"

	SEQUENCE_INTERRUPTION_LIST DtaDesignScript::loadDesign(const char* sFile, DtaScriptVariable& thisContext) {
		setFilename(sFile);
		return execute(thisContext);
	}

	SEQUENCE_INTERRUPTION_LIST DtaDesignScript::execute(DtaScriptVariable& thisContext) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		ScpStream* pOldInputStream = CGRuntime::_pInputStream;
		CGRuntime::_pInputStream = new ScpStream(getFilenamePtr(), ScpStream::IN | ScpStream::PATH);
		try {
			result = DtaScript::execute(thisContext);
		} catch(UtlException& e) {
			int iLine = CGRuntime::_pInputStream->getLineCount();
			CGRuntime::_pInputStream->close();
			delete CGRuntime::_pInputStream;
			CGRuntime::_pInputStream = pOldInputStream;
			std::string sException = e.getMessage();
			std::string sMessage;
			if (getFilenamePtr() == NULL) sMessage += "(unknown)";
			else sMessage += getFilenamePtr();
			char tcNumber[32];
			sprintf(tcNumber, "(%d):", iLine);
			sMessage += tcNumber;
			sMessage += CGRuntime::endl() + sException;
			throw UtlException(e.getTraceStack(), sMessage);
		} catch(std::exception&) {
			CGRuntime::_pInputStream->close();
			delete CGRuntime::_pInputStream;
			CGRuntime::_pInputStream = pOldInputStream;
			throw;
		}
		CGRuntime::_pInputStream->close();
		delete CGRuntime::_pInputStream;
		CGRuntime::_pInputStream = pOldInputStream;
		return result;
	}
}
