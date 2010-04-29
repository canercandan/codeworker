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

#ifndef _GrfSaveProject_h_
#define _GrfSaveProject_h_


//##protect##"INCLUDE FILES"
#include <ostream>

namespace CodeWorker {
	class UtlXMLStream;
	class DtaScriptVariable;
}
//##protect##"INCLUDE FILES"

#include "GrfCommand.h"

namespace CodeWorker {
class ExprScriptExpression;
class ExprScriptVariable;

	class GrfSaveProject : public GrfCommand {
		private:
			ExprScriptExpression* _pXMLorTXTFileName;
			ExprScriptVariable* _pNodeToSave;

//##protect##"attributes"
//##protect##"attributes"

		public:
			GrfSaveProject() : _pXMLorTXTFileName(NULL), _pNodeToSave(NULL) {
//##protect##"constructor"
//##protect##"constructor"
			}
			virtual ~GrfSaveProject();

			virtual const char* getFunctionName() const { return "saveProject"; }

			inline void setXMLorTXTFileName(ExprScriptExpression* pXMLorTXTFileName) { _pXMLorTXTFileName = pXMLorTXTFileName; }
			inline void setNodeToSave(ExprScriptVariable* pNodeToSave) { _pNodeToSave = pNodeToSave; }

			void populateDefaultParameters();

//##protect##"interface"
//##protect##"interface"

			virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

		protected:
			virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);

//##protect##"declarations"
public:
	static void generateXMLFile(UtlXMLStream& myXMLFile, DtaScriptVariable& myNode, const std::string& sObjectName = "");
	static void generateTextFile(std::ostream& theStream, DtaScriptVariable& myNode, const std::string& sIndent = "");
//##protect##"declarations"
	};
}

#endif
