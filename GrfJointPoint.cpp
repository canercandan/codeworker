/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2006 Cédric Lemaire

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
#include "UtlTrace.h"

#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"
#include "DtaPatternScript.h"
#include "GrfJointPoint.h"

namespace CodeWorker {
	GrfJointPoint::~GrfJointPoint() {
		delete _pContext;
	}

	SEQUENCE_INTERRUPTION_LIST GrfJointPoint::executeHeart(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		if (_bIterate) {
			const std::list<DtaScriptVariable*>* pArray = visibility.getArray();
			if (pArray != NULL) {
				for (std::list<DtaScriptVariable*>::const_iterator i = pArray->begin(); i != pArray->end(); ++i) {
					_pPatternScript->weaveBeforeIteration(*this, *(*i));
					if (!_pPatternScript->weaveAroundIteration(*this, *(*i))) {
						if (executeHeartIteration(*(*i)) == BREAK_INTERRUPTION) {
							break;
						}
					}
					_pPatternScript->weaveAfterIteration(*this, *(*i));
				}
			}
		} else {
			result = GrfBlock::executeInternal(visibility);
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST GrfJointPoint::executeHeartIteration(DtaScriptVariable& item) {
		CGThisModifier stackedThis(&item);
		return GrfBlock::executeInternal(item);
	}

	SEQUENCE_INTERRUPTION_LIST GrfJointPoint::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		DtaScriptVariable* pContext = (_pContext == NULL) ? &visibility : visibility.getExistingVariable(*_pContext);
		if (pContext != NULL) {
			UTLTRACE_STACK_FUNCTION(_sParsingFilePtr, _sName.c_str(), _iFileLocation);
			CGJointPointStack jpStack(this);
			_pPatternScript->weaveBefore(*this, *pContext);
			if (!_pPatternScript->weaveAround(*this, *pContext)) {
				result = executeHeart(*pContext);
			}
			_pPatternScript->weaveAfter(*this, *pContext);
		}
		return result;
	}

	void GrfJointPoint::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "<GrfJointPoint not handled yet!>";CW_BODY_ENDL;
	}
}
