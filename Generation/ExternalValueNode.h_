/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2004 Cédric Lemaire

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

#ifndef _ExternalValueNode_h_
#define _ExternalValueNode_h_

namespace CodeWorker {
	class ExternalValueNode;

	#ifndef EXTERNAL_VALUE_NODE_DESTRUCTOR_TYPE
	#define EXTERNAL_VALUE_NODE_DESTRUCTOR_TYPE
	typedef void (*EXTERNAL_VALUE_NODE_DESTRUCTOR)(ExternalValueNode*);
	#endif

	class ExternalValueNode {
	public:
		inline ExternalValueNode() {}
		virtual ~ExternalValueNode();

		virtual const char* getValue() const = 0;
		virtual size_t getValueLength() const;
		virtual double getDoubleValue() const;
		virtual bool   getBooleanValue() const;
		virtual int    getIntValue() const;
		virtual void setValue(const char* sValue) = 0;

		virtual EXTERNAL_VALUE_NODE_DESTRUCTOR getDestructor() const = 0;
	};
}

#endif
