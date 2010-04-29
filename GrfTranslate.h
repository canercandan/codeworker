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

#ifndef _GrfTranslate_h_
#define _GrfTranslate_h_

#include "GrfCommand.h"

namespace CodeWorker {
	class ExprScriptVariable;
	class ExprScriptExpression;
	class ExprScriptScriptFile;
	class DtaTranslateScript;

	class EXECUTE_FUNCTION;

	class GrfTranslate : public GrfCommand {
	private:
		ExprScriptExpression* _pPatternFileName;
		ExprScriptVariable* _pClass;
		ExprScriptExpression* _pInputFileName;
		ExprScriptExpression* _pOutputFileName;
		std::string _sCurrentDirectoryAtCompileTime;

	protected:
		mutable DtaTranslateScript* _pCachedScript;
		mutable std::string _sCachedPatternFile;

	public:
		GrfTranslate();
		virtual ~GrfTranslate();

		virtual const char* getFunctionName() const { return "translate"; }

		inline void setPatternFileName(ExprScriptExpression* pPatternFileName) { _pPatternFileName = pPatternFileName; }
		void setPatternFileName(ExprScriptScriptFile* pPatternFileName);
		inline void setThis(ExprScriptVariable* pClass) { _pClass = pClass; }
		inline void setInputFileName(ExprScriptExpression* pFileName) { _pInputFileName = pFileName; }
		inline void setOutputFileName(ExprScriptExpression* pFileName) { _pOutputFileName = pFileName; }

		std::string translateString(DtaScriptVariable& visibility);

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		inline DtaTranslateScript* getCachedScript() const { return _pCachedScript; }

		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
		virtual SEQUENCE_INTERRUPTION_LIST executeScript(const std::string& sInputFile, const std::string& sOutputFile, DtaScriptVariable* pThisContext, EXECUTE_FUNCTION* executeFunction);
	};
}

#endif
