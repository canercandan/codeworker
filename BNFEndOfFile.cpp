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
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "DtaBNFScript.h"
#include "BNFClause.h"
#include "DtaVisitor.h"
#include "BNFEndOfFile.h"

namespace CodeWorker {
	BNFEndOfFile::BNFEndOfFile(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue) : _pBNFScript(pBNFScript), GrfCommand(pParent), _bContinue(bContinue) {}

	BNFEndOfFile::~BNFEndOfFile() {
	}

	void BNFEndOfFile::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFEndOfFile(*this, env);
	}

	bool BNFEndOfFile::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFEndOfFile::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = _pBNFScript->skipEmptyChars(visibility);
		if (!CGRuntime::readChar().empty()) {
			BNF_SYMBOL_HAS_FAILED
		}
		return NO_INTERRUPTION;
	}

	void BNFEndOfFile::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CPP_COMPILER_BNF_SYMBOL_BEGIN;
		CW_BODY_INDENT << "_compilerClauseSuccess = (CGRuntime::readCharAsInt() < 0);";CW_BODY_ENDL;
		CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
		CPP_COMPILER_BNF_SYMBOL_HAS_FAILED;
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
	}

	std::string BNFEndOfFile::toString() const {
		std::string sText;
		if (_bContinue) sText = "#continue ";
		sText += "#empty";
		return sText;
	}
}
