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

#include <math.h>

#include "UtlException.h"
#include "ScpStream.h"

#include "DtaProject.h"
#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"

#include "ExprScriptMotif.h"

namespace CodeWorker {
	ExprScriptMotif::~ExprScriptMotif() {}

	std::string ExprScriptMotif::getValue(DtaScriptVariable& visibility) const {
		throw UtlException("internal error: can't ask for the value of a motif");
	}


	ExprScriptMotifPath::ExprScriptMotifPath(ExprScriptMotifPath* pPrecedentPath) : _pPrecedentPath(pPrecedentPath), _pNextPath(NULL) {
		_pPrecedentPath->_pNextPath = this;
	}

	ExprScriptMotifPath::ExprScriptMotifPath(ExprScriptMotifPath* pPrecedentPath, ExprScriptMotifPath* pNextPath) : _pPrecedentPath(pPrecedentPath), _pNextPath(pNextPath) {
		_pPrecedentPath->_pNextPath = this;
		_pNextPath->_pPrecedentPath = this;
	}

	ExprScriptMotifPath::~ExprScriptMotifPath() {
		delete _pNextPath;
	}



	ExprScriptMotifStep::~ExprScriptMotifStep() {}

	DtaScriptVariable* ExprScriptMotifStep::filterNodes(DtaScriptVariable& visibility, DtaScriptVariable& currentNode, std::list<DtaScriptVariable*>& listOfNodes) const {
		DtaScriptVariable* pCurrent = &currentNode;
		while (pCurrent->getReferencedVariable()!= NULL) pCurrent = pCurrent->getReferencedVariable();
		if (_sName == "*") {
			DtaScriptVariableList* pAttributes = pCurrent->getAttributes();
			DtaScriptVariable* pLastCurrent = NULL;
			while (pAttributes != NULL) {
				if (endNode()) {
					pLastCurrent = pAttributes->getNode();
					listOfNodes.push_back(pLastCurrent);
				} else {
					DtaScriptVariable* pResult = getNextPath()->filterNodes(visibility, *(pAttributes->getNode()), listOfNodes);
					if (pResult != NULL) pLastCurrent = pResult;
				}
			}
			pCurrent = pLastCurrent;
		} else {
			pCurrent = pCurrent->getVariable(_sName.c_str(), startNode(), false, true);
			if (pCurrent != NULL) {
				if (endNode()) listOfNodes.push_back(pCurrent);
				else pCurrent = getNextPath()->filterNodes(visibility, *pCurrent, listOfNodes);
			}
		}
		return pCurrent;
	}

	std::string ExprScriptMotifStep::toString() const {
		if (dynamic_cast<ExprScriptMotifStep*>(getPrecedentPath()) != NULL) return "." + _sName;
		return _sName;
	}


	ExprScriptMotifEllipsis::~ExprScriptMotifEllipsis() {}

	DtaScriptVariable* ExprScriptMotifEllipsis::filterNodes(DtaScriptVariable& visibility, DtaScriptVariable& currentNode, std::list<DtaScriptVariable*>& listOfNodes) const {
		DtaScriptVariable* pCurrent = &currentNode;
		while (pCurrent->getReferencedVariable()!= NULL) pCurrent = pCurrent->getReferencedVariable();
		DtaScriptVariableList* pAttributes = pCurrent->getAttributes();
		DtaScriptVariable* pLastCurrent = NULL;
		while (pAttributes != NULL) {
			if (endNode()) {
				pLastCurrent = pAttributes->getNode();
				listOfNodes.push_back(pLastCurrent);
			} else {
				DtaScriptVariable* pResult = getNextPath()->filterNodes(visibility, *(pAttributes->getNode()), listOfNodes);
				if (pResult != NULL) pLastCurrent = pResult;
				pResult = filterNodes(visibility, *(pAttributes->getNode()), listOfNodes);
				if (pResult != NULL) pLastCurrent = pResult;
			}
		}
		const std::list<DtaScriptVariable*>* array = pCurrent->getArray();
		if (array != NULL) {
			for (std::list<DtaScriptVariable*>::const_iterator i = array->begin(); i != array->end(); i++) {
				DtaScriptVariable* pResult;
				if (endNode()) {
					pLastCurrent = *i;
					listOfNodes.push_back(pLastCurrent);
				} else {
					pResult = getNextPath()->filterNodes(visibility, *(*i), listOfNodes);
					if (pResult != NULL) pLastCurrent = pResult;
				}
				pResult = filterNodes(visibility, *(*i), listOfNodes);
				if (pResult != NULL) pLastCurrent = pResult;
			}
		}
		return pLastCurrent;
	}

	std::string ExprScriptMotifEllipsis::toString() const {
		std::string sText = getPrecedentPath()->toString() + "..." + getNextPath()->toString();
		return sText;
	}


	ExprScriptMotifArray::~ExprScriptMotifArray() {
		delete _pPosition;
	}

	DtaScriptVariable* ExprScriptMotifArray::filterNodes(DtaScriptVariable& visibility, DtaScriptVariable& currentNode, std::list<DtaScriptVariable*>& listOfNodes) const {
		DtaScriptVariable* pCurrent = &currentNode;
		while (pCurrent->getReferencedVariable()!= NULL) pCurrent = pCurrent->getReferencedVariable();
		if (_pPosition == NULL) {
			const std::list<DtaScriptVariable*>* array = pCurrent->getArray();
			DtaScriptVariable* pLastCurrent = NULL;
			if (array != NULL) {
				for (std::list<DtaScriptVariable*>::const_iterator i = array->begin(); i != array->end(); i++) {
					if (endNode()) {
						pLastCurrent = *i;
						listOfNodes.push_back(pLastCurrent);
					} else {
						DtaScriptVariable* pResult = getNextPath()->filterNodes(visibility, *(*i), listOfNodes);
						if (pResult != NULL) pLastCurrent = pResult;
					}
				}
			}
			pCurrent = pLastCurrent;
		} else {
			std::string sPosition = _pPosition->getValue(visibility);
			pCurrent = pCurrent->getArrayElement(sPosition);
			if (endNode() && (pCurrent != NULL)) listOfNodes.push_back(pCurrent);
		}
		return pCurrent;
	}

	std::string ExprScriptMotifArray::toString() const {
		std::string sText = getPrecedentPath()->toString() + "[";
		if (_pPosition != NULL) sText += _pPosition->toString();
		sText += "]";
		return sText;
	}


	ExprScriptMotifBoolean::~ExprScriptMotifBoolean() {}

	DtaScriptVariable* ExprScriptMotifBoolean::filterNodes(DtaScriptVariable& visibility, DtaScriptVariable& currentNode, std::list<DtaScriptVariable*>& listOfNodes) const {
		DtaScriptVariable* pCurrent;
		if (_cOperator == '&') {
			std::list<DtaScriptVariable*> list;
			for (std::vector<ExprScriptMotif*>::const_iterator i = _members.begin(); i != _members.end(); i++) {
				pCurrent = (*i)->filterNodes(visibility, currentNode, list);
				if (pCurrent == NULL) break;
			}
			if (pCurrent != NULL) {
				for (std::list<DtaScriptVariable*>::const_iterator i = list.begin(); i != list.end(); i++) {
					listOfNodes.push_back(*i);
				}
			}
		} else if (_cOperator == '|') {
			for (std::vector<ExprScriptMotif*>::const_iterator i = _members.begin(); i != _members.end(); i++) {
				pCurrent = (*i)->filterNodes(visibility, currentNode, listOfNodes);
				if (pCurrent != NULL) break;
			}
		} else if (_cOperator == '+') {
			pCurrent = NULL;
			for (std::vector<ExprScriptMotif*>::const_iterator i = _members.begin(); i != _members.end(); i++) {
				register DtaScriptVariable* pResult = (*i)->filterNodes(visibility, currentNode, listOfNodes);
				if (pResult != NULL) pCurrent = pResult;
			}
		}
		return pCurrent;
	}

	std::string ExprScriptMotifBoolean::toString() const {
		std::string sText;
		for (std::vector<ExprScriptMotif*>::const_iterator i = _members.begin(); i != _members.end(); i++) {
			if (!sText.empty()) {
				sText += " ";
				sText += _cOperator;
				sText += " ";
			}
			sText += "(" + (*i)->toString() + ")";
		}
		return sText;
	}
}
