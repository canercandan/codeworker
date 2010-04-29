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
#include "DtaProject.h"
#include "ExprScriptVariable.h"
#include "GrfBlock.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "GrfGlobalVariable.h"

namespace CodeWorker {
	GrfGlobalVariable::~GrfGlobalVariable() {
		if (_pValue != NULL) delete _pValue;
	}

	void GrfGlobalVariable::setVariable(const std::string& sVariable, EXPRESSION_TYPE varType) {
		_sVariable = sVariable;
		DtaProject::getInstance().setGlobalVariableType(sVariable, varType);
	}

	SEQUENCE_INTERRUPTION_LIST GrfGlobalVariable::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pGlobalVariable = DtaProject::getInstance().setGlobalVariable(_sVariable);
		pGlobalVariable->clearContent();
		if (_pValue != NULL) {
			ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(_pValue);
			if (pConstantTree != NULL) {
				pConstantTree->setTree(visibility, *pGlobalVariable);
			} else {
				std::string sValue = _pValue->getValue(visibility);
				pGlobalVariable->setValue(sValue.c_str());
			}
		}
		return NO_INTERRUPTION;
	}

	void GrfGlobalVariable::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		theCompilerEnvironment.addGlobalVariable(_sVariable);
		CW_BODY_INDENT << _sVariable << ".clearNode();";CW_BODY_ENDL;
		if (_pValue != NULL) {
			ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(_pValue);
			if (pConstantTree != NULL) {
				pConstantTree->compileCppInit(theCompilerEnvironment, _sVariable);
			} else {
				// a classical value type
				CW_BODY_INDENT << _sVariable << ".setValue(";
				_pValue->compileCpp(theCompilerEnvironment);
				CW_BODY_STREAM << ");";
				CW_BODY_ENDL;
			}
		}
	}
}
