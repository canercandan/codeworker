/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2010 Cédric Lemaire

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
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"
#include "ExprScriptExpression.h"
#include <string>

//##protect##"INCLUDE FILES"
#include "DtaScriptVariable.h"
//##protect##"INCLUDE FILES"

#include "GrfInsertText.h"

namespace CodeWorker {
	GrfInsertText::~GrfInsertText() {
		delete _pLocation;
		delete _pText;
	}

	SEQUENCE_INTERRUPTION_LIST GrfInsertText::executeInternal(DtaScriptVariable& visibility) {
		std::string sLocation = _pLocation->getValue(visibility);
		int iLocation = atoi(sLocation.c_str());
		std::string sText = _pText->getValue(visibility);
//##protect##"execute"
		if ((*_pOutputCoverage != NULL) && CGRuntime::getOutputStream()->insertText(sText, iLocation)) {
			int iCoverageLocation = iLocation + (*_pOutputCoverage)->getIntValue();
			DtaScriptVariable* pCoverage = getCoverageItem(*_pOutputCoverage, sText.size(), iCoverageLocation, "I");
			pCoverage->insertNode("script")->setValue(_iFileLocation);
			pCoverage->insertNode("output")->setValue(iCoverageLocation);
			return NO_INTERRUPTION;
		}
//##protect##"execute"
		return CGRuntime::insertText(iLocation, sText);
	}

//##protect##"implementation"
	void GrfInsertText::prepareCoverage(DtaScriptVariable* const* pOutputCoverage) {
		_pOutputCoverage = pOutputCoverage;
	}

	int GrfInsertText::shiftCoverageItem(DtaScriptVariable* pCoverage, int iLocation, int iLength) {
		std::list<DtaScriptVariable*>* pList = const_cast<std::list<DtaScriptVariable*>*>(pCoverage->getArray());
		if (pList == NULL) return -1;
		int iRank = pList->size();
		for (std::list<DtaScriptVariable*>::reverse_iterator i = pList->rbegin(); i != pList->rend(); ++i) {
			DtaScriptVariable* pOutput = (*i)->getNode("output");
			int iOutput = pOutput->getIntValue();
			if (iOutput < iLocation) {
				break;
			}
			pOutput->setValue(iOutput + iLength);
			--iRank;
		}
		return iRank;
	}

	DtaScriptVariable* GrfInsertText::getCoverageItem(DtaScriptVariable* pCoverage, int iLocation, int iLength, const std::string& sType) {
		std::list<DtaScriptVariable*>* pList = const_cast<std::list<DtaScriptVariable*>*>(pCoverage->getArray());
		if (pList == NULL) {
			return pCoverage->pushItem(sType);
		}
		int iRank = shiftCoverageItem(pCoverage, iLocation, iLength);
		if (iRank < 0) return NULL;
		DtaScriptVariable* pItem = pCoverage->insertElementAt("", iRank);
		pItem->setValue(sType.c_str());
		return pItem;
	}
//##protect##"implementation"

	void GrfInsertText::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::insertText(";
		_pLocation->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pText->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
