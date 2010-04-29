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

#ifndef _GrfWritefileHook_h_
#define _GrfWritefileHook_h_

#include "GrfFunction.h"

namespace CodeWorker {
	class CppParsingTree_value;

	#ifndef WRITEFILEHOOK_FUNCTION_TYPE
	#define WRITEFILEHOOK_FUNCTION_TYPE
	typedef std::string (*WRITEFILEHOOK_FUNCTION)(CppParsingTree_value, CppParsingTree_value, CppParsingTree_value);
	#endif

	class GrfWritefileHook : public GrfFunction {
	private:
		WRITEFILEHOOK_FUNCTION _writefileHook;

	public:
		GrfWritefileHook(GrfBlock* pParent);
		GrfWritefileHook(WRITEFILEHOOK_FUNCTION writefileHook);
		virtual ~GrfWritefileHook();

		bool setFileNameArgument(const char* sArgument);
		bool setPositionArgument(const char* sArgument);
		bool setCreationArgument(const char* sArgument);

		std::string executeHook(DtaScriptVariable& visibility, const std::string& sFile, int iPosition, bool bCreation);
	};
}

#endif
