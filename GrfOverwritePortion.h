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

#ifndef _GrfOverwritePortion_h_
#define _GrfOverwritePortion_h_


//##protect##"INCLUDE FILES"
//##protect##"INCLUDE FILES"

#include "GrfCommand.h"

namespace CodeWorker {
class ExprScriptExpression;

	class GrfOverwritePortion : public GrfCommand {
		private:
			ExprScriptExpression* _pLocation;
			ExprScriptExpression* _pText;
			ExprScriptExpression* _pSize;

//##protect##"attributes"
			DtaScriptVariable* const* _pOutputCoverage;
//##protect##"attributes"

		public:
			GrfOverwritePortion() : _pLocation(NULL), _pText(NULL), _pSize(NULL) {
//##protect##"constructor"
				_pOutputCoverage = NULL;
//##protect##"constructor"
			}
			virtual ~GrfOverwritePortion();

			virtual const char* getFunctionName() const { return "overwritePortion"; }

			inline void setLocation(ExprScriptExpression* pLocation) { _pLocation = pLocation; }
			inline void setText(ExprScriptExpression* pText) { _pText = pText; }
			inline void setSize(ExprScriptExpression* pSize) { _pSize = pSize; }


//##protect##"interface"
			void prepareCoverage(DtaScriptVariable* const* pOutputCoverage);
//##protect##"interface"

			virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

		protected:
			virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);

//##protect##"declarations"
//##protect##"declarations"
	};
}

#endif
