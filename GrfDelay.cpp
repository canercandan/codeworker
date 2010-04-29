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
#include "DtaProject.h"
#include "ScpStream.h"
#include "UtlTimer.h"
#include "CppCompilerEnvironment.h"
#include "GrfDelay.h"

namespace CodeWorker {
	class GrfDelayTimer {
	private:
		UtlTimer* _pOldTimer;
		UtlTimer _timer;

	public:
		GrfDelayTimer() {
			_pOldTimer = DtaProject::getInstance().getDelayTimer();
			DtaProject::getInstance().setDelayTimer(&_timer);
			_timer.start();
		}
		~GrfDelayTimer() {
			_timer.stop();
			double dDuration = _timer.getTimeInSec();
			DtaProject::getInstance().setLastDelay(dDuration);
			DtaProject::getInstance().setDelayTimer(_pOldTimer);
		}
	};

	GrfDelay::~GrfDelay() {}

	SEQUENCE_INTERRUPTION_LIST GrfDelay::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result;
		GrfDelayTimer theTimer;
		result = GrfBlock::executeInternal(visibility);
		return result;
	}

	void GrfDelay::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "clock_t _compiler_start = clock();";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "clock_t _compiler_finish = clock();";
		CW_BODY_ENDL;
		CW_BODY_INDENT;
		GrfBlock::compileCpp(theCompilerEnvironment);
		CW_BODY_INDENT << "double _compiler_duration = (double)(_compiler_finish - _compiler_start) / CLOCKS_PER_SEC;";
		CW_BODY_ENDL;
		CW_BODY_INDENT << "CGRuntime::setLastDelay(_compiler_duration);";
		CW_BODY_ENDL;
	}
}
