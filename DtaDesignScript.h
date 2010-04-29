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

#ifndef _DtaDesignScript_h_
#define _DtaDesignScript_h_

#include "DtaScript.h"

#include <map>

namespace CodeWorker {
	class ScpStream;
	class DtaProtectedAreasBag;

	class DtaDesignScript : public DtaScript {
	private:
		DtaProtectedAreasBag* _pProtectedAreasBag;

	public:
		DtaDesignScript(GrfBlock* pParentBlock);
		virtual ~DtaDesignScript();

		DtaProtectedAreasBag& getProtectedAreasBag();

		virtual DtaScriptFactory::SCRIPT_TYPE getType() const;
		virtual bool isAParseScript() const;

		virtual void traceEngine() const;

		SEQUENCE_INTERRUPTION_LIST loadDesign(const char* sFile, DtaScriptVariable& thisContext);
		virtual SEQUENCE_INTERRUPTION_LIST execute(DtaScriptVariable& thisContext);

	protected:
		virtual void handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block);
//##markup##"parsing"
//##begin##"parsing"
		virtual void parseAttachInputToSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseDetachInputFromSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseGoBack(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseSetInputLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
//##end##"parsing"
	};
}

#endif
