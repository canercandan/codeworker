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
#include "ExprScriptExpression.h"
#include "ExprScriptVariable.h"
#include "CGRuntime.h"
#include "DtaPatternScript.h"
#include "DtaScriptVariable.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "DtaProject.h"
#include "GrfGenerateString.h"

namespace CodeWorker {
	GrfGenerateString::~GrfGenerateString() {
		delete _pPatternFileName;
		delete _pClass;
		delete _pOutputString;
		if (_pCachedScript != NULL) delete _pCachedScript;
	}

	void GrfGenerateString::setPatternFileName(ExprScriptScriptFile* pPatternFileName) {
		if (pPatternFileName->isFileName()) _pPatternFileName = pPatternFileName->getFileName();
		_pCachedScript = dynamic_cast<DtaPatternScript*>(pPatternFileName->getBody());
		pPatternFileName->release();
	}

	SEQUENCE_INTERRUPTION_LIST GrfGenerateString::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pClass = visibility.getExistingVariable(*_pClass);
		if (pClass == NULL) {
			throw UtlException("runtime error: variable " + _pClass->toString() + " doesn't exist while calling procedure '" + std::string(getFunctionName()) + "()'");
		}
		DtaScriptVariable* pOutputString = visibility.getExistingVariable(*_pOutputString);
		if (pOutputString == NULL) {
			throw UtlException("runtime error: variable " + _pOutputString->toString() + " doesn't exist while calling procedure '" + std::string(getFunctionName()) + "()'");
		}

		EXECUTE_FUNCTION* executeFunction = NULL;
		if (_pPatternFileName != NULL) {
			std::string sPatternFileName = _pPatternFileName->getValue(visibility);
			if ((_pCachedScript == NULL) || (_sCachedPatternFile != sPatternFileName) || ScpStream::existVirtualFile(sPatternFileName)) {
				executeFunction = DtaScript::getRegisteredScript(sPatternFileName.c_str());
				if (executeFunction == NULL) {
					if (_pCachedScript != NULL) delete _pCachedScript;
					_pCachedScript = new DtaPatternScript(getParent());
					_sCachedPatternFile = sPatternFileName;
					_pCachedScript->parseFile(sPatternFileName.c_str());
				}
			}
		}

		return executeScript(pOutputString, pClass, executeFunction);
	}

	SEQUENCE_INTERRUPTION_LIST GrfGenerateString::executeScript(DtaScriptVariable* pOutputString, DtaScriptVariable* pThisContext, EXECUTE_FUNCTION* executeFunction) {
		if (executeFunction != NULL) {
			CGRuntime::generateString(executeFunction, CppParsingTree_var(pThisContext), CppParsingTree_var(pOutputString));
			return NO_INTERRUPTION;
		}
		std::string sOutputString;
		SEQUENCE_INTERRUPTION_LIST result = _pCachedScript->generateString(sOutputString, *pThisContext);
		if (result == NO_INTERRUPTION) pOutputString->setValue(sOutputString.c_str());
		return result;
	}

	void GrfGenerateString::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
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
		_pOutputString->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
		if (_pCachedScript == NULL) {
			_pCachedScript = new DtaPatternScript(getParent());
			_sCachedPatternFile = sScriptFilename;
			_pCachedScript->parseFile(_sCachedPatternFile.c_str());
		}
		_pCachedScript->compileCpp(theCompilerEnvironment, sScriptFilename);
	}
}
