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

#ifndef _DtaVisitor_h_
#define _DtaVisitor_h_

namespace CodeWorker {
	class BNFAndOrJunction;
	class BNFBreak;
	class BNFCharBoundaries;
	class BNFCharLitteral;
	class BNFCheck;
	class BNFClause;
	class BNFClauseCall;
	class BNFComplementary;
	class BNFConjunction;
	class BNFContinue;
	class BNFDisjunction;
	class BNFEndOfFile;
	class BNFEndOfLine;
	class BNFFindToken;
	class BNFForeach;
	class BNFIgnore;
	class BNFInsert;
	class BNFMoveAhead;
	class BNFMultiplicity;
	class BNFNextStep;
	class BNFNot;
	class BNFPushItem;
	class BNFRatchet;
	class BNFReadAdaString;
	class BNFReadByte;
	class BNFReadBytes;
	class BNFReadCChar;
	class BNFReadChar;
	class BNFReadChars;
	class BNFReadCompleteIdentifier;
	class BNFReadCString;
	class BNFReadIdentifier;
	class BNFReadInteger;
	class BNFReadNumeric;
	class BNFReadPythonString;
	class BNFReadText;
	class BNFReadToken;
	class BNFReadUptoIgnore;
	class BNFScanWindow;
	class BNFSkipIgnore;
	class BNFStepintoHook;
	class BNFStepoutHook;
	class BNFStepper;
	class BNFString;
	class BNFTransformationMode;
	class BNFTryCatch;

	class DtaVisitorEnvironment {
		public:
			inline DtaVisitorEnvironment() {}
			virtual ~DtaVisitorEnvironment();
	};

	class DtaVisitor {
		public:
			inline DtaVisitor() {}
			virtual ~DtaVisitor();

			virtual void visitBNFAndOrJunction(BNFAndOrJunction& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFBreak(BNFBreak& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFCharBoundaries(BNFCharBoundaries& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFCharLitteral(BNFCharLitteral& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFCheck(BNFCheck& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFClause(BNFClause& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFClauseCall(BNFClauseCall& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFComplementary(BNFComplementary& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFConjunction(BNFConjunction& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFContinue(BNFContinue& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFDisjunction(BNFDisjunction& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFEndOfFile(BNFEndOfFile& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFEndOfLine(BNFEndOfLine& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFFindToken(BNFFindToken& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFForeach(BNFForeach& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFIgnore(BNFIgnore& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFInsert(BNFInsert& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFMoveAhead(BNFMoveAhead& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFMultiplicity(BNFMultiplicity& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFNextStep(BNFNextStep& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFNot(BNFNot& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFPushItem(BNFPushItem& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFRatchet(BNFRatchet& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadAdaString(BNFReadAdaString& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadByte(BNFReadByte& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadBytes(BNFReadBytes& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadCChar(BNFReadCChar& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadChar(BNFReadChar& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadChars(BNFReadChars& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadCompleteIdentifier(BNFReadCompleteIdentifier& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadCString(BNFReadCString& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadIdentifier(BNFReadIdentifier& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadInteger(BNFReadInteger& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadNumeric(BNFReadNumeric& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadPythonString(BNFReadPythonString& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadText(BNFReadText& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadToken(BNFReadToken& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFReadUptoIgnore(BNFReadUptoIgnore& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFScanWindow(BNFScanWindow& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFSkipIgnore(BNFSkipIgnore& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFStepintoHook(BNFStepintoHook& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFStepoutHook(BNFStepoutHook& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFStepper(BNFStepper& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFString(BNFString& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFTransformationMode(BNFTransformationMode& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFTryCatch(BNFTryCatch& theObject, DtaVisitorEnvironment& env);
	};
}

#endif
