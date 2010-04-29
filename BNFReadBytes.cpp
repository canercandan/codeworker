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

#include "ExprScriptExpression.h"
#include "CGRuntime.h"
#include "DtaVisitor.h"
#include "BNFReadBytes.h"

namespace CodeWorker {
	BNFReadBytes::BNFReadBytes(DtaBNFScript* pBNFScript, GrfBlock* pParent, bool bContinue, bool bNoCase) : BNFReadToken(pBNFScript, pParent, bContinue, bNoCase), _pLength(NULL) {}

	BNFReadBytes::~BNFReadBytes() {
		delete _pLength;
	}

	void BNFReadBytes::accept(DtaVisitor& visitor, DtaVisitorEnvironment& env) {
		visitor.visitBNFReadBytes(*this, env);
	}

	const char* BNFReadBytes::getFunctionName() const { return "#readBytes"; }

	std::string BNFReadBytes::executeExtraction(DtaScriptVariable& visibility) const {
		int iLength = _pLength->getIntValue(visibility);
		return CGRuntime::readBytes(iLength);
	}

	std::string BNFReadBytes::compileCppExtraction() const {
		return "CGRuntime::readBytes(" + _pLength->toString() + ")";
	}
}
