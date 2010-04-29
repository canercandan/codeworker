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

#include <sstream>

#include "UtlString.h"


#ifndef WIN32
int stricmp(const char* s1, const char* s2) {
	char* u1 = (char*) s1;
	char* u2 = (char*) s2;
	while ((*u1 != '\0') && (*u2 != '\0')) {
		if (*u1 != *u2) {
			char a1 = *u1;
			if ((a1 >= 'A') && (a1 <= 'Z')) a1 += ' ';
			char a2 = *u2;
			if ((a2 >= 'A') && (a2 <= 'Z')) a2 += ' ';
			if (a1 != a2) return ((int) a1) - ((int) a2);
		}
		u1++;
		u2++;
	}
	return ((int) *u1) - ((int) *u2);
}

int strnicmp(const char* tc1, const char* tc2, size_t iLength) {
	int iComp = 0;
	char* tcCursor1 = (char*) tc1;
	char* tcCursor2 = (char*) tc2;
	while (iLength-- > 0) {
		if (*tcCursor1 == *tcCursor2) {
			if (*tcCursor1 == '\0') break;
		} else {
			char a = *tcCursor1;
			if ((a >= 'A') && (a <= 'Z')) a += ' ';
			char b = *tcCursor2;
			if ((b >= 'A') && (b <= 'Z')) b += ' ';
			if (a != b) {
				iComp = ((int) a) - ((int) b);
				break;
			}
		}
		tcCursor1++;
		tcCursor2++;
	}
	return iComp;
}
#endif

namespace CodeWorker {
	int stricmp(const std::string& s1, const std::string& s2) {
		return ::stricmp(s1.c_str(), s2.c_str());
	}

	int strnicmp(const std::string& tc1, const std::string& tc2, size_t iLength) {
		return ::strnicmp(tc1.c_str(), tc2.c_str(), iLength);
	}

	std::string toUppercase(const char* sText) {
		std::string sResult = sText;
		for (unsigned int i = 0; i < sResult.size(); i++) {
			char a = sResult[i];
			if ((a >= 'a') && (a <= 'z')) sResult[i] -= ' ';
		}
		return sResult;
	}

	std::string toLowercase(const char* sText) {
		std::string sResult = sText;
		for (unsigned int i = 0; i < sResult.size(); i++) {
			char a = sResult[i];
			if ((a >= 'A') && (a <= 'Z')) sResult[i] += ' ';
		}
		return sResult;
	}


	bool trimLeft(std::string& sText) {
		bool bChanged;
		std::string::size_type iIndex = sText.find_first_not_of(' ');
		if (iIndex == 0) bChanged = false;
		else if (iIndex == std::string::npos) {
			bChanged = !sText.empty();
			sText = "";
		} else {
			bChanged = true;
			sText = sText.substr(iIndex);
		}
		return bChanged;
	}

	bool trimRight(std::string& sText) {
		bool bChanged;
		std::string::size_type iIndex = sText.find_last_not_of(' ');
		if (iIndex == std::string::npos) {
			bChanged = !sText.empty();
			sText = "";
		} else {
			iIndex++;
			if (iIndex == sText.size()) bChanged = false;
			else {
				bChanged = true;
				sText = sText.substr(0, iIndex);
			}
		}
		return bChanged;
	}

	bool trim(std::string& sText) {
		bool bChanged1 = CodeWorker::trimLeft(sText);
		bool bChanged2 = CodeWorker::trimRight(sText);
		return bChanged1 | bChanged2;
	}


	bool splitString(const std::string& sCompletString, char iChar, std::string& sBegin, std::string& sEnd) {

		// search the character and split the string in two
		bool bFind=false;
		unsigned int j=0;
		for (unsigned int i = 0; i < sCompletString.size(); i++) {
			char a = sCompletString[i];
			
			if (!bFind) {
				if (a==iChar) {
					bFind=true;
					j=i;
				}
			}

			if (bFind) {
				sEnd[i-j]=a;
			}
			else {
				sBegin[i]=a;
			}
		}
		
		return bFind;
	}


	std::string toString(int iNumber) {
		std::stringstream theText;
		theText << iNumber;
		return theText.str();
	}

	std::string toString(unsigned int iNumber) {
		std::stringstream theText;
		theText << iNumber;
		return theText.str();
	}

	std::string toString(double dNumber) {
		std::stringstream theText;
		theText << dNumber;
		return theText.str();
	}
}
