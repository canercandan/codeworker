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

#ifndef _GrfDebugExecution_h_
#define _GrfDebugExecution_h_

#include <string>
#include <list>
#include <vector>
#include <map>

#include "ScpStream.h"

#include "GrfExecutionContext.h"

namespace CodeWorker {
	class ExprScriptExpression;

	class DtaBreakpoint {
	public:
		int _iBeginning;
		int _iEnd;
		std::string _sCondition;
		ExprScriptExpression* _pCondition;
		std::string _sAction;
		GrfBlock* _pAction;

	public:
		DtaBreakpoint() : _iBeginning(0), _iEnd(0), _pCondition(NULL), _pAction(NULL) {}
		~DtaBreakpoint();

		inline const std::string& getCondition() const { return _sCondition; }
		void setCondition(const std::string& sCondition);
		void setAction(const std::string& sAction);
		bool executeCondition(GrfCommand* pCommand, DtaScriptVariable& visibility);
		void executeAction(GrfCommand* pCommand, DtaScriptVariable& visibility);
	};

	class DtaWatchNodeListener;

	class GrfDebugExecution : public GrfExecutionContext {
	private:
		std::map<std::string, std::map<int, DtaBreakpoint*> > _listOfBreakpoints;
		std::string _sCursor;
		bool _bStop;
		DtaBreakpoint* _pConditionalStop;
		bool _bRun;
		GrfCommand* _pNext;
		int  _iIterations;
		int  _iDisplaySize;
		ScpStream _history;
		bool _bReadingHistory;

		std::list<DtaWatchNodeListener*> _watchListeners;
		friend class DtaWatchNodeListener;

	public:
		GrfDebugExecution(GrfBlock* pParent) : GrfExecutionContext(pParent), _bRun(false), _bStop(true), _pNext(NULL), _iDisplaySize(2), _iIterations(0), _pConditionalStop(NULL), _bReadingHistory(false) {}
		virtual ~GrfDebugExecution();

		inline void removeWatchNodeListener(DtaWatchNodeListener* pListener) { _watchListeners.remove(pListener); }

		virtual void handleBeforeExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& visibility);
		virtual void handleAfterExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/);
		virtual void handleAfterExceptionCBK(GrfCommand* pCommand, DtaScriptVariable& visibility, UtlException& exception);
		
		static void help();

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST openSession(DtaScriptVariable& visibility);

	private:
		static GrfDebugExecution* getCurrentDebugger() { return (GrfDebugExecution*) GrfCommand::getCurrentExecutionContext(); }

		bool executeCommand(GrfCommand* pCommand, DtaScriptVariable& visibility, ScpStream& stream);
		std::string display(GrfCommand* pCommand, DtaScriptVariable& visibility, int iSize, bool bEchoOn = true);
		void traceStack(GrfCommand* pCommand, DtaScriptVariable& visibility);

		DtaBreakpoint* registerBreakpoint(DtaScriptVariable& visibility, const char* sFilename, int iLineNumber);
		void clearBreakpoint(DtaScriptVariable& /*visibility*/, const char* sFilename, int iLineNumber);
		DtaBreakpoint* stopOnBreakpoint(GrfCommand* pCommand, DtaScriptVariable& visibility);

		void clearBreakpoints();
		void clearWatchpoints();
		void clearWatchpoint(const std::string& sKey);
	};
}

#endif
