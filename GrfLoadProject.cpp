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
#include "DtaProject.h"
//##protect##"INCLUDE FILES"

#include "GrfLoadProject.h"

namespace CodeWorker {
	GrfLoadProject::~GrfLoadProject() {
		delete _pXMLorTXTFileName;
		delete _pNodeToLoad;
	}

	SEQUENCE_INTERRUPTION_LIST GrfLoadProject::executeInternal(DtaScriptVariable& visibility) {
		std::string sXMLorTXTFileName = _pXMLorTXTFileName->getValue(visibility);
		DtaScriptVariable* pNodeToLoad = visibility.getVariable(*_pNodeToLoad);
//##protect##"execute"
//##protect##"execute"
		return CGRuntime::loadProject(sXMLorTXTFileName, pNodeToLoad);
	}

	void GrfLoadProject::populateDefaultParameters() {
		if (_pNodeToLoad == NULL) _pNodeToLoad = new ExprScriptVariable("project");
	}

//##protect##"implementation"
	void GrfLoadProject::parseTextFile(std::istream& theStream, DtaScriptVariable* pNode) {
		std::string sNodeName;
		if (!CodeWorker::readString(theStream, sNodeName)) {
			throw UtlException(theStream, "complete name of the parse tree to load is expected between double quotes");
		}
		CodeWorker::skipEmpty(theStream);
		if (!CodeWorker::isEqualTo(theStream, '=')) {
			throw UtlException(theStream, "'=' expected");
		}
		if (pNode == NULL) {
			pNode = &(DtaProject::getInstance());
		}
		populateConstantTree(theStream, *pNode);
	}

	bool GrfLoadProject::populateConstantTree(std::istream& theStream, DtaScriptVariable& theNode) {
		std::string sValue;
		CodeWorker::skipEmpty(theStream);
		if (CodeWorker::isEqualTo(theStream, '&')) {
			// reference to a node
			CodeWorker::skipEmpty(theStream);
			if (!CodeWorker::isEqualToIdentifier(theStream, "ref")) {
				throw UtlException(theStream, "reference node should be announced by '&ref'");
			}
			CodeWorker::skipEmpty(theStream);
			std::string sReferenceNode;
			if (!CodeWorker::readString(theStream, sReferenceNode)) {
				throw UtlException(theStream, "reference node expected between double quotes");
			}
			CodeWorker::skipEmpty(theStream);
			if (CodeWorker::isEqualTo(theStream, '=')) {
				CodeWorker::skipEmpty(theStream);
				if (!CodeWorker::readString(theStream, sValue)) {
					throw UtlException(theStream, "syntax error: value expected between double quotes");
				}
			}
		} else {
			CodeWorker::readString(theStream, sValue);
		}
		theNode.setValue(sValue.c_str());
		CodeWorker::skipEmpty(theStream);
		if (!CodeWorker::isEqualTo(theStream, '{')) {
			return false;
		}
		CodeWorker::skipEmpty(theStream);
		if (!CodeWorker::isEqualTo(theStream, '}')) {
			do {
				CodeWorker::skipEmpty(theStream);
				if (CodeWorker::isEqualTo(theStream, '.')) {
					CodeWorker::skipEmpty(theStream);
					std::string sAttribute;
					if (!CodeWorker::readIdentifier(theStream, sAttribute)) {
						throw UtlException(theStream, "attribute name expected");
					}
					CodeWorker::skipEmpty(theStream);
					if (!CodeWorker::isEqualTo(theStream, '=')) {
						throw UtlException(theStream, "syntax error, '=' expected");
					}
					populateConstantTree(theStream, *theNode.insertNode(sAttribute.c_str()));
				} else if (CodeWorker::isEqualTo(theStream, '[')) {
					do {
						CodeWorker::skipEmpty(theStream);
						if (CodeWorker::readString(theStream, sValue) && skipEmpty(theStream) && CodeWorker::isEqualTo(theStream, ':')) {
							// "key" : "value" or "node"
							populateConstantTree(theStream, *theNode.addElement(sValue));
						} else {
							populateConstantTree(theStream, *theNode.pushItem(sValue));
						}
						CodeWorker::skipEmpty(theStream);
					} while (CodeWorker::isEqualTo(theStream, ','));
					if (!CodeWorker::isEqualTo(theStream, ']')) {
						throw UtlException(theStream, "syntax error, ']' expected");
					}
				} else {
					throw UtlException(theStream, "syntax error");
				}
				CodeWorker::skipEmpty(theStream);
			} while (CodeWorker::isEqualTo(theStream, ','));
			if (!CodeWorker::isEqualTo(theStream, '}')) {
				throw UtlException(theStream, "syntax error, '}' expected");
			}
		}
		return true;
	}
//##protect##"implementation"

	void GrfLoadProject::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_INDENT << "CGRuntime::loadProject(";
		_pXMLorTXTFileName->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pNodeToLoad->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
	}
}
