/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2003 Cédric Lemaire

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

#include "ExprScriptFunction.h"
#include "DtaProject.h"
#include "DtaBNFScript.h"
#include "DtaVisitor.h"
#include "BNFStepoutHook.h"

namespace CodeWorker {
	BNFStepoutHook::BNFStepoutHook(DtaBNFScript* pBNFScript, GrfBlock* pParent) : GrfFunction(pParent, "stepintoHook", "", false), _pBNFScript(pBNFScript) {
		pBNFScript->setStepoutHook(this);
	}

	BNFStepoutHook::~BNFStepoutHook() {
		_pBNFScript->setStepintoHook(NULL);
	}

	void BNFStepoutHook::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFStepoutHook(*this, env);
	}

	bool BNFStepoutHook::setParameters(const char* sParameters) {
		return addParameterAndType(sParameters, NODE_EXPRTYPE, NULL);
	}

	bool BNFStepoutHook::setClauseSignature(const char* sSignature) {
		return addParameterAndType(sSignature, VALUE_EXPRTYPE, NULL);
	}

	bool BNFStepoutHook::setSuccess(const char* sSuccess) {
		return addParameterAndType(sSuccess, VALUE_EXPRTYPE, NULL);
	}

	SEQUENCE_INTERRUPTION_LIST BNFStepoutHook::executeHook(DtaScriptVariable& visibility, const std::string& sSignature, DtaScriptVariable& parameters, bool bSuccess) {
		std::auto_ptr<ExprScriptFunction> pFunctionCall(new ExprScriptFunction(this));
		pFunctionCall->addParameter(new ExprScriptConstant(sSignature.c_str()));
		pFunctionCall->addParameter(new ExprScriptResolvedVariable(&parameters));
		pFunctionCall->addParameter(new ExprScriptConstant(bSuccess));
		launchExecution(visibility, *pFunctionCall);
		return NO_INTERRUPTION;
	}
}
