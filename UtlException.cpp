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

#include "ScpStream.h"
#include "UtlTrace.h"
#include "UtlException.h"

namespace CodeWorker {

	UtlException::UtlException(const UtlException& exception) {
		_sMessage = exception._sMessage;
		_sTraceStack = exception._sTraceStack;
		_bFinalMessage = exception._bFinalMessage;
	}

	UtlException::UtlException(std::istream& stream, const std::string& sMessage) : _bFinalMessage(false){
		char sText[10000];
		sprintf(sText, "line %d, %s", CodeWorker::getLineCount(stream), sMessage.c_str());
		_sMessage = sText;
		_sTraceStack = UtlTrace::getTraceStack();
	}

	UtlException::UtlException(std::istream& stream, const char* sMessage) : _bFinalMessage(false) {
		char sText[10000];
		sprintf(sText, "line %d, %s", CodeWorker::getLineCount(stream), sMessage);
		_sMessage = sText;
		_sTraceStack = UtlTrace::getTraceStack();
	}

	UtlException::UtlException(std::istream& stream, const std::string& sTraceStack, const std::string& sMessage) : _sTraceStack(sTraceStack), _bFinalMessage(false) {
		char sText[10000];
		sprintf(sText, "line %d, %s", CodeWorker::getLineCount(stream), sMessage.c_str());
		_sMessage = sText;
	}

	UtlException::UtlException(const ScpStream& stream, const std::string& sMessage) : _bFinalMessage(false) {
		char sText[10000];
		sprintf(sText, "line %d, %s", stream.getLineCount(), sMessage.c_str());
		_sMessage = sText;
		_sTraceStack = UtlTrace::getTraceStack();
	}

	UtlException::UtlException(const ScpStream& stream, const char* sMessage) : _bFinalMessage(false) {
		char sText[10000];
		sprintf(sText, "line %d, %s", stream.getLineCount(), sMessage);
		_sMessage = sText;
		_sTraceStack = UtlTrace::getTraceStack();
	}

	UtlException::UtlException(const ScpStream& stream, const std::string& sTraceStack, const std::string& sMessage) : _sTraceStack(sTraceStack), _bFinalMessage(false) {
		char sText[10000];
		sprintf(sText, "line %d, %s", stream.getLineCount(), sMessage.c_str());
		_sMessage = sText;
	}

	UtlException::UtlException(const std::string& sMessage) : _sMessage(sMessage), _bFinalMessage(false) {
		_sTraceStack = UtlTrace::getTraceStack();
	}

	UtlException::UtlException(const char* sMessage) : _sMessage(sMessage), _bFinalMessage(false) {
		_sTraceStack = UtlTrace::getTraceStack();
	}

	UtlException::UtlException(const std::string& sTraceStack, const std::string& sMessage, bool bFinalMessage) : _sTraceStack(sTraceStack), _sMessage(sMessage), _bFinalMessage(bFinalMessage) {
	}

	UtlException::~UtlException() throw() {
	}

	const char* UtlException::what() const throw() {
		return _sMessage.c_str();
	}

}
