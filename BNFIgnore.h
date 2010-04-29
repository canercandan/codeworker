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

#ifndef _BNFIgnore_h_
#define _BNFIgnore_h_

#include "GrfBlock.h"

// please include this header after including "DtaBNFScript.h"

namespace CodeWorker {
	class DtaBNFScript;
	class BNFClause;

	enum IGNORE_MODE {
		UNDEFINED_IGNORE,
		NOT_IGNORE,
		IGNORE_CPP,
		IGNORE_JAVA,
		IGNORE_HTML,
		IGNORE_BLANKS,
		IGNORE_SPACES,
		IGNORE_ADA,
		IGNORE_LATEX,
		IGNORE_CPP_EXCEPT_DOXYGEN,
		IGNORE_CLAUSE
	};

	class BNFIgnoreScope {
	private:
		DtaBNFScript* _pBNFScript;
		IGNORE_MODE _eOldMode;
		BNFClause* _pOldIgnoreClause;

	public:
		BNFIgnoreScope(DtaBNFScript* pBNFScript, IGNORE_MODE eNewMode, BNFClause* pNewIgnoreClause);
		~BNFIgnoreScope();
	};

	class BNFIgnore : public GrfBlock {
	private:
		DtaBNFScript* _pBNFScript;
		IGNORE_MODE _eMode;
		BNFClause* _pIgnoreClause;

	public:
		BNFIgnore(DtaBNFScript* pBNFScript, GrfBlock* pParent, IGNORE_MODE eMode, BNFClause* pIgnoreClause);
		virtual ~BNFIgnore();

		virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

		virtual bool isABNFCommand() const;

		virtual std::string toString() const;

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		static void compileCppRuntimeIgnore(CppCompilerEnvironment& theCompilerEnvironment, IGNORE_MODE eMode, BNFClause* pIgnoreClause, int iCursor);

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
	};
}

#endif
