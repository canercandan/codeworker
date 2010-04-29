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

#include "ScpStream.h"
#include "DtaScriptVariable.h"
#include "CppCompilerEnvironment.h"
#include "GrfText.h"


namespace CodeWorker {
	GrfText::~GrfText() {}

	SEQUENCE_INTERRUPTION_LIST GrfText::executeInternal(DtaScriptVariable&) {
		if (*_pOutputCoverage != NULL) {
			DtaScriptVariable* pCoverage = (*_pOutputCoverage)->pushItem("R");
			pCoverage->insertNode("script")->setValue(_iFileLocation);
			pCoverage->insertNode("output")->setValue((*_pOutput)->getOutputLocation() + (*_pOutputCoverage)->getIntValue());
		}
		(*_pOutput)->writeBinaryData(_sText.c_str(), _sText.size());
		return NO_INTERRUPTION;
	}

	void GrfText::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::string::size_type iStart = 0;
		char tcLine[513];
		for(;;) {
			if (iStart + 512 < _sText.size()) {
				tcLine[512] = '\0';
				memcpy(tcLine, _sText.c_str() + iStart, 512);
				CW_BODY_INDENT << "CGRuntime::writeText(";
				CW_BODY_STREAM.writeString(tcLine);
				CW_BODY_STREAM << ");";
				CW_BODY_ENDL;
				iStart += 512;
			} else {
				CW_BODY_INDENT << "CGRuntime::writeText(";
				CW_BODY_STREAM.writeString(_sText.c_str() + iStart);
				CW_BODY_STREAM << ");";
				CW_BODY_ENDL;
				break;
			}
		}
	}
}
