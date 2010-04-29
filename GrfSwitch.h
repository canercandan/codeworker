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

#ifndef _GrfSwitch_h_
#define _GrfSwitch_h_

#include <string>
#include <map>

#include "GrfBlock.h"

namespace CodeWorker {
	class ExprScriptVariable;

	class GrfSwitch : public GrfBlock {
	private:
		ExprScriptExpression* _pExpression;
		std::map<std::string, int> _directionEqualList;
		std::map<std::string, int> _directionStartList;
		int _iDefault;

	public:
		GrfSwitch(GrfBlock* pBlock) : GrfBlock(pBlock), _pExpression(NULL), _iDefault(-1) {}
		virtual ~GrfSwitch();

		inline void setExpression(ExprScriptExpression* pExpression) { _pExpression = pExpression; }
		inline bool equalString(const char* sText) { return (_directionEqualList.find(sText) != _directionEqualList.end()); }
		inline void insertEqualString(const char* sText) { _directionEqualList[sText] = getNbCommands(); }
		inline bool startString(const char* sText) { return (_directionStartList.find(sText) != _directionStartList.end()); }
		inline void insertStartString(const char* sText) { _directionStartList[sText] = getNbCommands(); }
		inline void insertDefault() { _iDefault = getNbCommands(); }

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
	};
}

#endif
