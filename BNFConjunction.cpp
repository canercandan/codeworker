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
#include "DtaBNFScript.h"
#include "BNFClause.h"
#include "DtaVisitor.h"
#include "BNFConjunction.h"

namespace CodeWorker {
	BNFConjunction::BNFConjunction(DtaBNFScript* pBNFScript, GrfBlock* pParent) : _pBNFScript(pBNFScript), GrfBlock(pParent) {}

	BNFConjunction::~BNFConjunction() {
	}

	void BNFConjunction::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFConjunction(*this, env);
	}

	bool BNFConjunction::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFConjunction::execute(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		int iLocation = CGRuntime::getInputLocation();
		int iImplicitCopyPosition = ((_pBNFScript->implicitCopy()) ? CGRuntime::getOutputLocation() : -1);
		result = GrfBlock::executeInternal(visibility);
		if (result == BREAK_INTERRUPTION) {
			if (_pBNFScript->getParentClauseMatching() != NULL) {
				// if the coverage recording in ON, purge it from here
				_pBNFScript->getParentClauseMatching()->purgeChildsAfterPosition(iLocation);
			}
			if (_pBNFScript->getRatchetPosition() > iLocation) {
				CGRuntime::throwBNFExecutionError(toString(), "#ratchet position reached");
			}
			CGRuntime::setInputLocation(iLocation);
			if (iImplicitCopyPosition >= 0) CGRuntime::resizeOutputStream(iImplicitCopyPosition);
		}
		return result;
	}

	void BNFConjunction::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			CW_BODY_INDENT << "// " << toString();
			CW_BODY_ENDL;
			int iCursor = theCompilerEnvironment.newCursor();
			char tcLocation[64];
			sprintf(tcLocation, "_compilerClauseLocation_%d", iCursor);
			char tcImplicitCopy[64];
			sprintf(tcImplicitCopy, "_compilerClauseImplicitCopy_%d", iCursor);
			CW_BODY_INDENT << "int " << tcLocation << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
			CW_BODY_INDENT << "int " << tcImplicitCopy << " = ((theEnvironment.implicitCopy()) ? CGRuntime::getOutputLocation() : -1);";CW_BODY_ENDL;
			GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
			CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
			if (_pBNFScript->hasCoverageRecording()) {
				// coverage recording required
				CW_BODY_INDENT << "\t_compilerClauseMatching.purgeChildsAfterPosition(" << tcLocation << ");";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "\tCGRuntime::setInputLocation(" << tcLocation << ");";CW_BODY_ENDL;
			CW_BODY_INDENT << "\tif (" << tcImplicitCopy << " >= 0) CGRuntime::resizeOutputStream(" << tcImplicitCopy << ");";CW_BODY_ENDL;
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
		}
	}

	std::string BNFConjunction::toString() const {
		std::string sText;
		for (std::vector<GrfCommand*>::const_iterator i = getCommands().begin(); i != getCommands().end(); i++) {
			if ((*i)->isABNFCommand()) {
				if (i != getCommands().begin()) sText += " ";
				sText += (*i)->toString();
			}
		}
		return sText;
	}
}
