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

#ifndef _UtlString_h_
#define _UtlString_h_

#ifndef WIN32
#	include <features.h>
#	if defined(__cplusplus) && __GNUC_PREREQ (4, 3)
#		include <cstring>
#		include <cstdlib>
#	endif
#endif

#include <string>

#ifndef WIN32
	int strnicmp(const char* tc1, const char* tc2, size_t iLength);
	int stricmp(const char* tc1, const char* tc2);
#endif

namespace CodeWorker {
	int stricmp(const std::string& s1, const std::string& s2);
	int strnicmp(const std::string& tc1, const std::string& tc2, size_t iLength);
	std::string toUppercase(const char* sText);
	std::string toLowercase(const char* sText);

	bool trimLeft(std::string& sText);
	bool trimRight(std::string& sText);
	bool trim(std::string& sText);
	bool splitString(const std::string& sCompletString, char iChar, std::string& sBegin, std::string& sEnd) ;

	std::string toString(int iNumber);
	std::string toString(unsigned int iNumber);
	std::string toString(double dNumber);
}

#endif
