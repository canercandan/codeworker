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

#ifndef _DtaArrayIterator_h_
#define _DtaArrayIterator_h_

#pragma warning (disable : 4786)

#include <list>

namespace CodeWorker {
	class DtaScriptVariable;
	
	class DtaArrayIterator {
	protected:
		int _iCurrentPosition;
		mutable DtaScriptVariable* _pItemNoRef;

	public:
		inline DtaArrayIterator(const DtaArrayIterator& copy) : _iCurrentPosition(copy._iCurrentPosition), _pItemNoRef(NULL) {}
		inline DtaArrayIterator() : _iCurrentPosition(0), _pItemNoRef(NULL) {}
		virtual ~DtaArrayIterator();

		virtual DtaArrayIterator* clone() const = 0;
		inline int index() const { return _iCurrentPosition; }
		inline bool first() const { return (_iCurrentPosition == 0); }
		inline bool last() const { return _iCurrentPosition + 1 == size(); }
		DtaScriptVariable* itemNoRef() const;
		virtual int size() const = 0;
		virtual bool end() const = 0;
		virtual DtaScriptVariable* item() const = 0;
		virtual const char* key() const = 0;
		virtual bool prec() = 0;
		virtual bool next() = 0;
	};

	class DtaListIterator : public DtaArrayIterator {
	private:
		const std::list<DtaScriptVariable*>& _list;
		std::list<DtaScriptVariable*>::const_iterator i;

	public:
		inline DtaListIterator(const DtaListIterator& copy) : DtaArrayIterator(copy), _list(copy._list), i(copy.i) {}
		inline DtaListIterator(const std::list<DtaScriptVariable*>& listOfNodes) : _list(listOfNodes), i(listOfNodes.begin()) {}
		virtual ~DtaListIterator();

		virtual DtaArrayIterator* clone() const;
		virtual int size() const;
		virtual bool end() const;
		virtual DtaScriptVariable* item() const;
		virtual const char* key() const;
		virtual bool prec();
		virtual bool next();
	};

	class DtaReverseListIterator : public DtaArrayIterator {
	private:
		const std::list<DtaScriptVariable*>& _list;
		std::list<DtaScriptVariable*>::const_reverse_iterator i;

	public:
		inline DtaReverseListIterator(const DtaReverseListIterator& copy) : DtaArrayIterator(copy), _list(copy._list), i(copy.i) {}
		inline DtaReverseListIterator(const std::list<DtaScriptVariable*>& listOfNodes) : _list(listOfNodes), i(listOfNodes.rbegin()) {}
		virtual ~DtaReverseListIterator();

		virtual DtaArrayIterator* clone() const;
		virtual int size() const;
		virtual bool end() const;
		virtual const char* key() const;
		virtual DtaScriptVariable* item() const;
		virtual bool prec();
		virtual bool next();
	};
}

#endif
