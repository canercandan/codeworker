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

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "GrfInsertAssignment.h"

namespace CodeWorker {
	GrfInsertAssignment::~GrfInsertAssignment() {
		if (_pVariable != NULL) delete _pVariable;
		if (_pValue != NULL) delete _pValue;
	}

	SEQUENCE_INTERRUPTION_LIST GrfInsertAssignment::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pVariable = visibility.getVariable(*_pVariable);
		if (_pValue != NULL) {
			ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(_pValue);
			if (pConstantTree != NULL) {
				if (!_bContatenation) pVariable->clearContent();
				pConstantTree->setTree(visibility, *pVariable);
			} else {
				std::string sValue = _pValue->getValue(visibility);
				if (_bContatenation) {
					const char* tcValue = pVariable->getValue();
					if (tcValue != NULL) sValue = std::string(tcValue) + sValue;
				}
				pVariable->setValue(sValue.c_str());
			}
		} 
		return NO_INTERRUPTION;
	}

	void GrfInsertAssignment::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		ExprScriptConstantTree* pConstantTree = dynamic_cast<ExprScriptConstantTree*>(_pValue);
		if (pConstantTree != NULL) {
			if (!_bContatenation) {
				CW_BODY_INDENT;
				_pVariable->compileCppForSet(theCompilerEnvironment);
				CW_BODY_STREAM << ".clearNode()";CW_BODY_ENDL;
			}
			pConstantTree->compileCppInit(theCompilerEnvironment, *_pVariable);
		} else {
			// a classical value
			CW_BODY_INDENT;
			_pVariable->compileCppForSet(theCompilerEnvironment);
			if (_pValue != NULL) {
				if (_bContatenation) CW_BODY_STREAM << ".concatenateValue(";
				else CW_BODY_STREAM << ".setValue(";
				_pValue->compileCpp(theCompilerEnvironment);
				CW_BODY_STREAM << ");";
			} else {
				CW_BODY_STREAM << ";";
			}
			CW_BODY_ENDL;
		}
	}
}
