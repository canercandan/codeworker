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

#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "DtaBNFScript.h"
#include "DtaVisitor.h"
#include "BNFInsert.h"

namespace CodeWorker {

	BNFInsert::~BNFInsert() {
	}

	void BNFInsert::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFInsert(*this, env);
	}

	bool BNFInsert::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFInsert::execute(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		DtaScriptVariable* pVariable = visibility.getExistingVariable(*_pVariable);
		bool bCreation = (pVariable == NULL);
		if (bCreation) {
			pVariable = visibility.getVariable(*_pVariable);
			if (pVariable->isLocal()) CGRuntime::throwBNFExecutionError("declare the local variable '" + _pVariable->toString() + "' before calling '#insert'");
		}
		int iCursor = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = ((_pBNFScript->implicitCopy()) ? CGRuntime::getOutputLocation() : -1);
		result = GrfBlock::executeInternal(visibility);
		if (result != NO_INTERRUPTION) {
			if (bCreation) CGRuntime::removeVariable(pVariable);
			CGRuntime::setInputLocation(iCursor);
			if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
		}
		return result;
	}

	void BNFInsert::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			CW_BODY_INDENT << "// " << toString();
			CW_BODY_ENDL;
			int iCursor = theCompilerEnvironment.newCursor();
			CW_BODY_INDENT << "int _compilerClauseCursor_" << iCursor << " = CGRuntime::getInputLocation();";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "bool _compilerClauseCreation_" << iCursor << " = CGRuntime::existVariable(";
			_pVariable->compileCpp(theCompilerEnvironment);
			CW_BODY_STREAM << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT;
			_pVariable->compileCppForSet(theCompilerEnvironment);
			CW_BODY_STREAM << ";";CW_BODY_ENDL;
			CW_BODY_INDENT << "if (_compilerClauseCreation_" << iCursor << " && ";
			_pVariable->compileCpp(theCompilerEnvironment);
			CW_BODY_STREAM << ".isLocal()) CGRuntime::throwBNFExecutionError(\"declare the local variable '" << _pVariable->toString() << "' before calling '#insert'\");";CW_BODY_ENDL;
			GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
			CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tif (_compilerClauseCreation_" << iCursor << ") CGRuntime::removeVariable(";
			_pVariable->compileCppForSet(theCompilerEnvironment);
			CW_BODY_STREAM << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "\tCGRuntime::setInputLocation(_compilerClauseCursor_" << iCursor << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "}";
			CW_BODY_ENDL;
		}
	}

	std::string BNFInsert::toString() const {
		std::string sText = "#insert(" + _pVariable->toString() + ")";
		for (std::vector<GrfCommand*>::const_iterator i = getCommands().begin(); i != getCommands().end(); i++) {
			if ((*i)->isABNFCommand()) {
				if (i != getCommands().begin()) sText += " ";
				sText += (*i)->toString();
			}
		}
		return sText;
	}

}
