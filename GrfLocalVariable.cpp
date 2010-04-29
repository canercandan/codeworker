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
#include "GrfLocalVariable.h"

namespace CodeWorker {
	GrfLocalVariable::~GrfLocalVariable() {
		if (_pVariable != NULL) delete _pVariable;
		if (_pValue != NULL) delete _pValue;
	}

	void GrfLocalVariable::setLocalVariable(ExprScriptVariable* pLocalVariable, EXPRESSION_TYPE variableType) {
		_pVariable = pLocalVariable;
		getParent()->addLocalVariable(pLocalVariable->getName(), variableType);
	}

	SEQUENCE_INTERRUPTION_LIST GrfLocalVariable::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pLocalVariable = visibility.getNonRecursiveNonReferencedVariable(visibility, *_pVariable);
		if (pLocalVariable->getReferencedVariable() != 0) pLocalVariable->clearValue();
		else pLocalVariable->clearContent();
		if (_pValue != NULL) {
			ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(_pValue);
			if (pConstantTree != NULL) {
				pConstantTree->setTree(visibility, *pLocalVariable);
			} else {
				std::string sValue = _pValue->getValue(visibility);
				pLocalVariable->setValue(sValue.c_str());
			}
		}
		return NO_INTERRUPTION;
	}

	void GrfLocalVariable::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		if (!theCompilerEnvironment.addVariable(_pVariable->getName())) {
			CW_BODY_INDENT << "if (" << _pVariable->getName() << ".getReference() != NULL) " << _pVariable->getName() << ".clearValue();";CW_BODY_ENDL;
			CW_BODY_INDENT << "else " << _pVariable->getName() << ".clearNode();";CW_BODY_ENDL;
			if (_pValue != NULL) {
				ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(_pValue);
				if (pConstantTree != NULL) {
					pConstantTree->compileCppInit(theCompilerEnvironment, _pVariable->getName());
				} else {
					// a classical value type
					CW_BODY_INDENT << _pVariable->getName() << ".setValue(";
					_pValue->compileCpp(theCompilerEnvironment);
					CW_BODY_STREAM << ");";CW_BODY_ENDL;
				}
			}
		} else {
			CW_BODY_INDENT << "CppParsingTree_value " << _pVariable->getName();
			if (_pValue != NULL) {
				ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(_pValue);
				if (pConstantTree != NULL) {
					CW_BODY_STREAM << ";";CW_BODY_ENDL;
					pConstantTree->compileCppInit(theCompilerEnvironment, _pVariable->getName());
				} else {
					// a classical value type
					CW_BODY_STREAM << "(";
					_pValue->compileCpp(theCompilerEnvironment);
					CW_BODY_STREAM << ");";CW_BODY_ENDL;
				}
			} else {
				CW_BODY_STREAM << ";";CW_BODY_ENDL;
			}
		}
	}
}
