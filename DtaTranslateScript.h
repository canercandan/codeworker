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

#ifndef _DtaTranslateScript_h_
#define _DtaTranslateScript_h_

#include "DtaBNFScript.h"

namespace CodeWorker {
	class DtaTranslateScript : public DtaBNFScript {
	private:
		std::string _sOutputFile;
		bool _bGeneratedFile;

	public:
		DtaTranslateScript() : _bGeneratedFile(true) {}
		DtaTranslateScript(/*DtaScriptVariable* pVisibility, */GrfBlock* pParentBlock) : DtaBNFScript(/*pVisibility, */pParentBlock), _bGeneratedFile(true) {}
		DtaTranslateScript(EXECUTE_FUNCTION* executeFunction) : DtaBNFScript(executeFunction), _bGeneratedFile(true) {}
		virtual ~DtaTranslateScript();

		virtual DtaScriptFactory::SCRIPT_TYPE getType() const;
		virtual bool isAParseScript() const;
		virtual bool isAGenerateScript() const;

		virtual void traceEngine() const;

		virtual SEQUENCE_INTERRUPTION_LIST translate(const std::string& sInputFile, const std::string& sOutputFile, DtaScriptVariable& thisContext);
		virtual SEQUENCE_INTERRUPTION_LIST translateString(const std::string& sInputString, std::string& sOutputString, DtaScriptVariable& thisContext);
		virtual SEQUENCE_INTERRUPTION_LIST execute(DtaScriptVariable& thisContext);

	protected:
		virtual bool betweenCommands(ScpStream& script, GrfBlock& block);
		virtual void handleUnknownCommand(const std::string& sCommand, ScpStream& script, GrfBlock& block);
		virtual void handleNotAWordCommand(ScpStream& script, GrfBlock& block);
		virtual void handleUnknownTokenDirective(const std::string& sDirective, ScpStream& script, BNFClause& rule, GrfBlock& block, bool& bContinue, bool& bNoCase, bool bLiteralOnly, BNFStepper* pStepper);

		virtual void parseBNFPreprocessorDirective(int iStatementBeginning, const std::string& sDirective, ScpStream& script, GrfBlock& block);

//##markup##"parsing"
//##begin##"parsing"
		virtual void parseAllFloatingLocations(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseAttachOutputToSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseDetachOutputFromSocket(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseIncrementIndentLevel(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseInsertText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseInsertTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseInsertTextToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseInsertTextOnceToFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseOverwritePortion(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parsePopulateProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseResizeOutputStream(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseSetFloatingLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseSetOutputLocation(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseSetProtectedArea(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseWriteBytes(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseWriteText(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
		virtual void parseWriteTextOnce(GrfBlock& block, ScpStream& script, ExprScriptVariable* pMethodCaller);
//##end##"parsing"

		virtual DtaBNFScript& getAlienParser() const;

	private:
		ExprScriptFunction* parseImplicitCopyFunction(GrfBlock& block, ScpStream& script);
	};
}

#endif
