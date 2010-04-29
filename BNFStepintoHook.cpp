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
#include "BNFStepintoHook.h"

namespace CodeWorker {
	BNFStepintoHook::BNFStepintoHook(DtaBNFScript* pBNFScript, GrfBlock* pParent) : GrfFunction(pParent, "stepintoHook", "", false), _pBNFScript(pBNFScript) {
		pBNFScript->setStepintoHook(this);
	}

	BNFStepintoHook::~BNFStepintoHook() {
		_pBNFScript->setStepintoHook(NULL);
	}

	void BNFStepintoHook::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFStepintoHook(*this, env);
	}

	bool BNFStepintoHook::setParameters(const char* sParameters) {
		return addParameterAndType(sParameters, NODE_EXPRTYPE, NULL);
	}

	bool BNFStepintoHook::setClauseSignature(const char* sSignature) {
		return addParameterAndType(sSignature, VALUE_EXPRTYPE, NULL);
	}

	SEQUENCE_INTERRUPTION_LIST BNFStepintoHook::executeHook(DtaScriptVariable& visibility, const std::string& sSignature, DtaScriptVariable& parameters) {
		std::auto_ptr<ExprScriptFunction> pFunctionCall(new ExprScriptFunction(this));
		pFunctionCall->addParameter(new ExprScriptConstant(sSignature.c_str()));
		pFunctionCall->addParameter(new ExprScriptResolvedVariable(&parameters));
		launchExecution(visibility, *pFunctionCall);
		return NO_INTERRUPTION;
	}
}
