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

#ifndef _ExprScriptMotif_h_
#define _ExprScriptMotif_h_

#include <list>

#include "ExprScriptExpression.h"

namespace CodeWorker {
	class ExprScriptMotif : public ExprScriptExpression {
	public:
		inline ExprScriptMotif() {}
		virtual ~ExprScriptMotif();

		virtual std::string getValue(DtaScriptVariable& visibility) const;

		virtual DtaScriptVariable* filterNodes(DtaScriptVariable& visibility, DtaScriptVariable& currentNode, std::list<DtaScriptVariable*>& listOfNodes) const = 0;
	};

	class ExprScriptMotifPath : public ExprScriptMotif {
	private:
		ExprScriptMotifPath* _pPrecedentPath;
		ExprScriptMotifPath* _pNextPath;

	public:
		inline ExprScriptMotifPath() : _pPrecedentPath(NULL), _pNextPath(NULL) {}
		ExprScriptMotifPath(ExprScriptMotifPath* pPrecedentPath);
		ExprScriptMotifPath(ExprScriptMotifPath* pPrecedentPath, ExprScriptMotifPath* pNextPath);
		virtual ~ExprScriptMotifPath();

		inline ExprScriptMotifPath* getPrecedentPath() const { return _pPrecedentPath; }
		inline ExprScriptMotifPath* getNextPath() const { return _pNextPath; }
		inline void setNextPath(ExprScriptMotifPath* pNextPath) { _pNextPath = pNextPath;_pNextPath->_pPrecedentPath = this; }

		inline bool startNode() const { return _pPrecedentPath == NULL; }
		inline bool endNode() const { return _pNextPath == NULL; }
	};

	class ExprScriptMotifStep : public ExprScriptMotifPath {
	private:
		std::string _sName;

	public:
		inline ExprScriptMotifStep(const std::string& sName) : _sName(sName) {}
		virtual ~ExprScriptMotifStep();

		virtual DtaScriptVariable* filterNodes(DtaScriptVariable& visibility, DtaScriptVariable& currentNode, std::list<DtaScriptVariable*>& listOfNodes) const;

		virtual std::string toString() const;
	};

	class ExprScriptMotifEllipsis : public ExprScriptMotifPath {
	public:
		inline ExprScriptMotifEllipsis(ExprScriptMotifPath* pLeft, ExprScriptMotifPath* pRight) : ExprScriptMotifPath(pLeft, pRight) {}
		virtual ~ExprScriptMotifEllipsis();

		virtual DtaScriptVariable* filterNodes(DtaScriptVariable& visibility, DtaScriptVariable& currentNode, std::list<DtaScriptVariable*>& listOfNodes) const;

		virtual std::string toString() const;
	};

	class ExprScriptMotifArray : public ExprScriptMotifPath {
	private:
		ExprScriptExpression* _pPosition;

	public:
		inline ExprScriptMotifArray(ExprScriptMotifPath* pPrec, ExprScriptExpression* pPosition = NULL) : ExprScriptMotifPath(pPrec), _pPosition(pPosition) {}
		virtual ~ExprScriptMotifArray();

		virtual DtaScriptVariable* filterNodes(DtaScriptVariable& visibility, DtaScriptVariable& currentNode, std::list<DtaScriptVariable*>& listOfNodes) const;

		virtual std::string toString() const;
	};

	class ExprScriptMotifBoolean : public ExprScriptMotif {
	private:
		char _cOperator;
		std::vector<ExprScriptMotif*> _members;

	public:
		inline ExprScriptMotifBoolean(char cOperator, ExprScriptMotif* pMember) : _cOperator(cOperator) { _members.push_back(pMember); }
		virtual ~ExprScriptMotifBoolean();

		inline void addMember(ExprScriptMotif* pMember) { _members.push_back(pMember); }

		virtual DtaScriptVariable* filterNodes(DtaScriptVariable& visibility, DtaScriptVariable& currentNode, std::list<DtaScriptVariable*>& listOfNodes) const;

		virtual std::string toString() const;
	};
}

#endif
