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

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include "ScpStream.h"

#include "CGRuntime.h"
#include "DtaScript.h"
#include "DtaPatternScript.h"
#include "DtaBNFScript.h"
#include "DtaTranslateScript.h"

#include "CGCompiler.h"

namespace CodeWorker {
	CGCompiledCommonScript::~CGCompiledCommonScript() {
		delete _pScript;
	}

	void CGCompiledCommonScript::buildFromFile(const std::string& sScriptFile) {
		if (_pScript != NULL) {
			delete _pScript;
			_pScript = NULL;
		}
		std::auto_ptr<DtaScript> pScript(new DtaScript);
		pScript->parseFile(sScriptFile.c_str());
		_pScript = pScript.release();
	}

	void CGCompiledCommonScript::buildFromString(const std::string& sText) {
		if (_pScript != NULL) {
			delete _pScript;
			_pScript = NULL;
		}
		std::auto_ptr<DtaScript> pScript(new DtaScript);
		ScpStream theCommand(sText);
		pScript->parseStream(theCommand);
		_pScript = pScript.release();
	}

	void CGCompiledCommonScript::execute(DtaScriptVariable* pContext) {
		CGThisModifier pThis(pContext);
		if (_pScript == NULL) throw UtlException("the common script is missing; specify it before an execution");
		_pScript->execute(*pContext);
	}

	void CGCompiledCommonScript::execute(CppParsingTree_var pContext) {
		CGThisModifier pThis(pContext._pInternalNode);
		execute(pContext._pInternalNode);
	}


	CGCompiledTemplateScript::CGCompiledTemplateScript(const std::string& sFile) : _pScript(NULL) {
		std::auto_ptr<DtaPatternScript> pScript(new DtaPatternScript);
		pScript->parseFile(sFile.c_str());
		_pScript = pScript.release();
	}

	CGCompiledTemplateScript::~CGCompiledTemplateScript() {
		delete _pScript;
	}

	void CGCompiledTemplateScript::buildFromFile(const std::string& sScriptFile) {
		if (_pScript != NULL) {
			delete _pScript;
			_pScript = NULL;
		}
		std::auto_ptr<DtaPatternScript> pScript(new DtaPatternScript);
		pScript->parseFile(sScriptFile.c_str());
		_pScript = pScript.release();
	}

	void CGCompiledTemplateScript::buildFromString(const std::string& sText) {
		if (_pScript != NULL) {
			delete _pScript;
			_pScript = NULL;
		}
		std::auto_ptr<DtaPatternScript> pScript(new DtaPatternScript);
		ScpStream theCommand(sText);
		pScript->parseStream(theCommand);
		_pScript = pScript.release();
	}

	void CGCompiledTemplateScript::generate(DtaScriptVariable* pContext, const std::string& sOutputFile) {
		CGThisModifier pThis(pContext);
		if (_pScript == NULL) throw UtlException("the template-based script is missing; specify it before an execution");
		_pScript->generate(sOutputFile.c_str(), *pContext);
	}

	void CGCompiledTemplateScript::generateString(DtaScriptVariable* pContext, std::string& sOutputText) {
		CGThisModifier pThis(pContext);
		if (_pScript == NULL) throw UtlException("the template-based script is missing; specify it before an execution");
		_pScript->generateString(sOutputText, *pContext);
	}

	void CGCompiledTemplateScript::expand(DtaScriptVariable* pContext, const std::string& sOutputFile) {
		CGThisModifier pThis(pContext);
		if (_pScript == NULL) throw UtlException("the template-based script is missing; specify it before an execution");
		_pScript->expand(sOutputFile.c_str(), *pContext);
	}

	void CGCompiledTemplateScript::generate(CppParsingTree_var pContext, const std::string& sOutputFile) {
		CGThisModifier pThis(pContext._pInternalNode);
		if (_pScript == NULL) throw UtlException("the template-based script is missing; specify it before an execution");
		_pScript->generate(sOutputFile.c_str(), *pContext._pInternalNode);
	}

	void CGCompiledTemplateScript::generateString(CppParsingTree_var pContext, std::string& sOutputText) {
		CGThisModifier pThis(pContext._pInternalNode);
		generateString(pContext._pInternalNode, sOutputText);
	}

	void CGCompiledTemplateScript::expand(CppParsingTree_var pContext, const std::string& sOutputFile) {
		CGThisModifier pThis(pContext._pInternalNode);
		expand(pContext._pInternalNode, sOutputFile);
	}


	CGCompiledBNFScript::CGCompiledBNFScript(const std::string& sFile) : _pScript(NULL) {
		std::auto_ptr<DtaBNFScript> pScript(new DtaBNFScript);
		pScript->parseFile(sFile.c_str());
		_pScript = pScript.release();
	}

	CGCompiledBNFScript::~CGCompiledBNFScript() {
		delete _pScript;
	}

	void CGCompiledBNFScript::buildFromFile(const std::string& sScriptFile) {
		if (_pScript != NULL) {
			delete _pScript;
			_pScript = NULL;
		}
		std::auto_ptr<DtaBNFScript> pScript(new DtaBNFScript);
		pScript->parseFile(sScriptFile.c_str());
		_pScript = pScript.release();
	}

	void CGCompiledBNFScript::buildFromString(const std::string& sText) {
		if (_pScript != NULL) {
			delete _pScript;
			_pScript = NULL;
		}
		std::auto_ptr<DtaBNFScript> pScript(new DtaBNFScript);
		ScpStream theCommand(sText);
		pScript->parseStream(theCommand);
		_pScript = pScript.release();
	}

	void CGCompiledBNFScript::parse(DtaScriptVariable* pContext, const std::string& sParsedFile) {
		CGThisModifier pThis(pContext);
		if (_pScript == NULL) throw UtlException("the BNF parse script is missing; specify it before an execution");
		_pScript->generate(sParsedFile.c_str(), *pContext);
	}

	void CGCompiledBNFScript::parseString(DtaScriptVariable* pContext, const std::string& sText) {
		CGThisModifier pThis(pContext);
		std::string sParsedText = sText;
		if (_pScript == NULL) throw UtlException("the BNF parse script is missing; specify it before an execution");
		_pScript->generateString(sParsedText, *pContext);
	}

	void CGCompiledBNFScript::parse(CppParsingTree_var pContext, const std::string& sParsedFile) {
		CGThisModifier pThis(pContext._pInternalNode);
		if (_pScript == NULL) throw UtlException("the BNF parse script is missing; specify it before an execution");
		_pScript->generate(sParsedFile.c_str(), *pContext._pInternalNode);
	}

	void CGCompiledBNFScript::parseString(CppParsingTree_var pContext, const std::string& sText) {
		CGThisModifier pThis(pContext._pInternalNode);
		std::string sParsedText = sText;
		if (_pScript == NULL) throw UtlException("the BNF parse script is missing; specify it before an execution");
		_pScript->generateString(sParsedText, *pContext._pInternalNode);
	}


	CGCompiledTranslationScript::CGCompiledTranslationScript(const std::string& sFile) : _pScript(NULL) {
		std::auto_ptr<DtaTranslateScript> pScript(new DtaTranslateScript);
		pScript->parseFile(sFile.c_str());
		_pScript = pScript.release();
	}

	CGCompiledTranslationScript::~CGCompiledTranslationScript() {
		delete _pScript;
	}

	void CGCompiledTranslationScript::buildFromFile(const std::string& sScriptFile) {
		if (_pScript != NULL) {
			delete _pScript;
			_pScript = NULL;
		}
		std::auto_ptr<DtaTranslateScript> pScript(new DtaTranslateScript);
		pScript->parseFile(sScriptFile.c_str());
		_pScript = pScript.release();
	}

	void CGCompiledTranslationScript::buildFromString(const std::string& sText) {
		if (_pScript != NULL) {
			delete _pScript;
			_pScript = NULL;
		}
		std::auto_ptr<DtaTranslateScript> pScript(new DtaTranslateScript);
		ScpStream theCommand(sText);
		pScript->parseStream(theCommand);
		_pScript = pScript.release();
	}

	void CGCompiledTranslationScript::translate(DtaScriptVariable* pContext, const std::string& sParsedFile, const std::string& sOutputFile) {
		CGThisModifier pThis(pContext);
		if (_pScript == NULL) throw UtlException("the translation script is missing; specify it before an execution");
		_pScript->translate(sParsedFile, sOutputFile, *pContext);
	}

	void CGCompiledTranslationScript::translateString(DtaScriptVariable* pContext, const std::string& sInputText, std::string& sOutputText) {
		CGThisModifier pThis(pContext);
		if (_pScript == NULL) throw UtlException("the translation script is missing; specify it before an execution");
		_pScript->translateString(sInputText, sOutputText, *pContext);
	}

	void CGCompiledTranslationScript::translate(CppParsingTree_var pContext, const std::string& sParsedFile, const std::string& sOutputFile) {
		CGThisModifier pThis(pContext._pInternalNode);
		if (_pScript == NULL) throw UtlException("the translation script is missing; specify it before an execution");
		_pScript->translate(sParsedFile, sOutputFile, *pContext._pInternalNode);
	}

	void CGCompiledTranslationScript::translateString(CppParsingTree_var pContext, const std::string& sInputText, std::string& sOutputText) {
		CGThisModifier pThis(pContext._pInternalNode);
		if (_pScript == NULL) throw UtlException("the translation script is missing; specify it before an execution");
		_pScript->translateString(sInputText, sOutputText, *pContext._pInternalNode);
	}
}
