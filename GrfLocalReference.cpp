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
#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "GrfBlock.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "GrfLocalReference.h"

namespace CodeWorker {
	GrfLocalReference::~GrfLocalReference() {
		if (_pVariable != NULL) delete _pVariable;
		if (_pReference != NULL) delete _pReference;
	}

	void GrfLocalReference::setLocalVariable(ExprScriptVariable* pLocalVariable, EXPRESSION_TYPE variableType) {
		_pVariable = pLocalVariable;
		getParent()->addLocalVariable(pLocalVariable->getName(), variableType);
	}

	SEQUENCE_INTERRUPTION_LIST GrfLocalReference::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pVariable = visibility.getNonRecursiveVariable(visibility, *_pVariable);
		pVariable->clearContent();
		DtaScriptVariable* pReference = visibility.getExistingVariable(*_pReference);
		if (pReference == NULL) {
			std::string sCompleteName = pVariable->getCompleteName();
			std::string sExpression = _pReference->toString();
			throw UtlException("runtime error: can't refer to a variable that doesn't exist, see 'localref " + sCompleteName + " = " + sExpression + ";'");
		}
		if (pVariable != pReference) pVariable->setValue(pReference);
		return NO_INTERRUPTION;
	}

	void GrfLocalReference::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (!theCompilerEnvironment.addVariable(_pVariable->getName())) {
			CW_BODY_INDENT << _pVariable->getName() << ".clearNode();";
			CW_BODY_ENDL;
		} else {
			CW_BODY_INDENT << "CppParsingTree_value " << _pVariable->getName() << ";";
			CW_BODY_ENDL;
		}
		CW_BODY_INDENT << _pVariable->getName() << ".setReference(";
		_pReference->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
