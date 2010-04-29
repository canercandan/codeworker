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
#include "UtlXMLStream.h"
#include "DtaAttributeType.h"
//##protect##"INCLUDE FILES"

#include "GrfSaveProjectTypes.h"

namespace CodeWorker {
	GrfSaveProjectTypes::~GrfSaveProjectTypes() {
		delete _pXMLFileName;
	}

	SEQUENCE_INTERRUPTION_LIST GrfSaveProjectTypes::executeInternal(DtaScriptVariable& visibility) {
		std::string sXMLFileName = _pXMLFileName->getValue(visibility);
//##protect##"execute"
//##protect##"execute"
		return CGRuntime::saveProjectTypes(sXMLFileName);
	}

//##protect##"implementation"
void GrfSaveProjectTypes::generateXMLFile(UtlXMLStream& myXMLFile, DtaAttributeType& myType, std::set<std::string>& listForAvoidingCycles) {
	if (listForAvoidingCycles.find(myType.getName()) != listForAvoidingCycles.end()) return;
	listForAvoidingCycles.insert(myType.getName());
	myXMLFile.writeBeginningOfObject(myType.getName());
	const std::map<std::string, DtaFollowingAttributeInfo*>& listOfAttributes = myType.getAttributes();
	std::map<std::string, DtaFollowingAttributeInfo*>::const_iterator i;
	for (i = listOfAttributes.begin(); i != listOfAttributes.end(); i++) if (i->second->isAttribute()) {
		std::string sMode;
		if (i->second->getNumberOfUse() == myType.getNumberOfUse()) sMode = "compulsory";
		else {
			char sNumber[64];
			int iPercentOfUse = (100 * i->second->getNumberOfUse()) / myType.getNumberOfUse();
			sprintf(sNumber, "optional[%d%%]", iPercentOfUse);
			sMode = sNumber;
		}
		myXMLFile.writeAttribute(i->second->getAttributeType().getName(), sMode);
	}
	myXMLFile.writeEndOfAttributes();
	for (i = listOfAttributes.begin(); i != listOfAttributes.end(); i++) if (!i->second->isAttribute()) {
		generateXMLFile(myXMLFile, i->second->getAttributeType(), listForAvoidingCycles);
	}
	bool bElements = false;
	const std::map<std::string, DtaFollowingAttributeInfo*>& listOfElements = myType.getAttributeElements();
	for (i = listOfElements.begin(); i != listOfElements.end(); i++) if (i->second->isAttribute()) {
		if (!bElements) {
			myXMLFile.writeBeginningOfObject("__ARRAY_TYPE");
			bElements = true;
		}
		std::string sMode;
		if (i->second->getNumberOfUse() == myType.getNumberOfElementUse()) sMode = "compulsory";
		else {
			char sNumber[64];
			int iPercentOfUse = (100 * i->second->getNumberOfUse()) / myType.getNumberOfElementUse();
			sprintf(sNumber, "optional[%d%%]", iPercentOfUse);
			sMode = sNumber;
		}
		myXMLFile.writeAttribute(i->second->getAttributeType().getName(), sMode);
	}
	if (bElements) myXMLFile.writeEndOfAttributes();
	for (i = listOfElements.begin(); i != listOfElements.end(); i++) if (!i->second->isAttribute()) {
		if (!bElements) {
			myXMLFile.writeBeginningOfObject("__ARRAY_TYPE");
			myXMLFile.writeEndOfAttributes();
			bElements = true;
		}
		generateXMLFile(myXMLFile, i->second->getAttributeType(), listForAvoidingCycles);
	}
	if (bElements) myXMLFile.writeEndOfObject("__ARRAY_TYPE");
	myXMLFile.writeEndOfObject(myType.getName());
	listForAvoidingCycles.erase(myType.getName());
}

//##protect##"implementation"

	void GrfSaveProjectTypes::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::saveProjectTypes(";
		_pXMLFileName->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
