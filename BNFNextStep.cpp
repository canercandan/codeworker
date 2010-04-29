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
#include "BNFStepper.h"
#include "BNFClause.h"
#include "DtaVisitor.h"
#include "BNFNextStep.h"

namespace CodeWorker {
	BNFNextStep::BNFNextStep(DtaBNFScript* pBNFScript, GrfBlock* pParent, BNFStepper* pStepperRE) : _pBNFScript(pBNFScript), GrfCommand(pParent), _pStepper(pStepperRE) {}

	BNFNextStep::~BNFNextStep() {
	}

	void BNFNextStep::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFNextStep(*this, env);
	}

	bool BNFNextStep::isABNFCommand() const { return true; }

	SEQUENCE_INTERRUPTION_LIST BNFNextStep::executeInternal(DtaScriptVariable& visibility) {
		_pBNFScript->skipEmptyChars(visibility);
		int iStepLocation = CGRuntime::getInputLocation();
		if (iStepLocation > _pStepper->getStepLocation()) {
			_pStepper->setStepLocation(iStepLocation);
		}
		return NO_INTERRUPTION;
	}

	void BNFNextStep::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "// " << toString();
		CW_BODY_ENDL;
		CW_BODY_INDENT << "theEnvironment.skipEmptyChars();";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "_compilerClauseNextLocation_" << theCompilerEnvironment.getBNFStepperCursor() << " = CGRuntime::getInputLocation();";
		CW_BODY_ENDL;
	}

	std::string BNFNextStep::toString() const {
		return "#nextStep";
	}
}
