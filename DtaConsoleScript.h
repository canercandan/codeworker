/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2002 C�dric Lemaire

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

#ifndef _DtaConsoleScript_h_
#define _DtaConsoleScript_h_

#include "DtaScript.h"

namespace CodeWorker {
	class DtaConsoleScript : public DtaScript {
	private:
		std::string _sCursor;

	public:
		DtaConsoleScript(GrfBlock* pParentBlock) : DtaScript(pParentBlock) {}
		virtual ~DtaConsoleScript();

		virtual void traceEngine() const;

		virtual SEQUENCE_INTERRUPTION_LIST execute(DtaScriptVariable& thisContext);

	protected:
		virtual void beforeParsingABlock(ScpStream&, GrfBlock&);
		virtual void afterParsingABlock(ScpStream&, GrfBlock&);
		virtual bool betweenCommands(ScpStream& script, GrfBlock&);
		virtual void handleNotAWordCommand(ScpStream& script, GrfBlock& block);
		virtual void handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block);

	private:
		void waitForCommand(ScpStream& stream);
	};
}

#endif
