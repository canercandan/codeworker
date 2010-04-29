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

#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include "UtlException.h"
#include "CGRuntime.h"
#include "ExprScriptExpression.h"
#include "ExprScriptVariable.h"
#include "DtaTranslateScript.h"
#include "DtaScriptVariable.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "DtaProject.h"
#include "GrfTranslate.h"

namespace CodeWorker {
	GrfTranslate::GrfTranslate() : _pClass(NULL), _pCachedScript(NULL), _pPatternFileName(NULL), _pInputFileName(NULL), _pOutputFileName(NULL) {
		_sCurrentDirectoryAtCompileTime = CGRuntime::getCurrentDirectory();
	}

	GrfTranslate::~GrfTranslate() {
		delete _pPatternFileName;
		delete _pClass;
		delete _pInputFileName;
		delete _pOutputFileName;
		if (_pCachedScript != NULL) delete _pCachedScript;
	}

	void GrfTranslate::setPatternFileName(ExprScriptScriptFile* pPatternFileName) {
		if (pPatternFileName->isFileName()) _pPatternFileName = pPatternFileName->getFileName();
		_pCachedScript = dynamic_cast<DtaTranslateScript*>(pPatternFileName->getBody());
		pPatternFileName->release();
	}

	SEQUENCE_INTERRUPTION_LIST GrfTranslate::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pClass = visibility.getExistingVariable(*_pClass);
		if (pClass == NULL) {
			throw UtlException("runtime error: variable '" + _pClass->toString() + "' doesn't exist while calling procedure '" + std::string(getFunctionName()) + "()'");
		}
		std::string sInputFile = _pInputFileName->getValue(visibility);
		std::string sOutputFile = _pOutputFileName->getValue(visibility);

		EXECUTE_FUNCTION* executeFunction = NULL;
		if (_pPatternFileName != NULL) {
			std::string sPatternFileName = _pPatternFileName->getValue(visibility);
			if ((_pCachedScript == NULL) || (_sCachedPatternFile != sPatternFileName) || ScpStream::existVirtualFile(sPatternFileName)) {
				executeFunction = DtaScript::getRegisteredScript(sPatternFileName.c_str());
				if (executeFunction == NULL) {
					if (_pCachedScript != NULL) delete _pCachedScript;
					_pCachedScript = new DtaTranslateScript(getParent());
					_sCachedPatternFile = sPatternFileName;
					_pCachedScript->parseFile(sPatternFileName.c_str(), _sCurrentDirectoryAtCompileTime);
				}
			}
		}

		SEQUENCE_INTERRUPTION_LIST result = executeScript(sInputFile, sOutputFile, pClass, executeFunction);
		switch(result) {
			case CONTINUE_INTERRUPTION:
			case BREAK_INTERRUPTION:
			case RETURN_INTERRUPTION:
				result = NO_INTERRUPTION;
				break;
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST GrfTranslate::executeScript(const std::string& sInputFile, const std::string& sOutputFile, DtaScriptVariable* pThisContext, EXECUTE_FUNCTION* executeFunction) {
		if (executeFunction != NULL) {
			CGRuntime::translate(executeFunction, CppParsingTree_var(pThisContext), sInputFile, sOutputFile);
			return NO_INTERRUPTION;
		}
		return _pCachedScript->translate(sInputFile, sOutputFile, *pThisContext);
	}

	std::string GrfTranslate::translateString(DtaScriptVariable& visibility) {
		DtaScriptVariable* pClass = visibility.getExistingVariable(*_pClass);
		if (pClass == NULL) {
			throw UtlException("runtime error: variable '" + _pClass->toString() + "' doesn't exist while calling procedure '" + std::string(getFunctionName()) + "()'");
		}
		std::string sInputString = _pInputFileName->getValue(visibility);

		EXECUTE_FUNCTION* executeFunction = NULL;
		if (_pPatternFileName != NULL) {
			std::string sPatternFileName = _pPatternFileName->getValue(visibility);
			if ((_pCachedScript == NULL) || (_sCachedPatternFile != sPatternFileName) || ScpStream::existVirtualFile(sPatternFileName)) {
				executeFunction = DtaScript::getRegisteredScript(sPatternFileName.c_str());
				if (executeFunction == NULL) {
					if (_pCachedScript != NULL) delete _pCachedScript;
					_pCachedScript = new DtaTranslateScript(getParent());
					_sCachedPatternFile = sPatternFileName;
					_pCachedScript->parseFile(sPatternFileName.c_str(), _sCurrentDirectoryAtCompileTime);
				}
			}
		}

		std::string sOutputString;
		_pCachedScript->translateString(sInputString, sOutputString, *pClass);
		return sOutputString;
	}

	void GrfTranslate::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		ExprScriptConstant* pConstantFileName = dynamic_cast<ExprScriptConstant*>(_pPatternFileName);
		std::string sScriptFilename;
		if (pConstantFileName == NULL) {
			sScriptFilename = theCompilerEnvironment.newInlineScriptFilename();
		} else {
			sScriptFilename = pConstantFileName->getConstant();
		}
		std::string sRadical = DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(CppCompilerEnvironment::filename2Module(sScriptFilename)));
		CW_BODY_INDENT << "CGRuntime::" << getFunctionName() << "(&Execute" << sRadical << "::instance(), ";
		_pClass->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pInputFileName->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pOutputFileName->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
		if (_pCachedScript == NULL) {
			_pCachedScript = new DtaTranslateScript(getParent());
			_sCachedPatternFile = sScriptFilename;
			_pCachedScript->parseFile(_sCachedPatternFile.c_str(), _sCurrentDirectoryAtCompileTime);
		}
		_pCachedScript->compileCpp(theCompilerEnvironment, sScriptFilename);
	}
}
