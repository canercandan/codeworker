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

#include <math.h>

#include "UtlException.h"
#include "ScpStream.h"

#include "CppCompilerEnvironment.h"
#include "CGRuntime.h"
#include "CGExternalHandling.h"
#include "DtaArrayIterator.h"
#include "GrfFunction.h"
#include "GrfForeach.h"
#include "GrfParseFree.h"
#include "GrfTranslate.h"
#include "DtaProject.h"
#include "DtaDesignScript.h"
#include "DtaPatternScript.h"
#include "DtaBNFScript.h"
#include "DtaScriptVariable.h"
#include "ExprScriptVariable.h"
#include "ExprScriptFunction.h"

namespace CodeWorker {
	class ExprScriptFirst : public ExprScriptFunction {
	private:
		static unsigned int _iCounter;
		ExprScriptFirst() {}

	public:
		virtual ~ExprScriptFirst() {}

		virtual const char* getName() const { return "first"; }
		virtual unsigned int getArity() const { return 1; }
		virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
			if (iIndex == 0) return ITERATOR_EXPRTYPE;
			return UNKNOWN_EXPRTYPE;
		}

		virtual std::string getValue(DtaScriptVariable& visibility) const {
			_iCounter++;
			ExprScriptVariable* pIndexParam = (ExprScriptVariable*) _parameters.front();
			DtaScriptVariable* pIndexExpr = visibility.getIterator(pIndexParam->getName().c_str());
			if (pIndexExpr == NULL) throw UtlException("'first(" + pIndexParam->getName() + ")' has no sense: '" + pIndexParam->getName() + "' isn't declared as an index");
			DtaArrayIterator* pIteratorData = pIndexExpr->getIteratorData();
			if (pIteratorData == NULL) throw UtlException("'first(" + pIndexParam->getName() + ")' can't work here: '" + pIndexParam->getName() + "' is a reference to an index\nif '" + pIndexParam->getName() + "' is a function parameter, write '" + pIndexParam->getName() + " : index' in the prototype");
			if (pIteratorData->first()) return "true";
			return "";
		}

		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
			GrfForeach* pForeach = theCompilerEnvironment.getLastForeach();
			std::string sModifier;
			if (pForeach->getSorted()) {
				sModifier = "Sorted";
				if (pForeach->getSortedNoCase()) sModifier += "NoCase";
			}
			if (bNegative) {
				CW_BODY_STREAM << "_compilerIndex" << sModifier << "Foreach_" << ((ExprScriptVariable*) _parameters.front())->getName() << " != _compilerMap" << sModifier << "Foreach_" << ((ExprScriptVariable*) _parameters.front())->getName() << "->begin()";
			} else {
				CW_BODY_STREAM << "_compilerIndex" << sModifier << "Foreach_" << ((ExprScriptVariable*) _parameters.front())->getName() << " == _compilerMap" << sModifier << "Foreach_" << ((ExprScriptVariable*) _parameters.front())->getName() << "->begin()";
			}
			return true;
		}

		static ExprScriptFunction* create(GrfBlock&) {
			return new ExprScriptFirst;
		}

		static void registerFunction(DtaFunctionInfo& info) {
			info.constructor = create;
			info.pCounter = &_iCounter;
		}
	};
	unsigned int ExprScriptFirst::_iCounter = 0;

	class ExprScriptLast : public ExprScriptFunction {
	private:
		static unsigned int _iCounter;
		ExprScriptLast() {}

	public:
		virtual ~ExprScriptLast() {}

		virtual const char* getName() const { return "last"; }
		virtual unsigned int getArity() const { return 1; }
		virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
			if (iIndex == 0) return ITERATOR_EXPRTYPE;
			return UNKNOWN_EXPRTYPE;
		}

		virtual std::string getValue(DtaScriptVariable& visibility) const {
			_iCounter++;
			ExprScriptVariable* pIndexParam = (ExprScriptVariable*) _parameters.front();
			DtaScriptVariable* pIndexExpr = visibility.getIterator(pIndexParam->getName().c_str());
			if (pIndexExpr == NULL) throw UtlException("'last(" + pIndexParam->getName() + ")' has no sense: '" + pIndexParam->getName() + "' isn't declared as an index");
			DtaArrayIterator* pIteratorData = pIndexExpr->getIteratorData();
			if (pIteratorData == NULL) throw UtlException("'last(" + pIndexParam->getName() + ")' can't work here: '" + pIndexParam->getName() + "' is a reference to an index\nif '" + pIndexParam->getName() + "' is a function parameter, write '" + pIndexParam->getName() + " : index' in the prototype");
			if (pIteratorData->last()) return "true";
			return "";
		}

		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
			GrfForeach* pForeach = theCompilerEnvironment.getLastForeach();
			std::string sModifier;
			if (pForeach->getSorted()) {
				sModifier = "Sorted";
				if (pForeach->getSortedNoCase()) sModifier += "NoCase";
			}
			if (bNegative) {
				CW_BODY_STREAM << "CGRuntime::nextIteration(_compilerIndex" << sModifier << "Foreach_" << ((ExprScriptVariable*) _parameters.front())->getName() << ") != _compilerMap" << sModifier << "Foreach_" << ((ExprScriptVariable*) _parameters.front())->getName() << "->end()";
			} else {
				CW_BODY_STREAM << "CGRuntime::nextIteration(_compilerIndex" << sModifier << "Foreach_" << ((ExprScriptVariable*) _parameters.front())->getName() << ") == _compilerMap" << sModifier << "Foreach_" << ((ExprScriptVariable*) _parameters.front())->getName() << "->end()";
			}
			return true;
		}

		static ExprScriptFunction* create(GrfBlock&) {
			return new ExprScriptLast;
		}

		static void registerFunction(DtaFunctionInfo& info) {
			info.constructor = create;
			info.pCounter = &_iCounter;
		}
	};
	unsigned int ExprScriptLast::_iCounter = 0;

	class ExprScriptKey : public ExprScriptFunction {
	private:
		static unsigned int _iCounter;
		ExprScriptKey() {}

	public:
		virtual ~ExprScriptKey() {}

		virtual const char* getName() const { return "key"; }
		virtual unsigned int getArity() const { return 1; }
		virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
			if (iIndex == 0) return ITERATOR_EXPRTYPE;
			return UNKNOWN_EXPRTYPE;
		}

		virtual std::string getValue(DtaScriptVariable& visibility) const {
			_iCounter++;
			ExprScriptVariable* pIndexParam = (ExprScriptVariable*) _parameters.front();
			DtaScriptVariable* pIndexExpr = visibility.getIterator(pIndexParam->getName().c_str());
			if (pIndexExpr == NULL) throw UtlException("'key(" + pIndexParam->getName() + ")' has no sense: '" + pIndexParam->getName() + "' isn't declared as an index");
			DtaArrayIterator* pIteratorData = pIndexExpr->getIteratorData();
			if (pIteratorData == NULL) throw UtlException("'key(" + pIndexParam->getName() + ")' can't work here: '" + pIndexParam->getName() + "' is a reference to an index\nif '" + pIndexParam->getName() + "' is a function parameter, write '" + pIndexParam->getName() + " : index' in the prototype");
			return pIteratorData->key();
		}

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
			compileCppString(theCompilerEnvironment);
			return STRING_TYPE;
		}

		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
			if (!bNegative) CW_BODY_STREAM << "!";
			compileCpp(theCompilerEnvironment);
			CW_BODY_STREAM << ".empty()";
			return true;
		}

		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
			CW_BODY_STREAM << "_compilerKeyForeach_" << ((ExprScriptVariable*) _parameters.front())->getName();
			return true;
		}

		static ExprScriptFunction* create(GrfBlock&) {
			return new ExprScriptKey;
		}

		static void registerFunction(DtaFunctionInfo& info) {
			info.constructor = create;
			info.pCounter = &_iCounter;
		}
	};
	unsigned int ExprScriptKey::_iCounter = 0;


	class ExprScriptIndex : public ExprScriptFunction {
	private:
		static unsigned int _iCounter;
		ExprScriptIndex() {}

	public:
		virtual ~ExprScriptIndex() {}

		virtual const char* getName() const { return "index"; }
		virtual unsigned int getArity() const { return 1; }
		virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
			if (iIndex == 0) return ITERATOR_EXPRTYPE;
			return UNKNOWN_EXPRTYPE;
		}

		virtual std::string getValue(DtaScriptVariable& visibility) const {
			_iCounter++;
			ExprScriptVariable* pIndexParam = (ExprScriptVariable*) _parameters.front();
			DtaScriptVariable* pIndexExpr = visibility.getIterator(pIndexParam->getName().c_str());
			if (pIndexExpr == NULL) throw UtlException("'index(" + pIndexParam->getName() + ")' has no sense: '" + pIndexParam->getName() + "' isn't declared as an iterator");
			DtaArrayIterator* pIteratorData = pIndexExpr->getIteratorData();
			if (pIteratorData == NULL) throw UtlException("'index(" + pIndexParam->getName() + ")' can't work here: '" + pIndexParam->getName() + "' is a reference to an iterator\nif '" + pIndexParam->getName() + "' is a function parameter, write '" + pIndexParam->getName() + " : index' in the prototype");
			char tcResult[32];
			sprintf(tcResult, "%d", pIteratorData->index());
			return tcResult;
		}

		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
			CW_BODY_STREAM << "<index() not handled yet!>";
			return true;
		}

		static ExprScriptFunction* create(GrfBlock&) {
			return new ExprScriptIndex;
		}

		static void registerFunction(DtaFunctionInfo& info) {
			info.constructor = create;
			info.pCounter = &_iCounter;
		}
	};
	unsigned int ExprScriptIndex::_iCounter = 0;


	class ExprScriptPrec : public ExprScriptFunction {
	private:
		static unsigned int _iCounter;
		ExprScriptPrec() {}

	public:
		virtual ~ExprScriptPrec() {}

		virtual const char* getName() const { return "prec"; }
		virtual unsigned int getArity() const { return 1; }
		virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
			if (iIndex == 0) return ITERATOR_EXPRTYPE;
			return UNKNOWN_EXPRTYPE;
		}

		virtual std::string getValue(DtaScriptVariable& visibility) const {
			_iCounter++;
			ExprScriptVariable* pIndexParam = (ExprScriptVariable*) _parameters.front();
			DtaScriptVariable* pIndexExpr = visibility.getIterator(pIndexParam->getName().c_str());
			if (pIndexExpr == NULL) throw UtlException("'prec(" + pIndexParam->getName() + ")' has no sense: '" + pIndexParam->getName() + "' isn't declared as an iterator");
			DtaArrayIterator* pIteratorData = pIndexExpr->getIteratorData();
			if (pIteratorData == NULL) throw UtlException("'prec(" + pIndexParam->getName() + ")' can't work here: '" + pIndexParam->getName() + "' is a reference to an iterator\nif '" + pIndexParam->getName() + "' is a function parameter, write '" + pIndexParam->getName() + " : Prec' in the prototype");
			if (pIteratorData->prec()) return "true";
			return "";
		}

		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
			CW_BODY_STREAM << "<prec() not handled yet!>";
			return true;
		}

		static ExprScriptFunction* create(GrfBlock&) {
			return new ExprScriptPrec;
		}

		static void registerFunction(DtaFunctionInfo& info) {
			info.constructor = create;
			info.pCounter = &_iCounter;
		}
	};
	unsigned int ExprScriptPrec::_iCounter = 0;


	class ExprScriptNext : public ExprScriptFunction {
	private:
		static unsigned int _iCounter;
		ExprScriptNext() {}

	public:
		virtual ~ExprScriptNext() {}

		virtual const char* getName() const { return "next"; }
		virtual unsigned int getArity() const { return 1; }
		virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
			if (iIndex == 0) return ITERATOR_EXPRTYPE;
			return UNKNOWN_EXPRTYPE;
		}

		virtual std::string getValue(DtaScriptVariable& visibility) const {
			_iCounter++;
			ExprScriptVariable* pIndexParam = (ExprScriptVariable*) _parameters.front();
			DtaScriptVariable* pIndexExpr = visibility.getIterator(pIndexParam->getName().c_str());
			if (pIndexExpr == NULL) throw UtlException("'next(" + pIndexParam->getName() + ")' has no sense: '" + pIndexParam->getName() + "' isn't declared as an iterator");
			DtaArrayIterator* pIteratorData = pIndexExpr->getIteratorData();
			if (pIteratorData == NULL) throw UtlException("'next(" + pIndexParam->getName() + ")' can't work here: '" + pIndexParam->getName() + "' is a reference to an iterator\nif '" + pIndexParam->getName() + "' is a function parameter, write '" + pIndexParam->getName() + " : Next' in the prototype");
			if (pIteratorData->next()) return "true";
			return "";
		}

		virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
			CW_BODY_STREAM << "<next() not handled yet!>";
			return true;
		}

		static ExprScriptFunction* create(GrfBlock&) {
			return new ExprScriptNext;
		}

		static void registerFunction(DtaFunctionInfo& info) {
			info.constructor = create;
			info.pCounter = &_iCounter;
		}
	};
	unsigned int ExprScriptNext::_iCounter = 0;


	class ExprScriptNot : public ExprScriptFunction {
	private:
		static unsigned int _iCounter;
		ExprScriptNot() {}

	public:
		virtual ~ExprScriptNot() {}

		virtual const char* getName() const { return "not"; }
		virtual unsigned int getArity() const { return 1; }
		virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
			if (iIndex == 0) return VALUE_EXPRTYPE;
			return UNKNOWN_EXPRTYPE;
		}

		virtual std::string getValue(DtaScriptVariable& visibility) const {
			_iCounter++;
			std::string sValue = _parameters.front()->getValue(visibility);
			if (sValue.empty()) return "true";
			return "";
		}

		virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
			compileCppBoolean(theCompilerEnvironment, true);
			return BOOL_TYPE;
		}

		virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
			return _parameters.front()->compileCppBoolean(theCompilerEnvironment, !bNegative);
		}

		virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
			CW_BODY_STREAM << "((";
			compileCppBoolean(theCompilerEnvironment, false);
			CW_BODY_STREAM << ") ? \"\" : \"true\")";
			return true;
		}

		static ExprScriptFunction* create(GrfBlock&) {
			return new ExprScriptNot;
		}

		static void registerFunction(DtaFunctionInfo& info) {
			info.constructor = create;
			info.pCounter = &_iCounter;
		}
	};
	unsigned int ExprScriptNot::_iCounter = 0;


//##markup##"declarations"
//##begin##"declarations"

class ExprScriptFlushOutputToSocket : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFlushOutputToSocket() {
	}

public:
    virtual ~ExprScriptFlushOutputToSocket() {	}

	virtual const char* getName() const { return "flushOutputToSocket"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sSocket = (*cursor)->getValue(visibility);
		int iSocket = atoi(sSocket.c_str());
		bool returnValue = CGRuntime::flushOutputToSocket(iSocket);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::flushOutputToSocket(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFlushOutputToSocket;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFlushOutputToSocket::_iCounter = 0;


class ExprScriptAcceptSocket : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptAcceptSocket() {
	}

public:
    virtual ~ExprScriptAcceptSocket() {	}

	virtual const char* getName() const { return "acceptSocket"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sServerSocket = (*cursor)->getValue(visibility);
		int iServerSocket = atoi(sServerSocket.c_str());
		int returnValue = CGRuntime::acceptSocket(iServerSocket);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::acceptSocket(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptAcceptSocket;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptAcceptSocket::_iCounter = 0;


class ExprScriptAdd : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptAdd() {
	}

public:
    virtual ~ExprScriptAdd() {	}

	virtual const char* getName() const { return "add"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLeft = (*cursor)->getValue(visibility);
		double dLeft = atof(sLeft.c_str());
		++cursor;
		std::string sRight = (*cursor)->getValue(visibility);
		double dRight = atof(sRight.c_str());
		double returnValue = CGRuntime::add(dLeft, dRight);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::add(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptAdd;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptAdd::_iCounter = 0;


class ExprScriptAddGenerationTagsHandler : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptAddGenerationTagsHandler(GrfBlock& block) {
//##protect##"addGenerationTagsHandler::constructor"
		_pReaderCache = NULL;
		_pWriterCache = NULL;
//##protect##"addGenerationTagsHandler::constructor"
	}


private:
//##protect##"addGenerationTagsHandler::attributes"
	mutable DtaBNFScript*     _pReaderCache;
	mutable DtaPatternScript* _pWriterCache;
//##protect##"addGenerationTagsHandler::attributes"

public:
    virtual ~ExprScriptAddGenerationTagsHandler() {
//##protect##"addGenerationTagsHandler::destructor"
		if (_pReaderCache != NULL) delete _pReaderCache;
		if (_pWriterCache != NULL) delete _pWriterCache;
//##protect##"addGenerationTagsHandler::destructor"
	}

	virtual const char* getName() const { return "addGenerationTagsHandler"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return SCRIPTFILE_BNF_EXPRTYPE;
		if (iIndex == 2) return SCRIPTFILE_PATTERN_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual void initializationDone() {
//##protect##"addGenerationTagsHandler::initializationDone"
//##protect##"addGenerationTagsHandler::initializationDone"
	}


    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		bool returnValue;
//##protect##"addGenerationTagsHandler::execute"
		std::string sKey = _parameters[0]->getValue(visibility);
		returnValue = !DtaProject::getInstance().existGenerationTagsHandler(sKey);
		if (returnValue) {
			// the reader
			DtaBNFScript* pReader;
			ExprScriptScriptFile* pBNFFileName = dynamic_cast<ExprScriptScriptFile*>(_parameters[1]);
			if (pBNFFileName->isFileName()) {
				if (_pReaderCache == NULL) {
					std::string sScriptFile = pBNFFileName->getFileName()->getValue(visibility);
					std::auto_ptr<DtaBNFScript> pScript(new DtaBNFScript);
					pScript->parseFile(sScriptFile.c_str());
					_pReaderCache = pScript.release();
				}
				pReader = _pReaderCache;
			} else {
				pReader = dynamic_cast<DtaBNFScript*>(pBNFFileName->getBody());
			}
			// the writer
			DtaPatternScript* pWriter;
			ExprScriptScriptFile* pPatternFileName = dynamic_cast<ExprScriptScriptFile*>(_parameters[2]);
			if (pPatternFileName->isFileName()) {
				if (_pWriterCache == NULL) {
					std::string sScriptFile = pPatternFileName->getFileName()->getValue(visibility);
					std::auto_ptr<DtaPatternScript> pScript(new DtaPatternScript);
					pScript->parseFile(sScriptFile.c_str());
					_pWriterCache = pScript.release();
				}
				pWriter = _pWriterCache;
			} else {
				pWriter = dynamic_cast<DtaPatternScript*>(pPatternFileName->getBody());
			}
			// register the handler
			returnValue = DtaProject::getInstance().addGenerationTagsHandler(sKey, pReader, pWriter);
		}
//##protect##"addGenerationTagsHandler::execute"
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
//##protect##"addGenerationTagsHandler::compileCppBoolean"
//##protect##"addGenerationTagsHandler::compileCppBoolean"
		return true;
	}

	static ExprScriptFunction* create(GrfBlock& block) {
        return new ExprScriptAddGenerationTagsHandler(block);
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptAddGenerationTagsHandler::_iCounter = 0;


class ExprScriptAddToDate : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptAddToDate() {
	}

public:
    virtual ~ExprScriptAddToDate() {	}

	virtual const char* getName() const { return "addToDate"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sDate = (*cursor)->getValue(visibility);
		++cursor;
		std::string sFormat = (*cursor)->getValue(visibility);
		++cursor;
		std::string sShifting = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::addToDate(sDate, sFormat, sShifting);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::addToDate(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptAddToDate;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptAddToDate::_iCounter = 0;


class ExprScriptByteToChar : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptByteToChar() {
	}

public:
    virtual ~ExprScriptByteToChar() {	}

	virtual const char* getName() const { return "byteToChar"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sByte = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::byteToChar(sByte);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::byteToChar(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptByteToChar;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptByteToChar::_iCounter = 0;


class ExprScriptBytesToLong : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptBytesToLong() {
	}

public:
    virtual ~ExprScriptBytesToLong() {	}

	virtual const char* getName() const { return "bytesToLong"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sBytes = (*cursor)->getValue(visibility);
		unsigned long returnValue = CGRuntime::bytesToLong(sBytes);
		char tcNumber[16];
		sprintf(tcNumber, "%Lu", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppUnsignedLong(theCompilerEnvironment);
		return ULONG_TYPE;
	}

	virtual bool compileCppUnsignedLong(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::bytesToLong(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptBytesToLong;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptBytesToLong::_iCounter = 0;


class ExprScriptBytesToShort : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptBytesToShort() {
	}

public:
    virtual ~ExprScriptBytesToShort() {	}

	virtual const char* getName() const { return "bytesToShort"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sBytes = (*cursor)->getValue(visibility);
		unsigned short returnValue = CGRuntime::bytesToShort(sBytes);
		char tcNumber[16];
		sprintf(tcNumber, "%hu", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppUnsignedShort(theCompilerEnvironment);
		return USHORT_TYPE;
	}

	virtual bool compileCppUnsignedShort(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::bytesToShort(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptBytesToShort;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptBytesToShort::_iCounter = 0;


class ExprScriptCanonizePath : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCanonizePath() {
	}

public:
    virtual ~ExprScriptCanonizePath() {	}

	virtual const char* getName() const { return "canonizePath"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sPath = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::canonizePath(sPath);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::canonizePath(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCanonizePath;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCanonizePath::_iCounter = 0;


class ExprScriptChangeDirectory : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptChangeDirectory() {
	}

public:
    virtual ~ExprScriptChangeDirectory() {	}

	virtual const char* getName() const { return "changeDirectory"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sPath = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::changeDirectory(sPath);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::changeDirectory(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptChangeDirectory;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptChangeDirectory::_iCounter = 0;


class ExprScriptChangeFileTime : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptChangeFileTime() {
	}

public:
    virtual ~ExprScriptChangeFileTime() {	}

	virtual const char* getName() const { return "changeFileTime"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		++cursor;
		std::string sAccessTime = (*cursor)->getValue(visibility);
		++cursor;
		std::string sModificationTime = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::changeFileTime(sFilename, sAccessTime, sModificationTime);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::changeFileTime(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptChangeFileTime;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptChangeFileTime::_iCounter = 0;


class ExprScriptCharAt : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCharAt() {
	}

public:
    virtual ~ExprScriptCharAt() {	}

	virtual const char* getName() const { return "charAt"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sIndex = (*cursor)->getValue(visibility);
		int iIndex = atoi(sIndex.c_str());
		std::string returnValue = CGRuntime::charAt(sText, iIndex);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::charAt(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCharAt;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCharAt::_iCounter = 0;


class ExprScriptCharToByte : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCharToByte() {
	}

public:
    virtual ~ExprScriptCharToByte() {	}

	virtual const char* getName() const { return "charToByte"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sChar = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::charToByte(sChar);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::charToByte(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCharToByte;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCharToByte::_iCounter = 0;


class ExprScriptCharToInt : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCharToInt() {
	}

public:
    virtual ~ExprScriptCharToInt() {	}

	virtual const char* getName() const { return "charToInt"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sChar = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::charToInt(sChar);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::charToInt(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCharToInt;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCharToInt::_iCounter = 0;


class ExprScriptChmod : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptChmod() {
	}

public:
    virtual ~ExprScriptChmod() {	}

	virtual const char* getName() const { return "chmod"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		++cursor;
		std::string sMode = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::chmod(sFilename, sMode);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::chmod(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptChmod;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptChmod::_iCounter = 0;


class ExprScriptCeil : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCeil() {
	}

public:
    virtual ~ExprScriptCeil() {	}

	virtual const char* getName() const { return "ceil"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sNumber = (*cursor)->getValue(visibility);
		double dNumber = atof(sNumber.c_str());
		int returnValue = CGRuntime::ceil(dNumber);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::ceil(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCeil;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCeil::_iCounter = 0;


class ExprScriptCompareDate : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCompareDate() {
	}

public:
    virtual ~ExprScriptCompareDate() {	}

	virtual const char* getName() const { return "compareDate"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sDate1 = (*cursor)->getValue(visibility);
		++cursor;
		std::string sDate2 = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::compareDate(sDate1, sDate2);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::compareDate(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCompareDate;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCompareDate::_iCounter = 0;


class ExprScriptCompleteDate : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCompleteDate() {
	}

public:
    virtual ~ExprScriptCompleteDate() {	}

	virtual const char* getName() const { return "completeDate"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sDate = (*cursor)->getValue(visibility);
		++cursor;
		std::string sFormat = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::completeDate(sDate, sFormat);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::completeDate(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCompleteDate;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCompleteDate::_iCounter = 0;


class ExprScriptCompleteLeftSpaces : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCompleteLeftSpaces() {
	}

public:
    virtual ~ExprScriptCompleteLeftSpaces() {	}

	virtual const char* getName() const { return "completeLeftSpaces"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::completeLeftSpaces(sText, iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::completeLeftSpaces(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCompleteLeftSpaces;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCompleteLeftSpaces::_iCounter = 0;


class ExprScriptCompleteRightSpaces : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCompleteRightSpaces() {
	}

public:
    virtual ~ExprScriptCompleteRightSpaces() {	}

	virtual const char* getName() const { return "completeRightSpaces"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::completeRightSpaces(sText, iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::completeRightSpaces(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCompleteRightSpaces;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCompleteRightSpaces::_iCounter = 0;


class ExprScriptComposeAdaLikeString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptComposeAdaLikeString() {
	}

public:
    virtual ~ExprScriptComposeAdaLikeString() {	}

	virtual const char* getName() const { return "composeAdaLikeString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::composeAdaLikeString(sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::composeAdaLikeString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptComposeAdaLikeString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptComposeAdaLikeString::_iCounter = 0;


class ExprScriptComposeCLikeString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptComposeCLikeString() {
	}

public:
    virtual ~ExprScriptComposeCLikeString() {	}

	virtual const char* getName() const { return "composeCLikeString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::composeCLikeString(sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::composeCLikeString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptComposeCLikeString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptComposeCLikeString::_iCounter = 0;


class ExprScriptComposeHTMLLikeString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptComposeHTMLLikeString() {
	}

public:
    virtual ~ExprScriptComposeHTMLLikeString() {	}

	virtual const char* getName() const { return "composeHTMLLikeString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::composeHTMLLikeString(sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::composeHTMLLikeString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptComposeHTMLLikeString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptComposeHTMLLikeString::_iCounter = 0;


class ExprScriptComposeSQLLikeString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptComposeSQLLikeString() {
	}

public:
    virtual ~ExprScriptComposeSQLLikeString() {	}

	virtual const char* getName() const { return "composeSQLLikeString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::composeSQLLikeString(sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::composeSQLLikeString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptComposeSQLLikeString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptComposeSQLLikeString::_iCounter = 0;


class ExprScriptComputeMD5 : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptComputeMD5() {
	}

public:
    virtual ~ExprScriptComputeMD5() {	}

	virtual const char* getName() const { return "computeMD5"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::computeMD5(sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::computeMD5(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptComputeMD5;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptComputeMD5::_iCounter = 0;


class ExprScriptCopySmartFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCopySmartFile() {
	}

public:
    virtual ~ExprScriptCopySmartFile() {	}

	virtual const char* getName() const { return "copySmartFile"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sSourceFileName = (*cursor)->getValue(visibility);
		++cursor;
		std::string sDestinationFileName = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::copySmartFile(sSourceFileName, sDestinationFileName);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::copySmartFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCopySmartFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCopySmartFile::_iCounter = 0;


class ExprScriptCoreString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCoreString() {
	}

public:
    virtual ~ExprScriptCoreString() {	}

	virtual const char* getName() const { return "coreString"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sPos = (*cursor)->getValue(visibility);
		int iPos = atoi(sPos.c_str());
		++cursor;
		std::string sLastRemoved = (*cursor)->getValue(visibility);
		int iLastRemoved = atoi(sLastRemoved.c_str());
		std::string returnValue = CGRuntime::coreString(sText, iPos, iLastRemoved);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::coreString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCoreString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCoreString::_iCounter = 0;


class ExprScriptCountStringOccurences : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCountStringOccurences() {
	}

public:
    virtual ~ExprScriptCountStringOccurences() {	}

	virtual const char* getName() const { return "countStringOccurences"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sString = (*cursor)->getValue(visibility);
		++cursor;
		std::string sText = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::countStringOccurences(sString, sText);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::countStringOccurences(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCountStringOccurences;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCountStringOccurences::_iCounter = 0;


class ExprScriptCreateDirectory : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCreateDirectory() {
	}

public:
    virtual ~ExprScriptCreateDirectory() {	}

	virtual const char* getName() const { return "createDirectory"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sPath = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::createDirectory(sPath);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::createDirectory(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCreateDirectory;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCreateDirectory::_iCounter = 0;


class ExprScriptCreateINETClientSocket : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCreateINETClientSocket() {
	}

public:
    virtual ~ExprScriptCreateINETClientSocket() {	}

	virtual const char* getName() const { return "createINETClientSocket"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sRemoteAddress = (*cursor)->getValue(visibility);
		++cursor;
		std::string sPort = (*cursor)->getValue(visibility);
		int iPort = atoi(sPort.c_str());
		int returnValue = CGRuntime::createINETClientSocket(sRemoteAddress, iPort);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::createINETClientSocket(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCreateINETClientSocket;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCreateINETClientSocket::_iCounter = 0;


class ExprScriptCreateINETServerSocket : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCreateINETServerSocket() {
	}

public:
    virtual ~ExprScriptCreateINETServerSocket() {	}

	virtual const char* getName() const { return "createINETServerSocket"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sPort = (*cursor)->getValue(visibility);
		int iPort = atoi(sPort.c_str());
		++cursor;
		std::string sBackLog = (*cursor)->getValue(visibility);
		int iBackLog = atoi(sBackLog.c_str());
		int returnValue = CGRuntime::createINETServerSocket(iPort, iBackLog);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::createINETServerSocket(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCreateINETServerSocket;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCreateINETServerSocket::_iCounter = 0;


class ExprScriptCreateIterator : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCreateIterator() {
	}

public:
    virtual ~ExprScriptCreateIterator() {	}

	virtual const char* getName() const { return "createIterator"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return ITERATOR_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pI = visibility.getIterator(dynamic_cast<ExprScriptVariable*>(*cursor)->getName().c_str());
		if (pI == NULL) pI = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		DtaScriptVariable* pList = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		bool returnValue = CGRuntime::createIterator(pI, pList);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::createIterator(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCreateIterator;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCreateIterator::_iCounter = 0;


class ExprScriptCreateReverseIterator : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCreateReverseIterator() {
	}

public:
    virtual ~ExprScriptCreateReverseIterator() {	}

	virtual const char* getName() const { return "createReverseIterator"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return ITERATOR_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pI = visibility.getIterator(dynamic_cast<ExprScriptVariable*>(*cursor)->getName().c_str());
		if (pI == NULL) pI = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		DtaScriptVariable* pList = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		bool returnValue = CGRuntime::createReverseIterator(pI, pList);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::createReverseIterator(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCreateReverseIterator;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCreateReverseIterator::_iCounter = 0;


class ExprScriptCreateVirtualFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCreateVirtualFile() {
	}

public:
    virtual ~ExprScriptCreateVirtualFile() {	}

	virtual const char* getName() const { return "createVirtualFile"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sHandle = (*cursor)->getValue(visibility);
		++cursor;
		std::string sContent = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::createVirtualFile(sHandle, sContent);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::createVirtualFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCreateVirtualFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCreateVirtualFile::_iCounter = 0;


class ExprScriptCreateVirtualTemporaryFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCreateVirtualTemporaryFile() {
	}

public:
    virtual ~ExprScriptCreateVirtualTemporaryFile() {	}

	virtual const char* getName() const { return "createVirtualTemporaryFile"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sContent = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::createVirtualTemporaryFile(sContent);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::createVirtualTemporaryFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCreateVirtualTemporaryFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCreateVirtualTemporaryFile::_iCounter = 0;


class ExprScriptDecodeURL : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptDecodeURL() {
	}

public:
    virtual ~ExprScriptDecodeURL() {	}

	virtual const char* getName() const { return "decodeURL"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sURL = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::decodeURL(sURL);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::decodeURL(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptDecodeURL;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptDecodeURL::_iCounter = 0;


class ExprScriptDecrement : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptDecrement() {
	}

public:
    virtual ~ExprScriptDecrement() {	}

	virtual const char* getName() const { return "decrement"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pNumber = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		double dNumber = pNumber->getDoubleValue();
		double returnValue = CGRuntime::decrement(dNumber);
		pNumber->setValue(dNumber);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::decrement(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptDecrement;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptDecrement::_iCounter = 0;


class ExprScriptDeleteFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptDeleteFile() {
	}

public:
    virtual ~ExprScriptDeleteFile() {	}

	virtual const char* getName() const { return "deleteFile"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::deleteFile(sFilename);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::deleteFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptDeleteFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptDeleteFile::_iCounter = 0;


class ExprScriptDeleteVirtualFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptDeleteVirtualFile() {
	}

public:
    virtual ~ExprScriptDeleteVirtualFile() {	}

	virtual const char* getName() const { return "deleteVirtualFile"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sHandle = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::deleteVirtualFile(sHandle);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::deleteVirtualFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptDeleteVirtualFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptDeleteVirtualFile::_iCounter = 0;


class ExprScriptDiv : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptDiv() {
	}

public:
    virtual ~ExprScriptDiv() {	}

	virtual const char* getName() const { return "div"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sDividend = (*cursor)->getValue(visibility);
		double dDividend = atof(sDividend.c_str());
		++cursor;
		std::string sDivisor = (*cursor)->getValue(visibility);
		double dDivisor = atof(sDivisor.c_str());
		double returnValue = CGRuntime::div(dDividend, dDivisor);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::div(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptDiv;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptDiv::_iCounter = 0;


class ExprScriptDuplicateIterator : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptDuplicateIterator() {
	}

public:
    virtual ~ExprScriptDuplicateIterator() {	}

	virtual const char* getName() const { return "duplicateIterator"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return ITERATOR_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pOldIt = visibility.getIterator(dynamic_cast<ExprScriptVariable*>(*cursor)->getName().c_str());
		if (pOldIt == NULL) pOldIt = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		DtaScriptVariable* pNewIt = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		bool returnValue = CGRuntime::duplicateIterator(pOldIt, pNewIt);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::duplicateIterator(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptDuplicateIterator;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptDuplicateIterator::_iCounter = 0;


class ExprScriptEncodeURL : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptEncodeURL() {
	}

public:
    virtual ~ExprScriptEncodeURL() {	}

	virtual const char* getName() const { return "encodeURL"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sURL = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::encodeURL(sURL);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::encodeURL(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptEncodeURL;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptEncodeURL::_iCounter = 0;


class ExprScriptEndl : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptEndl() {
	}

public:
    virtual ~ExprScriptEndl() {	}

	virtual const char* getName() const { return "endl"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::endl();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::endl(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptEndl;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptEndl::_iCounter = 0;


class ExprScriptEndString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptEndString() {
	}

public:
    virtual ~ExprScriptEndString() {	}

	virtual const char* getName() const { return "endString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sEnd = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::endString(sText, sEnd);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::endString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptEndString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptEndString::_iCounter = 0;


class ExprScriptEqual : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptEqual() {
	}

public:
    virtual ~ExprScriptEqual() {	}

	virtual const char* getName() const { return "equal"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLeft = (*cursor)->getValue(visibility);
		double dLeft = atof(sLeft.c_str());
		++cursor;
		std::string sRight = (*cursor)->getValue(visibility);
		double dRight = atof(sRight.c_str());
		bool returnValue = CGRuntime::equal(dLeft, dRight);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::equal(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptEqual;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptEqual::_iCounter = 0;


class ExprScriptEqualsIgnoreCase : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptEqualsIgnoreCase() {
	}

public:
    virtual ~ExprScriptEqualsIgnoreCase() {	}

	virtual const char* getName() const { return "equalsIgnoreCase"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLeft = (*cursor)->getValue(visibility);
		++cursor;
		std::string sRight = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::equalsIgnoreCase(sLeft, sRight);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::equalsIgnoreCase(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptEqualsIgnoreCase;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptEqualsIgnoreCase::_iCounter = 0;


class ExprScriptEqualTrees : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptEqualTrees() {
	}

public:
    virtual ~ExprScriptEqualTrees() {	}

	virtual const char* getName() const { return "equalTrees"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pFirstTree = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		DtaScriptVariable* pSecondTree = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		bool returnValue = CGRuntime::equalTrees(pFirstTree, pSecondTree);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::equalTrees(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptEqualTrees;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptEqualTrees::_iCounter = 0;


class ExprScriptExecuteStringQuiet : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExecuteStringQuiet() {
	}

public:
    virtual ~ExprScriptExecuteStringQuiet() {	}

	virtual const char* getName() const { return "executeStringQuiet"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pThis = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		std::string sCommand = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::executeStringQuiet(pThis, sCommand);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::executeStringQuiet(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExecuteStringQuiet;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExecuteStringQuiet::_iCounter = 0;


class ExprScriptExistDirectory : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExistDirectory() {
	}

public:
    virtual ~ExprScriptExistDirectory() {	}

	virtual const char* getName() const { return "existDirectory"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sPath = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::existDirectory(sPath);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::existDirectory(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExistDirectory;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExistDirectory::_iCounter = 0;


class ExprScriptExistEnv : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExistEnv() {
	}

public:
    virtual ~ExprScriptExistEnv() {	}

	virtual const char* getName() const { return "existEnv"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sVariable = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::existEnv(sVariable);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::existEnv(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExistEnv;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExistEnv::_iCounter = 0;


class ExprScriptExistFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExistFile() {
	}

public:
    virtual ~ExprScriptExistFile() {	}

	virtual const char* getName() const { return "existFile"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFileName = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::existFile(sFileName);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::existFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExistFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExistFile::_iCounter = 0;


class ExprScriptExistVirtualFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExistVirtualFile() {
	}

public:
    virtual ~ExprScriptExistVirtualFile() {	}

	virtual const char* getName() const { return "existVirtualFile"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sHandle = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::existVirtualFile(sHandle);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::existVirtualFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExistVirtualFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExistVirtualFile::_iCounter = 0;


class ExprScriptExistVariable : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExistVariable() {
	}

public:
    virtual ~ExprScriptExistVariable() {	}

	virtual const char* getName() const { return "existVariable"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pVariable = visibility.getNoWarningExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		bool returnValue = CGRuntime::existVariable(pVariable);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::existVariable(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExistVariable;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExistVariable::_iCounter = 0;


class ExprScriptExp : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExp() {
	}

public:
    virtual ~ExprScriptExp() {	}

	virtual const char* getName() const { return "exp"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sX = (*cursor)->getValue(visibility);
		double dX = atof(sX.c_str());
		double returnValue = CGRuntime::exp(dX);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::exp(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExp;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExp::_iCounter = 0;


class ExprScriptExploreDirectory : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExploreDirectory() {
	}

public:
    virtual ~ExprScriptExploreDirectory() {	}

	virtual const char* getName() const { return "exploreDirectory"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pDirectory = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		std::string sPath = (*cursor)->getValue(visibility);
		++cursor;
		std::string sSubfolders = (*cursor)->getValue(visibility);
		bool bSubfolders = !sSubfolders.empty();
		bool returnValue = CGRuntime::exploreDirectory(pDirectory, sPath, bSubfolders);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::exploreDirectory(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppBoolean(theCompilerEnvironment, false);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExploreDirectory;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExploreDirectory::_iCounter = 0;


class ExprScriptExtractGenerationHeader : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExtractGenerationHeader() {
	}

public:
    virtual ~ExprScriptExtractGenerationHeader() {	}

	virtual const char* getName() const { return "extractGenerationHeader"; }
	virtual unsigned int getArity() const { return 4; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		if (iIndex == 2) return NODE_EXPRTYPE;
		if (iIndex == 3) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		++cursor;
		DtaScriptVariable* pGenerator = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sGenerator;
		const char* tsGenerator = pGenerator->getValue();
		if (tsGenerator != NULL) sGenerator = tsGenerator;
		++cursor;
		DtaScriptVariable* pVersion = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sVersion;
		const char* tsVersion = pVersion->getValue();
		if (tsVersion != NULL) sVersion = tsVersion;
		++cursor;
		DtaScriptVariable* pDate = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sDate;
		const char* tsDate = pDate->getValue();
		if (tsDate != NULL) sDate = tsDate;
		std::string returnValue = CGRuntime::extractGenerationHeader(sFilename, sGenerator, sVersion, sDate);
		pGenerator->setValue(sGenerator.c_str());
		pVersion->setValue(sVersion.c_str());
		pDate->setValue(sDate.c_str());
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::extractGenerationHeader(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExtractGenerationHeader;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExtractGenerationHeader::_iCounter = 0;


class ExprScriptFileCreation : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFileCreation() {
	}

public:
    virtual ~ExprScriptFileCreation() {	}

	virtual const char* getName() const { return "fileCreation"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::fileCreation(sFilename);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::fileCreation(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFileCreation;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFileCreation::_iCounter = 0;


class ExprScriptFileLastAccess : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFileLastAccess() {
	}

public:
    virtual ~ExprScriptFileLastAccess() {	}

	virtual const char* getName() const { return "fileLastAccess"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::fileLastAccess(sFilename);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::fileLastAccess(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFileLastAccess;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFileLastAccess::_iCounter = 0;


class ExprScriptFileLastModification : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFileLastModification() {
	}

public:
    virtual ~ExprScriptFileLastModification() {	}

	virtual const char* getName() const { return "fileLastModification"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::fileLastModification(sFilename);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::fileLastModification(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFileLastModification;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFileLastModification::_iCounter = 0;


class ExprScriptFileLines : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFileLines() {
	}

public:
    virtual ~ExprScriptFileLines() {	}

	virtual const char* getName() const { return "fileLines"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::fileLines(sFilename);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::fileLines(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFileLines;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFileLines::_iCounter = 0;


class ExprScriptFileMode : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFileMode() {
	}

public:
    virtual ~ExprScriptFileMode() {	}

	virtual const char* getName() const { return "fileMode"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::fileMode(sFilename);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::fileMode(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFileMode;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFileMode::_iCounter = 0;


class ExprScriptFileSize : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFileSize() {
	}

public:
    virtual ~ExprScriptFileSize() {	}

	virtual const char* getName() const { return "fileSize"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::fileSize(sFilename);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::fileSize(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFileSize;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFileSize::_iCounter = 0;


class ExprScriptFindElement : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFindElement() {
	}

public:
    virtual ~ExprScriptFindElement() {	}

	virtual const char* getName() const { return "findElement"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual int getThisPosition() const { return 1; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sValue = (*cursor)->getValue(visibility);
		++cursor;
		DtaScriptVariable* pVariable = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		bool returnValue = CGRuntime::findElement(sValue, pVariable);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::findElement(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFindElement;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFindElement::_iCounter = 0;


class ExprScriptFindFirstChar : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFindFirstChar() {
	}

public:
    virtual ~ExprScriptFindFirstChar() {	}

	virtual const char* getName() const { return "findFirstChar"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sSomeChars = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::findFirstChar(sText, sSomeChars);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::findFirstChar(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFindFirstChar;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFindFirstChar::_iCounter = 0;


class ExprScriptFindFirstSubstringIntoKeys : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFindFirstSubstringIntoKeys() {
	}

public:
    virtual ~ExprScriptFindFirstSubstringIntoKeys() {	}

	virtual const char* getName() const { return "findFirstSubstringIntoKeys"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sSubstring = (*cursor)->getValue(visibility);
		++cursor;
		DtaScriptVariable* pArray = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		int returnValue = CGRuntime::findFirstSubstringIntoKeys(sSubstring, pArray);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::findFirstSubstringIntoKeys(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFindFirstSubstringIntoKeys;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFindFirstSubstringIntoKeys::_iCounter = 0;


class ExprScriptFindLastString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFindLastString() {
	}

public:
    virtual ~ExprScriptFindLastString() {	}

	virtual const char* getName() const { return "findLastString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sFind = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::findLastString(sText, sFind);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::findLastString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFindLastString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFindLastString::_iCounter = 0;


class ExprScriptFindNextString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFindNextString() {
	}

public:
    virtual ~ExprScriptFindNextString() {	}

	virtual const char* getName() const { return "findNextString"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sFind = (*cursor)->getValue(visibility);
		++cursor;
		std::string sPosition = (*cursor)->getValue(visibility);
		int iPosition = atoi(sPosition.c_str());
		int returnValue = CGRuntime::findNextString(sText, sFind, iPosition);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::findNextString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFindNextString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFindNextString::_iCounter = 0;


class ExprScriptFindNextSubstringIntoKeys : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFindNextSubstringIntoKeys() {
	}

public:
    virtual ~ExprScriptFindNextSubstringIntoKeys() {	}

	virtual const char* getName() const { return "findNextSubstringIntoKeys"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sSubstring = (*cursor)->getValue(visibility);
		++cursor;
		DtaScriptVariable* pArray = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		std::string sNext = (*cursor)->getValue(visibility);
		int iNext = atoi(sNext.c_str());
		int returnValue = CGRuntime::findNextSubstringIntoKeys(sSubstring, pArray, iNext);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::findNextSubstringIntoKeys(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFindNextSubstringIntoKeys;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFindNextSubstringIntoKeys::_iCounter = 0;


class ExprScriptFindString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFindString() {
	}

public:
    virtual ~ExprScriptFindString() {	}

	virtual const char* getName() const { return "findString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sFind = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::findString(sText, sFind);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::findString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFindString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFindString::_iCounter = 0;


class ExprScriptFloor : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFloor() {
	}

public:
    virtual ~ExprScriptFloor() {	}

	virtual const char* getName() const { return "floor"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sNumber = (*cursor)->getValue(visibility);
		double dNumber = atof(sNumber.c_str());
		int returnValue = CGRuntime::floor(dNumber);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::floor(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFloor;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFloor::_iCounter = 0;


class ExprScriptFormatDate : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptFormatDate() {
	}

public:
    virtual ~ExprScriptFormatDate() {	}

	virtual const char* getName() const { return "formatDate"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sDate = (*cursor)->getValue(visibility);
		++cursor;
		std::string sFormat = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::formatDate(sDate, sFormat);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::formatDate(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptFormatDate;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptFormatDate::_iCounter = 0;


class ExprScriptGetArraySize : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetArraySize() {
	}

public:
    virtual ~ExprScriptGetArraySize() {	}

	virtual const char* getName() const { return "getArraySize"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pVariable = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		int returnValue = CGRuntime::getArraySize(pVariable);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getArraySize(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetArraySize;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetArraySize::_iCounter = 0;


class ExprScriptGetCommentBegin : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetCommentBegin() {
	}

public:
    virtual ~ExprScriptGetCommentBegin() {	}

	virtual const char* getName() const { return "getCommentBegin"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getCommentBegin();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getCommentBegin(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetCommentBegin;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetCommentBegin::_iCounter = 0;


class ExprScriptGetCommentEnd : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetCommentEnd() {
	}

public:
    virtual ~ExprScriptGetCommentEnd() {	}

	virtual const char* getName() const { return "getCommentEnd"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getCommentEnd();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getCommentEnd(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetCommentEnd;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetCommentEnd::_iCounter = 0;


class ExprScriptGetCurrentDirectory : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetCurrentDirectory() {
	}

public:
    virtual ~ExprScriptGetCurrentDirectory() {	}

	virtual const char* getName() const { return "getCurrentDirectory"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getCurrentDirectory();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getCurrentDirectory(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetCurrentDirectory;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetCurrentDirectory::_iCounter = 0;


class ExprScriptGetEnv : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetEnv() {
	}

public:
    virtual ~ExprScriptGetEnv() {	}

	virtual const char* getName() const { return "getEnv"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sVariable = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::getEnv(sVariable);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getEnv(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetEnv;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetEnv::_iCounter = 0;


class ExprScriptGetGenerationHeader : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetGenerationHeader() {
	}

public:
    virtual ~ExprScriptGetGenerationHeader() {	}

	virtual const char* getName() const { return "getGenerationHeader"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getGenerationHeader();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getGenerationHeader(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetGenerationHeader;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetGenerationHeader::_iCounter = 0;


class ExprScriptGetHTTPRequest : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetHTTPRequest() {
	}

public:
    virtual ~ExprScriptGetHTTPRequest() {	}

	virtual const char* getName() const { return "getHTTPRequest"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		if (iIndex == 2) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sURL = (*cursor)->getValue(visibility);
		++cursor;
		DtaScriptVariable* pHTTPSession = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		DtaScriptVariable* pArguments = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string returnValue = CGRuntime::getHTTPRequest(sURL, pHTTPSession, pArguments);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getHTTPRequest(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetHTTPRequest;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetHTTPRequest::_iCounter = 0;


class ExprScriptGetIncludePath : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetIncludePath() {
	}

public:
    virtual ~ExprScriptGetIncludePath() {	}

	virtual const char* getName() const { return "getIncludePath"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getIncludePath();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getIncludePath(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetIncludePath;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetIncludePath::_iCounter = 0;


class ExprScriptGetLastDelay : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetLastDelay() {
	}

public:
    virtual ~ExprScriptGetLastDelay() {	}

	virtual const char* getName() const { return "getLastDelay"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		double returnValue = CGRuntime::getLastDelay();
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getLastDelay(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetLastDelay;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetLastDelay::_iCounter = 0;


class ExprScriptGetNow : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetNow() {
	}

public:
    virtual ~ExprScriptGetNow() {	}

	virtual const char* getName() const { return "getNow"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getNow();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getNow(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetNow;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetNow::_iCounter = 0;


class ExprScriptGetProperty : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetProperty() {
	}

public:
    virtual ~ExprScriptGetProperty() {	}

	virtual const char* getName() const { return "getProperty"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sDefine = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::getProperty(sDefine);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getProperty(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetProperty;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetProperty::_iCounter = 0;


class ExprScriptGetShortFilename : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetShortFilename() {
	}

public:
    virtual ~ExprScriptGetShortFilename() {	}

	virtual const char* getName() const { return "getShortFilename"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sPathFilename = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::getShortFilename(sPathFilename);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getShortFilename(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetShortFilename;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetShortFilename::_iCounter = 0;


class ExprScriptGetTextMode : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetTextMode() {
	}

public:
    virtual ~ExprScriptGetTextMode() {	}

	virtual const char* getName() const { return "getTextMode"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getTextMode();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getTextMode(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetTextMode;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetTextMode::_iCounter = 0;


class ExprScriptGetVariableAttributes : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetVariableAttributes() {
	}

public:
    virtual ~ExprScriptGetVariableAttributes() {	}

	virtual const char* getName() const { return "getVariableAttributes"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pVariable = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		DtaScriptVariable* pList = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		int returnValue = CGRuntime::getVariableAttributes(pVariable, pList);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getVariableAttributes(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetVariableAttributes;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetVariableAttributes::_iCounter = 0;


class ExprScriptGetVersion : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetVersion() {
	}

public:
    virtual ~ExprScriptGetVersion() {	}

	virtual const char* getName() const { return "getVersion"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getVersion();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getVersion(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetVersion;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetVersion::_iCounter = 0;


class ExprScriptGetWorkingPath : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetWorkingPath() {
	}

public:
    virtual ~ExprScriptGetWorkingPath() {	}

	virtual const char* getName() const { return "getWorkingPath"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getWorkingPath();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getWorkingPath(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetWorkingPath;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetWorkingPath::_iCounter = 0;


class ExprScriptGetWriteMode : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetWriteMode() {
	}

public:
    virtual ~ExprScriptGetWriteMode() {	}

	virtual const char* getName() const { return "getWriteMode"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getWriteMode();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getWriteMode(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetWriteMode;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetWriteMode::_iCounter = 0;


class ExprScriptHexaToDecimal : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptHexaToDecimal() {
	}

public:
    virtual ~ExprScriptHexaToDecimal() {	}

	virtual const char* getName() const { return "hexaToDecimal"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sHexaNumber = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::hexaToDecimal(sHexaNumber);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::hexaToDecimal(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptHexaToDecimal;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptHexaToDecimal::_iCounter = 0;


class ExprScriptHostToNetworkLong : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptHostToNetworkLong() {
	}

public:
    virtual ~ExprScriptHostToNetworkLong() {	}

	virtual const char* getName() const { return "hostToNetworkLong"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sBytes = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::hostToNetworkLong(sBytes);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::hostToNetworkLong(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptHostToNetworkLong;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptHostToNetworkLong::_iCounter = 0;


class ExprScriptHostToNetworkShort : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptHostToNetworkShort() {
	}

public:
    virtual ~ExprScriptHostToNetworkShort() {	}

	virtual const char* getName() const { return "hostToNetworkShort"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sBytes = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::hostToNetworkShort(sBytes);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::hostToNetworkShort(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptHostToNetworkShort;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptHostToNetworkShort::_iCounter = 0;


class ExprScriptIncrement : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptIncrement() {
	}

public:
    virtual ~ExprScriptIncrement() {	}

	virtual const char* getName() const { return "increment"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pNumber = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		double dNumber = pNumber->getDoubleValue();
		double returnValue = CGRuntime::increment(dNumber);
		pNumber->setValue(dNumber);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::increment(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptIncrement;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptIncrement::_iCounter = 0;


class ExprScriptIndentFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptIndentFile() {
	}

public:
    virtual ~ExprScriptIndentFile() {	}

	virtual const char* getName() const { return "indentFile"; }
	virtual unsigned int getArity() const { return 2; }
	virtual unsigned int getMinArity() const { return 1; }
	virtual ExprScriptExpression* getDefaultParameter(unsigned int iIndex) const {
		static ExprScriptConstant _internalmode("");
		if (iIndex == 1) return &_internalmode;
		return NULL;
	}
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFile = (*cursor)->getValue(visibility);
		++cursor;
		std::string sMode = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::indentFile(sFile, sMode);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::indentFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptIndentFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptIndentFile::_iCounter = 0;


class ExprScriptInf : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptInf() {
	}

public:
    virtual ~ExprScriptInf() {	}

	virtual const char* getName() const { return "inf"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLeft = (*cursor)->getValue(visibility);
		double dLeft = atof(sLeft.c_str());
		++cursor;
		std::string sRight = (*cursor)->getValue(visibility);
		double dRight = atof(sRight.c_str());
		bool returnValue = CGRuntime::inf(dLeft, dRight);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::inf(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptInf;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptInf::_iCounter = 0;


class ExprScriptInputKey : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptInputKey() {
	}

public:
    virtual ~ExprScriptInputKey() {	}

	virtual const char* getName() const { return "inputKey"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sEcho = (*cursor)->getValue(visibility);
		bool bEcho = !sEcho.empty();
		std::string returnValue = CGRuntime::inputKey(bEcho);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::inputKey(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppBoolean(theCompilerEnvironment, false);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptInputKey;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptInputKey::_iCounter = 0;


class ExprScriptInputLine : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptInputLine() {
	}

public:
    virtual ~ExprScriptInputLine() {	}

	virtual const char* getName() const { return "inputLine"; }
	virtual unsigned int getArity() const { return 2; }
	virtual unsigned int getMinArity() const { return 1; }
	virtual ExprScriptExpression* getDefaultParameter(unsigned int iIndex) const {
		static ExprScriptConstant _internalprompt("");
		if (iIndex == 1) return &_internalprompt;
		return NULL;
	}
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sEcho = (*cursor)->getValue(visibility);
		bool bEcho = !sEcho.empty();
		++cursor;
		std::string sPrompt = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::inputLine(bEcho, sPrompt);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::inputLine(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppBoolean(theCompilerEnvironment, false);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptInputLine;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptInputLine::_iCounter = 0;


class ExprScriptIsEmpty : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptIsEmpty() {
	}

public:
    virtual ~ExprScriptIsEmpty() {	}

	virtual const char* getName() const { return "isEmpty"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pArray = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		bool returnValue = CGRuntime::isEmpty(pArray);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::isEmpty(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptIsEmpty;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptIsEmpty::_iCounter = 0;


class ExprScriptIsIdentifier : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptIsIdentifier() {
	}

public:
    virtual ~ExprScriptIsIdentifier() {	}

	virtual const char* getName() const { return "isIdentifier"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sIdentifier = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::isIdentifier(sIdentifier);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::isIdentifier(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptIsIdentifier;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptIsIdentifier::_iCounter = 0;


class ExprScriptIsNegative : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptIsNegative() {
	}

public:
    virtual ~ExprScriptIsNegative() {	}

	virtual const char* getName() const { return "isNegative"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sNumber = (*cursor)->getValue(visibility);
		double dNumber = atof(sNumber.c_str());
		bool returnValue = CGRuntime::isNegative(dNumber);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::isNegative(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptIsNegative;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptIsNegative::_iCounter = 0;


class ExprScriptIsNumeric : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptIsNumeric() {
	}

public:
    virtual ~ExprScriptIsNumeric() {	}

	virtual const char* getName() const { return "isNumeric"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sNumber = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::isNumeric(sNumber);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::isNumeric(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptIsNumeric;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptIsNumeric::_iCounter = 0;


class ExprScriptIsPositive : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptIsPositive() {
	}

public:
    virtual ~ExprScriptIsPositive() {	}

	virtual const char* getName() const { return "isPositive"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sNumber = (*cursor)->getValue(visibility);
		double dNumber = atof(sNumber.c_str());
		bool returnValue = CGRuntime::isPositive(dNumber);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::isPositive(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptIsPositive;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptIsPositive::_iCounter = 0;


class ExprScriptJoinStrings : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptJoinStrings() {
	}

public:
    virtual ~ExprScriptJoinStrings() {	}

	virtual const char* getName() const { return "joinStrings"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pList = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		std::string sSeparator = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::joinStrings(pList, sSeparator);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::joinStrings(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptJoinStrings;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptJoinStrings::_iCounter = 0;


class ExprScriptLeftString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptLeftString() {
	}

public:
    virtual ~ExprScriptLeftString() {	}

	virtual const char* getName() const { return "leftString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::leftString(sText, iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::leftString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptLeftString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptLeftString::_iCounter = 0;


class ExprScriptLengthString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptLengthString() {
	}

public:
    virtual ~ExprScriptLengthString() {	}

	virtual const char* getName() const { return "lengthString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::lengthString(sText);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::lengthString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptLengthString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptLengthString::_iCounter = 0;


class ExprScriptLoadBinaryFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptLoadBinaryFile() {
	}

public:
    virtual ~ExprScriptLoadBinaryFile() {	}

	virtual const char* getName() const { return "loadBinaryFile"; }
	virtual unsigned int getArity() const { return 2; }
	virtual unsigned int getMinArity() const { return 1; }
	virtual ExprScriptExpression* getDefaultParameter(unsigned int iIndex) const {
		static ExprScriptConstant _internallength(-1);
		if (iIndex == 1) return &_internallength;
		return NULL;
	}
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFile = (*cursor)->getValue(visibility);
		++cursor;
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::loadBinaryFile(sFile, iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::loadBinaryFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptLoadBinaryFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptLoadBinaryFile::_iCounter = 0;


class ExprScriptLoadFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptLoadFile() {
	}

public:
    virtual ~ExprScriptLoadFile() {	}

	virtual const char* getName() const { return "loadFile"; }
	virtual unsigned int getArity() const { return 2; }
	virtual unsigned int getMinArity() const { return 1; }
	virtual ExprScriptExpression* getDefaultParameter(unsigned int iIndex) const {
		static ExprScriptConstant _internallength(-1);
		if (iIndex == 1) return &_internallength;
		return NULL;
	}
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFile = (*cursor)->getValue(visibility);
		++cursor;
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::loadFile(sFile, iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::loadFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptLoadFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptLoadFile::_iCounter = 0;


class ExprScriptLoadVirtualFile : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptLoadVirtualFile() {
	}

public:
    virtual ~ExprScriptLoadVirtualFile() {	}

	virtual const char* getName() const { return "loadVirtualFile"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sHandle = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::loadVirtualFile(sHandle);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::loadVirtualFile(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptLoadVirtualFile;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptLoadVirtualFile::_iCounter = 0;


class ExprScriptLog : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptLog() {
	}

public:
    virtual ~ExprScriptLog() {	}

	virtual const char* getName() const { return "log"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sX = (*cursor)->getValue(visibility);
		double dX = atof(sX.c_str());
		double returnValue = CGRuntime::log(dX);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::log(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptLog;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptLog::_iCounter = 0;


class ExprScriptLongToBytes : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptLongToBytes() {
	}

public:
    virtual ~ExprScriptLongToBytes() {	}

	virtual const char* getName() const { return "longToBytes"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLong = (*cursor)->getValue(visibility);
		char* tcLong;
		unsigned long ulLong = strtoul(sLong.c_str(), &tcLong, 10);
		std::string returnValue = CGRuntime::longToBytes(ulLong);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::longToBytes(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppUnsignedLong(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptLongToBytes;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptLongToBytes::_iCounter = 0;


class ExprScriptMidString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptMidString() {
	}

public:
    virtual ~ExprScriptMidString() {	}

	virtual const char* getName() const { return "midString"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sPos = (*cursor)->getValue(visibility);
		int iPos = atoi(sPos.c_str());
		++cursor;
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::midString(sText, iPos, iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::midString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptMidString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptMidString::_iCounter = 0;


class ExprScriptMod : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptMod() {
	}

public:
    virtual ~ExprScriptMod() {	}

	virtual const char* getName() const { return "mod"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sDividend = (*cursor)->getValue(visibility);
		int iDividend = atoi(sDividend.c_str());
		++cursor;
		std::string sDivisor = (*cursor)->getValue(visibility);
		int iDivisor = atoi(sDivisor.c_str());
		int returnValue = CGRuntime::mod(iDividend, iDivisor);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::mod(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptMod;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptMod::_iCounter = 0;


class ExprScriptMult : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptMult() {
	}

public:
    virtual ~ExprScriptMult() {	}

	virtual const char* getName() const { return "mult"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLeft = (*cursor)->getValue(visibility);
		double dLeft = atof(sLeft.c_str());
		++cursor;
		std::string sRight = (*cursor)->getValue(visibility);
		double dRight = atof(sRight.c_str());
		double returnValue = CGRuntime::mult(dLeft, dRight);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::mult(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptMult;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptMult::_iCounter = 0;


class ExprScriptNetworkLongToHost : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptNetworkLongToHost() {
	}

public:
    virtual ~ExprScriptNetworkLongToHost() {	}

	virtual const char* getName() const { return "networkLongToHost"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sBytes = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::networkLongToHost(sBytes);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::networkLongToHost(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptNetworkLongToHost;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptNetworkLongToHost::_iCounter = 0;


class ExprScriptNetworkShortToHost : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptNetworkShortToHost() {
	}

public:
    virtual ~ExprScriptNetworkShortToHost() {	}

	virtual const char* getName() const { return "networkShortToHost"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sBytes = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::networkShortToHost(sBytes);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::networkShortToHost(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptNetworkShortToHost;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptNetworkShortToHost::_iCounter = 0;


class ExprScriptOctalToDecimal : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptOctalToDecimal() {
	}

public:
    virtual ~ExprScriptOctalToDecimal() {	}

	virtual const char* getName() const { return "octalToDecimal"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sOctalNumber = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::octalToDecimal(sOctalNumber);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::octalToDecimal(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptOctalToDecimal;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptOctalToDecimal::_iCounter = 0;


class ExprScriptParseFreeQuiet : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptParseFreeQuiet(GrfBlock& block) {
//##protect##"parseFreeQuiet::constructor"
		_pParseFree = new GrfParseFree;
		_pParseFree->setParent(&block);
//##protect##"parseFreeQuiet::constructor"
	}


private:
//##protect##"parseFreeQuiet::attributes"
	GrfParseFree* _pParseFree;
//##protect##"parseFreeQuiet::attributes"

public:
    virtual ~ExprScriptParseFreeQuiet() {
//##protect##"parseFreeQuiet::destructor"
		delete _pParseFree;
//##protect##"parseFreeQuiet::destructor"
	}

	virtual const char* getName() const { return "parseFreeQuiet"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual void initializationDone() {
//##protect##"parseFreeQuiet::initializationDone"
		_pParseFree->setDesignFileName(_parameters[0]);
		_pParseFree->setThis((ExprScriptVariable*) _parameters[1]);
		_pParseFree->setInputFileName(_parameters[2]);
		_parameters = std::vector<ExprScriptExpression*>();
//##protect##"parseFreeQuiet::initializationDone"
	}


    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue;
//##protect##"parseFreeQuiet::execute"
		CGQuietOutput quiet;
		_pParseFree->execute(visibility);
		returnValue = quiet.getOutput();
//##protect##"parseFreeQuiet::execute"
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
//##protect##"parseFreeQuiet::compileCppString"
		int iCursor = theCompilerEnvironment.newCursor();
		char tcFunction[80];
		sprintf(tcFunction, "_compiler_internal_parseFreeQuiet%d", iCursor);
		std::string sDefinition = "std::string ";
		sDefinition += tcFunction;
		sDefinition += "(CppParsingTree_var pThisTree, const std::string& sFilename) {\n";
		sDefinition += "\tCGQuietOutput quiet;\n";
		ScpStream* pOwner;
		CW_BODY_STREAM.insertText(sDefinition, CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner));
		CW_BODY_STREAM << tcFunction << "(";
		_pParseFree->getThis()->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		_pParseFree->getInputFileName()->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		int iLocation = CW_BODY_STREAM.getOutputLocation();
		_pParseFree->compileCppForQuiet(theCompilerEnvironment);
		sDefinition = CW_BODY_STREAM.readBuffer() + iLocation;
		CW_BODY_STREAM.resize(iLocation);
		sDefinition += "\treturn quiet.getOutput();\n}\n";
		CW_BODY_STREAM.insertText(sDefinition, CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner));
//##protect##"parseFreeQuiet::compileCppString"
		return true;
	}

	static ExprScriptFunction* create(GrfBlock& block) {
        return new ExprScriptParseFreeQuiet(block);
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptParseFreeQuiet::_iCounter = 0;


class ExprScriptPathFromPackage : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptPathFromPackage() {
	}

public:
    virtual ~ExprScriptPathFromPackage() {	}

	virtual const char* getName() const { return "pathFromPackage"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sPackage = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::pathFromPackage(sPackage);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::pathFromPackage(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptPathFromPackage;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptPathFromPackage::_iCounter = 0;


class ExprScriptPostHTTPRequest : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptPostHTTPRequest() {
	}

public:
    virtual ~ExprScriptPostHTTPRequest() {	}

	virtual const char* getName() const { return "postHTTPRequest"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		if (iIndex == 2) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sURL = (*cursor)->getValue(visibility);
		++cursor;
		DtaScriptVariable* pHTTPSession = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		DtaScriptVariable* pArguments = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string returnValue = CGRuntime::postHTTPRequest(sURL, pHTTPSession, pArguments);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::postHTTPRequest(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptPostHTTPRequest;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptPostHTTPRequest::_iCounter = 0;


class ExprScriptPow : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptPow() {
	}

public:
    virtual ~ExprScriptPow() {	}

	virtual const char* getName() const { return "pow"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sX = (*cursor)->getValue(visibility);
		double dX = atof(sX.c_str());
		++cursor;
		std::string sY = (*cursor)->getValue(visibility);
		double dY = atof(sY.c_str());
		double returnValue = CGRuntime::pow(dX, dY);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::pow(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptPow;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptPow::_iCounter = 0;


class ExprScriptRandomInteger : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRandomInteger() {
	}

public:
    virtual ~ExprScriptRandomInteger() {	}

	virtual const char* getName() const { return "randomInteger"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		int returnValue = CGRuntime::randomInteger();
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::randomInteger(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRandomInteger;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRandomInteger::_iCounter = 0;


class ExprScriptReceiveBinaryFromSocket : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReceiveBinaryFromSocket() {
	}

public:
    virtual ~ExprScriptReceiveBinaryFromSocket() {	}

	virtual const char* getName() const { return "receiveBinaryFromSocket"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sSocket = (*cursor)->getValue(visibility);
		int iSocket = atoi(sSocket.c_str());
		++cursor;
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::receiveBinaryFromSocket(iSocket, iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::receiveBinaryFromSocket(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReceiveBinaryFromSocket;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReceiveBinaryFromSocket::_iCounter = 0;


class ExprScriptReceiveFromSocket : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReceiveFromSocket() {
	}

public:
    virtual ~ExprScriptReceiveFromSocket() {	}

	virtual const char* getName() const { return "receiveFromSocket"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sSocket = (*cursor)->getValue(visibility);
		int iSocket = atoi(sSocket.c_str());
		++cursor;
		DtaScriptVariable* pIsText = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		bool bIsText = pIsText->getBooleanValue();
		std::string returnValue = CGRuntime::receiveFromSocket(iSocket, bIsText);
		pIsText->setValue(bIsText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::receiveFromSocket(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReceiveFromSocket;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReceiveFromSocket::_iCounter = 0;


class ExprScriptReceiveTextFromSocket : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReceiveTextFromSocket() {
	}

public:
    virtual ~ExprScriptReceiveTextFromSocket() {	}

	virtual const char* getName() const { return "receiveTextFromSocket"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sSocket = (*cursor)->getValue(visibility);
		int iSocket = atoi(sSocket.c_str());
		++cursor;
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::receiveTextFromSocket(iSocket, iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::receiveTextFromSocket(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReceiveTextFromSocket;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReceiveTextFromSocket::_iCounter = 0;


class ExprScriptRelativePath : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRelativePath() {
	}

public:
    virtual ~ExprScriptRelativePath() {	}

	virtual const char* getName() const { return "relativePath"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sPath = (*cursor)->getValue(visibility);
		++cursor;
		std::string sReference = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::relativePath(sPath, sReference);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::relativePath(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRelativePath;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRelativePath::_iCounter = 0;


class ExprScriptRemoveDirectory : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRemoveDirectory() {
	}

public:
    virtual ~ExprScriptRemoveDirectory() {	}

	virtual const char* getName() const { return "removeDirectory"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sPath = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::removeDirectory(sPath);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::removeDirectory(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRemoveDirectory;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRemoveDirectory::_iCounter = 0;


class ExprScriptRemoveGenerationTagsHandler : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRemoveGenerationTagsHandler() {
	}

public:
    virtual ~ExprScriptRemoveGenerationTagsHandler() {	}

	virtual const char* getName() const { return "removeGenerationTagsHandler"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sKey = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::removeGenerationTagsHandler(sKey);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::removeGenerationTagsHandler(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRemoveGenerationTagsHandler;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRemoveGenerationTagsHandler::_iCounter = 0;


class ExprScriptRepeatString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRepeatString() {
	}

public:
    virtual ~ExprScriptRepeatString() {	}

	virtual const char* getName() const { return "repeatString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sOccurrences = (*cursor)->getValue(visibility);
		int iOccurrences = atoi(sOccurrences.c_str());
		std::string returnValue = CGRuntime::repeatString(sText, iOccurrences);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::repeatString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRepeatString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRepeatString::_iCounter = 0;


class ExprScriptReplaceString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReplaceString() {
	}

public:
    virtual ~ExprScriptReplaceString() {	}

	virtual const char* getName() const { return "replaceString"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual int getThisPosition() const { return 2; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sOld = (*cursor)->getValue(visibility);
		++cursor;
		std::string sNew = (*cursor)->getValue(visibility);
		++cursor;
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::replaceString(sOld, sNew, sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::replaceString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReplaceString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReplaceString::_iCounter = 0;


class ExprScriptReplaceTabulations : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReplaceTabulations() {
	}

public:
    virtual ~ExprScriptReplaceTabulations() {	}

	virtual const char* getName() const { return "replaceTabulations"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sTab = (*cursor)->getValue(visibility);
		int iTab = atoi(sTab.c_str());
		std::string returnValue = CGRuntime::replaceTabulations(sText, iTab);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::replaceTabulations(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReplaceTabulations;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReplaceTabulations::_iCounter = 0;


class ExprScriptResolveFilePath : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptResolveFilePath() {
	}

public:
    virtual ~ExprScriptResolveFilePath() {	}

	virtual const char* getName() const { return "resolveFilePath"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sFilename = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::resolveFilePath(sFilename);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::resolveFilePath(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptResolveFilePath;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptResolveFilePath::_iCounter = 0;


class ExprScriptRightString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRightString() {
	}

public:
    virtual ~ExprScriptRightString() {	}

	virtual const char* getName() const { return "rightString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::rightString(sText, iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::rightString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRightString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRightString::_iCounter = 0;


class ExprScriptRsubString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRsubString() {
	}

public:
    virtual ~ExprScriptRsubString() {	}

	virtual const char* getName() const { return "rsubString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sPos = (*cursor)->getValue(visibility);
		int iPos = atoi(sPos.c_str());
		std::string returnValue = CGRuntime::rsubString(sText, iPos);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::rsubString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRsubString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRsubString::_iCounter = 0;


class ExprScriptScanDirectories : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptScanDirectories() {
	}

public:
    virtual ~ExprScriptScanDirectories() {	}

	virtual const char* getName() const { return "scanDirectories"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pDirectory = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		std::string sPath = (*cursor)->getValue(visibility);
		++cursor;
		std::string sPattern = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::scanDirectories(pDirectory, sPath, sPattern);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::scanDirectories(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptScanDirectories;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptScanDirectories::_iCounter = 0;


class ExprScriptScanFiles : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptScanFiles() {
	}

public:
    virtual ~ExprScriptScanFiles() {	}

	virtual const char* getName() const { return "scanFiles"; }
	virtual unsigned int getArity() const { return 4; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		if (iIndex == 3) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pFiles = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		std::string sPath = (*cursor)->getValue(visibility);
		++cursor;
		std::string sPattern = (*cursor)->getValue(visibility);
		++cursor;
		std::string sSubfolders = (*cursor)->getValue(visibility);
		bool bSubfolders = !sSubfolders.empty();
		bool returnValue = CGRuntime::scanFiles(pFiles, sPath, sPattern, bSubfolders);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::scanFiles(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppBoolean(theCompilerEnvironment, false);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptScanFiles;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptScanFiles::_iCounter = 0;


class ExprScriptSendBinaryToSocket : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSendBinaryToSocket() {
	}

public:
    virtual ~ExprScriptSendBinaryToSocket() {	}

	virtual const char* getName() const { return "sendBinaryToSocket"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sSocket = (*cursor)->getValue(visibility);
		int iSocket = atoi(sSocket.c_str());
		++cursor;
		std::string sBytes = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::sendBinaryToSocket(iSocket, sBytes);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::sendBinaryToSocket(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSendBinaryToSocket;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSendBinaryToSocket::_iCounter = 0;


class ExprScriptSendHTTPRequest : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSendHTTPRequest() {
	}

public:
    virtual ~ExprScriptSendHTTPRequest() {	}

	virtual const char* getName() const { return "sendHTTPRequest"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sURL = (*cursor)->getValue(visibility);
		++cursor;
		DtaScriptVariable* pHTTPSession = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string returnValue = CGRuntime::sendHTTPRequest(sURL, pHTTPSession);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::sendHTTPRequest(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSendHTTPRequest;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSendHTTPRequest::_iCounter = 0;


class ExprScriptSendTextToSocket : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSendTextToSocket() {
	}

public:
    virtual ~ExprScriptSendTextToSocket() {	}

	virtual const char* getName() const { return "sendTextToSocket"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sSocket = (*cursor)->getValue(visibility);
		int iSocket = atoi(sSocket.c_str());
		++cursor;
		std::string sText = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::sendTextToSocket(iSocket, sText);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::sendTextToSocket(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSendTextToSocket;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSendTextToSocket::_iCounter = 0;


class ExprScriptSelectGenerationTagsHandler : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSelectGenerationTagsHandler() {
	}

public:
    virtual ~ExprScriptSelectGenerationTagsHandler() {	}

	virtual const char* getName() const { return "selectGenerationTagsHandler"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sKey = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::selectGenerationTagsHandler(sKey);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::selectGenerationTagsHandler(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSelectGenerationTagsHandler;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSelectGenerationTagsHandler::_iCounter = 0;


class ExprScriptShortToBytes : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptShortToBytes() {
	}

public:
    virtual ~ExprScriptShortToBytes() {	}

	virtual const char* getName() const { return "shortToBytes"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sShort = (*cursor)->getValue(visibility);
		unsigned short ulShort = (unsigned short) atoi(sShort.c_str());
		std::string returnValue = CGRuntime::shortToBytes(ulShort);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::shortToBytes(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppUnsignedShort(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptShortToBytes;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptShortToBytes::_iCounter = 0;


class ExprScriptSqrt : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSqrt() {
	}

public:
    virtual ~ExprScriptSqrt() {	}

	virtual const char* getName() const { return "sqrt"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sX = (*cursor)->getValue(visibility);
		double dX = atof(sX.c_str());
		double returnValue = CGRuntime::sqrt(dX);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::sqrt(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSqrt;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSqrt::_iCounter = 0;


class ExprScriptStartString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptStartString() {
	}

public:
    virtual ~ExprScriptStartString() {	}

	virtual const char* getName() const { return "startString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sStart = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::startString(sText, sStart);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::startString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptStartString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptStartString::_iCounter = 0;


class ExprScriptSub : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSub() {
	}

public:
    virtual ~ExprScriptSub() {	}

	virtual const char* getName() const { return "sub"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLeft = (*cursor)->getValue(visibility);
		double dLeft = atof(sLeft.c_str());
		++cursor;
		std::string sRight = (*cursor)->getValue(visibility);
		double dRight = atof(sRight.c_str());
		double returnValue = CGRuntime::sub(dLeft, dRight);
		return CGRuntime::toString(returnValue);
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppDouble(theCompilerEnvironment);
		return DOUBLE_TYPE;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(int) ";
		return compileCppDouble(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppDouble(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::sub(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSub;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSub::_iCounter = 0;


class ExprScriptSubString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSubString() {
	}

public:
    virtual ~ExprScriptSubString() {	}

	virtual const char* getName() const { return "subString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		++cursor;
		std::string sPos = (*cursor)->getValue(visibility);
		int iPos = atoi(sPos.c_str());
		std::string returnValue = CGRuntime::subString(sText, iPos);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::subString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSubString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSubString::_iCounter = 0;


class ExprScriptSup : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSup() {
	}

public:
    virtual ~ExprScriptSup() {	}

	virtual const char* getName() const { return "sup"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLeft = (*cursor)->getValue(visibility);
		double dLeft = atof(sLeft.c_str());
		++cursor;
		std::string sRight = (*cursor)->getValue(visibility);
		double dRight = atof(sRight.c_str());
		bool returnValue = CGRuntime::sup(dLeft, dRight);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::sup(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppDouble(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSup;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSup::_iCounter = 0;


class ExprScriptSystem : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSystem() {
	}

public:
    virtual ~ExprScriptSystem() {	}

	virtual const char* getName() const { return "system"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sCommand = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::system(sCommand);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::system(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSystem;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSystem::_iCounter = 0;


class ExprScriptToLowerString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptToLowerString() {
	}

public:
    virtual ~ExprScriptToLowerString() {	}

	virtual const char* getName() const { return "toLowerString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::toLowerString(sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toLowerString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptToLowerString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptToLowerString::_iCounter = 0;


class ExprScriptToUpperString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptToUpperString() {
	}

public:
    virtual ~ExprScriptToUpperString() {	}

	virtual const char* getName() const { return "toUpperString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::toUpperString(sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toUpperString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptToUpperString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptToUpperString::_iCounter = 0;


class ExprScriptTranslateString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptTranslateString(GrfBlock& block) {
//##protect##"translateString::constructor"
		_pTranslateString = new GrfTranslate;
		_pTranslateString->setParent(&block);
//##protect##"translateString::constructor"
	}


private:
//##protect##"translateString::attributes"
	GrfTranslate* _pTranslateString;
//##protect##"translateString::attributes"

public:
    virtual ~ExprScriptTranslateString() {
//##protect##"translateString::destructor"
		delete _pTranslateString;
//##protect##"translateString::destructor"
	}

	virtual const char* getName() const { return "translateString"; }
	virtual unsigned int getArity() const { return 3; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return SCRIPTFILE_TRANSLATE_EXPRTYPE;
		if (iIndex == 1) return NODE_EXPRTYPE;
		if (iIndex == 2) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual void initializationDone() {
//##protect##"translateString::initializationDone"
		_pTranslateString->setPatternFileName((ExprScriptScriptFile*) _parameters[0]);
		_pTranslateString->setThis((ExprScriptVariable*) _parameters[1]);
		_pTranslateString->setInputFileName(_parameters[2]);
		_parameters = std::vector<ExprScriptExpression*>();
//##protect##"translateString::initializationDone"
	}


    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue;
//##protect##"translateString::execute"
		returnValue = _pTranslateString->translateString(visibility);
//##protect##"translateString::execute"
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
//##protect##"translateString::compileCppString"
//##protect##"translateString::compileCppString"
		return true;
	}

	static ExprScriptFunction* create(GrfBlock& block) {
        return new ExprScriptTranslateString(block);
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptTranslateString::_iCounter = 0;


class ExprScriptTrimLeft : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptTrimLeft() {
	}

public:
    virtual ~ExprScriptTrimLeft() {	}

	virtual const char* getName() const { return "trimLeft"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pString = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sString;
		const char* tsString = pString->getValue();
		if (tsString != NULL) sString = tsString;
		int returnValue = CGRuntime::trimLeft(sString);
		pString->setValue(sString.c_str());
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::trimLeft(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptTrimLeft;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptTrimLeft::_iCounter = 0;


class ExprScriptTrimRight : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptTrimRight() {
	}

public:
    virtual ~ExprScriptTrimRight() {	}

	virtual const char* getName() const { return "trimRight"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pString = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sString;
		const char* tsString = pString->getValue();
		if (tsString != NULL) sString = tsString;
		int returnValue = CGRuntime::trimRight(sString);
		pString->setValue(sString.c_str());
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::trimRight(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptTrimRight;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptTrimRight::_iCounter = 0;


class ExprScriptTrim : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptTrim() {
	}

public:
    virtual ~ExprScriptTrim() {	}

	virtual const char* getName() const { return "trim"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pString = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sString;
		const char* tsString = pString->getValue();
		if (tsString != NULL) sString = tsString;
		int returnValue = CGRuntime::trim(sString);
		pString->setValue(sString.c_str());
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::trim(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptTrim;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptTrim::_iCounter = 0;


class ExprScriptTruncateAfterString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptTruncateAfterString() {
	}

public:
    virtual ~ExprScriptTruncateAfterString() {	}

	virtual const char* getName() const { return "truncateAfterString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pVariable = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::truncateAfterString(pVariable, sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::truncateAfterString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptTruncateAfterString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptTruncateAfterString::_iCounter = 0;


class ExprScriptTruncateBeforeString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptTruncateBeforeString() {
	}

public:
    virtual ~ExprScriptTruncateBeforeString() {	}

	virtual const char* getName() const { return "truncateBeforeString"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pVariable = visibility.getExistingVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		++cursor;
		std::string sText = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::truncateBeforeString(pVariable, sText);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::truncateBeforeString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptTruncateBeforeString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptTruncateBeforeString::_iCounter = 0;


class ExprScriptUUID : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptUUID() {
	}

public:
    virtual ~ExprScriptUUID() {	}

	virtual const char* getName() const { return "UUID"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::UUID();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::UUID(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptUUID;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptUUID::_iCounter = 0;


class ExprScriptCountInputCols : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCountInputCols() {
	}

public:
    virtual ~ExprScriptCountInputCols() {	}

	virtual const char* getName() const { return "countInputCols"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		int returnValue = CGRuntime::countInputCols();
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::countInputCols(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCountInputCols;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCountInputCols::_iCounter = 0;


class ExprScriptCountInputLines : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCountInputLines() {
	}

public:
    virtual ~ExprScriptCountInputLines() {	}

	virtual const char* getName() const { return "countInputLines"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		int returnValue = CGRuntime::countInputLines();
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::countInputLines(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCountInputLines;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCountInputLines::_iCounter = 0;


class ExprScriptGetInputFilename : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetInputFilename() {
	}

public:
    virtual ~ExprScriptGetInputFilename() {	}

	virtual const char* getName() const { return "getInputFilename"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getInputFilename();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getInputFilename(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetInputFilename;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetInputFilename::_iCounter = 0;


class ExprScriptGetLastReadChars : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetLastReadChars() {
	}

public:
    virtual ~ExprScriptGetLastReadChars() {	}

	virtual const char* getName() const { return "getLastReadChars"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::getLastReadChars(iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getLastReadChars(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetLastReadChars;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetLastReadChars::_iCounter = 0;


class ExprScriptGetInputLocation : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetInputLocation() {
	}

public:
    virtual ~ExprScriptGetInputLocation() {	}

	virtual const char* getName() const { return "getInputLocation"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		int returnValue = CGRuntime::getInputLocation();
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getInputLocation(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetInputLocation;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetInputLocation::_iCounter = 0;


class ExprScriptLookAhead : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptLookAhead() {
	}

public:
    virtual ~ExprScriptLookAhead() {	}

	virtual const char* getName() const { return "lookAhead"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::lookAhead(sText);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::lookAhead(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptLookAhead;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptLookAhead::_iCounter = 0;


class ExprScriptPeekChar : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptPeekChar() {
	}

public:
    virtual ~ExprScriptPeekChar() {	}

	virtual const char* getName() const { return "peekChar"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::peekChar();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::peekChar(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptPeekChar;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptPeekChar::_iCounter = 0;


class ExprScriptReadAdaString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadAdaString() {
	}

public:
    virtual ~ExprScriptReadAdaString() {	}

	virtual const char* getName() const { return "readAdaString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pText = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sText;
		const char* tsText = pText->getValue();
		if (tsText != NULL) sText = tsText;
		bool returnValue = CGRuntime::readAdaString(sText);
		pText->setValue(sText.c_str());
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::readAdaString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadAdaString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadAdaString::_iCounter = 0;


class ExprScriptReadByte : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadByte() {
	}

public:
    virtual ~ExprScriptReadByte() {	}

	virtual const char* getName() const { return "readByte"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::readByte();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::readByte(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadByte;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadByte::_iCounter = 0;


class ExprScriptReadBytes : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadBytes() {
	}

public:
    virtual ~ExprScriptReadBytes() {	}

	virtual const char* getName() const { return "readBytes"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::readBytes(iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::readBytes(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadBytes;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadBytes::_iCounter = 0;


class ExprScriptReadCChar : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadCChar() {
	}

public:
    virtual ~ExprScriptReadCChar() {	}

	virtual const char* getName() const { return "readCChar"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::readCChar();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::readCChar(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadCChar;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadCChar::_iCounter = 0;


class ExprScriptReadChar : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadChar() {
	}

public:
    virtual ~ExprScriptReadChar() {	}

	virtual const char* getName() const { return "readChar"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::readChar();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::readChar(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadChar;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadChar::_iCounter = 0;


class ExprScriptReadCharAsInt : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadCharAsInt() {
	}

public:
    virtual ~ExprScriptReadCharAsInt() {	}

	virtual const char* getName() const { return "readCharAsInt"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		int returnValue = CGRuntime::readCharAsInt();
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::readCharAsInt(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadCharAsInt;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadCharAsInt::_iCounter = 0;


class ExprScriptReadChars : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadChars() {
	}

public:
    virtual ~ExprScriptReadChars() {	}

	virtual const char* getName() const { return "readChars"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLength = (*cursor)->getValue(visibility);
		int iLength = atoi(sLength.c_str());
		std::string returnValue = CGRuntime::readChars(iLength);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::readChars(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadChars;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadChars::_iCounter = 0;


class ExprScriptReadIdentifier : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadIdentifier() {
	}

public:
    virtual ~ExprScriptReadIdentifier() {	}

	virtual const char* getName() const { return "readIdentifier"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::readIdentifier();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::readIdentifier(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadIdentifier;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadIdentifier::_iCounter = 0;


class ExprScriptReadIfEqualTo : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadIfEqualTo() {
	}

public:
    virtual ~ExprScriptReadIfEqualTo() {	}

	virtual const char* getName() const { return "readIfEqualTo"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::readIfEqualTo(sText);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::readIfEqualTo(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadIfEqualTo;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadIfEqualTo::_iCounter = 0;


class ExprScriptReadIfEqualToIgnoreCase : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadIfEqualToIgnoreCase() {
	}

public:
    virtual ~ExprScriptReadIfEqualToIgnoreCase() {	}

	virtual const char* getName() const { return "readIfEqualToIgnoreCase"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::readIfEqualToIgnoreCase(sText);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::readIfEqualToIgnoreCase(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadIfEqualToIgnoreCase;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadIfEqualToIgnoreCase::_iCounter = 0;


class ExprScriptReadIfEqualToIdentifier : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadIfEqualToIdentifier() {
	}

public:
    virtual ~ExprScriptReadIfEqualToIdentifier() {	}

	virtual const char* getName() const { return "readIfEqualToIdentifier"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sIdentifier = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::readIfEqualToIdentifier(sIdentifier);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::readIfEqualToIdentifier(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadIfEqualToIdentifier;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadIfEqualToIdentifier::_iCounter = 0;


class ExprScriptReadLine : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadLine() {
	}

public:
    virtual ~ExprScriptReadLine() {	}

	virtual const char* getName() const { return "readLine"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pText = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sText;
		const char* tsText = pText->getValue();
		if (tsText != NULL) sText = tsText;
		bool returnValue = CGRuntime::readLine(sText);
		pText->setValue(sText.c_str());
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::readLine(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadLine;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadLine::_iCounter = 0;


class ExprScriptReadNextText : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadNextText() {
	}

public:
    virtual ~ExprScriptReadNextText() {	}

	virtual const char* getName() const { return "readNextText"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::readNextText(sText);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::readNextText(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadNextText;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadNextText::_iCounter = 0;


class ExprScriptReadNumber : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadNumber() {
	}

public:
    virtual ~ExprScriptReadNumber() {	}

	virtual const char* getName() const { return "readNumber"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pNumber = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		double dNumber = pNumber->getDoubleValue();
		bool returnValue = CGRuntime::readNumber(dNumber);
		pNumber->setValue(dNumber);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::readNumber(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadNumber;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadNumber::_iCounter = 0;


class ExprScriptReadPythonString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadPythonString() {
	}

public:
    virtual ~ExprScriptReadPythonString() {	}

	virtual const char* getName() const { return "readPythonString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pText = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sText;
		const char* tsText = pText->getValue();
		if (tsText != NULL) sText = tsText;
		bool returnValue = CGRuntime::readPythonString(sText);
		pText->setValue(sText.c_str());
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::readPythonString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadPythonString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadPythonString::_iCounter = 0;


class ExprScriptReadString : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadString() {
	}

public:
    virtual ~ExprScriptReadString() {	}

	virtual const char* getName() const { return "readString"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pText = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		std::string sText;
		const char* tsText = pText->getValue();
		if (tsText != NULL) sText = tsText;
		bool returnValue = CGRuntime::readString(sText);
		pText->setValue(sText.c_str());
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::readString(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		dynamic_cast<ExprScriptVariable*>(*cursor)->compileCppForSet(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadString;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadString::_iCounter = 0;


class ExprScriptReadUptoJustOneChar : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadUptoJustOneChar() {
	}

public:
    virtual ~ExprScriptReadUptoJustOneChar() {	}

	virtual const char* getName() const { return "readUptoJustOneChar"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sOneAmongChars = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::readUptoJustOneChar(sOneAmongChars);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::readUptoJustOneChar(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadUptoJustOneChar;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadUptoJustOneChar::_iCounter = 0;


class ExprScriptReadWord : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptReadWord() {
	}

public:
    virtual ~ExprScriptReadWord() {	}

	virtual const char* getName() const { return "readWord"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::readWord();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::readWord(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptReadWord;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptReadWord::_iCounter = 0;


class ExprScriptSkipBlanks : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSkipBlanks() {
	}

public:
    virtual ~ExprScriptSkipBlanks() {	}

	virtual const char* getName() const { return "skipBlanks"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		bool returnValue = CGRuntime::skipBlanks();
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::skipBlanks(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSkipBlanks;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSkipBlanks::_iCounter = 0;


class ExprScriptSkipSpaces : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSkipSpaces() {
	}

public:
    virtual ~ExprScriptSkipSpaces() {	}

	virtual const char* getName() const { return "skipSpaces"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		bool returnValue = CGRuntime::skipSpaces();
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::skipSpaces(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSkipSpaces;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSkipSpaces::_iCounter = 0;


class ExprScriptSkipEmptyCpp : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSkipEmptyCpp() {
	}

public:
    virtual ~ExprScriptSkipEmptyCpp() {	}

	virtual const char* getName() const { return "skipEmptyCpp"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		bool returnValue = CGRuntime::skipEmptyCpp();
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::skipEmptyCpp(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSkipEmptyCpp;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSkipEmptyCpp::_iCounter = 0;


class ExprScriptSkipEmptyCppExceptDoxygen : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSkipEmptyCppExceptDoxygen() {
	}

public:
    virtual ~ExprScriptSkipEmptyCppExceptDoxygen() {	}

	virtual const char* getName() const { return "skipEmptyCppExceptDoxygen"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		bool returnValue = CGRuntime::skipEmptyCppExceptDoxygen();
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::skipEmptyCppExceptDoxygen(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSkipEmptyCppExceptDoxygen;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSkipEmptyCppExceptDoxygen::_iCounter = 0;


class ExprScriptSkipEmptyHTML : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSkipEmptyHTML() {
	}

public:
    virtual ~ExprScriptSkipEmptyHTML() {	}

	virtual const char* getName() const { return "skipEmptyHTML"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		bool returnValue = CGRuntime::skipEmptyHTML();
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::skipEmptyHTML(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSkipEmptyHTML;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSkipEmptyHTML::_iCounter = 0;


class ExprScriptSkipEmptyLaTeX : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptSkipEmptyLaTeX() {
	}

public:
    virtual ~ExprScriptSkipEmptyLaTeX() {	}

	virtual const char* getName() const { return "skipEmptyLaTeX"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAParseFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		bool returnValue = CGRuntime::skipEmptyLaTeX();
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::skipEmptyLaTeX(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptSkipEmptyLaTeX;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptSkipEmptyLaTeX::_iCounter = 0;


class ExprScriptCountOutputCols : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCountOutputCols() {
	}

public:
    virtual ~ExprScriptCountOutputCols() {	}

	virtual const char* getName() const { return "countOutputCols"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		int returnValue = CGRuntime::countOutputCols();
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::countOutputCols(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCountOutputCols;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCountOutputCols::_iCounter = 0;


class ExprScriptCountOutputLines : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptCountOutputLines() {
	}

public:
    virtual ~ExprScriptCountOutputLines() {	}

	virtual const char* getName() const { return "countOutputLines"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		int returnValue = CGRuntime::countOutputLines();
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::countOutputLines(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptCountOutputLines;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptCountOutputLines::_iCounter = 0;


class ExprScriptDecrementIndentLevel : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptDecrementIndentLevel() {
	}

public:
    virtual ~ExprScriptDecrementIndentLevel() {	}

	virtual const char* getName() const { return "decrementIndentLevel"; }
	virtual unsigned int getArity() const { return 1; }
	virtual unsigned int getMinArity() const { return 0; }
	virtual ExprScriptExpression* getDefaultParameter(unsigned int iIndex) const {
		static ExprScriptConstant _internallevel(1);
		if (iIndex == 0) return &_internallevel;
		return NULL;
	}
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sLevel = (*cursor)->getValue(visibility);
		int iLevel = atoi(sLevel.c_str());
		bool returnValue = CGRuntime::decrementIndentLevel(iLevel);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::decrementIndentLevel(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptDecrementIndentLevel;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptDecrementIndentLevel::_iCounter = 0;


class ExprScriptEqualLastWrittenChars : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptEqualLastWrittenChars() {
	}

public:
    virtual ~ExprScriptEqualLastWrittenChars() {	}

	virtual const char* getName() const { return "equalLastWrittenChars"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sText = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::equalLastWrittenChars(sText);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::equalLastWrittenChars(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptEqualLastWrittenChars;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptEqualLastWrittenChars::_iCounter = 0;


class ExprScriptExistFloatingLocation : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptExistFloatingLocation() {
	}

public:
    virtual ~ExprScriptExistFloatingLocation() {	}

	virtual const char* getName() const { return "existFloatingLocation"; }
	virtual unsigned int getArity() const { return 2; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		if (iIndex == 1) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sKey = (*cursor)->getValue(visibility);
		++cursor;
		std::string sParent = (*cursor)->getValue(visibility);
		bool bParent = !sParent.empty();
		bool returnValue = CGRuntime::existFloatingLocation(sKey, bParent);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::existFloatingLocation(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ", ";
		++cursor;
		(*cursor)->compileCppBoolean(theCompilerEnvironment, false);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptExistFloatingLocation;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptExistFloatingLocation::_iCounter = 0;


class ExprScriptGetFloatingLocation : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetFloatingLocation() {
	}

public:
    virtual ~ExprScriptGetFloatingLocation() {	}

	virtual const char* getName() const { return "getFloatingLocation"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sKey = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::getFloatingLocation(sKey);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getFloatingLocation(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetFloatingLocation;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetFloatingLocation::_iCounter = 0;


class ExprScriptGetLastWrittenChars : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetLastWrittenChars() {
	}

public:
    virtual ~ExprScriptGetLastWrittenChars() {	}

	virtual const char* getName() const { return "getLastWrittenChars"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sNbChars = (*cursor)->getValue(visibility);
		int iNbChars = atoi(sNbChars.c_str());
		std::string returnValue = CGRuntime::getLastWrittenChars(iNbChars);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getLastWrittenChars(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppInt(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetLastWrittenChars;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetLastWrittenChars::_iCounter = 0;


class ExprScriptGetMarkupKey : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetMarkupKey() {
	}

public:
    virtual ~ExprScriptGetMarkupKey() {	}

	virtual const char* getName() const { return "getMarkupKey"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getMarkupKey();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getMarkupKey(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetMarkupKey;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetMarkupKey::_iCounter = 0;


class ExprScriptGetMarkupValue : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetMarkupValue() {
	}

public:
    virtual ~ExprScriptGetMarkupValue() {	}

	virtual const char* getName() const { return "getMarkupValue"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getMarkupValue();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getMarkupValue(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetMarkupValue;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetMarkupValue::_iCounter = 0;


class ExprScriptGetOutputFilename : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetOutputFilename() {
	}

public:
    virtual ~ExprScriptGetOutputFilename() {	}

	virtual const char* getName() const { return "getOutputFilename"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::string returnValue = CGRuntime::getOutputFilename();
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getOutputFilename(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetOutputFilename;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetOutputFilename::_iCounter = 0;


class ExprScriptGetOutputLocation : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetOutputLocation() {
	}

public:
    virtual ~ExprScriptGetOutputLocation() {	}

	virtual const char* getName() const { return "getOutputLocation"; }
	virtual unsigned int getArity() const { return 0; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		int returnValue = CGRuntime::getOutputLocation();
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getOutputLocation(";
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetOutputLocation;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetOutputLocation::_iCounter = 0;


class ExprScriptGetProtectedArea : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetProtectedArea() {
	}

public:
    virtual ~ExprScriptGetProtectedArea() {	}

	virtual const char* getName() const { return "getProtectedArea"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sProtection = (*cursor)->getValue(visibility);
		std::string returnValue = CGRuntime::getProtectedArea(sProtection);
		return returnValue;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppString(theCompilerEnvironment);
		return STRING_TYPE;
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getProtectedArea(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetProtectedArea;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetProtectedArea::_iCounter = 0;


class ExprScriptGetProtectedAreaKeys : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptGetProtectedAreaKeys() {
	}

public:
    virtual ~ExprScriptGetProtectedAreaKeys() {	}

	virtual const char* getName() const { return "getProtectedAreaKeys"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pKeys = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		int returnValue = CGRuntime::getProtectedAreaKeys(pKeys);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::getProtectedAreaKeys(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptGetProtectedAreaKeys;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptGetProtectedAreaKeys::_iCounter = 0;


class ExprScriptIndentText : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptIndentText() {
	}

public:
    virtual ~ExprScriptIndentText() {	}

	virtual const char* getName() const { return "indentText"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sMode = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::indentText(sMode);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::indentText(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptIndentText;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptIndentText::_iCounter = 0;


class ExprScriptNewFloatingLocation : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptNewFloatingLocation() {
	}

public:
    virtual ~ExprScriptNewFloatingLocation() {	}

	virtual const char* getName() const { return "newFloatingLocation"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sKey = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::newFloatingLocation(sKey);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::newFloatingLocation(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptNewFloatingLocation;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptNewFloatingLocation::_iCounter = 0;


class ExprScriptRemainingProtectedAreas : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRemainingProtectedAreas() {
	}

public:
    virtual ~ExprScriptRemainingProtectedAreas() {	}

	virtual const char* getName() const { return "remainingProtectedAreas"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return NODE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		DtaScriptVariable* pKeys = visibility.getVariable(*dynamic_cast<ExprScriptVariable*>(*cursor));
		int returnValue = CGRuntime::remainingProtectedAreas(pKeys);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::remainingProtectedAreas(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRemainingProtectedAreas;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRemainingProtectedAreas::_iCounter = 0;


class ExprScriptRemoveFloatingLocation : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRemoveFloatingLocation() {
	}

public:
    virtual ~ExprScriptRemoveFloatingLocation() {	}

	virtual const char* getName() const { return "removeFloatingLocation"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sKey = (*cursor)->getValue(visibility);
		int returnValue = CGRuntime::removeFloatingLocation(sKey);
		char tcNumber[16];
		sprintf(tcNumber, "%d", returnValue);
		return tcNumber;
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppInt(theCompilerEnvironment);
		return INT_TYPE;
	}

	virtual bool compileCppDouble(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "(double) ";
		return compileCppInt(theCompilerEnvironment);
	}

	virtual bool compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::toString(";
		bool bSuccess = compileCppInt(theCompilerEnvironment);
		if (bSuccess) CW_BODY_STREAM << ")";
		return bSuccess;
	}

	virtual bool compileCppInt(CppCompilerEnvironment& theCompilerEnvironment) const {
		CW_BODY_STREAM << "CGRuntime::removeFloatingLocation(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRemoveFloatingLocation;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRemoveFloatingLocation::_iCounter = 0;


class ExprScriptRemoveProtectedArea : public ExprScriptFunction {
private:
	static unsigned int _iCounter;
    ExprScriptRemoveProtectedArea() {
	}

public:
    virtual ~ExprScriptRemoveProtectedArea() {	}

	virtual const char* getName() const { return "removeProtectedArea"; }
	virtual unsigned int getArity() const { return 1; }
	virtual EXPRESSION_TYPE getParameterType(unsigned int iIndex) const {
		if (iIndex == 0) return VALUE_EXPRTYPE;
		return UNKNOWN_EXPRTYPE;
	}
	virtual bool isAGenerateFunction() const { return true; }

    virtual std::string getValue(DtaScriptVariable& visibility) const {
		_iCounter++;
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		std::string sProtectedAreaName = (*cursor)->getValue(visibility);
		bool returnValue = CGRuntime::removeProtectedArea(sProtectedAreaName);
		return (returnValue ? "true" : "");
    }

	virtual EXPRESSION_RETURN_TYPE compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		compileCppBoolean(theCompilerEnvironment, false);
		return BOOL_TYPE;
	}

	virtual bool compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (bNegative) CW_BODY_STREAM << "!";
		CW_BODY_STREAM << "CGRuntime::removeProtectedArea(";
		std::vector<ExprScriptExpression*>::const_iterator cursor = _parameters.begin();
		(*cursor)->compileCppString(theCompilerEnvironment);
		CW_BODY_STREAM << ")";
		return true;
	}

	static ExprScriptFunction* create(GrfBlock&) {
        return new ExprScriptRemoveProtectedArea;
	}

	static void registerFunction(DtaFunctionInfo& info) {
		info.constructor = create;
		info.pCounter = &_iCounter;
	}
};
unsigned int ExprScriptRemoveProtectedArea::_iCounter = 0;

//##end##"declarations"



	ExprScriptFunction::ExprScriptFunction(GrfFunction* pPrototype) : _pPrototype(pPrototype), _pTemplate(NULL), _bIsExternal(false) {
	}

	ExprScriptFunction::~ExprScriptFunction() {
		clearParameters();
		delete _pTemplate;
	}

	void ExprScriptFunction::clearParameters() {
		for (std::vector<ExprScriptExpression*>::iterator i = _parameters.begin(); i != _parameters.end(); i++) delete (*i);
		_parameters = std::vector<ExprScriptExpression*>();
	}

std::map<std::string, DtaFunctionInfo>& ExprScriptFunction::getFunctionRegister() {
	static std::map<std::string, DtaFunctionInfo> _functionRegister;
	static bool _bInitialization = false;
	if (!_bInitialization) {
		_bInitialization = true;
//##markup##"registration"
//##begin##"registration"
		ExprScriptFlushOutputToSocket::registerFunction(_functionRegister["flushOutputToSocket"]);
		ExprScriptAcceptSocket::registerFunction(_functionRegister["acceptSocket"]);
		ExprScriptAdd::registerFunction(_functionRegister["add"]);
		ExprScriptAddGenerationTagsHandler::registerFunction(_functionRegister["addGenerationTagsHandler"]);
		ExprScriptAddToDate::registerFunction(_functionRegister["addToDate"]);
		ExprScriptByteToChar::registerFunction(_functionRegister["byteToChar"]);
		ExprScriptBytesToLong::registerFunction(_functionRegister["bytesToLong"]);
		ExprScriptBytesToShort::registerFunction(_functionRegister["bytesToShort"]);
		ExprScriptCanonizePath::registerFunction(_functionRegister["canonizePath"]);
		ExprScriptChangeDirectory::registerFunction(_functionRegister["changeDirectory"]);
		ExprScriptChangeFileTime::registerFunction(_functionRegister["changeFileTime"]);
		ExprScriptCharAt::registerFunction(_functionRegister["charAt"]);
		ExprScriptCharToByte::registerFunction(_functionRegister["charToByte"]);
		ExprScriptCharToInt::registerFunction(_functionRegister["charToInt"]);
		ExprScriptChmod::registerFunction(_functionRegister["chmod"]);
		ExprScriptCeil::registerFunction(_functionRegister["ceil"]);
		ExprScriptCompareDate::registerFunction(_functionRegister["compareDate"]);
		ExprScriptCompleteDate::registerFunction(_functionRegister["completeDate"]);
		ExprScriptCompleteLeftSpaces::registerFunction(_functionRegister["completeLeftSpaces"]);
		ExprScriptCompleteRightSpaces::registerFunction(_functionRegister["completeRightSpaces"]);
		ExprScriptComposeAdaLikeString::registerFunction(_functionRegister["composeAdaLikeString"]);
		ExprScriptComposeCLikeString::registerFunction(_functionRegister["composeCLikeString"]);
		ExprScriptComposeHTMLLikeString::registerFunction(_functionRegister["composeHTMLLikeString"]);
		ExprScriptComposeSQLLikeString::registerFunction(_functionRegister["composeSQLLikeString"]);
		ExprScriptComputeMD5::registerFunction(_functionRegister["computeMD5"]);
		ExprScriptCopySmartFile::registerFunction(_functionRegister["copySmartFile"]);
		ExprScriptCoreString::registerFunction(_functionRegister["coreString"]);
		ExprScriptCountStringOccurences::registerFunction(_functionRegister["countStringOccurences"]);
		ExprScriptCreateDirectory::registerFunction(_functionRegister["createDirectory"]);
		ExprScriptCreateINETClientSocket::registerFunction(_functionRegister["createINETClientSocket"]);
		ExprScriptCreateINETServerSocket::registerFunction(_functionRegister["createINETServerSocket"]);
		ExprScriptCreateIterator::registerFunction(_functionRegister["createIterator"]);
		ExprScriptCreateReverseIterator::registerFunction(_functionRegister["createReverseIterator"]);
		ExprScriptCreateVirtualFile::registerFunction(_functionRegister["createVirtualFile"]);
		ExprScriptCreateVirtualTemporaryFile::registerFunction(_functionRegister["createVirtualTemporaryFile"]);
		ExprScriptDecodeURL::registerFunction(_functionRegister["decodeURL"]);
		ExprScriptDecrement::registerFunction(_functionRegister["decrement"]);
		ExprScriptDeleteFile::registerFunction(_functionRegister["deleteFile"]);
		ExprScriptDeleteVirtualFile::registerFunction(_functionRegister["deleteVirtualFile"]);
		ExprScriptDiv::registerFunction(_functionRegister["div"]);
		ExprScriptDuplicateIterator::registerFunction(_functionRegister["duplicateIterator"]);
		ExprScriptEncodeURL::registerFunction(_functionRegister["encodeURL"]);
		ExprScriptEndl::registerFunction(_functionRegister["endl"]);
		ExprScriptEndString::registerFunction(_functionRegister["endString"]);
		ExprScriptEqual::registerFunction(_functionRegister["equal"]);
		ExprScriptEqualsIgnoreCase::registerFunction(_functionRegister["equalsIgnoreCase"]);
		ExprScriptEqualTrees::registerFunction(_functionRegister["equalTrees"]);
		ExprScriptExecuteStringQuiet::registerFunction(_functionRegister["executeStringQuiet"]);
		ExprScriptExistDirectory::registerFunction(_functionRegister["existDirectory"]);
		ExprScriptExistEnv::registerFunction(_functionRegister["existEnv"]);
		ExprScriptExistFile::registerFunction(_functionRegister["existFile"]);
		ExprScriptExistVirtualFile::registerFunction(_functionRegister["existVirtualFile"]);
		ExprScriptExistVariable::registerFunction(_functionRegister["existVariable"]);
		ExprScriptExp::registerFunction(_functionRegister["exp"]);
		ExprScriptExploreDirectory::registerFunction(_functionRegister["exploreDirectory"]);
		ExprScriptExtractGenerationHeader::registerFunction(_functionRegister["extractGenerationHeader"]);
		ExprScriptFileCreation::registerFunction(_functionRegister["fileCreation"]);
		ExprScriptFileLastAccess::registerFunction(_functionRegister["fileLastAccess"]);
		ExprScriptFileLastModification::registerFunction(_functionRegister["fileLastModification"]);
		ExprScriptFileLines::registerFunction(_functionRegister["fileLines"]);
		ExprScriptFileMode::registerFunction(_functionRegister["fileMode"]);
		ExprScriptFileSize::registerFunction(_functionRegister["fileSize"]);
		ExprScriptFindElement::registerFunction(_functionRegister["findElement"]);
		ExprScriptFindFirstChar::registerFunction(_functionRegister["findFirstChar"]);
		ExprScriptFindFirstSubstringIntoKeys::registerFunction(_functionRegister["findFirstSubstringIntoKeys"]);
		ExprScriptFindLastString::registerFunction(_functionRegister["findLastString"]);
		ExprScriptFindNextString::registerFunction(_functionRegister["findNextString"]);
		ExprScriptFindNextSubstringIntoKeys::registerFunction(_functionRegister["findNextSubstringIntoKeys"]);
		ExprScriptFindString::registerFunction(_functionRegister["findString"]);
		ExprScriptFirst::registerFunction(_functionRegister["first"]);
		ExprScriptFloor::registerFunction(_functionRegister["floor"]);
		ExprScriptFormatDate::registerFunction(_functionRegister["formatDate"]);
		ExprScriptGetArraySize::registerFunction(_functionRegister["getArraySize"]);
		ExprScriptGetCommentBegin::registerFunction(_functionRegister["getCommentBegin"]);
		ExprScriptGetCommentEnd::registerFunction(_functionRegister["getCommentEnd"]);
		ExprScriptGetCurrentDirectory::registerFunction(_functionRegister["getCurrentDirectory"]);
		ExprScriptGetEnv::registerFunction(_functionRegister["getEnv"]);
		ExprScriptGetGenerationHeader::registerFunction(_functionRegister["getGenerationHeader"]);
		ExprScriptGetHTTPRequest::registerFunction(_functionRegister["getHTTPRequest"]);
		ExprScriptGetIncludePath::registerFunction(_functionRegister["getIncludePath"]);
		ExprScriptGetLastDelay::registerFunction(_functionRegister["getLastDelay"]);
		ExprScriptGetNow::registerFunction(_functionRegister["getNow"]);
		ExprScriptGetProperty::registerFunction(_functionRegister["getProperty"]);
		ExprScriptGetShortFilename::registerFunction(_functionRegister["getShortFilename"]);
		ExprScriptGetTextMode::registerFunction(_functionRegister["getTextMode"]);
		ExprScriptGetVariableAttributes::registerFunction(_functionRegister["getVariableAttributes"]);
		ExprScriptGetVersion::registerFunction(_functionRegister["getVersion"]);
		ExprScriptGetWorkingPath::registerFunction(_functionRegister["getWorkingPath"]);
		ExprScriptGetWriteMode::registerFunction(_functionRegister["getWriteMode"]);
		ExprScriptHexaToDecimal::registerFunction(_functionRegister["hexaToDecimal"]);
		ExprScriptHostToNetworkLong::registerFunction(_functionRegister["hostToNetworkLong"]);
		ExprScriptHostToNetworkShort::registerFunction(_functionRegister["hostToNetworkShort"]);
		ExprScriptIncrement::registerFunction(_functionRegister["increment"]);
		ExprScriptIndentFile::registerFunction(_functionRegister["indentFile"]);
		ExprScriptIndex::registerFunction(_functionRegister["index"]);
		ExprScriptInf::registerFunction(_functionRegister["inf"]);
		ExprScriptInputKey::registerFunction(_functionRegister["inputKey"]);
		ExprScriptInputLine::registerFunction(_functionRegister["inputLine"]);
		ExprScriptIsEmpty::registerFunction(_functionRegister["isEmpty"]);
		ExprScriptIsIdentifier::registerFunction(_functionRegister["isIdentifier"]);
		ExprScriptIsNegative::registerFunction(_functionRegister["isNegative"]);
		ExprScriptIsNumeric::registerFunction(_functionRegister["isNumeric"]);
		ExprScriptIsPositive::registerFunction(_functionRegister["isPositive"]);
		ExprScriptJoinStrings::registerFunction(_functionRegister["joinStrings"]);
		ExprScriptKey::registerFunction(_functionRegister["key"]);
		ExprScriptLast::registerFunction(_functionRegister["last"]);
		ExprScriptLeftString::registerFunction(_functionRegister["leftString"]);
		ExprScriptLengthString::registerFunction(_functionRegister["lengthString"]);
		ExprScriptLoadBinaryFile::registerFunction(_functionRegister["loadBinaryFile"]);
		ExprScriptLoadFile::registerFunction(_functionRegister["loadFile"]);
		ExprScriptLoadVirtualFile::registerFunction(_functionRegister["loadVirtualFile"]);
		ExprScriptLog::registerFunction(_functionRegister["log"]);
		ExprScriptLongToBytes::registerFunction(_functionRegister["longToBytes"]);
		ExprScriptMidString::registerFunction(_functionRegister["midString"]);
		ExprScriptMod::registerFunction(_functionRegister["mod"]);
		ExprScriptMult::registerFunction(_functionRegister["mult"]);
		ExprScriptNetworkLongToHost::registerFunction(_functionRegister["networkLongToHost"]);
		ExprScriptNetworkShortToHost::registerFunction(_functionRegister["networkShortToHost"]);
		ExprScriptNext::registerFunction(_functionRegister["next"]);
		ExprScriptNot::registerFunction(_functionRegister["not"]);
		ExprScriptOctalToDecimal::registerFunction(_functionRegister["octalToDecimal"]);
		ExprScriptParseFreeQuiet::registerFunction(_functionRegister["parseFreeQuiet"]);
		ExprScriptPathFromPackage::registerFunction(_functionRegister["pathFromPackage"]);
		ExprScriptPostHTTPRequest::registerFunction(_functionRegister["postHTTPRequest"]);
		ExprScriptPow::registerFunction(_functionRegister["pow"]);
		ExprScriptPrec::registerFunction(_functionRegister["prec"]);
		ExprScriptRandomInteger::registerFunction(_functionRegister["randomInteger"]);
		ExprScriptReceiveBinaryFromSocket::registerFunction(_functionRegister["receiveBinaryFromSocket"]);
		ExprScriptReceiveFromSocket::registerFunction(_functionRegister["receiveFromSocket"]);
		ExprScriptReceiveTextFromSocket::registerFunction(_functionRegister["receiveTextFromSocket"]);
		ExprScriptRelativePath::registerFunction(_functionRegister["relativePath"]);
		ExprScriptRemoveDirectory::registerFunction(_functionRegister["removeDirectory"]);
		ExprScriptRemoveGenerationTagsHandler::registerFunction(_functionRegister["removeGenerationTagsHandler"]);
		ExprScriptRepeatString::registerFunction(_functionRegister["repeatString"]);
		ExprScriptReplaceString::registerFunction(_functionRegister["replaceString"]);
		ExprScriptReplaceTabulations::registerFunction(_functionRegister["replaceTabulations"]);
		ExprScriptResolveFilePath::registerFunction(_functionRegister["resolveFilePath"]);
		ExprScriptRightString::registerFunction(_functionRegister["rightString"]);
		ExprScriptRsubString::registerFunction(_functionRegister["rsubString"]);
		ExprScriptScanDirectories::registerFunction(_functionRegister["scanDirectories"]);
		ExprScriptScanFiles::registerFunction(_functionRegister["scanFiles"]);
		ExprScriptSendBinaryToSocket::registerFunction(_functionRegister["sendBinaryToSocket"]);
		ExprScriptSendHTTPRequest::registerFunction(_functionRegister["sendHTTPRequest"]);
		ExprScriptSendTextToSocket::registerFunction(_functionRegister["sendTextToSocket"]);
		ExprScriptSelectGenerationTagsHandler::registerFunction(_functionRegister["selectGenerationTagsHandler"]);
		ExprScriptShortToBytes::registerFunction(_functionRegister["shortToBytes"]);
		ExprScriptSqrt::registerFunction(_functionRegister["sqrt"]);
		ExprScriptStartString::registerFunction(_functionRegister["startString"]);
		ExprScriptSub::registerFunction(_functionRegister["sub"]);
		ExprScriptSubString::registerFunction(_functionRegister["subString"]);
		ExprScriptSup::registerFunction(_functionRegister["sup"]);
		ExprScriptSystem::registerFunction(_functionRegister["system"]);
		ExprScriptToLowerString::registerFunction(_functionRegister["toLowerString"]);
		ExprScriptToUpperString::registerFunction(_functionRegister["toUpperString"]);
		ExprScriptTranslateString::registerFunction(_functionRegister["translateString"]);
		ExprScriptTrimLeft::registerFunction(_functionRegister["trimLeft"]);
		ExprScriptTrimRight::registerFunction(_functionRegister["trimRight"]);
		ExprScriptTrim::registerFunction(_functionRegister["trim"]);
		ExprScriptTruncateAfterString::registerFunction(_functionRegister["truncateAfterString"]);
		ExprScriptTruncateBeforeString::registerFunction(_functionRegister["truncateBeforeString"]);
		ExprScriptUUID::registerFunction(_functionRegister["UUID"]);
		ExprScriptCountInputCols::registerFunction(_functionRegister["countInputCols"]);
		ExprScriptCountInputLines::registerFunction(_functionRegister["countInputLines"]);
		ExprScriptGetInputFilename::registerFunction(_functionRegister["getInputFilename"]);
		ExprScriptGetLastReadChars::registerFunction(_functionRegister["getLastReadChars"]);
		ExprScriptGetInputLocation::registerFunction(_functionRegister["getInputLocation"]);
		ExprScriptLookAhead::registerFunction(_functionRegister["lookAhead"]);
		ExprScriptPeekChar::registerFunction(_functionRegister["peekChar"]);
		ExprScriptReadAdaString::registerFunction(_functionRegister["readAdaString"]);
		ExprScriptReadByte::registerFunction(_functionRegister["readByte"]);
		ExprScriptReadBytes::registerFunction(_functionRegister["readBytes"]);
		ExprScriptReadCChar::registerFunction(_functionRegister["readCChar"]);
		ExprScriptReadChar::registerFunction(_functionRegister["readChar"]);
		ExprScriptReadCharAsInt::registerFunction(_functionRegister["readCharAsInt"]);
		ExprScriptReadChars::registerFunction(_functionRegister["readChars"]);
		ExprScriptReadIdentifier::registerFunction(_functionRegister["readIdentifier"]);
		ExprScriptReadIfEqualTo::registerFunction(_functionRegister["readIfEqualTo"]);
		ExprScriptReadIfEqualToIgnoreCase::registerFunction(_functionRegister["readIfEqualToIgnoreCase"]);
		ExprScriptReadIfEqualToIdentifier::registerFunction(_functionRegister["readIfEqualToIdentifier"]);
		ExprScriptReadLine::registerFunction(_functionRegister["readLine"]);
		ExprScriptReadNextText::registerFunction(_functionRegister["readNextText"]);
		ExprScriptReadNumber::registerFunction(_functionRegister["readNumber"]);
		ExprScriptReadPythonString::registerFunction(_functionRegister["readPythonString"]);
		ExprScriptReadString::registerFunction(_functionRegister["readString"]);
		ExprScriptReadUptoJustOneChar::registerFunction(_functionRegister["readUptoJustOneChar"]);
		ExprScriptReadWord::registerFunction(_functionRegister["readWord"]);
		ExprScriptSkipBlanks::registerFunction(_functionRegister["skipBlanks"]);
		ExprScriptSkipSpaces::registerFunction(_functionRegister["skipSpaces"]);
		ExprScriptSkipEmptyCpp::registerFunction(_functionRegister["skipEmptyCpp"]);
		ExprScriptSkipEmptyCppExceptDoxygen::registerFunction(_functionRegister["skipEmptyCppExceptDoxygen"]);
		ExprScriptSkipEmptyHTML::registerFunction(_functionRegister["skipEmptyHTML"]);
		ExprScriptSkipEmptyLaTeX::registerFunction(_functionRegister["skipEmptyLaTeX"]);
		ExprScriptCountOutputCols::registerFunction(_functionRegister["countOutputCols"]);
		ExprScriptCountOutputLines::registerFunction(_functionRegister["countOutputLines"]);
		ExprScriptDecrementIndentLevel::registerFunction(_functionRegister["decrementIndentLevel"]);
		ExprScriptEqualLastWrittenChars::registerFunction(_functionRegister["equalLastWrittenChars"]);
		ExprScriptExistFloatingLocation::registerFunction(_functionRegister["existFloatingLocation"]);
		ExprScriptGetFloatingLocation::registerFunction(_functionRegister["getFloatingLocation"]);
		ExprScriptGetLastWrittenChars::registerFunction(_functionRegister["getLastWrittenChars"]);
		ExprScriptGetMarkupKey::registerFunction(_functionRegister["getMarkupKey"]);
		ExprScriptGetMarkupValue::registerFunction(_functionRegister["getMarkupValue"]);
		ExprScriptGetOutputFilename::registerFunction(_functionRegister["getOutputFilename"]);
		ExprScriptGetOutputLocation::registerFunction(_functionRegister["getOutputLocation"]);
		ExprScriptGetProtectedArea::registerFunction(_functionRegister["getProtectedArea"]);
		ExprScriptGetProtectedAreaKeys::registerFunction(_functionRegister["getProtectedAreaKeys"]);
		ExprScriptIndentText::registerFunction(_functionRegister["indentText"]);
		ExprScriptNewFloatingLocation::registerFunction(_functionRegister["newFloatingLocation"]);
		ExprScriptRemainingProtectedAreas::registerFunction(_functionRegister["remainingProtectedAreas"]);
		ExprScriptRemoveFloatingLocation::registerFunction(_functionRegister["removeFloatingLocation"]);
		ExprScriptRemoveProtectedArea::registerFunction(_functionRegister["removeProtectedArea"]);
//##end##"registration"
	}
	return _functionRegister;
}

	void ExprScriptFunction::addParameter(ExprScriptExpression* pParameter) {
		_parameters.push_back(pParameter);
	}

	EXPRESSION_TYPE ExprScriptFunction::getParameterType(unsigned int iIndex) const {
		if (_pPrototype != NULL) return _pPrototype->getParameterType(iIndex);
		return UNKNOWN_EXPRTYPE;
	}

	ExprScriptExpression* ExprScriptFunction::getDefaultParameter(unsigned int iIndex) const {
		if (_pPrototype != NULL) return _pPrototype->getDefaultParameter(iIndex);
		return NULL;
	}

	unsigned int ExprScriptFunction::getArity() const {
		if (_pPrototype != NULL) return _pPrototype->getArity();
		throw UtlException("internal error in ExprScriptFunction::getArity(): this case shouldn't occur");
	}

	unsigned int ExprScriptFunction::getMinArity() const {
		if (_pPrototype != NULL) return _pPrototype->getMinArity();
		return getArity();
	}

	const char* ExprScriptFunction::getName() const {
		if (_pPrototype != NULL) return _pPrototype->getFunctionName();
		throw UtlException("internal error in ExprScriptFunction::getName(): this case shouldn't occur");
	}

	int ExprScriptFunction::getThisPosition() const { return 0; }
	void ExprScriptFunction::initializationDone() {}

	std::string ExprScriptFunction::getValue(DtaScriptVariable& visibility) const {
		if (_pTemplate != NULL) {
			std::string sInstantiationKey = _pTemplate->getValue(visibility);
			GrfFunction* pFunction = _pPrototype->getInstantiatedFunction(visibility, sInstantiationKey);
			if (pFunction != NULL) return pFunction->launchExecution(visibility, *this, sInstantiationKey);
			throw UtlException("template function '" +  _pPrototype->getName() + "<\"" + sInstantiationKey + "\">' hasn't been implemented");
		}
		if (_pPrototype == NULL) throw UtlException("ExprScriptFunction::getValue() -> must be implemented into derivated classes\nFor custom functions, see class GrfFunction");
		return _pPrototype->launchExecution(visibility, *this);
	}

ExprScriptFunction* ExprScriptFunction::create(GrfBlock& block, ScpStream& script, const std::string& sFunction, const std::string& sTemplate, bool bGenericKey) {
    ExprScriptFunction* pFunction = NULL;
	int iArity = -1;
	std::string sFunctionName;
//##markup##"create::deprecated"
//##begin##"create::deprecated"
	if (sFunction == "getVariableSize") {
		std::string sErrorMessage = "warning: function 'getVariableSize' has been deprecated since version 1.30 -> replace it by function 'getArraySize'" + CGRuntime::endl();
		sErrorMessage += DtaProject::getInstance().getTraceStack(script);
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		sFunctionName = "getArraySize";
	} else if (sFunction == "today") {
		std::string sErrorMessage = "warning: function 'today' has been deprecated since version 2.09 -> replace it by function 'getNow'" + CGRuntime::endl();
		sErrorMessage += DtaProject::getInstance().getTraceStack(script);
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		sFunctionName = "getNow";
	} else if (sFunction == "getDefineTarget") {
		std::string sErrorMessage = "warning: function 'getDefineTarget' has been deprecated since version 1.30 -> replace it by function 'getProperty'" + CGRuntime::endl();
		sErrorMessage += DtaProject::getInstance().getTraceStack(script);
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		sFunctionName = "getProperty";
	} else if (sFunction == "trimLeftString") {
		std::string sErrorMessage = "warning: function 'trimLeftString' has been deprecated since version 1.40 -> replace it by function 'trimLeft'" + CGRuntime::endl();
		sErrorMessage += DtaProject::getInstance().getTraceStack(script);
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		sFunctionName = "trimLeft";
	} else if (sFunction == "trimRightString") {
		std::string sErrorMessage = "warning: function 'trimRightString' has been deprecated since version 1.40 -> replace it by function 'trimRight'" + CGRuntime::endl();
		sErrorMessage += DtaProject::getInstance().getTraceStack(script);
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		sFunctionName = "trimRight";
	} else if (sFunction == "trimString") {
		std::string sErrorMessage = "warning: function 'trimString' has been deprecated since version 1.40 -> replace it by function 'trim'" + CGRuntime::endl();
		sErrorMessage += DtaProject::getInstance().getTraceStack(script);
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		sFunctionName = "trim";
	} else if (sFunction == "readLastChars") {
		std::string sErrorMessage = "warning: function 'readLastChars' has been deprecated since version 1.30 -> replace it by function 'getLastReadChars'" + CGRuntime::endl();
		sErrorMessage += DtaProject::getInstance().getTraceStack(script);
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		sFunctionName = "getLastReadChars";
	} else if (sFunction == "getLocation") {
		std::string sErrorMessage = "warning: function 'getLocation' has been deprecated since version 3.7.1 -> replace it by function 'getInputLocation'" + CGRuntime::endl();
		sErrorMessage += DtaProject::getInstance().getTraceStack(script);
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		sFunctionName = "getInputLocation";
	} else if (sFunction == "getMarkerKey") {
		std::string sErrorMessage = "warning: function 'getMarkerKey' has been deprecated since version 2.14 -> replace it by function 'getMarkupKey'" + CGRuntime::endl();
		sErrorMessage += DtaProject::getInstance().getTraceStack(script);
		if (DtaProject::getInstance().addWarning(sErrorMessage) == 1) CGRuntime::traceLine(sErrorMessage);
		sFunctionName = "getMarkupKey";
//##end##"create::deprecated"
	} else sFunctionName = sFunction;
	if (getFunctionRegister().find(sFunctionName) != getFunctionRegister().end()) {
		if (!sTemplate.empty()) throw UtlException("predefined function '" + sFunction + "' cannot instantiate template function '" + sFunction + "<" + sTemplate + ">'");
		pFunction = getFunctionRegister()[sFunctionName].constructor(block);
	} else {
		GrfFunction* pPrototype = block.getFunction(sFunctionName, sTemplate, bGenericKey);
		if (pPrototype != NULL) {
			pFunction = new ExprScriptFunction(pPrototype);
		}
	}
    return pFunction;
}

ExprScriptFunction* ExprScriptFunction::createMethod(GrfBlock& block, ScpStream& script, const std::string& sFunction, const std::string& sTemplate, bool bGenericKey) {
//##markup##"createMethod"
//##begin##"createMethod"
	if (sFunction == "size") return create(block, script, "getArraySize", sTemplate, bGenericKey);
	if (sFunction == "length") return create(block, script, "lengthString", sTemplate, bGenericKey);
	if (sFunction == "empty") return create(block, script, "isEmpty", sTemplate, bGenericKey);
//##end##"createMethod"
	return create(block, script, sFunction, sTemplate, bGenericKey);
}

	void ExprScriptFunction::clearCounters() {
		for (std::map<std::string, DtaFunctionInfo>::iterator i = getFunctionRegister().begin(); i != getFunctionRegister().end(); i++) {
			*(i->second.pCounter) = 0;
		}
	}

	EXPRESSION_RETURN_TYPE ExprScriptFunction::compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const {
		bool bDispatchedYet = true;
		if (_pPrototype != NULL) {
			if (_pPrototype->isATemplateInstantiation()) {
				CW_BODY_STREAM << DtaScript::convertFilenameAsIdentifier(CppCompilerEnvironment::getRadical(theCompilerEnvironment.getFunctionModule(_pPrototype->getFunctionName()))) << "::_compilerTemplateFunction_" << _pPrototype->getFunctionName() << "_compilerInstantiation_";
				ExprScriptConstant* pConstant = dynamic_cast<ExprScriptConstant*>(_pTemplate);
				DtaScriptVariable visibility;
				if ((pConstant != NULL) && !pConstant->getConstant().empty() && (_pPrototype->getInstantiatedFunction(visibility, pConstant->getConstant()) != NULL)) {
					CW_BODY_STREAM << theCompilerEnvironment.convertTemplateKey(pConstant->getConstant()) << "(";
				} else {
					CW_BODY_STREAM << "(";
					_pTemplate->compileCppString(theCompilerEnvironment);
					bDispatchedYet = false;
				}
			} else {
				_pPrototype->compileCppFunctionNameForCalling(theCompilerEnvironment);
				CW_BODY_STREAM << "(";
			}
		} else CW_BODY_STREAM << "CGRuntime::" << getName() << "(";
		int iIndex = 0;
		for (std::vector<ExprScriptExpression*>::const_iterator i = _parameters.begin(); i != _parameters.end(); i++) {
			if ((i != _parameters.begin()) || !bDispatchedYet) CW_BODY_STREAM << ", ";
			if ((getParameterType(iIndex) & 0x00FF) == VALUE_EXPRTYPE) (*i)->compileCpp(theCompilerEnvironment);
			else dynamic_cast<ExprScriptVariable*>(*i)->compileCppForSet(theCompilerEnvironment);
			iIndex++;
		}
		CW_BODY_STREAM << ")";
		return STRING_TYPE;
	}

	bool ExprScriptFunction::compileCppBoolean(CppCompilerEnvironment& theCompilerEnvironment, bool bNegative) const {
		if (!bNegative) CW_BODY_STREAM << "!";
		compileCpp(theCompilerEnvironment);
		CW_BODY_STREAM << ".empty()";
		return (_pPrototype != NULL);
	}

	bool ExprScriptFunction::compileCppString(CppCompilerEnvironment& theCompilerEnvironment) const {
		ScpStream* pOwner;
		int iInsertAreaLocation = CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner);
		int iLocation = CW_BODY_STREAM.getOutputLocation();
		EXPRESSION_RETURN_TYPE returnType = compileCpp(theCompilerEnvironment);
		if (returnType != STRING_TYPE) {
			CW_BODY_STREAM.insertText("CGRuntime::toString(", iLocation + (CW_BODY_STREAM.getFloatingLocation("INSERT AREA", pOwner) - iInsertAreaLocation));
			CW_BODY_STREAM << ")";
		}
		return (_pPrototype != NULL);
	}

	std::string ExprScriptFunction::toString() const {
		std::string sSerialize;
		if (_pPrototype != NULL) sSerialize = _pPrototype->getFunctionName();
		else sSerialize = getName();
		if (_pTemplate != NULL) sSerialize += "<" + _pTemplate->toString() + ">";
		sSerialize += "(";
		int iIndex = 0;
		for (std::vector<ExprScriptExpression*>::const_iterator i = _parameters.begin(); i != _parameters.end(); i++) {
			if (i != _parameters.begin()) sSerialize += ", ";
			sSerialize += (*i)->toString();
			iIndex++;
		}
		return sSerialize + ")";
	}
}
