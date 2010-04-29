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

#ifndef _GrfQuantifyExecution_h_
#define _GrfQuantifyExecution_h_

#pragma warning (disable : 4786)

#include <string>
#include <list>
#include <map>

#include "GrfExecutionContext.h"

namespace CodeWorker {
	class BNFClauseCall;

	class DtaQuantifyFunction {
	public:
		std::string _sName;
		const char* _sFile;
		int _iLocation;
		unsigned int _iCounter;
		long _iTimeInMillis;

	public:
		bool operator < (const DtaQuantifyFunction& function) const;
		bool operator > (const DtaQuantifyFunction& function) const;
		bool operator ==(const DtaQuantifyFunction& function) const;
		bool operator !=(const DtaQuantifyFunction& function) const;
	};


	class GrfQuantifyExecution : public GrfExecutionContext {
	private:
		std::map<std::string, int> _listOfPredefinedFunctions;
		std::map<std::string, std::map<std::string, DtaQuantifyFunction> > _listOfUserFunctions;
		std::map<std::string, std::map<int, int> > _coveredLines;
		ExprScriptExpression* _pFilename;
		unsigned int _iCoveredCode;
		unsigned int _iTotalCode;

	public:
		GrfQuantifyExecution(GrfBlock* pParent) : GrfExecutionContext(pParent), _pFilename(NULL) {}
		virtual ~GrfQuantifyExecution();

		inline void setFilename(ExprScriptExpression* pFilename) { _pFilename = pFilename; }

		virtual void handleBeforeExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/);
		virtual void handleAfterExecutionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/);
		virtual void handleAfterExceptionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/, UtlException& /*exception*/);
		virtual void handleStartingFunction(GrfFunction* pFunction);
		virtual void handleEndingFunction(GrfFunction* pFunction);
		virtual void handleStartingBNFClause(BNFClauseCall* pBNFClause);
		virtual void handleEndingBNFClause(BNFClauseCall* pBNFClause);
		virtual void handleBeforeScriptExecutionCBK(GrfCommand* /*pCommand*/, DtaScriptVariable& /*visibility*/);
		virtual void handleAfterScriptExecutionCBK(GrfCommand* pCommand, DtaScriptVariable& /*visibility*/);
		
	protected:
		virtual SEQUENCE_INTERRUPTION_LIST openSession(DtaScriptVariable& visibility);

	private:
		static GrfQuantifyExecution* getCurrentQuantify() { return (GrfQuantifyExecution*) GrfCommand::getCurrentExecutionContext(); }
		static void recoverData(GrfCommand* pCommand);

		void registerCode(GrfCommand* pCommand);
		void registerUserFunction(GrfFunction* pFunction);
		void registerPredefinedFunction(GrfCommand* pCommand);
		void displayResults(DtaScriptVariable& visibility);
		void generateHTMLFile(const char* sFilename, DtaScriptVariable& visibility);

		inline unsigned int getCoveredCode() const { return _iCoveredCode; }
		inline unsigned int getTotalCode() const { return _iTotalCode; }
	};
}

#endif
