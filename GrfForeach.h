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

#ifndef _GrfForeach_h_
#define _GrfForeach_h_

#pragma warning (disable : 4786)

#include <string>
#include <list>

#include "GrfBlock.h"

namespace CodeWorker {
	class ExprScriptVariable;
	class DtaArrayIterator;

	class GrfForeach : public GrfBlock {
	public:
		enum CASCADING {NO_CASCADING, CASCADING_FIRST, CASCADING_LAST};

	protected:
		ExprScriptVariable* _pIndexExpr;
		ExprScriptVariable* _pListExpr;
		bool _bSorted;
		bool _bSortedNoCase;
		bool _bSortOnValue;
		bool _bReverse;

		CASCADING _iCascading;
		std::string _sCascadedName;

		DtaArrayIterator* _pArrayIteratorData;

	public:
		GrfForeach(GrfBlock* pBlock) : GrfBlock(pBlock), _pListExpr(NULL), _pArrayIteratorData(NULL), _bSorted(false), _bSortedNoCase(false), _bSortOnValue(false), _bReverse(false), _iCascading(NO_CASCADING) {}
		virtual ~GrfForeach();

		void setIndex(ExprScriptVariable* pIndexExpr);
		inline void setList(ExprScriptVariable* pList) { _pListExpr = pList; }

//		inline int getArraySize() const { return _pArrayIteratorData->size(); }
//		inline int getCurrentPosition() const { return _pArrayIteratorData->index(); }
		inline bool getSorted() const { return _bSorted; }
		inline void setSorted(bool bSorted) { _bSorted = bSorted; }
		inline bool getSortedNoCase() const { return _bSortedNoCase; }
		void setSortedNoCase(bool bSortedNoCase);
		inline bool getSortOnValue() { return _bSortOnValue; }
		inline void setSortOnValue(bool bSortOnValue) { _bSortOnValue = bSortOnValue; }
		inline bool getReverse() const { return _bReverse; }
		void setReverse(bool bReverse);
		inline CASCADING getCascading() const { return _iCascading; }
		inline void setCascading(CASCADING iCascading) { _iCascading = iCascading; }
//		inline const char* getKey() const { return _pArrayIteratorData->key(); }

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
		virtual SEQUENCE_INTERRUPTION_LIST executeCorrectForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach);
		virtual SEQUENCE_INTERRUPTION_LIST executeForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach);
		virtual SEQUENCE_INTERRUPTION_LIST executeReverseForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach);
		virtual SEQUENCE_INTERRUPTION_LIST executeSortedForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach);
		virtual SEQUENCE_INTERRUPTION_LIST executeSortedIndirectForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach);
		virtual SEQUENCE_INTERRUPTION_LIST executeReverseSortedForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach);
		virtual SEQUENCE_INTERRUPTION_LIST executeReverseSortedIndirectForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach);

		SEQUENCE_INTERRUPTION_LIST iterate(DtaArrayIterator& iteratorData, DtaScriptVariable& stackForeach);
	};
}

#endif
