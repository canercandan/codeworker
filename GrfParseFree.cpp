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
#include "DtaDesignScript.h"
#include "DtaScriptVariable.h"
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"
#include "DtaProject.h"
#include "GrfParseFree.h"

namespace CodeWorker {
	GrfParseFree::~GrfParseFree() {
		delete _pDesignFileName;
		delete _pClass;
		delete _pFileName;
	}

	void GrfParseFree::setDesignFileName(ExprScriptScriptFile* pDesignFileName) {
		if (pDesignFileName->isFileName()) _pDesignFileName = pDesignFileName->getFileName();
		_pCachedScript = dynamic_cast<DtaDesignScript*>(pDesignFileName->getBody());
		pDesignFileName->release();
	}

	SEQUENCE_INTERRUPTION_LIST GrfParseFree::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pClass = visibility.getVariable(*_pClass);
		if (pClass == NULL) throw UtlException("runtime error: variable '" + _pClass->toString() + "' doesn't exist while calling procedure 'parseFree()'");
		std::string sDesignFileName = _pDesignFileName->getValue(visibility);
		std::string sInputFile = _pFileName->getValue(visibility);

		EXECUTE_FUNCTION* executeFunction = NULL;
		if (_pDesignFileName != NULL) {
			std::string sDesignFileName = _pDesignFileName->getValue(visibility);
			if ((_pCachedScript == NULL) || (_sCachedDesignFile != sDesignFileName) || ScpStream::existVirtualFile(sDesignFileName)) {
				executeFunction = DtaScript::getRegisteredScript(sDesignFileName.c_str());
				if (executeFunction == NULL) {
					if (_pCachedScript != NULL) delete _pCachedScript;
					_pCachedScript = new DtaDesignScript(getParent());
					_sCachedDesignFile = sDesignFileName;
					_pCachedScript->parseFile(sDesignFileName.c_str());
				}
			}
		}

		SEQUENCE_INTERRUPTION_LIST result = executeScript(sInputFile.c_str(), pClass, executeFunction);
		switch(result) {
			case CONTINUE_INTERRUPTION:
			case BREAK_INTERRUPTION:
			case RETURN_INTERRUPTION:
				result = NO_INTERRUPTION;
				break;
		}
		return result;
	}

	SEQUENCE_INTERRUPTION_LIST GrfParseFree::executeScript(const char* sInputFile, DtaScriptVariable* pThisContext, EXECUTE_FUNCTION* executeFunction) {
		if (executeFunction != NULL) {
			CGRuntime::parseFree(executeFunction, CppParsingTree_var(pThisContext), sInputFile);
			return NO_INTERRUPTION;
		}
		return _pCachedScript->loadDesign(sInputFile, *pThisContext);
	}

	void GrfParseFree::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		ExprScriptConstant* pConstantFileName = dynamic_cast<ExprScriptConstant*>(_pDesignFileName);
		std::string sScriptFilename;
		if (pConstantFileName == NULL) {
			sScriptFilename = theCompilerEnvironment.newInlineScriptFilename();
		} else {
			sScriptFilename = pConstantFileName->getConstant();
		}
		std::string sRadical = DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(CppCompilerEnvironment::filename2Module(sScriptFilename)));
		CW_BODY_INDENT << "CGRuntime::parseFree(&Execute" << sRadical << "::instance(), ";
		_pClass->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pFileName->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
		if (_pCachedScript == NULL) {
			_pCachedScript = new DtaDesignScript(getParent());
			_sCachedDesignFile = sScriptFilename;
			_pCachedScript->parseFile(_sCachedDesignFile.c_str());
		}
		_pCachedScript->compileCpp(theCompilerEnvironment, sScriptFilename);
	}

	void GrfParseFree::compileCppForQuiet(CppCompilerEnvironment& theCompilerEnvironment) const {
		ExprScriptConstant* pConstantFileName = dynamic_cast<ExprScriptConstant*>(_pDesignFileName);
		std::string sScriptFilename;
		if (pConstantFileName == NULL) {
			sScriptFilename = theCompilerEnvironment.newInlineScriptFilename();
		} else {
			sScriptFilename = pConstantFileName->getConstant();
		}
		std::string sRadical = DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(CppCompilerEnvironment::filename2Module(sScriptFilename)));
		CW_BODY_STREAM << "\tCGRuntime::parseFree(&Execute" << sRadical << "::instance(), pThisTree, sFilename);";
		CW_BODY_ENDL;
		if (_pCachedScript == NULL) {
			_pCachedScript = new DtaDesignScript(getParent());
			_sCachedDesignFile = sScriptFilename;
			_pCachedScript->parseFile(_sCachedDesignFile.c_str());
		}
		_pCachedScript->compileCpp(theCompilerEnvironment, sScriptFilename);
	}
}
