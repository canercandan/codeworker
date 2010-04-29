/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2003 Cédric Lemaire

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

#ifndef _BNFStepoutHook_h_
#define _BNFStepoutHook_h_

#include "GrfFunction.h"

namespace CodeWorker {
	class DtaBNFScript;

	class BNFStepoutHook : public GrfFunction {
	private:
		DtaBNFScript* _pBNFScript;

	public:
		BNFStepoutHook(DtaBNFScript* pBNFScript, GrfBlock* pParent);
		virtual ~BNFStepoutHook();

		virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

		bool setClauseSignature(const char* sSignature);
		bool setParameters(const char* sParameters);
		bool setSuccess(const char* sSuccess);

		SEQUENCE_INTERRUPTION_LIST executeHook(DtaScriptVariable& visibility, const std::string& sSignature, DtaScriptVariable& parameters, bool bSuccess);
	};
}

#endif
