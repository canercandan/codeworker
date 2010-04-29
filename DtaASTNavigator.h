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

#ifndef _DtaASTNavigator_h_
#define _DtaASTNavigator_h_

//##protect##"declarations"
#include <map>

#include "DtaVisitor.h"

namespace CodeWorker {
	class GrfBlock;
	class DtaBNFScript;
	class ASTCommandEnvironment;

	struct DtaASTStructure {
		struct Clause {
			struct Sequence {
				struct NonTerminal {
					int _iMin;
					int _iMax;

					inline NonTerminal() : _iMin(1), _iMax(1) {}
					void addCallInSequence(const NonTerminal& call);
					void addCallInSequence(int iNbCalls);
					void repeatCall(int iMin, int iMax);
					void merge(const NonTerminal& call);
					void build(DtaScriptVariable& theStorage);
				};
				std::map<std::string, NonTerminal*> _allNonTerminals;

				inline Sequence() {}
				~Sequence();
				void repeatCalls(int iMin, int iMax);
				void merge(Sequence& seq);
				void addSequence(Sequence& seq);
			};
			Sequence _structure;
			void build(DtaScriptVariable& theStorage);
		};
		std::map<std::string, Clause*> _allClauses;

		inline DtaASTStructure() {}
		~DtaASTStructure();
		void build(DtaScriptVariable& theStorage);
		void populateAST(DtaScriptVariable& theStorage, BNFClauseMatchingArea& matchingAreasContainer, ScpStream& inputStream, bool bBranchDone = false);

	private:
		void populateAST(ASTCommandEnvironment& env, DtaScriptVariable& theStorage, BNFClauseMatchingArea& matchingAreasContainer, ScpStream& inputStream, bool bBranchDone = false);
		bool confirmASTnodeCreation(ASTCommandEnvironment& env, BNFClauseMatchingArea& clauseMatching, DtaASTStructure::Clause& clauseType);
		bool optimizeASTchild(ASTCommandEnvironment& env, DtaScriptVariable& clauseNode, BNFClauseMatchingArea& clauseMatching, DtaASTStructure::Clause::Sequence& sequenceType, BNFClauseMatchingArea& nonTerminalMatching, DtaASTStructure::Clause::Sequence::NonTerminal& nonTerminalType, ScpStream& inputStream);
		bool refactorASTnode(ASTCommandEnvironment& env, DtaScriptVariable& clauseNode, BNFClauseMatchingArea& clauseMatching, DtaASTStructure::Clause::Sequence& sequenceType);
	};

	class DtaASTNavigatorEnvironment : public DtaVisitorEnvironment {
		public:
			DtaASTStructure::Clause::Sequence* _pCurrentSequence;
			DtaASTStructure::Clause::Sequence* _pParentSequence;

		public:
			inline DtaASTNavigatorEnvironment(DtaASTStructure::Clause::Sequence& currentSequence) : _pCurrentSequence(&currentSequence), _pParentSequence(NULL) {}
			inline DtaASTNavigatorEnvironment(DtaASTNavigatorEnvironment& env) : _pCurrentSequence(new DtaASTStructure::Clause::Sequence), _pParentSequence(env._pCurrentSequence) {}
			~DtaASTNavigatorEnvironment();
	};
}
//##protect##"declarations"

#include "DtaVisitor.h"

namespace CodeWorker {
	class DtaASTNavigator : public DtaVisitor {
		public:
			inline DtaASTNavigator() {}
			virtual ~DtaASTNavigator();

			virtual void visitBNFAndOrJunction(BNFAndOrJunction& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFBreak(BNFBreak& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFCharBoundaries(BNFCharBoundaries& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFCharLitteral(BNFCharLitteral& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFCheck(BNFCheck& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFClause(BNFClause& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFClauseCall(BNFClauseCall& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFComplementary(BNFComplementary& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFConjunction(BNFConjunction& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFDisjunction(BNFDisjunction& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFEndOfFile(BNFEndOfFile& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFEndOfLine(BNFEndOfLine& theObject, DtaVisitorEnvironment& env);
			virtual void visitBNFFindToken(BNFFindToken& theObject, DtaVisitorEnvironment& env);
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
//##protect##"members"
			DtaASTStructure* detectASTStructure(DtaBNFScript& theBNFScript);

		private:
			void iterateSequence(GrfBlock& theObject, DtaVisitorEnvironment& env);
//##protect##"members"
	};
}

#endif
