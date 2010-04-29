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

#ifndef _UtlTrace_h_
#define _UtlTrace_h_

#include <fstream>
#include <string>
#include <memory>

namespace CodeWorker {

#define UTLTRACE_METHOD(iLevel, sClass, sMethod) UtlTrace traceInstance(iLevel, sClass, sMethod)
#define UTLTRACE_TEXT(iLevel, sText) if (UtlTrace::isValidLevel(iLevel)) UtlTrace::traceText(__FILE__, __LINE__, sText)
#define UTLTRACE_FORMAT UtlTrace::traceFormat
#define UTLTRACE_STACK_INSTRUCTION(sFileName, iLine) UtlTrace::traceStackInstruction(sFileName, iLine)
#define UTLTRACE_STACK_FUNCTION(sFileName, sFunctionName, iLine) UtlTraceStackFunction myTraceStackFunction(sFileName, sFunctionName, iLine)

#ifdef THROW_UTLEXCEPTION
#undef THROW_UTLEXCEPTION
#endif
#define THROW_UTLEXCEPTION(sText) { std::string sExceptionTrace = std::string("EXCEPTION: \"") + sText + "\"";UtlTrace::writeText(sExceptionTrace.c_str());throw UtlException(sText); }

#ifdef THROW_UTLEXCEPTION2
#undef THROW_UTLEXCEPTION2
#endif
#define THROW_UTLEXCEPTION2(myStream, sText) { std::string sExceptionTrace = std::string("EXCEPTION: \"") + sText + "\"";UtlTrace::writeText(sExceptionTrace.c_str());throw UtlException(myStream, sText); }

#ifdef THROW_UTLEXCEPTION_STATIC
#undef THROW_UTLEXCEPTION_STATIC
#endif
#define THROW_UTLEXCEPTION_STATIC(sText) { std::string sExceptionTrace = std::string("EXCEPTION: \"") + sText + "\"";UtlTrace::writeText(sExceptionTrace.c_str());throw UtlException(sText); }

	class UtlTrace {
	private:
		int _iLevel;
		std::string _sClass;
		std::string _sMethod;

		static int _iLowerLevel;
		static int _iUpperLevel;
		static int _iCountLine;
		static int _iIndentation;
		static char* _sIndentation;

		static const char** _tsTraceStack;
		static const char** _tsTraceFileStack;
		static int* _tiTraceLocationStack;
		static int _iTraceSize;
		static int _iMaxTraceSize;

		static std::string _sTraceFile;
		static bool  _bPersistent;
		static std::auto_ptr<std::ofstream> _pFile;

	public:
		UtlTrace(int iLevel, const char* sClass, const char* sMethod);
		~UtlTrace();

		int getLevel() const { return _iLevel; }

		static int getLowerLevel() { return _iLowerLevel; }
		static int getUpperLevel() { return _iUpperLevel; }
		static void setUpperLevel(int iLevel) { _iUpperLevel = iLevel; }
		static bool isValidLevel(int iLevel) { return (iLevel >= getLowerLevel()) && (iLevel <= getUpperLevel()); }

		static void traceText(const char* sSourceFile, int iLineCode, const char* sText);
		static void traceFormat(int iLevel, const char* sFormat, ...);
		static void writeText(const char* sText);

		static inline int getTraceSize() { return _iTraceSize; }
		static void pushTraceMethod(const char* sTraceMethod);
		static void popTraceMethod();

		static void traceStackInstruction(const char* sFileName, int iLine);
		static void traceStackFunction(const char* sFileName, const char* sFunctionName, int iLine);
		static void popTraceStack();

		static std::string getTraceStack();
		static const char** copyTraceStack();
		static const char** copyTraceFileStack();
		static int* copyTraceLocationStack();

		static void writeTraceText(const char* sText);

		static void initialize(const char* sTraceFile, bool bPersistent, int iLowerLevel, int iUpperLevel);
		static void setConfiguration(const char* sTraceFile, bool bPersistent, int iLowerLevel, int iUpperLevel);
		static void finalize();
	};

	class UtlTraceStackFunction {
	public:
		UtlTraceStackFunction(const char* sFileName, const char* sFunctionName, int iLine);
		~UtlTraceStackFunction();
	};

	class UtlTraceSession {
	public:
		UtlTraceSession();
		~UtlTraceSession();
	};
}

#endif
