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

#ifndef _GrfTryCatch_h_
#define _GrfTryCatch_h_

#include "GrfCommand.h"

namespace CodeWorker {
	class GrfBlock;
	class ExprScriptVariable;

	class GrfTryCatch : public GrfCommand {
	private:
		ExprScriptVariable* _pVariable;
		GrfBlock* _pTryBlock;
		GrfBlock* _pCatchBlock;

	public:
		GrfTryCatch() : _pVariable(NULL), _pTryBlock(NULL), _pCatchBlock(NULL) {}
		virtual ~GrfTryCatch();

		inline ExprScriptVariable* getErrorVariable() const { return _pVariable; }
		inline void setErrorVariable(ExprScriptVariable* pVariable) { _pVariable = pVariable; }
		inline GrfBlock* getTryBlock() const { return _pTryBlock; }
		inline void setTryBlock(GrfBlock* pBlock) { _pTryBlock = pBlock; }
		inline GrfBlock* getCatchBlock() const { return _pCatchBlock; }
		inline void setCatchBlock(GrfBlock* pBlock) { _pCatchBlock = pBlock; }

		virtual void applyRecursively(APPLY_ON_COMMAND_FUNCTION apply);

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);

	private:
		SEQUENCE_INTERRUPTION_LIST  handleCatch(DtaScriptVariable& visibility, const std::string& sError);
	};
}

#endif
