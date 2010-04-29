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
#include "BNFTransformationMode.h"


namespace CodeWorker {
	class BNFTransformationModeScope {
	private:
		DtaBNFScript* _pBNFScript;
		bool _bOldMode;
		ExprScriptFunction* _pOldFunction;

	public:
		BNFTransformationModeScope(DtaBNFScript* pBNFScript, bool bNewMode, ExprScriptFunction* pFunction) : _pBNFScript(pBNFScript) {
			_bOldMode = pBNFScript->implicitCopy();
			_pOldFunction = pBNFScript->getImplicitCopyFunction();
			pBNFScript->implicitCopy(bNewMode);
			pBNFScript->setImplicitCopyFunction(pFunction);
		}

		~BNFTransformationModeScope() {
			_pBNFScript->implicitCopy(_bOldMode);
			_pBNFScript->setImplicitCopyFunction(_pOldFunction);
		}
	};


	BNFTransformationMode::BNFTransformationMode(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bImplicitCopy, ExprScriptFunction* pFunction) : _pBNFScript(pBNFScript), GrfBlock(pParent), _bImplicitCopy(bImplicitCopy), _pFunction(pFunction) {}

	BNFTransformationMode::~BNFTransformationMode() {
	}

	void BNFTransformationMode::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFTransformationMode(*this, env);
	}

	bool BNFTransformationMode::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFTransformationMode::executeInternal(DtaScriptVariable& visibility) {
		BNFTransformationModeScope theScope(_pBNFScript, _bImplicitCopy, _pFunction);
		int iCursor = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = ((_pBNFScript->implicitCopy()) ? CGRuntime::getOutputLocation() : -1);
		SEQUENCE_INTERRUPTION_LIST result = GrfBlock::executeInternal(visibility);
		if ((result != NO_INTERRUPTION) && (result != CONTINUE_INTERRUPTION)) {
			CGRuntime::setInputLocation(iCursor);
			if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
		}
		return result;
	}

	std::string BNFTransformationMode::toString() const {
		std::string sText;
		if (_bImplicitCopy) sText = "#implicitCopy";
		else sText = "#explicitCopy";
		for (std::vector<GrfCommand*>::const_iterator i = getCommands().begin(); i != getCommands().end(); i++) {
			if ((*i)->isABNFCommand()) {
				sText += " ";
				sText += (*i)->toString();
			}
		}
		return sText;
	}

	void BNFTransformationMode::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			bool bFirstOfRule = (dynamic_cast<BNFClause*>(getParent()) != NULL);
			if (!bFirstOfRule) {
				CW_BODY_INDENT << "{";CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
			}
			CW_BODY_INDENT << "// " << toString();CW_BODY_ENDL;
			CW_BODY_INDENT << "CGBNFRuntimeTransformationMode _compilerClause_transformationMode(theEnvironment, " << ((_bImplicitCopy) ? "true" : "false") << ");";CW_BODY_ENDL;
			int iCursor = theCompilerEnvironment.newCursor();
			char tcImplicitCopy[64];
			sprintf(tcImplicitCopy, "_compilerClauseImplicitCopy_%d", iCursor);
			CW_BODY_INDENT << "int " << tcImplicitCopy << " = CGRuntime::getOutputLocation();";CW_BODY_ENDL;
			CW_BODY_INDENT << "int _compilerClauseCursor_" << iCursor << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
			GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
			CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\tCGRuntime::setInputLocation(_compilerClauseCursor_" << iCursor << ");";CW_BODY_ENDL;
			CW_BODY_INDENT << "\tCGRuntime::resizeOutputStream(" << tcImplicitCopy << ");";CW_BODY_ENDL;
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
			if (!bFirstOfRule) {
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_INDENT << "}";CW_BODY_ENDL;
			}
		}
	}
}
