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

#ifndef _GrfGenerate_h_
#define _GrfGenerate_h_

#include "GrfCommand.h"

namespace CodeWorker {
	class ExprScriptVariable;
	class ExprScriptExpression;
	class ExprScriptScriptFile;
	class DtaPatternScript;

	class EXECUTE_FUNCTION;

	class GrfGenerate : public GrfCommand {
	private:
		ExprScriptExpression* _pPatternFileName;
		ExprScriptVariable* _pClass;
		ExprScriptExpression* _pFileName;
		std::string _sCurrentDirectoryAtCompileTime;

	protected:
		mutable DtaPatternScript* _pCachedScript;
		mutable std::string _sCachedPatternFile;

	public:
		GrfGenerate();
		virtual ~GrfGenerate();

		virtual const char* getFunctionName() const { return "generate"; }

		void setPatternFileName(ExprScriptScriptFile* pPatternFileName);
		inline void setPatternFileName(ExprScriptExpression* pPatternFileName) { _pPatternFileName = pPatternFileName; }
		inline void setThis(ExprScriptVariable* pClass) { _pClass = pClass; }
		inline void setOutputFileName(ExprScriptExpression* pFileName) { _pFileName = pFileName; }

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		inline DtaPatternScript* getCachedScript() const { return _pCachedScript; }

		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
		virtual SEQUENCE_INTERRUPTION_LIST executeScript(const char* sOutputFile, DtaScriptVariable* pThisContext, EXECUTE_FUNCTION* executeFunction);
	};
}

#endif
