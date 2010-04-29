/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2003 Cédric Lemaire

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
#	pragma warning (disable : 4786)
#endif

#include "UtlException.h"

#include "DtaBNFScript.h"
#include "DtaDesignScript.h"
#include "DtaPatternScript.h"
#include "DtaTranslateScript.h"

#include "DtaScriptFactory.h"

namespace CodeWorker {
	DtaScript* DtaScriptFactory::create(SCRIPT_TYPE eScriptType, GrfBlock& block) {
		DtaScript* pScript;
		switch(eScriptType) {
			case BNF_SCRIPT: pScript = new DtaBNFScript(&block);break;
			case FREE_SCRIPT: pScript = new DtaDesignScript(&block);break;
			case COMMON_SCRIPT: pScript = new DtaScript(&block);break;
			case PATTERN_SCRIPT: pScript = new DtaPatternScript(&block);break;
			case TRANSLATE_SCRIPT: pScript = new DtaTranslateScript(&block);break;
			default:
				throw UtlException("internal error: unhandled script type");
		}
		return pScript;
	}
}
