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

#include "DtaBNFScript.h"
#include "BNFClause.h"
#include "DtaVisitor.h"
#include "BNFIgnore.h"

namespace CodeWorker {
	BNFIgnoreScope::BNFIgnoreScope(DtaBNFScript* pBNFScript, IGNORE_MODE eNewMode, BNFClause* pNewIgnoreClause) : _pBNFScript(pBNFScript) {
		_eOldMode = pBNFScript->getIgnoreMode();
		_pOldIgnoreClause = pBNFScript->getIgnoreClause();
		if (eNewMode != UNDEFINED_IGNORE) {
			pBNFScript->setIgnoreMode(eNewMode);
			pBNFScript->setIgnoreClause(pNewIgnoreClause);
		}
	}

	BNFIgnoreScope::~BNFIgnoreScope() {
		_pBNFScript->setIgnoreMode(_eOldMode);
		_pBNFScript->setIgnoreClause(_pOldIgnoreClause);
	}


	BNFIgnore::BNFIgnore(DtaBNFScript* pBNFScript, GrfBlock* pParent, IGNORE_MODE eMode, BNFClause* pIgnoreClause) : _pBNFScript(pBNFScript), GrfBlock(pParent), _eMode(eMode), _pIgnoreClause(pIgnoreClause) {}

	BNFIgnore::~BNFIgnore() {
	}

	void BNFIgnore::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFIgnore(*this, env);
	}

	bool BNFIgnore::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFIgnore::executeInternal(DtaScriptVariable& visibility) {
		BNFIgnoreScope theScope(_pBNFScript, _eMode, _pIgnoreClause);
		int iImplicitCopyPosition = (_pBNFScript->implicitCopy()) ? CGRuntime::getOutputLocation() : -1;
		SEQUENCE_INTERRUPTION_LIST result = GrfBlock::executeInternal(visibility);
		if ((result == BREAK_INTERRUPTION) && (iImplicitCopyPosition >= 0)) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
		return result;
	}

	std::string BNFIgnore::toString() const {
		std::string sText;
		if (_eMode == NOT_IGNORE) sText = "#!ignore";
		else if (_eMode == IGNORE_CLAUSE) {
			std::string sTemp = _pIgnoreClause->getSignature();
			if (sTemp.size() == 7 /*sizeof("#ignore")*/) sText = "#ignore";
			else {
				sTemp = sTemp.substr(8, sTemp.size() - 9);
				sText += "#ignore(\"" + sTemp + "\")";
			}
		} else sText = "#ignore(" + DtaBNFScript::IGNORE_MODEtoString(_eMode) + ")";
		for (std::vector<GrfCommand*>::const_iterator i = getCommands().begin(); i != getCommands().end(); i++) {
			if ((*i)->isABNFCommand()) {
				sText += " ";
				sText += (*i)->toString();
			}
		}
		return sText;
	}

	void BNFIgnore::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "{";CW_BODY_ENDL;
		theCompilerEnvironment.incrementIndentation();
		CW_BODY_INDENT << "// " << toString();CW_BODY_ENDL;
		int iCursor = theCompilerEnvironment.newCursor();
		compileCppRuntimeIgnore(theCompilerEnvironment, _eMode, _pIgnoreClause, iCursor);
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			CW_BODY_INDENT << "int _compilerClauseCursor_" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
			GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
			CW_BODY_INDENT << "if (!_compilerClauseSuccess) CGRuntime::setInputLocation(_compilerClauseCursor_" << iCursor << ");";
			CW_BODY_ENDL;
		}
		theCompilerEnvironment.decrementIndentation();
		CW_BODY_INDENT << "}";CW_BODY_ENDL;
	}

	void BNFIgnore::compileCppRuntimeIgnore(CppCompilerEnvironment& theCompilerEnvironment, IGNORE_MODE eMode, BNFClause* pIgnoreClause, int iCursor) {
		if (eMode == UNDEFINED_IGNORE) return;
		std::string sIgnoreClauseName;
		if (pIgnoreClause != NULL) {
			sIgnoreClauseName = pIgnoreClause->getName();
			if (sIgnoreClauseName[0] == '#') sIgnoreClauseName = "_compilerDirectiveClause_" + sIgnoreClauseName.substr(1);
			sIgnoreClauseName = DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(theCompilerEnvironment.getClauseModule(pIgnoreClause->getName()))) + "::" + sIgnoreClauseName;
		}
		if (sIgnoreClauseName.empty()) {
			sIgnoreClauseName = "NULL";
		} else {
			sIgnoreClauseName = "new CodeWorker::DEFAULT_EXECUTE_CLAUSE(" + sIgnoreClauseName + ")";
		}
		CW_BODY_INDENT << "CGBNFRuntimeIgnore _compilerClause_ignore" << iCursor << "(theEnvironment, " << (int) eMode << ", " << sIgnoreClauseName << ");";CW_BODY_ENDL;
	}
}
