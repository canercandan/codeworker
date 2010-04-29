/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2006 Cédric Lemaire

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

#include "DtaVisitor.h"

namespace CodeWorker {
	DtaVisitorEnvironment::~DtaVisitorEnvironment() {}
	DtaVisitor::~DtaVisitor() {}

	void DtaVisitor::visitBNFAndOrJunction(BNFAndOrJunction& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFBreak(BNFBreak& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFCharBoundaries(BNFCharBoundaries& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFCharLitteral(BNFCharLitteral& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFCheck(BNFCheck& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFClause(BNFClause& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFClauseCall(BNFClauseCall& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFComplementary(BNFComplementary& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFConjunction(BNFConjunction& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFContinue(BNFContinue& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFDisjunction(BNFDisjunction& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFEndOfFile(BNFEndOfFile& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFEndOfLine(BNFEndOfLine& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFFindToken(BNFFindToken& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFForeach(BNFForeach& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFIgnore(BNFIgnore& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFInsert(BNFInsert& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFMoveAhead(BNFMoveAhead& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFMultiplicity(BNFMultiplicity& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFNextStep(BNFNextStep& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFNot(BNFNot& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFPushItem(BNFPushItem& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFRatchet(BNFRatchet& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadAdaString(BNFReadAdaString& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadByte(BNFReadByte& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadBytes(BNFReadBytes& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadCChar(BNFReadCChar& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadChar(BNFReadChar& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadChars(BNFReadChars& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadCompleteIdentifier(BNFReadCompleteIdentifier& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadCString(BNFReadCString& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadIdentifier(BNFReadIdentifier& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadInteger(BNFReadInteger& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadNumeric(BNFReadNumeric& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadPythonString(BNFReadPythonString& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadText(BNFReadText& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadToken(BNFReadToken& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFReadUptoIgnore(BNFReadUptoIgnore& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFScanWindow(BNFScanWindow& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFSkipIgnore(BNFSkipIgnore& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFStepintoHook(BNFStepintoHook& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFStepoutHook(BNFStepoutHook& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFStepper(BNFStepper& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFString(BNFString& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFTransformationMode(BNFTransformationMode& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
	void DtaVisitor::visitBNFTryCatch(BNFTryCatch& /*theObject*/, DtaVisitorEnvironment& /*env*/) {}
}
