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
#include "DtaArrayIterator.h"
#include "ExprScriptVariable.h"
#include "ExprScriptMotif.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "GrfSelect.h"

namespace CodeWorker {
	GrfSelect::~GrfSelect() {
		delete _pMotifExpr;
	}

	SEQUENCE_INTERRUPTION_LIST GrfSelect::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		std::list<DtaScriptVariable*> listOfNodes;
		_pMotifExpr->filterNodes(visibility, visibility, listOfNodes);
		if (!listOfNodes.empty()) {
			DtaScriptVariable stackSelect(&visibility, "##stack## select");
			stackSelect.createIterator(_pIndexExpr->getName().c_str(), &_pArrayIteratorData);
			result = executeSelect(listOfNodes, stackSelect);
			if (result == BREAK_INTERRUPTION) result = NO_INTERRUPTION;
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST GrfSelect::executeSelect(const std::list<DtaScriptVariable*>& listOfNodes, DtaScriptVariable& stackSelect) {
		SEQUENCE_INTERRUPTION_LIST result;
		DtaListIterator iteratorData(listOfNodes);
		DtaArrayIterator* pOldArrayIteratorData = _pArrayIteratorData;
		_pArrayIteratorData = &iteratorData;
		if (!iteratorData.end()) {
			do {
				result = GrfBlock::executeInternal(stackSelect);
				if ((result != CONTINUE_INTERRUPTION) && (result != NO_INTERRUPTION)) return result;
			} while (iteratorData.next());
		}
		_pArrayIteratorData = pOldArrayIteratorData;
		return NO_INTERRUPTION;
	}

	void GrfSelect::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "<unhandled 'select' statement>";
		CW_BODY_ENDL;
	}
}
