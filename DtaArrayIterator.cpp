/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2006 Cédric Lemaire

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

namespace CodeWorker {
	DtaArrayIterator::~DtaArrayIterator() {}
	DtaScriptVariable* DtaArrayIterator::itemNoRef() const {
		if (_pItemNoRef == NULL) {
			_pItemNoRef = item();
			while (_pItemNoRef->getReferencedVariable()) _pItemNoRef = _pItemNoRef->getReferencedVariable();
		}
		return _pItemNoRef;
	}

	DtaListIterator::~DtaListIterator() {}
	DtaArrayIterator* DtaListIterator::clone() const { return new DtaListIterator(*this); }
	int DtaListIterator::size() const { return _list.size(); }
	bool DtaListIterator::end() const { return i == _list.end(); }
	const char* DtaListIterator::key() const { return (*i)->getName(); }
	DtaScriptVariable* DtaListIterator::item() const { return *i; }

	bool DtaListIterator::prec() {
		if (i == _list.begin()) return false;
		--_iCurrentPosition;
		--i;
		_pItemNoRef = NULL;
		return true;
	}

	bool DtaListIterator::next() {
		++_iCurrentPosition;
		++i;
		_pItemNoRef = NULL;
		return i != _list.end();
	}


	DtaReverseListIterator::~DtaReverseListIterator() {}
	DtaArrayIterator* DtaReverseListIterator::clone() const { return new DtaReverseListIterator(*this); }
	int DtaReverseListIterator::size() const { return _list.size(); }
	bool DtaReverseListIterator::end() const { return i == _list.rend(); }
	const char* DtaReverseListIterator::key() const { return (*i)->getName(); }
	DtaScriptVariable* DtaReverseListIterator::item() const { return *i; }

	bool DtaReverseListIterator::prec() {
		if (i == _list.rbegin()) return false;
		--_iCurrentPosition;
		--i;
		_pItemNoRef = NULL;
		return true;
	}

	bool DtaReverseListIterator::next() {
		++_iCurrentPosition;
		++i;
		_pItemNoRef = NULL;
		return i != _list.rend();
	}
}
