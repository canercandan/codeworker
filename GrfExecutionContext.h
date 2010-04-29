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

#ifndef _GrfExecutionContext_h_
#define _GrfExecutionContext_h_

#include "GrfBlock.h"

namespace CodeWorker {
	class UtlException;
	class BNFClauseCall;

	class GrfExecutionContext : public GrfBlock {
	private:
		GrfCommand* _pLastCommand;
		GrfExecutionContext* _pLastExecutionContext;

	protected:
		std::list<GrfCommand*> _stack;

	protected:
		GrfExecutionContext(GrfBlock* pParent = NULL);

		inline GrfCommand* getLastCommand() const { return _pLastCommand; }
		inline unsigned int getCounter(GrfCommand* pCommand) const { return pCommand->_iCounter; }
		inline long getTimeInMillis(GrfCommand* pCommand) const { return pCommand->getTimeInMillis(); }
		inline void incrementCounter(GrfCommand* pCommand) { pCommand->_iCounter++; }
		inline const char* getParsingFilePtr(GrfCommand* pCommand) const { return pCommand->_sParsingFilePtr; }
		inline int getFileLocation(GrfCommand* pCommand) const { return pCommand->_iFileLocation; }

		virtual SEQUENCE_INTERRUPTION_LIST openSession(DtaScriptVariable& visibility) = 0;

		// must be 'final'!
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);

	public:
		virtual ~GrfExecutionContext();

		inline GrfExecutionContext* getLastExecutionContext() { return _pLastExecutionContext; }

		// to call into redefined methods if symbols information
		virtual void handleBeforeExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/);
		virtual void handleAfterExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/) = 0;
		virtual void handleAfterExceptionCBK(GrfCommand* pCommand, DtaScriptVariable& visibility, UtlException& exception) = 0;
		virtual void handleStartingFunction(GrfFunction* pFunction);
		virtual void handleEndingFunction(GrfFunction* pFunction);
		virtual void handleStartingBNFClause(BNFClauseCall* pClauseCall);
		virtual void handleEndingBNFClause(BNFClauseCall* pClauseCall);
		virtual void handleBeforeScriptExecutionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/);
		virtual void handleAfterScriptExecutionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/);

	private:
		static void clearRecursively(GrfCommand* pCommand);
	};
}

#endif
