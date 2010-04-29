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
#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"

//##protect##"INCLUDE FILES"
#include "UtlXMLStream.h"
#include "DtaScriptVariable.h"
//##protect##"INCLUDE FILES"

#include "GrfSaveProject.h"

namespace CodeWorker {
	GrfSaveProject::~GrfSaveProject() {
		delete _pXMLorTXTFileName;
		delete _pNodeToSave;
	}

	SEQUENCE_INTERRUPTION_LIST GrfSaveProject::executeInternal(DtaScriptVariable& visibility) {
		std::string sXMLorTXTFileName = _pXMLorTXTFileName->getValue(visibility);
		DtaScriptVariable* pNodeToSave = visibility.getVariable(*_pNodeToSave);
//##protect##"execute"
//##protect##"execute"
		return CGRuntime::saveProject(sXMLorTXTFileName, pNodeToSave);
	}

	void GrfSaveProject::populateDefaultParameters() {
		if (_pNodeToSave == NULL) _pNodeToSave = new ExprScriptVariable("project");
	}

//##protect##"implementation"
	void GrfSaveProject::generateXMLFile(UtlXMLStream& myXMLFile, DtaScriptVariable& myNode, const std::string& sObjectName) {
		std::string sTagName = sObjectName;
		if (sTagName.empty()) sTagName = myNode.getName();
		DtaScriptVariable* pReference = myNode.getReferencedVariable();
		if (pReference != NULL) {
			std::string sReference = pReference->getCompleteName();
			sReference = myXMLFile.convertToXMLText(sReference);
			sTagName += " __REFERENCE=\"" + sReference + "\"";
		} else {
			const char* tcValue = myNode.getValue();
			if ((tcValue != NULL) && (*tcValue != '\0')) {
				std::string sValue = myXMLFile.convertToXMLText(tcValue);
				sTagName += " __VALUE=\"" + sValue + "\"";
			}
		}
		myXMLFile.writeBeginningOfObject(sTagName);
		if (pReference != NULL) {
			myXMLFile.writeEndOfObject();
		} else {
			const std::list<DtaScriptVariable*>* pArray = myNode.getArray();
			bool bEmpty = (pArray == NULL) || pArray->empty();
			if (!bEmpty) {
				myXMLFile.writeEndOfAttributes();
				for (std::list<DtaScriptVariable*>::const_iterator i = pArray->begin(); i != pArray->end(); i++) {
					std::string sKey = myXMLFile.convertToXMLText((*i)->getName());
					std::string sObjectName = "__ARRAY_ENTRY __KEY=\"" + sKey + "\"";
					generateXMLFile(myXMLFile, *(*i), sObjectName);
				}
			}
			DtaScriptVariableList* pAttributes = myNode.getAttributes();
			if (bEmpty && (pAttributes != NULL)) {
				myXMLFile.writeEndOfAttributes();
				bEmpty = false;
			}
			while (pAttributes != NULL) {
				generateXMLFile(myXMLFile, *(pAttributes->getNode()));
				pAttributes = pAttributes->getNext();
			}
			if (bEmpty) {
				myXMLFile.writeEndOfObject();
			} else if (sObjectName.empty()) {
				myXMLFile.writeEndOfObject(myNode.getName());
			} else {
				std::string::size_type iIndex = sObjectName.find(" ");
				if (iIndex == std::string::npos) myXMLFile.writeEndOfObject(sObjectName);
				else myXMLFile.writeEndOfObject(sObjectName.substr(0, iIndex));
			}
		}
	}

	void GrfSaveProject::generateTextFile(std::ostream& theStream, DtaScriptVariable& myNode, const std::string& sIndent) {
		if (sIndent.empty()) {
			std::string sObjectName = myNode.getCompleteName();
			sObjectName = CGRuntime::composeCLikeString(sObjectName);
			theStream << "\"" << sObjectName << "\" = ";
		}
		DtaScriptVariable* pReference = myNode.getReferencedVariable();
		if (pReference != NULL) {
			std::string sReference = pReference->getCompleteName();
			sReference = CGRuntime::composeCLikeString(sReference);
			theStream << "&ref \"" << sReference << "\"";
			const char* tcValue = pReference->getValue();
			if ((tcValue != NULL) && (*tcValue != '\0')) {
				std::string sValue = CGRuntime::composeCLikeString(tcValue);
				theStream << " = \"" << sValue << "\"";
			}
			return;
		}
		const char* tcValue = myNode.getValue();
		if ((tcValue != NULL) && (*tcValue != '\0')) {
			std::string sValue = CGRuntime::composeCLikeString(tcValue);
			theStream << "\"" << sValue << "\"";
		}
		DtaScriptVariableList* pAttributes = myNode.getAttributes();
		bool bEmptyAttributes = (pAttributes == NULL);
		const std::list<DtaScriptVariable*>* pArray = myNode.getArray();
		bool bEmptyArray = (pArray == NULL) || pArray->empty();
		std::string sNewIndent = sIndent + "\t";
		if (!bEmptyAttributes || !bEmptyArray) {
			theStream << " {";
			while (pAttributes != NULL) {
				theStream << std::endl << sNewIndent << "." << pAttributes->getNode()->getName() << " = ";
				generateTextFile(theStream, *(pAttributes->getNode()), sNewIndent);
				pAttributes = pAttributes->getNext();
				if (pAttributes != NULL || !bEmptyArray) {
					theStream << ",";
				}
			}
			if (!bEmptyArray) {
				theStream << std::endl << sNewIndent << "[";
				sNewIndent += "\t";
				for (std::list<DtaScriptVariable*>::const_iterator i = pArray->begin(); i != pArray->end(); i++) {
					if (i != pArray->begin()) {
						theStream << ",";
					}
					std::string sKey = CGRuntime::composeCLikeString((*i)->getName());
					theStream << std::endl << sNewIndent << "\"" << sKey<< "\":";
					generateTextFile(theStream, *(*i), sNewIndent);
				}
				theStream << std::endl << sIndent << "\t]";
			}
			theStream << std::endl << sIndent << "}";
		} else if ((tcValue == NULL) || (*tcValue == '\0')) {
			theStream << "\"\"";
		}
	}
//##protect##"implementation"

	void GrfSaveProject::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::saveProject(";
		_pXMLorTXTFileName->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pNodeToSave->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
