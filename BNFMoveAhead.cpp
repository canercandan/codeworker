/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2004 Cédric Lemaire

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
#include "DtaVisitor.h"
#include "BNFMoveAhead.h"

namespace CodeWorker {
	BNFMoveAhead::BNFMoveAhead(GrfBlock* pParent, bool bContinue) : GrfBlock(pParent), _bContinue(bContinue) {}

	BNFMoveAhead::~BNFMoveAhead() {}

	void BNFMoveAhead::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFMoveAhead(*this, env);
	}

	bool BNFMoveAhead::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFMoveAhead::executeInternal(DtaScriptVariable& visibility) {
		int iLocation = CGRuntime::getInputLocation();
		SEQUENCE_INTERRUPTION_LIST result = GrfBlock::executeInternal(visibility);
		if (result == NO_INTERRUPTION) {
			if (iLocation == CGRuntime::getInputLocation()) {
				if (_bContinue) CGRuntime::throwBNFExecutionError(toString());
				CGRuntime::setInputLocation(iLocation);
				return BREAK_INTERRUPTION;
			}
		}
		return result;
	}

	std::string BNFMoveAhead::toString() const {
		std::string sText = "#moveAhead";
		if (_bContinue) sText = "#continue " + sText;
		for (std::vector<GrfCommand*>::const_iterator i = getCommands().begin(); i != getCommands().end(); i++) {
			if ((*i)->isABNFCommand()) {
				if (i != getCommands().begin()) sText += " ";
				sText += (*i)->toString();
			}
		}
		return sText;
	}

	void BNFMoveAhead::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::vector<GrfCommand*>::const_iterator i = getCommands().begin();
		if (i != getCommands().end()) {
			CW_BODY_INDENT << "// " << toString();
			CW_BODY_ENDL;
			int iCursor = theCompilerEnvironment.newCursor();
			char tcLocation[64];
			sprintf(tcLocation, "_compilerClauseLocation_%d", iCursor);
			CW_BODY_INDENT << "int " << tcLocation << " = CGRuntime::getInputLocation();";CW_BODY_ENDL;
			GrfBlock::compileCppBNFSequence(theCompilerEnvironment);
			CW_BODY_INDENT << "if (_compilerClauseSuccess && (" << tcLocation << " == CGRuntime::getInputLocation())) {";CW_BODY_ENDL;
			if (_bContinue) {
				CW_BODY_INDENT << "\tCGRuntime::throwBNFExecutionError(";
				CW_BODY_STREAM.writeString(toString());
				CW_BODY_STREAM << ");";CW_BODY_ENDL;
			} else {
				CW_BODY_INDENT << "\t_compilerClauseSuccess = false;";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
		}
	}
}
