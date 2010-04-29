/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2006 Cédric Lemaire

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
#include "ExprScriptVariable.h"
#include "DtaScript.h"
#include "ASTCommand.h"

namespace CodeWorker {
	ASTCommand::~ASTCommand() {}

	ExprScriptExpression* ASTCommand::parseExpression(ScpStream& script) {
		DtaScript theEmptyScript(NULL);
		GrfBlock* pBlock = NULL;
		GrfBlock& myNullBlock = *pBlock;
		return theEmptyScript.parseExpression(myNullBlock, script);
	}

	ExprScriptVariable* ASTCommand::parseVariableExpression(ScpStream& script) {
		DtaScript theEmptyScript(NULL);
		GrfBlock* pBlock = NULL;
		GrfBlock& myNullBlock = *pBlock;
		return theEmptyScript.parseVariableExpression(myNullBlock, script);
	}


	ASTThisCommand::ASTThisCommand(ScpStream& script) : _pVariable(NULL) {
		script.skipEmpty();
		std::string sAlias;
		if (script.readIdentifier(sAlias)) {
			script.skipEmpty();
			_pVariable = new ExprScriptVariable(sAlias.c_str());
		}
		if (!script.isEqualTo('=')) throw UtlException(script, "syntax error in #thisAST; '=' expected before the non-terminal");
	}

	ASTThisCommand::~ASTThisCommand() {
		delete _pVariable;
	}

	void ASTThisCommand::execute(ASTCommandEnvironment& env) {
	}


	ASTValueCommand::ASTValueCommand(ScpStream& script) : _pVariable(NULL) {
		script.skipEmpty();
		if (script.peekChar() != '=') {
			_pVariable = parseVariableExpression(script);
			script.skipEmpty();
		}
		if (!script.isEqualTo('=')) throw UtlException(script, "syntax error in #valueAST; '=' expected before the non-terminal");
	}

	ASTValueCommand::~ASTValueCommand() {
		delete _pVariable;
	}

	void ASTValueCommand::execute(ASTCommandEnvironment& env) {
	}


	ASTRefCommand::ASTRefCommand(ScpStream& script) : _pVariable(NULL) {
		_pVariable = parseVariableExpression(script);
		script.skipEmpty();
		if (!script.isEqualTo('=')) throw UtlException(script, "syntax error in #refAST; '=' expected before the non-terminal");
	}

	ASTRefCommand::~ASTRefCommand() {
		delete _pVariable;
	}

	void ASTRefCommand::execute(ASTCommandEnvironment& env) {
	}


	ASTSlideCommand::ASTSlideCommand(ScpStream& script) : _pVariable(NULL) {
		script.skipEmpty();
		if (!script.isEqualTo('(')) throw UtlException(script, "syntax error in #slideAST; '(' expected");
		script.skipEmpty();
		_pVariable = parseVariableExpression(script);
		script.skipEmpty();
		if (!script.isEqualTo(',')) throw UtlException(script, "syntax error in #slideAST; ',' expected");
		script.skipEmpty();
		if (!script.readIdentifier(_sAttribute)) throw UtlException(script, "syntax error in #slideAST; attribute expected");
		script.skipEmpty();
		if (!script.isEqualTo(')')) throw UtlException(script, "syntax error in #slideAST; ')' expected");
	}

	ASTSlideCommand::~ASTSlideCommand() {
		delete _pVariable;
	}

	void ASTSlideCommand::execute(ASTCommandEnvironment& env) {
	}
}
