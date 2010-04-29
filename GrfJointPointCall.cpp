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
#include "GrfJointPoint.h"
#include "GrfJointPointCall.h"

namespace CodeWorker {
	GrfJointPointCall::~GrfJointPointCall() {
		delete _pContext;
	}

	SEQUENCE_INTERRUPTION_LIST GrfJointPointCall::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		DtaScriptVariable* pContext = (_pContext == NULL) ? &visibility : visibility.getExistingVariable(*_pContext);
		if (pContext != NULL) {
			if (CGRuntime::getActiveJointPoint() == NULL) {
				THROW_UTLEXCEPTION("cannot execute a '#jointcut' directive out of an advice");
			}
			std::string sLabel = "#jointpoint '" + CGRuntime::getActiveJointPoint()->getName() + "'";
			UTLTRACE_STACK_FUNCTION(_sParsingFilePtr, sLabel.c_str(), _iFileLocation);
			CGRuntime::getActiveJointPoint()->executeHeartIteration(*pContext);
		}
		return result;
	}

	void GrfJointPointCall::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "<GrfJointPointCall not handled yet!>";CW_BODY_ENDL;
	}
}
