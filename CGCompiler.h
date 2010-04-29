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
#ifndef _CGCompiler_h_
#define _CGCompiler_h_

#include "CppParsingTree.h"

namespace CodeWorker {
	class DtaScript;
	class DtaPatternScript;
	class DtaBNFScript;
	class DtaTranslateScript;

	class CGCompiledCommonScript {
		private:
			DtaScript* _pScript;

		public:
			inline CGCompiledCommonScript() : _pScript(NULL) {}
			inline CGCompiledCommonScript(DtaScript* pScript) : _pScript(pScript) {}
			virtual ~CGCompiledCommonScript();

			void buildFromString(const std::string& sText);
			void buildFromFile(const std::string& sScriptFile);

			void execute(DtaScriptVariable* pContext);
			void execute(CppParsingTree_var pContext);
	};

	class CGCompiledTemplateScript {
		private:
			DtaPatternScript* _pScript;

		public:
			inline CGCompiledTemplateScript() : _pScript(NULL) {}
			CGCompiledTemplateScript(const std::string& sFile);
			inline CGCompiledTemplateScript(DtaPatternScript* pScript) : _pScript(pScript) {}
			virtual ~CGCompiledTemplateScript();

			void buildFromString(const std::string& sText);
			void buildFromFile(const std::string& sScriptFile);

			void generate(DtaScriptVariable* pContext, const std::string& sOutputFile);
			void generateString(DtaScriptVariable* pContext, std::string& sOutputText);
			void expand(DtaScriptVariable* pContext, const std::string& sOutputFile);

			void generate(CppParsingTree_var pContext, const std::string& sOutputFile);
			void generateString(CppParsingTree_var pContext, std::string& sOutputText);
			void expand(CppParsingTree_var pContext, const std::string& sOutputFile);
	};

	class CGCompiledBNFScript {
		private:
			DtaBNFScript* _pScript;

		public:
			inline CGCompiledBNFScript() : _pScript(NULL) {}
			CGCompiledBNFScript(const std::string& sFile);
			inline CGCompiledBNFScript(DtaBNFScript* pScript) : _pScript(pScript) {}
			virtual ~CGCompiledBNFScript();

			void buildFromString(const std::string& sText);
			void buildFromFile(const std::string& sScriptFile);

			void parse(DtaScriptVariable* pContext, const std::string& sParsedFile);
			void parseString(DtaScriptVariable* pContext, const std::string& sText);

			void parse(CppParsingTree_var pContext, const std::string& sParsedFile);
			void parseString(CppParsingTree_var pContext, const std::string& sText);
	};

	class CGCompiledTranslationScript {
		private:
			DtaTranslateScript* _pScript;

		public:
			inline CGCompiledTranslationScript() : _pScript(NULL) {}
			CGCompiledTranslationScript(const std::string& sFile);
			inline CGCompiledTranslationScript(DtaTranslateScript* pScript) : _pScript(pScript) {}
			virtual ~CGCompiledTranslationScript();

			void buildFromString(const std::string& sText);
			void buildFromFile(const std::string& sScriptFile);

			void translate(DtaScriptVariable* pContext, const std::string& sParsedFile, const std::string& sOutputFile);
			void translateString(DtaScriptVariable* pContext, const std::string& sInputText, std::string& sOutputText);
			void translateString(CppParsingTree_var pContext, const std::string& sInputText, std::string& sOutputText);

			void translate(CppParsingTree_var pContext, const std::string& sParsedFile, const std::string& sOutputFile);
	};
}

#endif
