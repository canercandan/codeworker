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

#ifndef _UtlException_h_
#define _UtlException_h_

#include <exception>
#include <string>
#include <iostream>

namespace CodeWorker {

#ifndef THROW_UTLEXCEPTION
#define THROW_UTLEXCEPTION throw UtlException
#endif

#ifndef THROW_UTLEXCEPTION2
#define THROW_UTLEXCEPTION2 throw UtlException
#endif

#ifndef THROW_UTLEXCEPTION_INTERFACE
#define THROW_UTLEXCEPTION_INTERFACE throw UtlException
#endif

#ifndef THROW_UTLEXCEPTION_STATIC
#define THROW_UTLEXCEPTION_STATIC throw UtlException
#endif

	class ScpStream;

	class UtlException : public std::exception {
	private:
		std::string _sTraceStack;
		std::string _sMessage;
		bool _bFinalMessage;

	public:
		UtlException(const UtlException& exception);
		UtlException(std::istream& stream, const std::string& sMessage);
		UtlException(std::istream& stream, const char* sMessage);
		UtlException(std::istream& stream, const std::string& sTraceStack, const std::string& sMessage);
		UtlException(const ScpStream& stream, const std::string& sMessage);
		UtlException(const ScpStream& stream, const char* sMessage);
		UtlException(const ScpStream& stream, const std::string& sTraceStack, const std::string& sMessage);
		UtlException(const std::string& sMessage);
		UtlException(const char* sMessage);
		UtlException(const std::string& sTraceStack, const std::string& sMessage, bool bFinalMessage = false);
		virtual ~UtlException() throw();

		inline const std::string& getMessage() const { return _sMessage; }
		inline const std::string& getTraceStack() const { return _sTraceStack; }
		inline bool isFinalMessage() const { return _bFinalMessage; }
		virtual const char* what() const throw();

	private:
		UtlException();
		UtlException& operator = (const UtlException& exception);
	};


	class UtlExitException : public std::exception {
	private:
		int _iCode;

	public:
		UtlExitException(int iCode) : _iCode(iCode) {}

		inline int getCode() const { return _iCode; }
		inline void setCode(int iCode) { _iCode = iCode; }
	};
}

#endif
