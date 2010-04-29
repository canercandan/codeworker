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
#include "ScpStream.h"
#include "CppCompilerEnvironment.h"

#include "ExprScriptExpression.h"
#include "ExprScriptVariable.h"
#include "CGRuntime.h" // for CGRuntime::getCurrentDirectory()
#include "DtaBNFScript.h"
#include "DtaScriptVariable.h"
#include "BNFClause.h"
#include "DtaProject.h"
#include "GrfParseAsBNF.h"

namespace CodeWorker {
	GrfParseAsBNF::GrfParseAsBNF() : _pClass(NULL), _pCachedScript(NULL), _pBNFFileName(NULL), _pFileName(NULL) {
		_sCurrentDirectoryAtCompileTime = CGRuntime::getCurrentDirectory();
	}

	GrfParseAsBNF::~GrfParseAsBNF() {
		delete _pBNFFileName;
		delete _pClass;
		delete _pFileName;
		if (_pCachedScript != NULL) delete _pCachedScript;
	}

	void GrfParseAsBNF::setBNFFileName(ExprScriptScriptFile* pBNFFileName) {
		if (pBNFFileName->isFileName()) _pBNFFileName = pBNFFileName->getFileName();
		_pCachedScript = dynamic_cast<DtaBNFScript*>(pBNFFileName->getBody());
		pBNFFileName->release();
	}

	SEQUENCE_INTERRUPTION_LIST GrfParseAsBNF::executeInternal(DtaScriptVariable& visibility) {
		DtaScriptVariable* pClass = visibility.getExistingVariable(*_pClass);
		if (pClass == NULL) throw UtlException("runtime error: variable '" + _pClass->toString() + "' doesn't exist while calling procedure 'parseAsBNF()'");
		std::string sOutputFile = _pFileName->getValue(visibility);

		if (_pBNFFileName != NULL) {
			std::string sBNFFileName = _pBNFFileName->getValue(visibility);
			if ((_pCachedScript == NULL) || (_sCachedBNFFile != sBNFFileName) || ScpStream::existVirtualFile(sBNFFileName)) {
				if (_pCachedScript != NULL) delete _pCachedScript;
				_pCachedScript = new DtaBNFScript(getParent());
				_sCachedBNFFile = sBNFFileName;
				_pCachedScript->parseFile(sBNFFileName.c_str(), _sCurrentDirectoryAtCompileTime);
			}
		}

		SEQUENCE_INTERRUPTION_LIST result = _pCachedScript->generate(sOutputFile.c_str(), *pClass);
		switch(result) {
			case CONTINUE_INTERRUPTION:
			case BREAK_INTERRUPTION:
			case RETURN_INTERRUPTION:
				result = NO_INTERRUPTION;
				break;
		}
		return result;
	}

	void GrfParseAsBNF::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		ExprScriptConstant* pConstantFileName = dynamic_cast<ExprScriptConstant*>(_pBNFFileName);
		std::string sScriptFilename;
		if (pConstantFileName == NULL) {
			sScriptFilename = theCompilerEnvironment.newInlineScriptFilename();
		} else {
			sScriptFilename = pConstantFileName->getConstant();
		}
		std::string sRadical = DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(CppCompilerEnvironment::filename2Module(sScriptFilename)));
		CW_BODY_INDENT << "CGRuntime::parseAsBNF(&Execute" << sRadical << "::instance(), ";
		_pClass->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pFileName->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ");";
		CW_BODY_ENDL;
		if (_pCachedScript == NULL) {
			_pCachedScript = new DtaBNFScript(getParent());
			_sCachedBNFFile = sScriptFilename;
			_pCachedScript->parseFile(_sCachedBNFFile.c_str());
		}
		_pCachedScript->compileCpp(theCompilerEnvironment, sScriptFilename);
	}
}
