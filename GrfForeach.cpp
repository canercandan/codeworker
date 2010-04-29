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

#include "DtaScriptVariable.h"
#include "DtaArrayIterator.h"
#include "ExprScriptVariable.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "GrfForeach.h"

namespace CodeWorker {
	class DtaVectorIterator : public DtaArrayIterator {
	private:
		const std::vector<DtaScriptVariable*>& _list;
		std::vector<DtaScriptVariable*>::const_iterator i;

	public:
		inline DtaVectorIterator(const DtaVectorIterator& copy) : DtaArrayIterator(copy), _list(copy._list), i(copy.i) {}
		inline DtaVectorIterator(const std::vector<DtaScriptVariable*>& listOfNodes) : _list(listOfNodes), i(listOfNodes.begin()) {}
		virtual ~DtaVectorIterator() {}

		virtual DtaArrayIterator* clone() const { return new DtaVectorIterator(*this); }
		virtual int size() const { return _list.size(); }
		virtual bool end() const { return i == _list.end(); }
		virtual const char* key() const { return (*i)->getName(); }
		virtual DtaScriptVariable* item() const { return *i; }

		virtual bool prec() {
			if (i == _list.begin()) return false;
			--_iCurrentPosition;
			--i;
			_pItemNoRef = NULL;
			return true;
		}

		virtual bool next() {
			++_iCurrentPosition;
			++i;
			_pItemNoRef = NULL;
			return i != _list.end();
		}
	};



	class DtaReverseVectorIterator : public DtaArrayIterator {
	private:
		const std::vector<DtaScriptVariable*>& _list;
		std::vector<DtaScriptVariable*>::const_reverse_iterator i;

	public:
		inline DtaReverseVectorIterator(const DtaReverseVectorIterator& copy) : DtaArrayIterator(copy), _list(copy._list), i(copy.i) {}
		inline DtaReverseVectorIterator(const std::vector<DtaScriptVariable*>& listOfNodes) : _list(listOfNodes), i(listOfNodes.rbegin()) {}
		virtual ~DtaReverseVectorIterator() {}

		virtual DtaArrayIterator* clone() const { return new DtaReverseVectorIterator(*this); }
		virtual int size() const { return _list.size(); }
		virtual bool end() const { return i == _list.rend(); }
		virtual const char* key() const { return (*i)->getName(); }
		virtual DtaScriptVariable* item() const { return *i; }

		virtual bool prec() {
			if (i == _list.rbegin()) return false;
			--_iCurrentPosition;
			--i;
			_pItemNoRef = NULL;
			return true;
		}

		virtual bool next() {
			++_iCurrentPosition;
			++i;
			_pItemNoRef = NULL;
			return i != _list.rend();
		}
	};



	class DtaMapIterator : public DtaArrayIterator {
	private:
		const std::map<std::string, DtaScriptVariable*>& _map;
		std::map<std::string, DtaScriptVariable*>::const_iterator i;

	public:
		inline DtaMapIterator(const DtaMapIterator& copy) : DtaArrayIterator(copy), _map(copy._map), i(copy.i) {}
		inline DtaMapIterator(const std::map<std::string, DtaScriptVariable*>& mapOfNodes) : _map(mapOfNodes), i(mapOfNodes.begin()) {}
		virtual ~DtaMapIterator() {}

		virtual DtaArrayIterator* clone() const { return new DtaMapIterator(*this); }
		virtual int size() const { return _map.size(); }
		virtual bool end() const { return i == _map.end(); }
		virtual const char* key() const { return i->second->getName(); }
		virtual DtaScriptVariable* item() const { return i->second; }

		virtual bool prec() {
			if (i == _map.begin()) return false;
			--_iCurrentPosition;
			--i;
			_pItemNoRef = NULL;
			return true;
		}

		virtual bool next() {
			++_iCurrentPosition;
			++i;
			_pItemNoRef = NULL;
			return i != _map.end();
		}
	};



	class DtaReverseMapIterator : public DtaArrayIterator {
	private:
		const std::map<std::string, DtaScriptVariable*>& _map;
		std::map<std::string, DtaScriptVariable*>::const_reverse_iterator i;

	public:
		inline DtaReverseMapIterator(const DtaReverseMapIterator& copy) : DtaArrayIterator(copy), _map(copy._map), i(copy.i) {}
		inline DtaReverseMapIterator(const std::map<std::string, DtaScriptVariable*>& mapOfNodes) : _map(mapOfNodes), i(mapOfNodes.rbegin()) {}
		virtual ~DtaReverseMapIterator() {}

		virtual DtaArrayIterator* clone() const { return new DtaReverseMapIterator(*this); }
		virtual int size() const { return _map.size(); }
		virtual bool end() const { return i == _map.rend(); }
		virtual const char* key() const { return i->second->getName(); }
		virtual DtaScriptVariable* item() const { return i->second; }

		virtual bool prec() {
			if (i == _map.rbegin()) return false;
			--_iCurrentPosition;
			--i;
			_pItemNoRef = NULL;
			return true;
		}

		virtual bool next() {
			++_iCurrentPosition;
			++i;
			_pItemNoRef = NULL;
			return i != _map.rend();
		}
	};



	GrfForeach::~GrfForeach() {
		delete _pIndexExpr;
		delete _pListExpr;
	}

	void GrfForeach::setIndex(ExprScriptVariable* pIndexExpr) {
		_pIndexExpr = pIndexExpr;
		addLocalVariable(pIndexExpr->getName(), ITERATOR_EXPRTYPE);
	}

	void GrfForeach::setSortedNoCase(bool bSortedNoCase) {
		if (bSortedNoCase) _bSorted = true;
		_bSortedNoCase = bSortedNoCase;
	}

	void GrfForeach::setReverse(bool bReverse) {
		_bReverse = bReverse;
	}

	SEQUENCE_INTERRUPTION_LIST GrfForeach::executeInternal(DtaScriptVariable& visibility) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		DtaScriptVariable* pVariable = visibility.getExistingVariable(*_pListExpr);
		if (pVariable != NULL) {
			const std::list<DtaScriptVariable*>* pArray = pVariable->getArray();
			if ((pArray != NULL) && !pArray->empty()) {
				DtaScriptVariable stackForeach(&visibility, "##stack## foreach");
				stackForeach.createIterator(_pIndexExpr->getName().c_str(), &_pArrayIteratorData);
				if (_iCascading != NO_CASCADING) _sCascadedName = pVariable->getName();
				result = executeCorrectForeach(*pVariable, stackForeach);
				if (result == BREAK_INTERRUPTION) result = NO_INTERRUPTION;
			}
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST GrfForeach::executeCorrectForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach) {
		SEQUENCE_INTERRUPTION_LIST result = NO_INTERRUPTION;
		if (_bSorted) {
			if (_bSortedNoCase) {
				if (_bReverse) result = executeReverseSortedIndirectForeach(theVariable, stackForeach);
				else result = executeSortedIndirectForeach(theVariable, stackForeach);
			} else {
				if (_bSortOnValue) result = executeSortedIndirectForeach(theVariable, stackForeach);
				else if (_bReverse) result = executeReverseSortedForeach(theVariable, stackForeach);
				else result = executeSortedForeach(theVariable, stackForeach);
			}
		} else {
			if (_bReverse) result = executeReverseForeach(theVariable, stackForeach);
			else result = executeForeach(theVariable, stackForeach);
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST GrfForeach::executeForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach) {
		const std::list<DtaScriptVariable*>* list = theVariable.getArray();
		if (list == NULL) return NO_INTERRUPTION;
		DtaListIterator iteratorData(*list);
		return iterate(iteratorData, stackForeach);
	}

	SEQUENCE_INTERRUPTION_LIST GrfForeach::executeReverseForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach) {
		const std::list<DtaScriptVariable*>* list = theVariable.getArray();
		if (list == NULL) return NO_INTERRUPTION;
		DtaReverseListIterator iteratorData(*list);
		return iterate(iteratorData, stackForeach);
	}

	SEQUENCE_INTERRUPTION_LIST GrfForeach::executeSortedForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach) {
		const std::map<std::string, DtaScriptVariable*>* mapOfElements = theVariable.getSortedArray();
		if (mapOfElements == NULL) return NO_INTERRUPTION;
		DtaMapIterator iteratorData(*mapOfElements);
		return iterate(iteratorData, stackForeach);
	}

	SEQUENCE_INTERRUPTION_LIST GrfForeach::executeSortedIndirectForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach) {
		std::auto_ptr<std::vector<DtaScriptVariable*> > listOfElements;
		if (_bSortedNoCase) {
			if (_bSortOnValue) {
				listOfElements = std::auto_ptr<std::vector<DtaScriptVariable*> >(theVariable.getSortedNoCaseArrayOnValue());
			} else {
				listOfElements = std::auto_ptr<std::vector<DtaScriptVariable*> >(theVariable.getSortedNoCaseArray());
			}
		} else if (_bSortOnValue) {
			listOfElements = std::auto_ptr<std::vector<DtaScriptVariable*> >(theVariable.getSortedArrayOnValue());
		}
		DtaVectorIterator iteratorData(*listOfElements);
		return iterate(iteratorData, stackForeach);
	}

	SEQUENCE_INTERRUPTION_LIST GrfForeach::executeReverseSortedForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach) {
		const std::map<std::string, DtaScriptVariable*>* mapOfElements = theVariable.getSortedArray();
		if (mapOfElements == NULL) return NO_INTERRUPTION;
		DtaReverseMapIterator iteratorData(*mapOfElements);
		return iterate(iteratorData, stackForeach);
	}

	SEQUENCE_INTERRUPTION_LIST GrfForeach::executeReverseSortedIndirectForeach(DtaScriptVariable& theVariable, DtaScriptVariable& stackForeach) {
		std::auto_ptr<std::vector<DtaScriptVariable*> > listOfElements;
		if (_bSortedNoCase) {
			if (_bSortOnValue) {
				listOfElements = std::auto_ptr<std::vector<DtaScriptVariable*> >(theVariable.getSortedNoCaseArrayOnValue());
			} else {
				listOfElements = std::auto_ptr<std::vector<DtaScriptVariable*> >(theVariable.getSortedNoCaseArray());
			}
		} else if (_bSortOnValue) {
			listOfElements = std::auto_ptr<std::vector<DtaScriptVariable*> >(theVariable.getSortedArrayOnValue());
		}
		DtaReverseVectorIterator iteratorData(*listOfElements);
		return iterate(iteratorData, stackForeach);
	}

	SEQUENCE_INTERRUPTION_LIST GrfForeach::iterate(DtaArrayIterator& iteratorData, DtaScriptVariable& stackForeach) {
		SEQUENCE_INTERRUPTION_LIST result;
		DtaArrayIterator* pOldArrayIteratorData = _pArrayIteratorData;
		_pArrayIteratorData = &iteratorData;
		if (!iteratorData.end()) {
			do {
				if (_iCascading == CASCADING_FIRST) {
					DtaScriptVariable* pNext = iteratorData.item()->getNode(_sCascadedName.c_str());
					if (pNext != NULL) {
						result = executeCorrectForeach(*pNext, stackForeach);
						if ((result != CONTINUE_INTERRUPTION) && (result != NO_INTERRUPTION)) return result;
					}
				}
				result = GrfBlock::executeInternal(stackForeach);
				if ((result != CONTINUE_INTERRUPTION) && (result != NO_INTERRUPTION)) return result;
				if (_iCascading == CASCADING_LAST) {
					DtaScriptVariable* pNext = iteratorData.item()->getNode(_sCascadedName.c_str());
					if (pNext != NULL) {
						result = executeCorrectForeach(*pNext, stackForeach);
						if ((result != CONTINUE_INTERRUPTION) && (result != NO_INTERRUPTION)) return result;
					}
				}
			} while (iteratorData.next());
		}
		_pArrayIteratorData = pOldArrayIteratorData;
		return NO_INTERRUPTION;
	}

	void GrfForeach::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		std::string sIndex = _pIndexExpr->getName();
		std::string sIndex2Cpp = theCompilerEnvironment.convertToCppVariable(sIndex);
		theCompilerEnvironment.addVariable(sIndex);
		theCompilerEnvironment.pushForeach(const_cast<GrfForeach*>(this));
		if (_bSorted) {
			if (_bSortedNoCase) {
				if (theCompilerEnvironment.addVariable("_compilerMapSortedNoCaseForeach_" + sIndex)) {
					CW_BODY_INDENT << "std::vector<DtaScriptVariable*>::const_iterator _compilerIndexSortedNoCaseForeach_" << sIndex << ";";
					CW_BODY_ENDL;
					CW_BODY_INDENT << "std::auto_ptr<std::vector<DtaScriptVariable*> > _compilerMapSortedNoCaseForeach_" << sIndex << "(";
					_pListExpr->compileCpp(theCompilerEnvironment);
					CW_BODY_STREAM << ".getSortedNoCaseArray());";
					CW_BODY_ENDL;
				} else {
					CW_BODY_INDENT << "_compilerMapSortedNoCaseForeach_" << sIndex << " = ";
					_pListExpr->compileCpp(theCompilerEnvironment);
					CW_BODY_STREAM << ".getSortedNoCaseArray();";
					CW_BODY_ENDL;
				}
				CW_BODY_INDENT << "for (_compilerIndexSortedNoCaseForeach_" << sIndex << " = _compilerMapSortedNoCaseForeach_" << sIndex << "->begin(); _compilerIndexSortedNoCaseForeach_" << sIndex << " != _compilerMapSortedNoCaseForeach_" << sIndex << "->end(); ++_compilerIndexSortedNoCaseForeach_" << sIndex << ") {";
				CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "CppParsingTree_var " << sIndex2Cpp << "(*_compilerIndexSortedNoCaseForeach_" << sIndex << ");";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "std::string _compilerKeyForeach_" << sIndex << "(" << sIndex2Cpp << ".getName());";
				CW_BODY_ENDL;
			} else {
				if (theCompilerEnvironment.addVariable("_compilerMapSortedForeach_" + sIndex)) {
					CW_BODY_INDENT << "std::map<std::string, DtaScriptVariable*>::const_iterator _compilerIndexSortedForeach_" << sIndex << ";";
					CW_BODY_ENDL;
					CW_BODY_INDENT << "const std::map<std::string, DtaScriptVariable*>* _compilerMapSortedForeach_" << sIndex << " = ";
					_pListExpr->compileCpp(theCompilerEnvironment);
					CW_BODY_STREAM << ".getSortedArray();";
					CW_BODY_ENDL;
				} else {
					CW_BODY_INDENT << "_compilerMapSortedForeach_" << sIndex << " = ";
					_pListExpr->compileCpp(theCompilerEnvironment);
					CW_BODY_STREAM << ".getSortedArray()";
					CW_BODY_ENDL;
				}
				CW_BODY_INDENT << "if (_compilerMapSortedForeach_" << sIndex << " != 0) for (_compilerIndexSortedForeach_" << sIndex << " = _compilerMapSortedForeach_" << sIndex << "->begin(); _compilerIndexSortedForeach_" << sIndex << " != _compilerMapSortedForeach_" << sIndex << "->end(); ++_compilerIndexSortedForeach_" << sIndex << ") {";
				CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "CppParsingTree_var " << sIndex2Cpp << "(_compilerIndexSortedForeach_" << sIndex << "->second);";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "std::string _compilerKeyForeach_" << sIndex << "(_compilerIndexSortedForeach_" << sIndex << "->first);";
				CW_BODY_ENDL;
			}
		} else {
			if (theCompilerEnvironment.addVariable("_compilerMapForeach_" + sIndex)) {
				CW_BODY_INDENT << "std::list<DtaScriptVariable*>::const_iterator _compilerIndexForeach_" << sIndex << ";";
				CW_BODY_ENDL;
				CW_BODY_INDENT << "const std::list<DtaScriptVariable*>* _compilerMapForeach_" << sIndex << " = ";
				_pListExpr->compileCpp(theCompilerEnvironment);
				CW_BODY_STREAM << ".getArray();";CW_BODY_ENDL;
			} else {
				CW_BODY_INDENT << "_compilerMapForeach_" << sIndex << " = ";
				_pListExpr->compileCpp(theCompilerEnvironment);
				CW_BODY_STREAM << ".getArray();";CW_BODY_ENDL;
			}
			if (_iCascading != NO_CASCADING) {
				CW_BODY_INDENT << "{";CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "struct _COMPILER_INDEX_FOREACH_STACK {";CW_BODY_ENDL;
				CW_BODY_INDENT << "\tstd::list<DtaScriptVariable*>::const_iterator i_;";CW_BODY_ENDL;
				CW_BODY_INDENT << "\tconst std::list<DtaScriptVariable*>* pList_;";CW_BODY_ENDL;
				CW_BODY_INDENT << "\t_COMPILER_INDEX_FOREACH_STACK* pParent_;";CW_BODY_ENDL;
				CW_BODY_INDENT << "\t_COMPILER_INDEX_FOREACH_STACK(std::list<DtaScriptVariable*>::const_iterator& i, const std::list<DtaScriptVariable*>* pList, _COMPILER_INDEX_FOREACH_STACK* pParent) : i_(i), pList_(pList), pParent_(pParent) {}";CW_BODY_ENDL;
				CW_BODY_INDENT << "} *_compilerIndexForeach_stack = NULL;";CW_BODY_ENDL;
				CW_BODY_INDENT << "if (_compilerMapForeach_" << sIndex << " != NULL) _compilerIndexForeach_" << sIndex << " = _compilerMapForeach_" << sIndex << "->begin();";CW_BODY_ENDL;
				CW_BODY_INDENT << "bool _compilerForeach_realBreak;";CW_BODY_ENDL;
				if (_iCascading == CASCADING_FIRST) {
					CW_BODY_INDENT << "bool _compilerForeach_firstDone = false;";CW_BODY_ENDL;
				}
				CW_BODY_INDENT << "do {";CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				CW_BODY_INDENT << "_compilerForeach_realBreak = true;";CW_BODY_ENDL;
				CW_BODY_INDENT << "if (_compilerMapForeach_" << sIndex << " != 0) for (; _compilerIndexForeach_" << sIndex << " != _compilerMapForeach_" << sIndex << "->end(); ++_compilerIndexForeach_" << sIndex << ") {";CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
				if (_iCascading == CASCADING_FIRST) {
					CW_BODY_INDENT << "CppParsingTree_var _compilerForeachNext(CppParsingTree_var(*_compilerIndexForeach_" << sIndex << ").getNode(\"" << _sCascadedName << "\"));";CW_BODY_ENDL;
					CW_BODY_INDENT << "if (!_compilerForeach_firstDone && !_compilerForeachNext.isNull() && (_compilerForeachNext.getArray() != NULL)) {";CW_BODY_ENDL;
					CW_BODY_INDENT << "\t_compilerIndexForeach_stack = new _COMPILER_INDEX_FOREACH_STACK(_compilerIndexForeach_" << sIndex << ", _compilerMapForeach_" << sIndex << ", _compilerIndexForeach_stack);";CW_BODY_ENDL;
					CW_BODY_INDENT << "\t_compilerMapForeach_" << sIndex << " = _compilerForeachNext.getArray();";CW_BODY_ENDL;
					CW_BODY_INDENT << "\t_compilerIndexForeach_" << sIndex << " = _compilerMapForeach_" << sIndex << "->begin();";CW_BODY_ENDL;
					CW_BODY_INDENT << "\t_compilerForeach_realBreak = false;";CW_BODY_ENDL;
					CW_BODY_INDENT << "\tbreak;";CW_BODY_ENDL;
					CW_BODY_INDENT << "} else {";CW_BODY_ENDL;
					CW_BODY_INDENT << "\t_compilerForeach_firstDone = false;";CW_BODY_ENDL;
					CW_BODY_INDENT << "}";CW_BODY_ENDL;
				}
			} else {
				CW_BODY_INDENT << "if (_compilerMapForeach_" << sIndex << " != 0) for (_compilerIndexForeach_" << sIndex << " = _compilerMapForeach_" << sIndex << "->begin(); _compilerIndexForeach_" << sIndex << " != _compilerMapForeach_" << sIndex << "->end(); ++_compilerIndexForeach_" << sIndex << ") {";
				CW_BODY_ENDL;
				theCompilerEnvironment.incrementIndentation();
			}
			CW_BODY_INDENT << "CppParsingTree_var " << sIndex2Cpp << "(*_compilerIndexForeach_" << sIndex << ");";
			CW_BODY_ENDL;
			CW_BODY_INDENT << "std::string _compilerKeyForeach_" << sIndex << "(" << sIndex2Cpp << ".getName());";
			CW_BODY_ENDL;
		}
		theCompilerEnvironment.bracketsToNextBlock(false);
		GrfBlock::compileCpp(theCompilerEnvironment);
		if (_iCascading != NO_CASCADING) {
			if (_iCascading == CASCADING_LAST) {
				CW_BODY_INDENT << "CppParsingTree_var _compilerForeachNext(CppParsingTree_var(*_compilerIndexForeach_" << sIndex << ").getNode(\"" << _sCascadedName << "\"));";CW_BODY_ENDL;
				CW_BODY_INDENT << "if (!_compilerForeachNext.isNull() && (_compilerForeachNext.getArray() != NULL)) {";CW_BODY_ENDL;
				CW_BODY_INDENT << "\t++_compilerIndexForeach_" << sIndex << ";";CW_BODY_ENDL;
				CW_BODY_INDENT << "\t_compilerIndexForeach_stack = new _COMPILER_INDEX_FOREACH_STACK(_compilerIndexForeach_" << sIndex << ", _compilerMapForeach_" << sIndex << ", _compilerIndexForeach_stack);";CW_BODY_ENDL;
				CW_BODY_INDENT << "\t_compilerMapForeach_" << sIndex << " = _compilerForeachNext.getArray();";CW_BODY_ENDL;
				CW_BODY_INDENT << "\t_compilerIndexForeach_" << sIndex << " = _compilerMapForeach_" << sIndex << "->begin();";CW_BODY_ENDL;
				CW_BODY_INDENT << "\t_compilerForeach_realBreak = false;";CW_BODY_ENDL;
				CW_BODY_INDENT << "\tbreak;";CW_BODY_ENDL;
				CW_BODY_INDENT << "}";CW_BODY_ENDL;
			}
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
			CW_BODY_INDENT << "if ((_compilerMapForeach_" << sIndex << " == NULL) || (_compilerIndexForeach_" << sIndex << " == _compilerMapForeach_" << sIndex << "->end())) {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\tif (_compilerIndexForeach_stack == NULL) _compilerForeach_realBreak = true;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\telse {";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\t_compilerForeach_realBreak = false;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\t_compilerIndexForeach_" << sIndex << " = _compilerIndexForeach_stack->i_;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\t_compilerMapForeach_" << sIndex << " = _compilerIndexForeach_stack->pList_;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\t_COMPILER_INDEX_FOREACH_STACK* pTemp = _compilerIndexForeach_stack;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\t_compilerIndexForeach_stack = pTemp->pParent_;";CW_BODY_ENDL;
			CW_BODY_INDENT << "\t\tdelete pTemp;";CW_BODY_ENDL;
			if (_iCascading == CASCADING_FIRST) {
				CW_BODY_INDENT << "\t\t_compilerForeach_firstDone = true;";CW_BODY_ENDL;
			}
			CW_BODY_INDENT << "\t}";CW_BODY_ENDL;
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "} while (!_compilerForeach_realBreak);";CW_BODY_ENDL;
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
		} else {
			theCompilerEnvironment.decrementIndentation();
			CW_BODY_INDENT << "}";CW_BODY_ENDL;
		}
		theCompilerEnvironment.popForeach();
	}
}
