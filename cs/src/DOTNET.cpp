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

#include "UtlException.h"
#include "ScpStream.h"

#include "CGRuntime.h"
#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "DtaScript.h"

#include "CGCompiler.h"
#include "DOTNET.h"

#define CW_STRING_TO_CHARARRAY(s) \
			const char* tc##s = NULL;\
			try {\
				tc##s = static_cast<const char*>(const_cast<void*>(static_cast<const void*>(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(s))));\
			} catch(System::ArgumentException* /*exceptArg*/) {\
				std::cout << "ArgumentException*" << std::endl;\
			} catch(System::OutOfMemoryException* /*exceptOOM*/) {\
				std::cout << "OutOfMemoryException*" << std::endl;\
			}
#define CW_FREE_CHARARRAY(s) System::Runtime::InteropServices::Marshal::FreeHGlobal(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(tc##s))));

#define CW_CHARARRAY_TO_STRING(tcText) System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(tcText))))
#define CW_STL_TO_STRING(sText) System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(sText.c_str()))), (int) sText.size())

namespace CodeWorker {

	ParseTree::ParseTree() : cppInstance_(NULL), owner_(true) {
		cppInstance_ = new DtaScriptVariable;
	}

	ParseTree::ParseTree(ParseTree* pCopy) : cppInstance_(NULL), owner_(false) {
		if (pCopy != NULL) cppInstance_ = pCopy->cppInstance_;
	}

	ParseTree::~ParseTree() {
		if (owner_) delete cppInstance_;
	}

	ParseTree* ParseTree::get_reference() {
		DtaScriptVariable* pRef = cppInstance_->getReferencedVariable();
		if (pRef == NULL) return NULL;
		ParseTree* pResult = new ParseTree(NULL);
		pResult->cppInstance_ = pRef;
		return pResult;
	}

	void ParseTree::set_reference(ParseTree* pRef) {
		if (pRef == NULL) {
			cppInstance_->setValue((DtaScriptVariable*) NULL);
		} else {
			cppInstance_->setValue(pRef->cppInstance_);
		}
	}

	System::String* ParseTree::get_name() {
		const char* tcValue = cppInstance_->getName();
		if (tcValue == NULL) return NULL;
		return CW_CHARARRAY_TO_STRING(tcValue);
	}

	System::String* ParseTree::get_text() {
		const char* tcValue = cppInstance_->getValue();
		if (tcValue == NULL) return NULL;
		return CW_CHARARRAY_TO_STRING(tcValue);
	}

	void ParseTree::set_text(System::String* pValue) {
		CW_STRING_TO_CHARARRAY(pValue);
		cppInstance_->setValue(tcpValue);
		CW_FREE_CHARARRAY(pValue);
	}

	ParseTree* ParseTree::get_array()[] {
		ParseTree* pResult[];
		const std::list<DtaScriptVariable*>* pArray = cppInstance_->getArray();
		if (pArray == NULL) {
			pResult = NULL;
		} else {
			pResult = new ParseTree*[(int) pArray->size()];
			int i = 0;
			for (std::list<DtaScriptVariable*>::const_iterator it = pArray->begin(); it != pArray->end(); ++it) {
				pResult[i] = new ParseTree(NULL);
				pResult[i]->cppInstance_ = *it;
				++i;
			}
		}
		return pResult;
	}

	System::String* ParseTree::get_attributeNames()[] {
		DtaScriptVariableList* pAttributes = cppInstance_->getAttributes();
		if (pAttributes == NULL) return NULL;
		std::vector<const char*> tsAttributes;
		while (pAttributes != NULL) {
			DtaScriptVariable* pNode = pAttributes->getNode();
			tsAttributes.push_back(pNode->getName());
			pAttributes = pAttributes->getNext();
		}
		System::String* pResult[] = new System::String*[(int) tsAttributes.size()];
		for (unsigned int i = 0; i < tsAttributes.size(); ++i) {
			pResult[i] = CW_CHARARRAY_TO_STRING(tsAttributes[i]);
		}
		return pResult;
	}

	ParseTree* ParseTree::getNode(System::String* pAttribute) {
		CW_STRING_TO_CHARARRAY(pAttribute);
		DtaScriptVariable* pNode = cppInstance_->getNode(tcpAttribute);
		if (pNode == NULL) return NULL;
		ParseTree* pResult = new ParseTree(NULL);
		pResult->cppInstance_ = pNode;
		CW_FREE_CHARARRAY(pAttribute);
		return pResult;
	}

	ParseTree* ParseTree::insertNode(System::String* pAttribute) {
		CW_STRING_TO_CHARARRAY(pAttribute);
		DtaScriptVariable* pNode = cppInstance_->insertNode(tcpAttribute);
		if (pNode == NULL) return NULL;
		ParseTree* pResult = new ParseTree(NULL);
		pResult->cppInstance_ = pNode;
		CW_FREE_CHARARRAY(pAttribute);
		return pResult;
	}

	ParseTree* ParseTree::addItem(System::String* pKey) {
		CW_STRING_TO_CHARARRAY(pKey);
		DtaScriptVariable* pNode = cppInstance_->addElement(tcpKey);
		if (pNode == NULL) return NULL;
		ParseTree* pResult = new ParseTree(NULL);
		pResult->cppInstance_ = pNode;
		CW_FREE_CHARARRAY(pKey);
		return pResult;
	}



	CompiledCommonScript::CompiledCommonScript() : _pScript(NULL) {
		_pScript = __nogc new CGCompiledCommonScript;
	}

	CompiledCommonScript::~CompiledCommonScript() {
		delete _pScript;
	}

	void CompiledCommonScript::buildFromFile(System::String* pFilename) {
		CW_STRING_TO_CHARARRAY(pFilename);
		try {
			_pScript->buildFromFile(tcpFilename);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(pFilename);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(pFilename);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledCommonScript::buildFromFile(...)");
		}
		CW_FREE_CHARARRAY(pFilename);
	}

	void CompiledCommonScript::buildFromString(System::String* pText) {
		CW_STRING_TO_CHARARRAY(pText);
		std::string sScriptText = tcpText;
		try {
			_pScript->buildFromString(sScriptText);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(pText);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(pText);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledCommonScript::buildFromString(...)");
		}
		CW_FREE_CHARARRAY(pText);
	}

	void CompiledCommonScript::execute(ParseTree* pContext) {
		try {
			_pScript->execute(pContext->cppInstance_);
		} catch(std::exception& exception) {
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			throw new System::Exception(L"an ellipsis exception has occured in CompiledCommonScript::execute(...)");
		}
	}


	CompiledTemplateScript::CompiledTemplateScript() : _pScript(NULL) {
		_pScript = new CGCompiledTemplateScript;
	}

	CompiledTemplateScript::~CompiledTemplateScript() {
		delete _pScript;
	}

	void CompiledTemplateScript::buildFromFile(System::String* pFilename) {
		CW_STRING_TO_CHARARRAY(pFilename);
		try {
			_pScript->buildFromFile(tcpFilename);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(pFilename);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(pFilename);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledTemplateScript::buildFromFile(...)");
		}
		CW_FREE_CHARARRAY(pFilename);
	}

	void CompiledTemplateScript::buildFromString(System::String* pText) {
		CW_STRING_TO_CHARARRAY(pText);
		std::string sScriptText = tcpText;
		try {
			_pScript->buildFromString(sScriptText);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(pText);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(pText);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledTemplateScript::buildFromString(...)");
		}
		CW_FREE_CHARARRAY(pText);
	}

	void CompiledTemplateScript::generate(ParseTree* pContext, System::String* sOutputFile) {
		CW_STRING_TO_CHARARRAY(sOutputFile);
		try {
			_pScript->generate(pContext->cppInstance_, tcsOutputFile);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(sOutputFile);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(sOutputFile);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledTemplateScript::generate(...)");
		}
		CW_FREE_CHARARRAY(sOutputFile);
	}

	System::String* CompiledTemplateScript::generateString(ParseTree* pContext, System::String* sOutputText) {
		CW_STRING_TO_CHARARRAY(sOutputText);
		std::string sSTDText = tcsOutputText;
		try {
			_pScript->generateString(pContext->cppInstance_, sSTDText);
			sSTDText = tcsOutputText;
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(sOutputText);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(sOutputText);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledTemplateScript::generateString(...)");
		}
		CW_FREE_CHARARRAY(sOutputText);
		return CW_STL_TO_STRING(sSTDText);
	}

	void CompiledTemplateScript::expand(ParseTree* pContext, System::String* sOutputFile) {
		CW_STRING_TO_CHARARRAY(sOutputFile);
		try {
			_pScript->expand(pContext->cppInstance_, tcsOutputFile);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(sOutputFile);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(sOutputFile);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledTemplateScript::expand(...)");
		}
		CW_FREE_CHARARRAY(sOutputFile);
	}


	CompiledBNFScript::CompiledBNFScript() : _pScript(NULL) {
		_pScript = new CGCompiledBNFScript;
	}

	CompiledBNFScript::~CompiledBNFScript() {
		delete _pScript;
	}

	void CompiledBNFScript::buildFromFile(System::String* pFilename) {
		CW_STRING_TO_CHARARRAY(pFilename);
		try {
			_pScript->buildFromFile(tcpFilename);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(pFilename);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(pFilename);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledBNFScript::buildFromFile(...)");
		}
		CW_FREE_CHARARRAY(pFilename);
	}

	void CompiledBNFScript::buildFromString(System::String* pText) {
		CW_STRING_TO_CHARARRAY(pText);
		std::string sScriptText = tcpText;
		try {
			_pScript->buildFromString(sScriptText);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(pText);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(pText);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledBNFScript::buildFromString(...)");
		}
		CW_FREE_CHARARRAY(pText);
	}

	void CompiledBNFScript::parse(ParseTree* pContext, System::String* sParsedFile) {
		CW_STRING_TO_CHARARRAY(sParsedFile);
		try {
			_pScript->parse(pContext->cppInstance_, tcsParsedFile);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(sParsedFile);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(sParsedFile);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledBNFScript::parse(...)");
		}
		CW_FREE_CHARARRAY(sParsedFile);
	}

	void CompiledBNFScript::parseString(ParseTree* pContext, System::String* sText) {
		CW_STRING_TO_CHARARRAY(sText);
		std::string sSTDText = tcsText;
		try {
			_pScript->parseString(pContext->cppInstance_, sSTDText);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(sText);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(sText);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledBNFScript::parseString(...)");
		}
		CW_FREE_CHARARRAY(sText);
	}


	CompiledTranslationScript::CompiledTranslationScript() : _pScript(NULL) {
		_pScript = new CGCompiledTranslationScript;
	}

	CompiledTranslationScript::~CompiledTranslationScript() {
		delete _pScript;
	}

	void CompiledTranslationScript::buildFromFile(System::String* pFilename) {
		CW_STRING_TO_CHARARRAY(pFilename);
		try {
			_pScript->buildFromFile(tcpFilename);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(pFilename);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(pFilename);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledTranslationScript::buildFromFile(...)");
		}
		CW_FREE_CHARARRAY(pFilename);
	}

	void CompiledTranslationScript::buildFromString(System::String* pText) {
		CW_STRING_TO_CHARARRAY(pText);
		std::string sScriptText = tcpText;
		try {
			_pScript->buildFromString(sScriptText);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(pText);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(pText);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledTranslationScript::buildFromString(...)");
		}
		CW_FREE_CHARARRAY(pText);
	}

	void CompiledTranslationScript::translate(ParseTree* pContext, System::String* sParsedFile, System::String* sOutputFile) {
		CW_STRING_TO_CHARARRAY(sParsedFile);
		CW_STRING_TO_CHARARRAY(sOutputFile);
		try {
			_pScript->translate(pContext->cppInstance_, tcsParsedFile, tcsOutputFile);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(sOutputFile);
			CW_FREE_CHARARRAY(sParsedFile);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(sOutputFile);
			CW_FREE_CHARARRAY(sParsedFile);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledTranslationScript::translate(...)");
		}
		CW_FREE_CHARARRAY(sOutputFile);
		CW_FREE_CHARARRAY(sParsedFile);
	}

	System::String* CompiledTranslationScript::translateString(ParseTree* pContext, System::String* sInputText) {
		CW_STRING_TO_CHARARRAY(sInputText);
		std::string sOutputText;
		try {
			_pScript->translateString(pContext->cppInstance_, tcsInputText, sOutputText);
		} catch(std::exception& exception) {
			CW_FREE_CHARARRAY(sInputText);
			System::String* pMessage = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(const_cast<void*>(static_cast<const void*>(exception.what()))));
			throw new System::Exception(pMessage);
		} catch(...) {
			CW_FREE_CHARARRAY(sInputText);
			throw new System::Exception(L"an ellipsis exception has occured in CompiledTranslationScript::translateString(...)");
		}
		CW_FREE_CHARARRAY(sInputText);
		return CW_STL_TO_STRING(sOutputText);
	}

//##markup##"C++ body"
//##begin##"C++ body"
	void Runtime::appendFile(System::String* hFilename, System::String* hContent) {
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string sContent;
		CW_STRING_TO_CHARARRAY(hContent);
		sContent = tchContent;
		CGRuntime::appendFile(sFilename, sContent);
		CW_FREE_CHARARRAY(hContent);
		CW_FREE_CHARARRAY(hFilename);
	}

	void Runtime::clearVariable(ParseTree* hNode) {
		CodeWorker::DtaScriptVariable* pNode;
		pNode = ((hNode == NULL) ? NULL : hNode->cppInstance_);
		CGRuntime::clearVariable(pNode);
	}

	void Runtime::compileToCpp(System::String* hScriptFileName, System::String* hProjectDirectory, System::String* hCodeWorkerDirectory) {
		std::string sScriptFileName;
		CW_STRING_TO_CHARARRAY(hScriptFileName);
		sScriptFileName = tchScriptFileName;
		std::string sProjectDirectory;
		CW_STRING_TO_CHARARRAY(hProjectDirectory);
		sProjectDirectory = tchProjectDirectory;
		std::string sCodeWorkerDirectory;
		CW_STRING_TO_CHARARRAY(hCodeWorkerDirectory);
		sCodeWorkerDirectory = tchCodeWorkerDirectory;
		CGRuntime::compileToCpp(sScriptFileName, sProjectDirectory, sCodeWorkerDirectory);
		CW_FREE_CHARARRAY(hCodeWorkerDirectory);
		CW_FREE_CHARARRAY(hProjectDirectory);
		CW_FREE_CHARARRAY(hScriptFileName);
	}

	void Runtime::copyFile(System::String* hSourceFileName, System::String* hDestinationFileName) {
		std::string sSourceFileName;
		CW_STRING_TO_CHARARRAY(hSourceFileName);
		sSourceFileName = tchSourceFileName;
		std::string sDestinationFileName;
		CW_STRING_TO_CHARARRAY(hDestinationFileName);
		sDestinationFileName = tchDestinationFileName;
		CGRuntime::copyFile(sSourceFileName, sDestinationFileName);
		CW_FREE_CHARARRAY(hDestinationFileName);
		CW_FREE_CHARARRAY(hSourceFileName);
	}

	void Runtime::copyGenerableFile(System::String* hSourceFileName, System::String* hDestinationFileName) {
		std::string sSourceFileName;
		CW_STRING_TO_CHARARRAY(hSourceFileName);
		sSourceFileName = tchSourceFileName;
		std::string sDestinationFileName;
		CW_STRING_TO_CHARARRAY(hDestinationFileName);
		sDestinationFileName = tchDestinationFileName;
		CGRuntime::copyGenerableFile(sSourceFileName, sDestinationFileName);
		CW_FREE_CHARARRAY(hDestinationFileName);
		CW_FREE_CHARARRAY(hSourceFileName);
	}

	void Runtime::copySmartDirectory(System::String* hSourceDirectory, System::String* hDestinationPath) {
		std::string sSourceDirectory;
		CW_STRING_TO_CHARARRAY(hSourceDirectory);
		sSourceDirectory = tchSourceDirectory;
		std::string sDestinationPath;
		CW_STRING_TO_CHARARRAY(hDestinationPath);
		sDestinationPath = tchDestinationPath;
		CGRuntime::copySmartDirectory(sSourceDirectory, sDestinationPath);
		CW_FREE_CHARARRAY(hDestinationPath);
		CW_FREE_CHARARRAY(hSourceDirectory);
	}

	void Runtime::cutString(System::String* hText, System::String* hSeparator, ParseTree* hList) {
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string sSeparator;
		CW_STRING_TO_CHARARRAY(hSeparator);
		sSeparator = tchSeparator;
		std::list<std::string> slList;
		// NOT HANDLED YET!
		CGRuntime::cutString(sText, sSeparator, slList);
		CW_FREE_CHARARRAY(hSeparator);
		CW_FREE_CHARARRAY(hText);
	}

	void Runtime::environTable(ParseTree* hTable) {
		CodeWorker::DtaScriptVariable* pTable;
		pTable = ((hTable == NULL) ? NULL : hTable->cppInstance_);
		CGRuntime::environTable(pTable);
	}

	void Runtime::extendExecutedScript(System::String* hScriptContent) {
		std::string sScriptContent;
		CW_STRING_TO_CHARARRAY(hScriptContent);
		sScriptContent = tchScriptContent;
		CGRuntime::extendExecutedScript(sScriptContent);
		CW_FREE_CHARARRAY(hScriptContent);
	}

	void Runtime::insertElementAt(ParseTree* hList, System::String* hKey, int hPosition) {
		CodeWorker::DtaScriptVariable* pList;
		pList = ((hList == NULL) ? NULL : hList->cppInstance_);
		std::string sKey;
		CW_STRING_TO_CHARARRAY(hKey);
		sKey = tchKey;
		int iPosition;
		iPosition = hPosition;
		CGRuntime::insertElementAt(pList, sKey, iPosition);
		CW_FREE_CHARARRAY(hKey);
	}

	void Runtime::invertArray(ParseTree* hArray) {
		CodeWorker::DtaScriptVariable* pArray;
		pArray = ((hArray == NULL) ? NULL : hArray->cppInstance_);
		CGRuntime::invertArray(pArray);
	}

	void Runtime::listAllGeneratedFiles(ParseTree* hFiles) {
		CodeWorker::DtaScriptVariable* pFiles;
		pFiles = ((hFiles == NULL) ? NULL : hFiles->cppInstance_);
		CGRuntime::listAllGeneratedFiles(pFiles);
	}

	void Runtime::loadProject(System::String* hXMLorTXTFileName, ParseTree* hNodeToLoad) {
		std::string sXMLorTXTFileName;
		CW_STRING_TO_CHARARRAY(hXMLorTXTFileName);
		sXMLorTXTFileName = tchXMLorTXTFileName;
		CodeWorker::DtaScriptVariable* pNodeToLoad;
		pNodeToLoad = ((hNodeToLoad == NULL) ? NULL : hNodeToLoad->cppInstance_);
		CGRuntime::loadProject(sXMLorTXTFileName, pNodeToLoad);
		CW_FREE_CHARARRAY(hXMLorTXTFileName);
	}

	void Runtime::openLogFile(System::String* hFilename) {
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		CGRuntime::openLogFile(sFilename);
		CW_FREE_CHARARRAY(hFilename);
	}

	void Runtime::produceHTML(System::String* hScriptFileName, System::String* hHTMLFileName) {
		std::string sScriptFileName;
		CW_STRING_TO_CHARARRAY(hScriptFileName);
		sScriptFileName = tchScriptFileName;
		std::string sHTMLFileName;
		CW_STRING_TO_CHARARRAY(hHTMLFileName);
		sHTMLFileName = tchHTMLFileName;
		CGRuntime::produceHTML(sScriptFileName, sHTMLFileName);
		CW_FREE_CHARARRAY(hHTMLFileName);
		CW_FREE_CHARARRAY(hScriptFileName);
	}

	void Runtime::putEnv(System::String* hName, System::String* hValue) {
		std::string sName;
		CW_STRING_TO_CHARARRAY(hName);
		sName = tchName;
		std::string sValue;
		CW_STRING_TO_CHARARRAY(hValue);
		sValue = tchValue;
		CGRuntime::putEnv(sName, sValue);
		CW_FREE_CHARARRAY(hValue);
		CW_FREE_CHARARRAY(hName);
	}

	void Runtime::randomSeed(int hSeed) {
		int iSeed;
		iSeed = hSeed;
		CGRuntime::randomSeed(iSeed);
	}

	void Runtime::removeAllElements(ParseTree* hVariable) {
		CodeWorker::DtaScriptVariable* pVariable;
		pVariable = ((hVariable == NULL) ? NULL : hVariable->cppInstance_);
		CGRuntime::removeAllElements(pVariable);
	}

	void Runtime::removeElement(ParseTree* hVariable, System::String* hKey) {
		CodeWorker::DtaScriptVariable* pVariable;
		pVariable = ((hVariable == NULL) ? NULL : hVariable->cppInstance_);
		std::string sKey;
		CW_STRING_TO_CHARARRAY(hKey);
		sKey = tchKey;
		CGRuntime::removeElement(pVariable, sKey);
		CW_FREE_CHARARRAY(hKey);
	}

	void Runtime::removeFirstElement(ParseTree* hList) {
		CodeWorker::DtaScriptVariable* pList;
		pList = ((hList == NULL) ? NULL : hList->cppInstance_);
		CGRuntime::removeFirstElement(pList);
	}

	void Runtime::removeLastElement(ParseTree* hList) {
		CodeWorker::DtaScriptVariable* pList;
		pList = ((hList == NULL) ? NULL : hList->cppInstance_);
		CGRuntime::removeLastElement(pList);
	}

	void Runtime::removeRecursive(ParseTree* hVariable, System::String* hAttribute) {
		CodeWorker::DtaScriptVariable* pVariable;
		pVariable = ((hVariable == NULL) ? NULL : hVariable->cppInstance_);
		std::string sAttribute;
		CW_STRING_TO_CHARARRAY(hAttribute);
		sAttribute = tchAttribute;
		CGRuntime::removeRecursive(pVariable, sAttribute);
		CW_FREE_CHARARRAY(hAttribute);
	}

	void Runtime::removeVariable(ParseTree* hNode) {
		CodeWorker::DtaScriptVariable* pNode;
		pNode = ((hNode == NULL) ? NULL : hNode->cppInstance_);
		CGRuntime::removeVariable(pNode);
	}

	void Runtime::saveBinaryToFile(System::String* hFilename, System::String* hContent) {
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string sContent;
		CW_STRING_TO_CHARARRAY(hContent);
		sContent = tchContent;
		CGRuntime::saveBinaryToFile(sFilename, sContent);
		CW_FREE_CHARARRAY(hContent);
		CW_FREE_CHARARRAY(hFilename);
	}

	void Runtime::saveProject(System::String* hXMLorTXTFileName, ParseTree* hNodeToSave) {
		std::string sXMLorTXTFileName;
		CW_STRING_TO_CHARARRAY(hXMLorTXTFileName);
		sXMLorTXTFileName = tchXMLorTXTFileName;
		CodeWorker::DtaScriptVariable* pNodeToSave;
		pNodeToSave = ((hNodeToSave == NULL) ? NULL : hNodeToSave->cppInstance_);
		CGRuntime::saveProject(sXMLorTXTFileName, pNodeToSave);
		CW_FREE_CHARARRAY(hXMLorTXTFileName);
	}

	void Runtime::saveProjectTypes(System::String* hXMLFileName) {
		std::string sXMLFileName;
		CW_STRING_TO_CHARARRAY(hXMLFileName);
		sXMLFileName = tchXMLFileName;
		CGRuntime::saveProjectTypes(sXMLFileName);
		CW_FREE_CHARARRAY(hXMLFileName);
	}

	void Runtime::saveToFile(System::String* hFilename, System::String* hContent) {
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string sContent;
		CW_STRING_TO_CHARARRAY(hContent);
		sContent = tchContent;
		CGRuntime::saveToFile(sFilename, sContent);
		CW_FREE_CHARARRAY(hContent);
		CW_FREE_CHARARRAY(hFilename);
	}

	void Runtime::setCommentBegin(System::String* hCommentBegin) {
		std::string sCommentBegin;
		CW_STRING_TO_CHARARRAY(hCommentBegin);
		sCommentBegin = tchCommentBegin;
		CGRuntime::setCommentBegin(sCommentBegin);
		CW_FREE_CHARARRAY(hCommentBegin);
	}

	void Runtime::setCommentEnd(System::String* hCommentEnd) {
		std::string sCommentEnd;
		CW_STRING_TO_CHARARRAY(hCommentEnd);
		sCommentEnd = tchCommentEnd;
		CGRuntime::setCommentEnd(sCommentEnd);
		CW_FREE_CHARARRAY(hCommentEnd);
	}

	void Runtime::setGenerationHeader(System::String* hComment) {
		std::string sComment;
		CW_STRING_TO_CHARARRAY(hComment);
		sComment = tchComment;
		CGRuntime::setGenerationHeader(sComment);
		CW_FREE_CHARARRAY(hComment);
	}

	void Runtime::setIncludePath(System::String* hPath) {
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		CGRuntime::setIncludePath(sPath);
		CW_FREE_CHARARRAY(hPath);
	}

	void Runtime::setNow(System::String* hConstantDateTime) {
		std::string sConstantDateTime;
		CW_STRING_TO_CHARARRAY(hConstantDateTime);
		sConstantDateTime = tchConstantDateTime;
		CGRuntime::setNow(sConstantDateTime);
		CW_FREE_CHARARRAY(hConstantDateTime);
	}

	void Runtime::setProperty(System::String* hDefine, System::String* hValue) {
		std::string sDefine;
		CW_STRING_TO_CHARARRAY(hDefine);
		sDefine = tchDefine;
		std::string sValue;
		CW_STRING_TO_CHARARRAY(hValue);
		sValue = tchValue;
		CGRuntime::setProperty(sDefine, sValue);
		CW_FREE_CHARARRAY(hValue);
		CW_FREE_CHARARRAY(hDefine);
	}

	void Runtime::setTextMode(System::String* hTextMode) {
		std::string sTextMode;
		CW_STRING_TO_CHARARRAY(hTextMode);
		sTextMode = tchTextMode;
		CGRuntime::setTextMode(sTextMode);
		CW_FREE_CHARARRAY(hTextMode);
	}

	void Runtime::setVersion(System::String* hVersion) {
		std::string sVersion;
		CW_STRING_TO_CHARARRAY(hVersion);
		sVersion = tchVersion;
		CGRuntime::setVersion(sVersion);
		CW_FREE_CHARARRAY(hVersion);
	}

	void Runtime::setWriteMode(System::String* hMode) {
		std::string sMode;
		CW_STRING_TO_CHARARRAY(hMode);
		sMode = tchMode;
		CGRuntime::setWriteMode(sMode);
		CW_FREE_CHARARRAY(hMode);
	}

	void Runtime::setWorkingPath(System::String* hPath) {
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		CGRuntime::setWorkingPath(sPath);
		CW_FREE_CHARARRAY(hPath);
	}

	void Runtime::sleep(int hMillis) {
		int iMillis;
		iMillis = hMillis;
		CGRuntime::sleep(iMillis);
	}

	void Runtime::slideNodeContent(ParseTree* hOrgNode, System::String* hDestNode) {
		CodeWorker::DtaScriptVariable* pOrgNode;
		pOrgNode = ((hOrgNode == NULL) ? NULL : hOrgNode->cppInstance_);
		ExprScriptVariable* exprdestNode;
		{
			CW_STRING_TO_CHARARRAY(hDestNode);
			DtaScript theEmptyScript(NULL);
			GrfBlock* pBlock = NULL;
			GrfBlock& myNullBlock = *pBlock;
			ScpStream myStream(tchDestNode);
			exprdestNode = theEmptyScript.parseVariableExpression(myNullBlock, myStream);
			CW_FREE_CHARARRAY(hDestNode);
		}
		std::auto_ptr<ExprScriptVariable> treexprdestNode(exprdestNode);
		CodeWorker::ExprScriptVariable& xDestNode = *treexprdestNode;
		CGRuntime::slideNodeContent(pOrgNode, xDestNode);
	}

	void Runtime::sortArray(ParseTree* hArray) {
		CodeWorker::DtaScriptVariable* pArray;
		pArray = ((hArray == NULL) ? NULL : hArray->cppInstance_);
		CGRuntime::sortArray(pArray);
	}

	void Runtime::traceEngine() {
		CGRuntime::traceEngine();
	}

	void Runtime::traceLine(System::String* hLine) {
		std::string sLine;
		CW_STRING_TO_CHARARRAY(hLine);
		sLine = tchLine;
		CGRuntime::traceLine(sLine);
		CW_FREE_CHARARRAY(hLine);
	}

	void Runtime::traceObject(ParseTree* hObject, int hDepth) {
		CodeWorker::DtaScriptVariable* pObject;
		pObject = ((hObject == NULL) ? NULL : hObject->cppInstance_);
		int iDepth;
		iDepth = hDepth;
		CGRuntime::traceObject(pObject, iDepth);
	}

	void Runtime::traceText(System::String* hText) {
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		CGRuntime::traceText(sText);
		CW_FREE_CHARARRAY(hText);
	}

	void Runtime::attachInputToSocket(int hSocket) {
		int iSocket;
		iSocket = hSocket;
		CGRuntime::attachInputToSocket(iSocket);
	}

	void Runtime::detachInputFromSocket(int hSocket) {
		int iSocket;
		iSocket = hSocket;
		CGRuntime::detachInputFromSocket(iSocket);
	}

	void Runtime::goBack() {
		CGRuntime::goBack();
	}

	void Runtime::setInputLocation(int hLocation) {
		int iLocation;
		iLocation = hLocation;
		CGRuntime::setInputLocation(iLocation);
	}

	void Runtime::allFloatingLocations(ParseTree* hList) {
		CodeWorker::DtaScriptVariable* pList;
		pList = ((hList == NULL) ? NULL : hList->cppInstance_);
		CGRuntime::allFloatingLocations(pList);
	}

	void Runtime::attachOutputToSocket(int hSocket) {
		int iSocket;
		iSocket = hSocket;
		CGRuntime::attachOutputToSocket(iSocket);
	}

	void Runtime::detachOutputFromSocket(int hSocket) {
		int iSocket;
		iSocket = hSocket;
		CGRuntime::detachOutputFromSocket(iSocket);
	}

	void Runtime::incrementIndentLevel(int hLevel) {
		int iLevel;
		iLevel = hLevel;
		CGRuntime::incrementIndentLevel(iLevel);
	}

	void Runtime::insertText(int hLocation, System::String* hText) {
		int iLocation;
		iLocation = hLocation;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		CGRuntime::insertText(iLocation, sText);
		CW_FREE_CHARARRAY(hText);
	}

	void Runtime::insertTextOnce(int hLocation, System::String* hText) {
		int iLocation;
		iLocation = hLocation;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		CGRuntime::insertTextOnce(iLocation, sText);
		CW_FREE_CHARARRAY(hText);
	}

	void Runtime::insertTextToFloatingLocation(System::String* hLocation, System::String* hText) {
		std::string sLocation;
		CW_STRING_TO_CHARARRAY(hLocation);
		sLocation = tchLocation;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		CGRuntime::insertTextToFloatingLocation(sLocation, sText);
		CW_FREE_CHARARRAY(hText);
		CW_FREE_CHARARRAY(hLocation);
	}

	void Runtime::insertTextOnceToFloatingLocation(System::String* hLocation, System::String* hText) {
		std::string sLocation;
		CW_STRING_TO_CHARARRAY(hLocation);
		sLocation = tchLocation;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		CGRuntime::insertTextOnceToFloatingLocation(sLocation, sText);
		CW_FREE_CHARARRAY(hText);
		CW_FREE_CHARARRAY(hLocation);
	}

	void Runtime::overwritePortion(int hLocation, System::String* hText, int hSize) {
		int iLocation;
		iLocation = hLocation;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iSize;
		iSize = hSize;
		CGRuntime::overwritePortion(iLocation, sText, iSize);
		CW_FREE_CHARARRAY(hText);
	}

	void Runtime::populateProtectedArea(System::String* hProtectedAreaName, System::String* hContent) {
		std::string sProtectedAreaName;
		CW_STRING_TO_CHARARRAY(hProtectedAreaName);
		sProtectedAreaName = tchProtectedAreaName;
		std::string sContent;
		CW_STRING_TO_CHARARRAY(hContent);
		sContent = tchContent;
		CGRuntime::populateProtectedArea(sProtectedAreaName, sContent);
		CW_FREE_CHARARRAY(hContent);
		CW_FREE_CHARARRAY(hProtectedAreaName);
	}

	void Runtime::resizeOutputStream(int hNewSize) {
		int iNewSize;
		iNewSize = hNewSize;
		CGRuntime::resizeOutputStream(iNewSize);
	}

	void Runtime::setFloatingLocation(System::String* hKey, int hLocation) {
		std::string sKey;
		CW_STRING_TO_CHARARRAY(hKey);
		sKey = tchKey;
		int iLocation;
		iLocation = hLocation;
		CGRuntime::setFloatingLocation(sKey, iLocation);
		CW_FREE_CHARARRAY(hKey);
	}

	void Runtime::setOutputLocation(int hLocation) {
		int iLocation;
		iLocation = hLocation;
		CGRuntime::setOutputLocation(iLocation);
	}

	void Runtime::setProtectedArea(System::String* hProtectedAreaName) {
		std::string sProtectedAreaName;
		CW_STRING_TO_CHARARRAY(hProtectedAreaName);
		sProtectedAreaName = tchProtectedAreaName;
		CGRuntime::setProtectedArea(sProtectedAreaName);
		CW_FREE_CHARARRAY(hProtectedAreaName);
	}

	void Runtime::writeBytes(System::String* hBytes) {
		std::string sBytes;
		CW_STRING_TO_CHARARRAY(hBytes);
		sBytes = tchBytes;
		CGRuntime::writeBytes(sBytes);
		CW_FREE_CHARARRAY(hBytes);
	}

	void Runtime::writeText(System::String* hText) {
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		CGRuntime::writeText(sText);
		CW_FREE_CHARARRAY(hText);
	}

	void Runtime::writeTextOnce(System::String* hText) {
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		CGRuntime::writeTextOnce(sText);
		CW_FREE_CHARARRAY(hText);
	}

	void Runtime::closeSocket(int hSocket) {
		int iSocket;
		iSocket = hSocket;
		CGRuntime::closeSocket(iSocket);
	}

		bool Runtime::flushOutputToSocket(int hSocket) {
		bool result;
		int iSocket;
		iSocket = hSocket;
		bool cppResult = CGRuntime::flushOutputToSocket(iSocket);
		result = cppResult;
		return result;
	}

		int Runtime::acceptSocket(int hServerSocket) {
		int result;
		int iServerSocket;
		iServerSocket = hServerSocket;
		int cppResult = CGRuntime::acceptSocket(iServerSocket);
		result = cppResult;
		return result;
	}

		double Runtime::add(double hLeft, double hRight) {
		double result;
		double dLeft;
		dLeft = hLeft;
		double dRight;
		dRight = hRight;
		double cppResult = CGRuntime::add(dLeft, dRight);
		result = cppResult;
		return result;
	}

		System::String* Runtime::addToDate(System::String* hDate, System::String* hFormat, System::String* hShifting) {
		System::String* result;
		std::string sDate;
		CW_STRING_TO_CHARARRAY(hDate);
		sDate = tchDate;
		std::string sFormat;
		CW_STRING_TO_CHARARRAY(hFormat);
		sFormat = tchFormat;
		std::string sShifting;
		CW_STRING_TO_CHARARRAY(hShifting);
		sShifting = tchShifting;
		std::string cppResult = CGRuntime::addToDate(sDate, sFormat, sShifting);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hShifting);
		CW_FREE_CHARARRAY(hFormat);
		CW_FREE_CHARARRAY(hDate);
		return result;
	}

		System::String* Runtime::byteToChar(System::String* hByte) {
		System::String* result;
		std::string sByte;
		CW_STRING_TO_CHARARRAY(hByte);
		sByte = tchByte;
		std::string cppResult = CGRuntime::byteToChar(sByte);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hByte);
		return result;
	}

		unsigned long Runtime::bytesToLong(System::String* hBytes) {
		unsigned long result;
		std::string sBytes;
		CW_STRING_TO_CHARARRAY(hBytes);
		sBytes = tchBytes;
		unsigned long cppResult = CGRuntime::bytesToLong(sBytes);
		result = cppResult;
		CW_FREE_CHARARRAY(hBytes);
		return result;
	}

		unsigned short Runtime::bytesToShort(System::String* hBytes) {
		unsigned short result;
		std::string sBytes;
		CW_STRING_TO_CHARARRAY(hBytes);
		sBytes = tchBytes;
		unsigned short cppResult = CGRuntime::bytesToShort(sBytes);
		result = cppResult;
		CW_FREE_CHARARRAY(hBytes);
		return result;
	}

		System::String* Runtime::canonizePath(System::String* hPath) {
		System::String* result;
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		std::string cppResult = CGRuntime::canonizePath(sPath);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hPath);
		return result;
	}

		bool Runtime::changeDirectory(System::String* hPath) {
		bool result;
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		bool cppResult = CGRuntime::changeDirectory(sPath);
		result = cppResult;
		CW_FREE_CHARARRAY(hPath);
		return result;
	}

		int Runtime::changeFileTime(System::String* hFilename, System::String* hAccessTime, System::String* hModificationTime) {
		int result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string sAccessTime;
		CW_STRING_TO_CHARARRAY(hAccessTime);
		sAccessTime = tchAccessTime;
		std::string sModificationTime;
		CW_STRING_TO_CHARARRAY(hModificationTime);
		sModificationTime = tchModificationTime;
		int cppResult = CGRuntime::changeFileTime(sFilename, sAccessTime, sModificationTime);
		result = cppResult;
		CW_FREE_CHARARRAY(hModificationTime);
		CW_FREE_CHARARRAY(hAccessTime);
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		System::String* Runtime::charAt(System::String* hText, int hIndex) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iIndex;
		iIndex = hIndex;
		std::string cppResult = CGRuntime::charAt(sText, iIndex);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::charToByte(System::String* hChar) {
		System::String* result;
		std::string sChar;
		CW_STRING_TO_CHARARRAY(hChar);
		sChar = tchChar;
		std::string cppResult = CGRuntime::charToByte(sChar);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hChar);
		return result;
	}

		int Runtime::charToInt(System::String* hChar) {
		int result;
		std::string sChar;
		CW_STRING_TO_CHARARRAY(hChar);
		sChar = tchChar;
		int cppResult = CGRuntime::charToInt(sChar);
		result = cppResult;
		CW_FREE_CHARARRAY(hChar);
		return result;
	}

		bool Runtime::chmod(System::String* hFilename, System::String* hMode) {
		bool result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string sMode;
		CW_STRING_TO_CHARARRAY(hMode);
		sMode = tchMode;
		bool cppResult = CGRuntime::chmod(sFilename, sMode);
		result = cppResult;
		CW_FREE_CHARARRAY(hMode);
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		int Runtime::ceil(double hNumber) {
		int result;
		double dNumber;
		dNumber = hNumber;
		int cppResult = CGRuntime::ceil(dNumber);
		result = cppResult;
		return result;
	}

		int Runtime::compareDate(System::String* hDate1, System::String* hDate2) {
		int result;
		std::string sDate1;
		CW_STRING_TO_CHARARRAY(hDate1);
		sDate1 = tchDate1;
		std::string sDate2;
		CW_STRING_TO_CHARARRAY(hDate2);
		sDate2 = tchDate2;
		int cppResult = CGRuntime::compareDate(sDate1, sDate2);
		result = cppResult;
		CW_FREE_CHARARRAY(hDate2);
		CW_FREE_CHARARRAY(hDate1);
		return result;
	}

		System::String* Runtime::completeDate(System::String* hDate, System::String* hFormat) {
		System::String* result;
		std::string sDate;
		CW_STRING_TO_CHARARRAY(hDate);
		sDate = tchDate;
		std::string sFormat;
		CW_STRING_TO_CHARARRAY(hFormat);
		sFormat = tchFormat;
		std::string cppResult = CGRuntime::completeDate(sDate, sFormat);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFormat);
		CW_FREE_CHARARRAY(hDate);
		return result;
	}

		System::String* Runtime::completeLeftSpaces(System::String* hText, int hLength) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::completeLeftSpaces(sText, iLength);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::completeRightSpaces(System::String* hText, int hLength) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::completeRightSpaces(sText, iLength);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::composeAdaLikeString(System::String* hText) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::composeAdaLikeString(sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::composeCLikeString(System::String* hText) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::composeCLikeString(sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::composeHTMLLikeString(System::String* hText) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::composeHTMLLikeString(sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::composeSQLLikeString(System::String* hText) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::composeSQLLikeString(sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::computeMD5(System::String* hText) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::computeMD5(sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		bool Runtime::copySmartFile(System::String* hSourceFileName, System::String* hDestinationFileName) {
		bool result;
		std::string sSourceFileName;
		CW_STRING_TO_CHARARRAY(hSourceFileName);
		sSourceFileName = tchSourceFileName;
		std::string sDestinationFileName;
		CW_STRING_TO_CHARARRAY(hDestinationFileName);
		sDestinationFileName = tchDestinationFileName;
		bool cppResult = CGRuntime::copySmartFile(sSourceFileName, sDestinationFileName);
		result = cppResult;
		CW_FREE_CHARARRAY(hDestinationFileName);
		CW_FREE_CHARARRAY(hSourceFileName);
		return result;
	}

		System::String* Runtime::coreString(System::String* hText, int hPos, int hLastRemoved) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iPos;
		iPos = hPos;
		int iLastRemoved;
		iLastRemoved = hLastRemoved;
		std::string cppResult = CGRuntime::coreString(sText, iPos, iLastRemoved);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		int Runtime::countStringOccurences(System::String* hString, System::String* hText) {
		int result;
		std::string sString;
		CW_STRING_TO_CHARARRAY(hString);
		sString = tchString;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int cppResult = CGRuntime::countStringOccurences(sString, sText);
		result = cppResult;
		CW_FREE_CHARARRAY(hText);
		CW_FREE_CHARARRAY(hString);
		return result;
	}

		bool Runtime::createDirectory(System::String* hPath) {
		bool result;
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		bool cppResult = CGRuntime::createDirectory(sPath);
		result = cppResult;
		CW_FREE_CHARARRAY(hPath);
		return result;
	}

		int Runtime::createINETClientSocket(System::String* hRemoteAddress, int hPort) {
		int result;
		std::string sRemoteAddress;
		CW_STRING_TO_CHARARRAY(hRemoteAddress);
		sRemoteAddress = tchRemoteAddress;
		int iPort;
		iPort = hPort;
		int cppResult = CGRuntime::createINETClientSocket(sRemoteAddress, iPort);
		result = cppResult;
		CW_FREE_CHARARRAY(hRemoteAddress);
		return result;
	}

		int Runtime::createINETServerSocket(int hPort, int hBackLog) {
		int result;
		int iPort;
		iPort = hPort;
		int iBackLog;
		iBackLog = hBackLog;
		int cppResult = CGRuntime::createINETServerSocket(iPort, iBackLog);
		result = cppResult;
		return result;
	}

		bool Runtime::createIterator(ParseTree* hI, ParseTree* hList) {
		bool result;
		CodeWorker::DtaScriptVariable* pI;
		pI = ((hI == NULL) ? NULL : hI->cppInstance_);
		CodeWorker::DtaScriptVariable* pList;
		pList = ((hList == NULL) ? NULL : hList->cppInstance_);
		bool cppResult = CGRuntime::createIterator(pI, pList);
		result = cppResult;
		return result;
	}

		bool Runtime::createReverseIterator(ParseTree* hI, ParseTree* hList) {
		bool result;
		CodeWorker::DtaScriptVariable* pI;
		pI = ((hI == NULL) ? NULL : hI->cppInstance_);
		CodeWorker::DtaScriptVariable* pList;
		pList = ((hList == NULL) ? NULL : hList->cppInstance_);
		bool cppResult = CGRuntime::createReverseIterator(pI, pList);
		result = cppResult;
		return result;
	}

		bool Runtime::createVirtualFile(System::String* hHandle, System::String* hContent) {
		bool result;
		std::string sHandle;
		CW_STRING_TO_CHARARRAY(hHandle);
		sHandle = tchHandle;
		std::string sContent;
		CW_STRING_TO_CHARARRAY(hContent);
		sContent = tchContent;
		bool cppResult = CGRuntime::createVirtualFile(sHandle, sContent);
		result = cppResult;
		CW_FREE_CHARARRAY(hContent);
		CW_FREE_CHARARRAY(hHandle);
		return result;
	}

		System::String* Runtime::createVirtualTemporaryFile(System::String* hContent) {
		System::String* result;
		std::string sContent;
		CW_STRING_TO_CHARARRAY(hContent);
		sContent = tchContent;
		std::string cppResult = CGRuntime::createVirtualTemporaryFile(sContent);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hContent);
		return result;
	}

		System::String* Runtime::decodeURL(System::String* hURL) {
		System::String* result;
		std::string sURL;
		CW_STRING_TO_CHARARRAY(hURL);
		sURL = tchURL;
		std::string cppResult = CGRuntime::decodeURL(sURL);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hURL);
		return result;
	}

		double Runtime::decrement(DoubleRef* hNumber) {
		double result;
		double dNumber;
		// NOT HANDLED YET!
		double cppResult = CGRuntime::decrement(dNumber);
		result = cppResult;
		return result;
	}

		bool Runtime::deleteFile(System::String* hFilename) {
		bool result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		bool cppResult = CGRuntime::deleteFile(sFilename);
		result = cppResult;
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		bool Runtime::deleteVirtualFile(System::String* hHandle) {
		bool result;
		std::string sHandle;
		CW_STRING_TO_CHARARRAY(hHandle);
		sHandle = tchHandle;
		bool cppResult = CGRuntime::deleteVirtualFile(sHandle);
		result = cppResult;
		CW_FREE_CHARARRAY(hHandle);
		return result;
	}

		double Runtime::div(double hDividend, double hDivisor) {
		double result;
		double dDividend;
		dDividend = hDividend;
		double dDivisor;
		dDivisor = hDivisor;
		double cppResult = CGRuntime::div(dDividend, dDivisor);
		result = cppResult;
		return result;
	}

		bool Runtime::duplicateIterator(ParseTree* hOldIt, ParseTree* hNewIt) {
		bool result;
		CodeWorker::DtaScriptVariable* pOldIt;
		pOldIt = ((hOldIt == NULL) ? NULL : hOldIt->cppInstance_);
		CodeWorker::DtaScriptVariable* pNewIt;
		pNewIt = ((hNewIt == NULL) ? NULL : hNewIt->cppInstance_);
		bool cppResult = CGRuntime::duplicateIterator(pOldIt, pNewIt);
		result = cppResult;
		return result;
	}

		System::String* Runtime::encodeURL(System::String* hURL) {
		System::String* result;
		std::string sURL;
		CW_STRING_TO_CHARARRAY(hURL);
		sURL = tchURL;
		std::string cppResult = CGRuntime::encodeURL(sURL);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hURL);
		return result;
	}

		System::String* Runtime::endl() {
		System::String* result;
		std::string cppResult = CGRuntime::endl();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		bool Runtime::endString(System::String* hText, System::String* hEnd) {
		bool result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string sEnd;
		CW_STRING_TO_CHARARRAY(hEnd);
		sEnd = tchEnd;
		bool cppResult = CGRuntime::endString(sText, sEnd);
		result = cppResult;
		CW_FREE_CHARARRAY(hEnd);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		bool Runtime::equal(double hLeft, double hRight) {
		bool result;
		double dLeft;
		dLeft = hLeft;
		double dRight;
		dRight = hRight;
		bool cppResult = CGRuntime::equal(dLeft, dRight);
		result = cppResult;
		return result;
	}

		bool Runtime::equalsIgnoreCase(System::String* hLeft, System::String* hRight) {
		bool result;
		std::string sLeft;
		CW_STRING_TO_CHARARRAY(hLeft);
		sLeft = tchLeft;
		std::string sRight;
		CW_STRING_TO_CHARARRAY(hRight);
		sRight = tchRight;
		bool cppResult = CGRuntime::equalsIgnoreCase(sLeft, sRight);
		result = cppResult;
		CW_FREE_CHARARRAY(hRight);
		CW_FREE_CHARARRAY(hLeft);
		return result;
	}

		bool Runtime::equalTrees(ParseTree* hFirstTree, ParseTree* hSecondTree) {
		bool result;
		CodeWorker::DtaScriptVariable* pFirstTree;
		pFirstTree = ((hFirstTree == NULL) ? NULL : hFirstTree->cppInstance_);
		CodeWorker::DtaScriptVariable* pSecondTree;
		pSecondTree = ((hSecondTree == NULL) ? NULL : hSecondTree->cppInstance_);
		bool cppResult = CGRuntime::equalTrees(pFirstTree, pSecondTree);
		result = cppResult;
		return result;
	}

		System::String* Runtime::executeStringQuiet(ParseTree* hThis, System::String* hCommand) {
		System::String* result;
		CodeWorker::DtaScriptVariable* pThis;
		pThis = ((hThis == NULL) ? NULL : hThis->cppInstance_);
		std::string sCommand;
		CW_STRING_TO_CHARARRAY(hCommand);
		sCommand = tchCommand;
		std::string cppResult = CGRuntime::executeStringQuiet(pThis, sCommand);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hCommand);
		return result;
	}

		bool Runtime::existDirectory(System::String* hPath) {
		bool result;
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		bool cppResult = CGRuntime::existDirectory(sPath);
		result = cppResult;
		CW_FREE_CHARARRAY(hPath);
		return result;
	}

		bool Runtime::existEnv(System::String* hVariable) {
		bool result;
		std::string sVariable;
		CW_STRING_TO_CHARARRAY(hVariable);
		sVariable = tchVariable;
		bool cppResult = CGRuntime::existEnv(sVariable);
		result = cppResult;
		CW_FREE_CHARARRAY(hVariable);
		return result;
	}

		bool Runtime::existFile(System::String* hFileName) {
		bool result;
		std::string sFileName;
		CW_STRING_TO_CHARARRAY(hFileName);
		sFileName = tchFileName;
		bool cppResult = CGRuntime::existFile(sFileName);
		result = cppResult;
		CW_FREE_CHARARRAY(hFileName);
		return result;
	}

		bool Runtime::existVirtualFile(System::String* hHandle) {
		bool result;
		std::string sHandle;
		CW_STRING_TO_CHARARRAY(hHandle);
		sHandle = tchHandle;
		bool cppResult = CGRuntime::existVirtualFile(sHandle);
		result = cppResult;
		CW_FREE_CHARARRAY(hHandle);
		return result;
	}

		bool Runtime::existVariable(ParseTree* hVariable) {
		bool result;
		CodeWorker::DtaScriptVariable* pVariable;
		pVariable = ((hVariable == NULL) ? NULL : hVariable->cppInstance_);
		bool cppResult = CGRuntime::existVariable(pVariable);
		result = cppResult;
		return result;
	}

		double Runtime::exp(double hX) {
		double result;
		double dX;
		dX = hX;
		double cppResult = CGRuntime::exp(dX);
		result = cppResult;
		return result;
	}

		bool Runtime::exploreDirectory(ParseTree* hDirectory, System::String* hPath, bool hSubfolders) {
		bool result;
		CodeWorker::DtaScriptVariable* pDirectory;
		pDirectory = ((hDirectory == NULL) ? NULL : hDirectory->cppInstance_);
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		bool bSubfolders;
		bSubfolders = hSubfolders;
		bool cppResult = CGRuntime::exploreDirectory(pDirectory, sPath, bSubfolders);
		result = cppResult;
		CW_FREE_CHARARRAY(hPath);
		return result;
	}

		System::String* Runtime::extractGenerationHeader(System::String* hFilename, System::Text::StringBuilder* hGenerator, System::Text::StringBuilder* hVersion, System::Text::StringBuilder* hDate) {
		System::String* result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string sGenerator;
		// NOT HANDLED YET!
		std::string sVersion;
		// NOT HANDLED YET!
		std::string sDate;
		// NOT HANDLED YET!
		std::string cppResult = CGRuntime::extractGenerationHeader(sFilename, sGenerator, sVersion, sDate);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		System::String* Runtime::fileCreation(System::String* hFilename) {
		System::String* result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string cppResult = CGRuntime::fileCreation(sFilename);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		System::String* Runtime::fileLastAccess(System::String* hFilename) {
		System::String* result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string cppResult = CGRuntime::fileLastAccess(sFilename);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		System::String* Runtime::fileLastModification(System::String* hFilename) {
		System::String* result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string cppResult = CGRuntime::fileLastModification(sFilename);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		int Runtime::fileLines(System::String* hFilename) {
		int result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		int cppResult = CGRuntime::fileLines(sFilename);
		result = cppResult;
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		System::String* Runtime::fileMode(System::String* hFilename) {
		System::String* result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string cppResult = CGRuntime::fileMode(sFilename);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		int Runtime::fileSize(System::String* hFilename) {
		int result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		int cppResult = CGRuntime::fileSize(sFilename);
		result = cppResult;
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		bool Runtime::findElement(System::String* hValue, ParseTree* hVariable) {
		bool result;
		std::string sValue;
		CW_STRING_TO_CHARARRAY(hValue);
		sValue = tchValue;
		CodeWorker::DtaScriptVariable* pVariable;
		pVariable = ((hVariable == NULL) ? NULL : hVariable->cppInstance_);
		bool cppResult = CGRuntime::findElement(sValue, pVariable);
		result = cppResult;
		CW_FREE_CHARARRAY(hValue);
		return result;
	}

		int Runtime::findFirstChar(System::String* hText, System::String* hSomeChars) {
		int result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string sSomeChars;
		CW_STRING_TO_CHARARRAY(hSomeChars);
		sSomeChars = tchSomeChars;
		int cppResult = CGRuntime::findFirstChar(sText, sSomeChars);
		result = cppResult;
		CW_FREE_CHARARRAY(hSomeChars);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		int Runtime::findFirstSubstringIntoKeys(System::String* hSubstring, ParseTree* hArray) {
		int result;
		std::string sSubstring;
		CW_STRING_TO_CHARARRAY(hSubstring);
		sSubstring = tchSubstring;
		CodeWorker::DtaScriptVariable* pArray;
		pArray = ((hArray == NULL) ? NULL : hArray->cppInstance_);
		int cppResult = CGRuntime::findFirstSubstringIntoKeys(sSubstring, pArray);
		result = cppResult;
		CW_FREE_CHARARRAY(hSubstring);
		return result;
	}

		int Runtime::findLastString(System::String* hText, System::String* hFind) {
		int result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string sFind;
		CW_STRING_TO_CHARARRAY(hFind);
		sFind = tchFind;
		int cppResult = CGRuntime::findLastString(sText, sFind);
		result = cppResult;
		CW_FREE_CHARARRAY(hFind);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		int Runtime::findNextString(System::String* hText, System::String* hFind, int hPosition) {
		int result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string sFind;
		CW_STRING_TO_CHARARRAY(hFind);
		sFind = tchFind;
		int iPosition;
		iPosition = hPosition;
		int cppResult = CGRuntime::findNextString(sText, sFind, iPosition);
		result = cppResult;
		CW_FREE_CHARARRAY(hFind);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		int Runtime::findNextSubstringIntoKeys(System::String* hSubstring, ParseTree* hArray, int hNext) {
		int result;
		std::string sSubstring;
		CW_STRING_TO_CHARARRAY(hSubstring);
		sSubstring = tchSubstring;
		CodeWorker::DtaScriptVariable* pArray;
		pArray = ((hArray == NULL) ? NULL : hArray->cppInstance_);
		int iNext;
		iNext = hNext;
		int cppResult = CGRuntime::findNextSubstringIntoKeys(sSubstring, pArray, iNext);
		result = cppResult;
		CW_FREE_CHARARRAY(hSubstring);
		return result;
	}

		int Runtime::findString(System::String* hText, System::String* hFind) {
		int result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string sFind;
		CW_STRING_TO_CHARARRAY(hFind);
		sFind = tchFind;
		int cppResult = CGRuntime::findString(sText, sFind);
		result = cppResult;
		CW_FREE_CHARARRAY(hFind);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		int Runtime::floor(double hNumber) {
		int result;
		double dNumber;
		dNumber = hNumber;
		int cppResult = CGRuntime::floor(dNumber);
		result = cppResult;
		return result;
	}

		System::String* Runtime::formatDate(System::String* hDate, System::String* hFormat) {
		System::String* result;
		std::string sDate;
		CW_STRING_TO_CHARARRAY(hDate);
		sDate = tchDate;
		std::string sFormat;
		CW_STRING_TO_CHARARRAY(hFormat);
		sFormat = tchFormat;
		std::string cppResult = CGRuntime::formatDate(sDate, sFormat);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFormat);
		CW_FREE_CHARARRAY(hDate);
		return result;
	}

		int Runtime::getArraySize(ParseTree* hVariable) {
		int result;
		CodeWorker::DtaScriptVariable* pVariable;
		pVariable = ((hVariable == NULL) ? NULL : hVariable->cppInstance_);
		int cppResult = CGRuntime::getArraySize(pVariable);
		result = cppResult;
		return result;
	}

		System::String* Runtime::getCommentBegin() {
		System::String* result;
		std::string cppResult = CGRuntime::getCommentBegin();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getCommentEnd() {
		System::String* result;
		std::string cppResult = CGRuntime::getCommentEnd();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getCurrentDirectory() {
		System::String* result;
		std::string cppResult = CGRuntime::getCurrentDirectory();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getEnv(System::String* hVariable) {
		System::String* result;
		std::string sVariable;
		CW_STRING_TO_CHARARRAY(hVariable);
		sVariable = tchVariable;
		std::string cppResult = CGRuntime::getEnv(sVariable);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hVariable);
		return result;
	}

		System::String* Runtime::getGenerationHeader() {
		System::String* result;
		std::string cppResult = CGRuntime::getGenerationHeader();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getHTTPRequest(System::String* hURL, ParseTree* hHTTPSession, ParseTree* hArguments) {
		System::String* result;
		std::string sURL;
		CW_STRING_TO_CHARARRAY(hURL);
		sURL = tchURL;
		CodeWorker::DtaScriptVariable* pHTTPSession;
		pHTTPSession = ((hHTTPSession == NULL) ? NULL : hHTTPSession->cppInstance_);
		CodeWorker::DtaScriptVariable* pArguments;
		pArguments = ((hArguments == NULL) ? NULL : hArguments->cppInstance_);
		std::string cppResult = CGRuntime::getHTTPRequest(sURL, pHTTPSession, pArguments);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hURL);
		return result;
	}

		System::String* Runtime::getIncludePath() {
		System::String* result;
		std::string cppResult = CGRuntime::getIncludePath();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		double Runtime::getLastDelay() {
		double result;
		double cppResult = CGRuntime::getLastDelay();
		result = cppResult;
		return result;
	}

		System::String* Runtime::getNow() {
		System::String* result;
		std::string cppResult = CGRuntime::getNow();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getProperty(System::String* hDefine) {
		System::String* result;
		std::string sDefine;
		CW_STRING_TO_CHARARRAY(hDefine);
		sDefine = tchDefine;
		std::string cppResult = CGRuntime::getProperty(sDefine);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hDefine);
		return result;
	}

		System::String* Runtime::getShortFilename(System::String* hPathFilename) {
		System::String* result;
		std::string sPathFilename;
		CW_STRING_TO_CHARARRAY(hPathFilename);
		sPathFilename = tchPathFilename;
		std::string cppResult = CGRuntime::getShortFilename(sPathFilename);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hPathFilename);
		return result;
	}

		System::String* Runtime::getTextMode() {
		System::String* result;
		std::string cppResult = CGRuntime::getTextMode();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		int Runtime::getVariableAttributes(ParseTree* hVariable, ParseTree* hList) {
		int result;
		CodeWorker::DtaScriptVariable* pVariable;
		pVariable = ((hVariable == NULL) ? NULL : hVariable->cppInstance_);
		CodeWorker::DtaScriptVariable* pList;
		pList = ((hList == NULL) ? NULL : hList->cppInstance_);
		int cppResult = CGRuntime::getVariableAttributes(pVariable, pList);
		result = cppResult;
		return result;
	}

		System::String* Runtime::getVersion() {
		System::String* result;
		std::string cppResult = CGRuntime::getVersion();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getWorkingPath() {
		System::String* result;
		std::string cppResult = CGRuntime::getWorkingPath();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getWriteMode() {
		System::String* result;
		std::string cppResult = CGRuntime::getWriteMode();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		int Runtime::hexaToDecimal(System::String* hHexaNumber) {
		int result;
		std::string sHexaNumber;
		CW_STRING_TO_CHARARRAY(hHexaNumber);
		sHexaNumber = tchHexaNumber;
		int cppResult = CGRuntime::hexaToDecimal(sHexaNumber);
		result = cppResult;
		CW_FREE_CHARARRAY(hHexaNumber);
		return result;
	}

		System::String* Runtime::hostToNetworkLong(System::String* hBytes) {
		System::String* result;
		std::string sBytes;
		CW_STRING_TO_CHARARRAY(hBytes);
		sBytes = tchBytes;
		std::string cppResult = CGRuntime::hostToNetworkLong(sBytes);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hBytes);
		return result;
	}

		System::String* Runtime::hostToNetworkShort(System::String* hBytes) {
		System::String* result;
		std::string sBytes;
		CW_STRING_TO_CHARARRAY(hBytes);
		sBytes = tchBytes;
		std::string cppResult = CGRuntime::hostToNetworkShort(sBytes);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hBytes);
		return result;
	}

		double Runtime::increment(DoubleRef* hNumber) {
		double result;
		double dNumber;
		// NOT HANDLED YET!
		double cppResult = CGRuntime::increment(dNumber);
		result = cppResult;
		return result;
	}

		bool Runtime::indentFile(System::String* hFile, System::String* hMode) {
		bool result;
		std::string sFile;
		CW_STRING_TO_CHARARRAY(hFile);
		sFile = tchFile;
		std::string sMode;
		CW_STRING_TO_CHARARRAY(hMode);
		sMode = tchMode;
		bool cppResult = CGRuntime::indentFile(sFile, sMode);
		result = cppResult;
		CW_FREE_CHARARRAY(hMode);
		CW_FREE_CHARARRAY(hFile);
		return result;
	}

		bool Runtime::inf(double hLeft, double hRight) {
		bool result;
		double dLeft;
		dLeft = hLeft;
		double dRight;
		dRight = hRight;
		bool cppResult = CGRuntime::inf(dLeft, dRight);
		result = cppResult;
		return result;
	}

		System::String* Runtime::inputKey(bool hEcho) {
		System::String* result;
		bool bEcho;
		bEcho = hEcho;
		std::string cppResult = CGRuntime::inputKey(bEcho);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::inputLine(bool hEcho, System::String* hPrompt) {
		System::String* result;
		bool bEcho;
		bEcho = hEcho;
		std::string sPrompt;
		CW_STRING_TO_CHARARRAY(hPrompt);
		sPrompt = tchPrompt;
		std::string cppResult = CGRuntime::inputLine(bEcho, sPrompt);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hPrompt);
		return result;
	}

		bool Runtime::isEmpty(ParseTree* hArray) {
		bool result;
		CodeWorker::DtaScriptVariable* pArray;
		pArray = ((hArray == NULL) ? NULL : hArray->cppInstance_);
		bool cppResult = CGRuntime::isEmpty(pArray);
		result = cppResult;
		return result;
	}

		bool Runtime::isIdentifier(System::String* hIdentifier) {
		bool result;
		std::string sIdentifier;
		CW_STRING_TO_CHARARRAY(hIdentifier);
		sIdentifier = tchIdentifier;
		bool cppResult = CGRuntime::isIdentifier(sIdentifier);
		result = cppResult;
		CW_FREE_CHARARRAY(hIdentifier);
		return result;
	}

		bool Runtime::isNegative(double hNumber) {
		bool result;
		double dNumber;
		dNumber = hNumber;
		bool cppResult = CGRuntime::isNegative(dNumber);
		result = cppResult;
		return result;
	}

		bool Runtime::isNumeric(System::String* hNumber) {
		bool result;
		std::string sNumber;
		CW_STRING_TO_CHARARRAY(hNumber);
		sNumber = tchNumber;
		bool cppResult = CGRuntime::isNumeric(sNumber);
		result = cppResult;
		CW_FREE_CHARARRAY(hNumber);
		return result;
	}

		bool Runtime::isPositive(double hNumber) {
		bool result;
		double dNumber;
		dNumber = hNumber;
		bool cppResult = CGRuntime::isPositive(dNumber);
		result = cppResult;
		return result;
	}

		System::String* Runtime::joinStrings(ParseTree* hList, System::String* hSeparator) {
		System::String* result;
		CodeWorker::DtaScriptVariable* pList;
		pList = ((hList == NULL) ? NULL : hList->cppInstance_);
		std::string sSeparator;
		CW_STRING_TO_CHARARRAY(hSeparator);
		sSeparator = tchSeparator;
		std::string cppResult = CGRuntime::joinStrings(pList, sSeparator);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hSeparator);
		return result;
	}

		System::String* Runtime::leftString(System::String* hText, int hLength) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::leftString(sText, iLength);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		int Runtime::lengthString(System::String* hText) {
		int result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int cppResult = CGRuntime::lengthString(sText);
		result = cppResult;
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::loadBinaryFile(System::String* hFile, int hLength) {
		System::String* result;
		std::string sFile;
		CW_STRING_TO_CHARARRAY(hFile);
		sFile = tchFile;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::loadBinaryFile(sFile, iLength);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFile);
		return result;
	}

		System::String* Runtime::loadFile(System::String* hFile, int hLength) {
		System::String* result;
		std::string sFile;
		CW_STRING_TO_CHARARRAY(hFile);
		sFile = tchFile;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::loadFile(sFile, iLength);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFile);
		return result;
	}

		System::String* Runtime::loadVirtualFile(System::String* hHandle) {
		System::String* result;
		std::string sHandle;
		CW_STRING_TO_CHARARRAY(hHandle);
		sHandle = tchHandle;
		std::string cppResult = CGRuntime::loadVirtualFile(sHandle);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hHandle);
		return result;
	}

		double Runtime::log(double hX) {
		double result;
		double dX;
		dX = hX;
		double cppResult = CGRuntime::log(dX);
		result = cppResult;
		return result;
	}

		System::String* Runtime::longToBytes(unsigned long hLong) {
		System::String* result;
		unsigned long ulLong;
		ulLong = hLong;
		std::string cppResult = CGRuntime::longToBytes(ulLong);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::midString(System::String* hText, int hPos, int hLength) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iPos;
		iPos = hPos;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::midString(sText, iPos, iLength);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		int Runtime::mod(int hDividend, int hDivisor) {
		int result;
		int iDividend;
		iDividend = hDividend;
		int iDivisor;
		iDivisor = hDivisor;
		int cppResult = CGRuntime::mod(iDividend, iDivisor);
		result = cppResult;
		return result;
	}

		double Runtime::mult(double hLeft, double hRight) {
		double result;
		double dLeft;
		dLeft = hLeft;
		double dRight;
		dRight = hRight;
		double cppResult = CGRuntime::mult(dLeft, dRight);
		result = cppResult;
		return result;
	}

		System::String* Runtime::networkLongToHost(System::String* hBytes) {
		System::String* result;
		std::string sBytes;
		CW_STRING_TO_CHARARRAY(hBytes);
		sBytes = tchBytes;
		std::string cppResult = CGRuntime::networkLongToHost(sBytes);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hBytes);
		return result;
	}

		System::String* Runtime::networkShortToHost(System::String* hBytes) {
		System::String* result;
		std::string sBytes;
		CW_STRING_TO_CHARARRAY(hBytes);
		sBytes = tchBytes;
		std::string cppResult = CGRuntime::networkShortToHost(sBytes);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hBytes);
		return result;
	}

		int Runtime::octalToDecimal(System::String* hOctalNumber) {
		int result;
		std::string sOctalNumber;
		CW_STRING_TO_CHARARRAY(hOctalNumber);
		sOctalNumber = tchOctalNumber;
		int cppResult = CGRuntime::octalToDecimal(sOctalNumber);
		result = cppResult;
		CW_FREE_CHARARRAY(hOctalNumber);
		return result;
	}

		System::String* Runtime::pathFromPackage(System::String* hPackage) {
		System::String* result;
		std::string sPackage;
		CW_STRING_TO_CHARARRAY(hPackage);
		sPackage = tchPackage;
		std::string cppResult = CGRuntime::pathFromPackage(sPackage);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hPackage);
		return result;
	}

		System::String* Runtime::postHTTPRequest(System::String* hURL, ParseTree* hHTTPSession, ParseTree* hArguments) {
		System::String* result;
		std::string sURL;
		CW_STRING_TO_CHARARRAY(hURL);
		sURL = tchURL;
		CodeWorker::DtaScriptVariable* pHTTPSession;
		pHTTPSession = ((hHTTPSession == NULL) ? NULL : hHTTPSession->cppInstance_);
		CodeWorker::DtaScriptVariable* pArguments;
		pArguments = ((hArguments == NULL) ? NULL : hArguments->cppInstance_);
		std::string cppResult = CGRuntime::postHTTPRequest(sURL, pHTTPSession, pArguments);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hURL);
		return result;
	}

		double Runtime::pow(double hX, double hY) {
		double result;
		double dX;
		dX = hX;
		double dY;
		dY = hY;
		double cppResult = CGRuntime::pow(dX, dY);
		result = cppResult;
		return result;
	}

		int Runtime::randomInteger() {
		int result;
		int cppResult = CGRuntime::randomInteger();
		result = cppResult;
		return result;
	}

		System::String* Runtime::receiveBinaryFromSocket(int hSocket, int hLength) {
		System::String* result;
		int iSocket;
		iSocket = hSocket;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::receiveBinaryFromSocket(iSocket, iLength);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::receiveFromSocket(int hSocket, BooleanRef* hIsText) {
		System::String* result;
		int iSocket;
		iSocket = hSocket;
		bool bIsText;
		// NOT HANDLED YET!
		std::string cppResult = CGRuntime::receiveFromSocket(iSocket, bIsText);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::receiveTextFromSocket(int hSocket, int hLength) {
		System::String* result;
		int iSocket;
		iSocket = hSocket;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::receiveTextFromSocket(iSocket, iLength);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::relativePath(System::String* hPath, System::String* hReference) {
		System::String* result;
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		std::string sReference;
		CW_STRING_TO_CHARARRAY(hReference);
		sReference = tchReference;
		std::string cppResult = CGRuntime::relativePath(sPath, sReference);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hReference);
		CW_FREE_CHARARRAY(hPath);
		return result;
	}

		bool Runtime::removeDirectory(System::String* hPath) {
		bool result;
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		bool cppResult = CGRuntime::removeDirectory(sPath);
		result = cppResult;
		CW_FREE_CHARARRAY(hPath);
		return result;
	}

		bool Runtime::removeGenerationTagsHandler(System::String* hKey) {
		bool result;
		std::string sKey;
		CW_STRING_TO_CHARARRAY(hKey);
		sKey = tchKey;
		bool cppResult = CGRuntime::removeGenerationTagsHandler(sKey);
		result = cppResult;
		CW_FREE_CHARARRAY(hKey);
		return result;
	}

		System::String* Runtime::repeatString(System::String* hText, int hOccurrences) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iOccurrences;
		iOccurrences = hOccurrences;
		std::string cppResult = CGRuntime::repeatString(sText, iOccurrences);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::replaceString(System::String* hOld, System::String* hNew, System::String* hText) {
		System::String* result;
		std::string sOld;
		CW_STRING_TO_CHARARRAY(hOld);
		sOld = tchOld;
		std::string sNew;
		CW_STRING_TO_CHARARRAY(hNew);
		sNew = tchNew;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::replaceString(sOld, sNew, sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		CW_FREE_CHARARRAY(hNew);
		CW_FREE_CHARARRAY(hOld);
		return result;
	}

		System::String* Runtime::replaceTabulations(System::String* hText, int hTab) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iTab;
		iTab = hTab;
		std::string cppResult = CGRuntime::replaceTabulations(sText, iTab);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::resolveFilePath(System::String* hFilename) {
		System::String* result;
		std::string sFilename;
		CW_STRING_TO_CHARARRAY(hFilename);
		sFilename = tchFilename;
		std::string cppResult = CGRuntime::resolveFilePath(sFilename);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hFilename);
		return result;
	}

		System::String* Runtime::rightString(System::String* hText, int hLength) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::rightString(sText, iLength);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::rsubString(System::String* hText, int hPos) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iPos;
		iPos = hPos;
		std::string cppResult = CGRuntime::rsubString(sText, iPos);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		bool Runtime::scanDirectories(ParseTree* hDirectory, System::String* hPath, System::String* hPattern) {
		bool result;
		CodeWorker::DtaScriptVariable* pDirectory;
		pDirectory = ((hDirectory == NULL) ? NULL : hDirectory->cppInstance_);
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		std::string sPattern;
		CW_STRING_TO_CHARARRAY(hPattern);
		sPattern = tchPattern;
		bool cppResult = CGRuntime::scanDirectories(pDirectory, sPath, sPattern);
		result = cppResult;
		CW_FREE_CHARARRAY(hPattern);
		CW_FREE_CHARARRAY(hPath);
		return result;
	}

		bool Runtime::scanFiles(ParseTree* hFiles, System::String* hPath, System::String* hPattern, bool hSubfolders) {
		bool result;
		CodeWorker::DtaScriptVariable* pFiles;
		pFiles = ((hFiles == NULL) ? NULL : hFiles->cppInstance_);
		std::string sPath;
		CW_STRING_TO_CHARARRAY(hPath);
		sPath = tchPath;
		std::string sPattern;
		CW_STRING_TO_CHARARRAY(hPattern);
		sPattern = tchPattern;
		bool bSubfolders;
		bSubfolders = hSubfolders;
		bool cppResult = CGRuntime::scanFiles(pFiles, sPath, sPattern, bSubfolders);
		result = cppResult;
		CW_FREE_CHARARRAY(hPattern);
		CW_FREE_CHARARRAY(hPath);
		return result;
	}

		bool Runtime::sendBinaryToSocket(int hSocket, System::String* hBytes) {
		bool result;
		int iSocket;
		iSocket = hSocket;
		std::string sBytes;
		CW_STRING_TO_CHARARRAY(hBytes);
		sBytes = tchBytes;
		bool cppResult = CGRuntime::sendBinaryToSocket(iSocket, sBytes);
		result = cppResult;
		CW_FREE_CHARARRAY(hBytes);
		return result;
	}

		System::String* Runtime::sendHTTPRequest(System::String* hURL, ParseTree* hHTTPSession) {
		System::String* result;
		std::string sURL;
		CW_STRING_TO_CHARARRAY(hURL);
		sURL = tchURL;
		CodeWorker::DtaScriptVariable* pHTTPSession;
		pHTTPSession = ((hHTTPSession == NULL) ? NULL : hHTTPSession->cppInstance_);
		std::string cppResult = CGRuntime::sendHTTPRequest(sURL, pHTTPSession);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hURL);
		return result;
	}

		bool Runtime::sendTextToSocket(int hSocket, System::String* hText) {
		bool result;
		int iSocket;
		iSocket = hSocket;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		bool cppResult = CGRuntime::sendTextToSocket(iSocket, sText);
		result = cppResult;
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		bool Runtime::selectGenerationTagsHandler(System::String* hKey) {
		bool result;
		std::string sKey;
		CW_STRING_TO_CHARARRAY(hKey);
		sKey = tchKey;
		bool cppResult = CGRuntime::selectGenerationTagsHandler(sKey);
		result = cppResult;
		CW_FREE_CHARARRAY(hKey);
		return result;
	}

		System::String* Runtime::shortToBytes(unsigned short hShort) {
		System::String* result;
		unsigned short ulShort;
		ulShort = hShort;
		std::string cppResult = CGRuntime::shortToBytes(ulShort);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		double Runtime::sqrt(double hX) {
		double result;
		double dX;
		dX = hX;
		double cppResult = CGRuntime::sqrt(dX);
		result = cppResult;
		return result;
	}

		bool Runtime::startString(System::String* hText, System::String* hStart) {
		bool result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string sStart;
		CW_STRING_TO_CHARARRAY(hStart);
		sStart = tchStart;
		bool cppResult = CGRuntime::startString(sText, sStart);
		result = cppResult;
		CW_FREE_CHARARRAY(hStart);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		double Runtime::sub(double hLeft, double hRight) {
		double result;
		double dLeft;
		dLeft = hLeft;
		double dRight;
		dRight = hRight;
		double cppResult = CGRuntime::sub(dLeft, dRight);
		result = cppResult;
		return result;
	}

		System::String* Runtime::subString(System::String* hText, int hPos) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		int iPos;
		iPos = hPos;
		std::string cppResult = CGRuntime::subString(sText, iPos);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		bool Runtime::sup(double hLeft, double hRight) {
		bool result;
		double dLeft;
		dLeft = hLeft;
		double dRight;
		dRight = hRight;
		bool cppResult = CGRuntime::sup(dLeft, dRight);
		result = cppResult;
		return result;
	}

		System::String* Runtime::system(System::String* hCommand) {
		System::String* result;
		std::string sCommand;
		CW_STRING_TO_CHARARRAY(hCommand);
		sCommand = tchCommand;
		std::string cppResult = CGRuntime::system(sCommand);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hCommand);
		return result;
	}

		System::String* Runtime::toLowerString(System::String* hText) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::toLowerString(sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::toUpperString(System::String* hText) {
		System::String* result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::toUpperString(sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		int Runtime::trimLeft(System::Text::StringBuilder* hString) {
		int result;
		std::string sString;
		// NOT HANDLED YET!
		int cppResult = CGRuntime::trimLeft(sString);
		result = cppResult;
		return result;
	}

		int Runtime::trimRight(System::Text::StringBuilder* hString) {
		int result;
		std::string sString;
		// NOT HANDLED YET!
		int cppResult = CGRuntime::trimRight(sString);
		result = cppResult;
		return result;
	}

		int Runtime::trim(System::Text::StringBuilder* hString) {
		int result;
		std::string sString;
		// NOT HANDLED YET!
		int cppResult = CGRuntime::trim(sString);
		result = cppResult;
		return result;
	}

		System::String* Runtime::truncateAfterString(ParseTree* hVariable, System::String* hText) {
		System::String* result;
		CodeWorker::DtaScriptVariable* pVariable;
		pVariable = ((hVariable == NULL) ? NULL : hVariable->cppInstance_);
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::truncateAfterString(pVariable, sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::truncateBeforeString(ParseTree* hVariable, System::String* hText) {
		System::String* result;
		CodeWorker::DtaScriptVariable* pVariable;
		pVariable = ((hVariable == NULL) ? NULL : hVariable->cppInstance_);
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		std::string cppResult = CGRuntime::truncateBeforeString(pVariable, sText);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::UUID() {
		System::String* result;
		std::string cppResult = CGRuntime::UUID();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		int Runtime::countInputCols() {
		int result;
		int cppResult = CGRuntime::countInputCols();
		result = cppResult;
		return result;
	}

		int Runtime::countInputLines() {
		int result;
		int cppResult = CGRuntime::countInputLines();
		result = cppResult;
		return result;
	}

		System::String* Runtime::getInputFilename() {
		System::String* result;
		std::string cppResult = CGRuntime::getInputFilename();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getLastReadChars(int hLength) {
		System::String* result;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::getLastReadChars(iLength);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		int Runtime::getInputLocation() {
		int result;
		int cppResult = CGRuntime::getInputLocation();
		result = cppResult;
		return result;
	}

		bool Runtime::lookAhead(System::String* hText) {
		bool result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		bool cppResult = CGRuntime::lookAhead(sText);
		result = cppResult;
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		System::String* Runtime::peekChar() {
		System::String* result;
		std::string cppResult = CGRuntime::peekChar();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		bool Runtime::readAdaString(System::Text::StringBuilder* hText) {
		bool result;
		std::string sText;
		// NOT HANDLED YET!
		bool cppResult = CGRuntime::readAdaString(sText);
		result = cppResult;
		return result;
	}

		System::String* Runtime::readByte() {
		System::String* result;
		std::string cppResult = CGRuntime::readByte();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::readBytes(int hLength) {
		System::String* result;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::readBytes(iLength);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::readCChar() {
		System::String* result;
		std::string cppResult = CGRuntime::readCChar();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::readChar() {
		System::String* result;
		std::string cppResult = CGRuntime::readChar();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		int Runtime::readCharAsInt() {
		int result;
		int cppResult = CGRuntime::readCharAsInt();
		result = cppResult;
		return result;
	}

		System::String* Runtime::readChars(int hLength) {
		System::String* result;
		int iLength;
		iLength = hLength;
		std::string cppResult = CGRuntime::readChars(iLength);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::readIdentifier() {
		System::String* result;
		std::string cppResult = CGRuntime::readIdentifier();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		bool Runtime::readIfEqualTo(System::String* hText) {
		bool result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		bool cppResult = CGRuntime::readIfEqualTo(sText);
		result = cppResult;
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		bool Runtime::readIfEqualToIgnoreCase(System::String* hText) {
		bool result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		bool cppResult = CGRuntime::readIfEqualToIgnoreCase(sText);
		result = cppResult;
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		bool Runtime::readIfEqualToIdentifier(System::String* hIdentifier) {
		bool result;
		std::string sIdentifier;
		CW_STRING_TO_CHARARRAY(hIdentifier);
		sIdentifier = tchIdentifier;
		bool cppResult = CGRuntime::readIfEqualToIdentifier(sIdentifier);
		result = cppResult;
		CW_FREE_CHARARRAY(hIdentifier);
		return result;
	}

		bool Runtime::readLine(System::Text::StringBuilder* hText) {
		bool result;
		std::string sText;
		// NOT HANDLED YET!
		bool cppResult = CGRuntime::readLine(sText);
		result = cppResult;
		return result;
	}

		bool Runtime::readNextText(System::String* hText) {
		bool result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		bool cppResult = CGRuntime::readNextText(sText);
		result = cppResult;
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		bool Runtime::readNumber(DoubleRef* hNumber) {
		bool result;
		double dNumber;
		// NOT HANDLED YET!
		bool cppResult = CGRuntime::readNumber(dNumber);
		result = cppResult;
		return result;
	}

		bool Runtime::readPythonString(System::Text::StringBuilder* hText) {
		bool result;
		std::string sText;
		// NOT HANDLED YET!
		bool cppResult = CGRuntime::readPythonString(sText);
		result = cppResult;
		return result;
	}

		bool Runtime::readString(System::Text::StringBuilder* hText) {
		bool result;
		std::string sText;
		// NOT HANDLED YET!
		bool cppResult = CGRuntime::readString(sText);
		result = cppResult;
		return result;
	}

		System::String* Runtime::readUptoJustOneChar(System::String* hOneAmongChars) {
		System::String* result;
		std::string sOneAmongChars;
		CW_STRING_TO_CHARARRAY(hOneAmongChars);
		sOneAmongChars = tchOneAmongChars;
		std::string cppResult = CGRuntime::readUptoJustOneChar(sOneAmongChars);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hOneAmongChars);
		return result;
	}

		System::String* Runtime::readWord() {
		System::String* result;
		std::string cppResult = CGRuntime::readWord();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		bool Runtime::skipBlanks() {
		bool result;
		bool cppResult = CGRuntime::skipBlanks();
		result = cppResult;
		return result;
	}

		bool Runtime::skipSpaces() {
		bool result;
		bool cppResult = CGRuntime::skipSpaces();
		result = cppResult;
		return result;
	}

		bool Runtime::skipEmptyCpp() {
		bool result;
		bool cppResult = CGRuntime::skipEmptyCpp();
		result = cppResult;
		return result;
	}

		bool Runtime::skipEmptyCppExceptDoxygen() {
		bool result;
		bool cppResult = CGRuntime::skipEmptyCppExceptDoxygen();
		result = cppResult;
		return result;
	}

		bool Runtime::skipEmptyHTML() {
		bool result;
		bool cppResult = CGRuntime::skipEmptyHTML();
		result = cppResult;
		return result;
	}

		bool Runtime::skipEmptyLaTeX() {
		bool result;
		bool cppResult = CGRuntime::skipEmptyLaTeX();
		result = cppResult;
		return result;
	}

		int Runtime::countOutputCols() {
		int result;
		int cppResult = CGRuntime::countOutputCols();
		result = cppResult;
		return result;
	}

		int Runtime::countOutputLines() {
		int result;
		int cppResult = CGRuntime::countOutputLines();
		result = cppResult;
		return result;
	}

		bool Runtime::decrementIndentLevel(int hLevel) {
		bool result;
		int iLevel;
		iLevel = hLevel;
		bool cppResult = CGRuntime::decrementIndentLevel(iLevel);
		result = cppResult;
		return result;
	}

		bool Runtime::equalLastWrittenChars(System::String* hText) {
		bool result;
		std::string sText;
		CW_STRING_TO_CHARARRAY(hText);
		sText = tchText;
		bool cppResult = CGRuntime::equalLastWrittenChars(sText);
		result = cppResult;
		CW_FREE_CHARARRAY(hText);
		return result;
	}

		bool Runtime::existFloatingLocation(System::String* hKey, bool hParent) {
		bool result;
		std::string sKey;
		CW_STRING_TO_CHARARRAY(hKey);
		sKey = tchKey;
		bool bParent;
		bParent = hParent;
		bool cppResult = CGRuntime::existFloatingLocation(sKey, bParent);
		result = cppResult;
		CW_FREE_CHARARRAY(hKey);
		return result;
	}

		int Runtime::getFloatingLocation(System::String* hKey) {
		int result;
		std::string sKey;
		CW_STRING_TO_CHARARRAY(hKey);
		sKey = tchKey;
		int cppResult = CGRuntime::getFloatingLocation(sKey);
		result = cppResult;
		CW_FREE_CHARARRAY(hKey);
		return result;
	}

		System::String* Runtime::getLastWrittenChars(int hNbChars) {
		System::String* result;
		int iNbChars;
		iNbChars = hNbChars;
		std::string cppResult = CGRuntime::getLastWrittenChars(iNbChars);
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getMarkupKey() {
		System::String* result;
		std::string cppResult = CGRuntime::getMarkupKey();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getMarkupValue() {
		System::String* result;
		std::string cppResult = CGRuntime::getMarkupValue();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		System::String* Runtime::getOutputFilename() {
		System::String* result;
		std::string cppResult = CGRuntime::getOutputFilename();
		result = CW_STL_TO_STRING(cppResult);
		return result;
	}

		int Runtime::getOutputLocation() {
		int result;
		int cppResult = CGRuntime::getOutputLocation();
		result = cppResult;
		return result;
	}

		System::String* Runtime::getProtectedArea(System::String* hProtection) {
		System::String* result;
		std::string sProtection;
		CW_STRING_TO_CHARARRAY(hProtection);
		sProtection = tchProtection;
		std::string cppResult = CGRuntime::getProtectedArea(sProtection);
		result = CW_STL_TO_STRING(cppResult);
		CW_FREE_CHARARRAY(hProtection);
		return result;
	}

		int Runtime::getProtectedAreaKeys(ParseTree* hKeys) {
		int result;
		CodeWorker::DtaScriptVariable* pKeys;
		pKeys = ((hKeys == NULL) ? NULL : hKeys->cppInstance_);
		int cppResult = CGRuntime::getProtectedAreaKeys(pKeys);
		result = cppResult;
		return result;
	}

		bool Runtime::indentText(System::String* hMode) {
		bool result;
		std::string sMode;
		CW_STRING_TO_CHARARRAY(hMode);
		sMode = tchMode;
		bool cppResult = CGRuntime::indentText(sMode);
		result = cppResult;
		CW_FREE_CHARARRAY(hMode);
		return result;
	}

		bool Runtime::newFloatingLocation(System::String* hKey) {
		bool result;
		std::string sKey;
		CW_STRING_TO_CHARARRAY(hKey);
		sKey = tchKey;
		bool cppResult = CGRuntime::newFloatingLocation(sKey);
		result = cppResult;
		CW_FREE_CHARARRAY(hKey);
		return result;
	}

		int Runtime::remainingProtectedAreas(ParseTree* hKeys) {
		int result;
		CodeWorker::DtaScriptVariable* pKeys;
		pKeys = ((hKeys == NULL) ? NULL : hKeys->cppInstance_);
		int cppResult = CGRuntime::remainingProtectedAreas(pKeys);
		result = cppResult;
		return result;
	}

		int Runtime::removeFloatingLocation(System::String* hKey) {
		int result;
		std::string sKey;
		CW_STRING_TO_CHARARRAY(hKey);
		sKey = tchKey;
		int cppResult = CGRuntime::removeFloatingLocation(sKey);
		result = cppResult;
		CW_FREE_CHARARRAY(hKey);
		return result;
	}

		bool Runtime::removeProtectedArea(System::String* hProtectedAreaName) {
		bool result;
		std::string sProtectedAreaName;
		CW_STRING_TO_CHARARRAY(hProtectedAreaName);
		sProtectedAreaName = tchProtectedAreaName;
		bool cppResult = CGRuntime::removeProtectedArea(sProtectedAreaName);
		result = cppResult;
		CW_FREE_CHARARRAY(hProtectedAreaName);
		return result;
	}

//##end##"C++ body"
}

namespace CodeWorker {
	public __gc class Main {
	public:
		static int initialize() {
			int iRetval = 0;
			try {
				__crt_dll_initialize();
			} catch(System::Exception* e) {
				System::Console::WriteLine(e);
				iRetval = 1;
			}
			return iRetval;
		}
		static int terminate() {
			int iRetval = 0;
			try {
				__crt_dll_terminate();
			} catch(System::Exception* e) {
				System::Console::WriteLine(e);
				iRetval = 1;
			}
			return iRetval;
		}
	};
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved) {
	return TRUE;
}
