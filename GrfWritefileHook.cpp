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

#include "ExprScriptFunction.h"
#include "DtaProject.h"
#include "CppParsingTree.h"
#include "GrfWritefileHook.h"

namespace CodeWorker {
	GrfWritefileHook::GrfWritefileHook(GrfBlock* pParent) : GrfFunction(pParent, "writefileHook", "", false), _writefileHook(NULL) {
		DtaProject::getInstance().setWritefileHook(this);
	}

	GrfWritefileHook::GrfWritefileHook(WRITEFILEHOOK_FUNCTION writefileHook) : GrfFunction(NULL, "writefileHook", "", false), _writefileHook(writefileHook) {
		DtaProject::getInstance().setWritefileHook(this);
	}

	GrfWritefileHook::~GrfWritefileHook() {
		if (DtaProject::existInstance()) DtaProject::getInstance().setWritefileHook(NULL);
	}

	bool GrfWritefileHook::setFileNameArgument(const char* sArgument) {
		if (!getParameterTypes().empty()) return false;
		return addParameterAndType(sArgument, VALUE_EXPRTYPE, NULL);
	}

	bool GrfWritefileHook::setPositionArgument(const char* sArgument) {
		if (getParameterTypes().size() != 1) return false;
		return addParameterAndType(sArgument, VALUE_EXPRTYPE, NULL);
	}

	bool GrfWritefileHook::setCreationArgument(const char* sArgument) {
		if (getParameterTypes().size() != 2) return false;
		return addParameterAndType(sArgument, VALUE_EXPRTYPE, NULL);
	}

	std::string GrfWritefileHook::executeHook(DtaScriptVariable& visibility, const std::string& sFile, int iPosition, bool bCreation) {
		std::string sSuccess;
		if (_writefileHook != NULL) {
			sSuccess = _writefileHook(sFile, iPosition, bCreation);
		} else {
			std::auto_ptr<ExprScriptFunction> pFunctionCall(new ExprScriptFunction(this));
			pFunctionCall->addParameter(new ExprScriptConstant(sFile.c_str()));
			pFunctionCall->addParameter(new ExprScriptConstant(iPosition));
			pFunctionCall->addParameter(new ExprScriptConstant(bCreation));
			sSuccess = launchExecution(visibility, *pFunctionCall);
		}
		return sSuccess;
	}
}
