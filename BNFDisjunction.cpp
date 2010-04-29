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
#include "BNFDisjunction.h"

namespace CodeWorker {
	BNFDisjunction::BNFDisjunction(DtaBNFScript* pBNFScript, GrfBlock* pParent) : _pBNFScript(pBNFScript), GrfBlock(pParent) {}

	BNFDisjunction::~BNFDisjunction() {
	}

	void BNFDisjunction::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFDisjunction(*this, env);
	}

	bool BNFDisjunction::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFDisjunction::execute(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		int iLocation = CGRuntime::getInputLocation();
		if (hasLocalVariables()) {
			DtaScriptVariable localVariables(&visibility, "##stack## block");
			for (int i = 0; i < getNbCommands(); i++) {
				result = GrfBlock::executeInto(localVariables, i, i);
				if (result != BREAK_INTERRUPTION) break;
				if (_pBNFScript->getRatchetPosition() > iLocation) {
					CGRuntime::throwBNFExecutionError(toString(), "#ratchet position reached");
				}
				CGRuntime::setInputLocation(iLocation);
			}
		} else {
			for (int i = 0; i < getNbCommands(); i++) {
				result = GrfBlock::executeInto(visibility, i, i);
				if (result != BREAK_INTERRUPTION) break;
				if (_pBNFScript->getRatchetPosition() > iLocation) {
					CGRuntime::throwBNFExecutionError(toString(), "#ratchet position reached");
				}
				CGRuntime::setInputLocation(iLocation);
			}
		}
		return result;
	}

	void BNFDisjunction::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			CW_BODY_INDENT << "// " << toString();CW_BODY_ENDL;
			CW_BODY_INDENT << "{";CW_BODY_ENDL;
			theCompilerEnvironment.incrementIndentation();
			for (;;) {
				(*i)->compileCpp(theCompilerEnvironment);
				i++;
				if (i == getCommands().end()) break;
				CW_BODY_INDENT << "if (!_compilerClauseSuccess) {";CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "_compilerClauseSuccess = true;";CW_BODY_ENDL;
			}
			int j = getNbCommands() - 1;
			while (j > 0) {
				theCompilerEnvironment.decrementIndentation();
				CW_BODY_INDENT << "}";CW_BODY_ENDL;
				j--;
			}
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
		}
	}

	std::string BNFDisjunction::toString() const {
		std::string sText;
		for (std::vector<GrfCommand*>::const_iterator i = getCommands().begin(); i != getCommands().end(); i++) {
			if ((*i)->isABNFCommand()) {
				if (i != getCommands().begin()) sText += " | ";
				sText += (*i)->toString();
			}
		}
		return sText;
	}
}
