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

#ifndef _GrfParseFree_h_
#define _GrfParseFree_h_

#include "GrfCommand.h"

namespace CodeWorker {
	class ExprScriptVariable;
	class ExprScriptExpression;
	class ExprScriptScriptFile;
	class DtaScriptVariable;
	class DtaDesignScript;

	class EXECUTE_FUNCTION;

	class GrfParseFree : public GrfCommand {
	private:
		ExprScriptExpression* _pDesignFileName;
		ExprScriptVariable* _pClass;
		ExprScriptExpression* _pFileName;
		mutable DtaDesignScript* _pCachedScript;
		mutable std::string _sCachedDesignFile;

	public:
		GrfParseFree() : _pClass(NULL), _pDesignFileName(NULL), _pFileName(NULL), _pCachedScript(NULL) {}
		virtual ~GrfParseFree();

		virtual const char* getFunctionName() const { return "parseFree"; }

		inline void setDesignFileName(ExprScriptExpression* pDesignFileName) { _pDesignFileName = pDesignFileName; }
		void setDesignFileName(ExprScriptScriptFile* pDesignFileName);
		inline ExprScriptVariable* getThis() const { return _pClass; }
		inline void setThis(ExprScriptVariable* pClass) { _pClass = pClass; }
		inline ExprScriptExpression*  getInputFileName() const { return _pFileName; }
		inline void setInputFileName(ExprScriptExpression* pFileName) { _pFileName = pFileName; }

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		void compileCppForQuiet(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
		virtual SEQUENCE_INTERRUPTION_LIST executeScript(const char* sInputFile, DtaScriptVariable* pThisContext, EXECUTE_FUNCTION* executeFunction);
	};
}

#endif
