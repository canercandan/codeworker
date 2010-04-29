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

#ifndef _CppCompilerEnvironment_h_
#define _CppCompilerEnvironment_h_

#include <string>
#include <list>
#include <set>

namespace CodeWorker {
#define CW_BODY_STREAM theCompilerEnvironment.getBody()
#define CW_BODY_INDENT theCompilerEnvironment.getBody() << theCompilerEnvironment.getIndentation()
#define CW_BODY_ENDL theCompilerEnvironment.getBody().endl()

	class ScpStream;
	class DtaPatternScript;
	class GrfFunction;
	class BNFClause;
	class CppVariableScope;
	class UtlException;
	class GrfForeach;


	struct CppScriptSession {
		std::string _sIndentation;
		std::set<std::string> _setOfProjectFunctions;
		std::set<std::string> _setOfProjectClauses;
		std::string _sFilename;
		std::string _sCppRadical;
		mutable DtaPatternScript* _header;
		mutable DtaPatternScript* _body;
		mutable std::list<CppVariableScope*> _stackOfScopes;
		std::list<GrfForeach*> _listOfForeachStatements;

		CppScriptSession(const std::string& sFilename, const std::string& sCppRadical);
		CppScriptSession(const CppScriptSession& copy);
		~CppScriptSession();

		CppScriptSession& operator =(const CppScriptSession& copy);

		void incrementIndentation();
		void decrementIndentation();
		void pushVariableScope() const;
		void popVariableScope() const;
		void pushForeach(GrfForeach* pForeach);
		GrfForeach* getLastForeach() const;
		void popForeach();
		void catchFilename();
	};

	class CppCompilerEnvironment {
	private:
		std::string _sCppProjectDirectory;
		std::string _sCodeWorkerDirectory;
		int _iInlineScriptCounter;
		int _iSwitchNumber;
		int _iCursorNumber;
		bool _bBracketsToNextBlock;
		bool _bCarriageReturnAfterBlock;
		const GrfFunction* _pCurrentFunction;
		const BNFClause* _pCurrentClause;
		int _iPointerToDeclarations;
		int _iBNFStepperCursor;
		std::list<int> _listOfLastRepeatCursors;
		std::list<bool> _listOfLastRepeatCursorUsed;
		std::list<std::string> _listOfProjectModules;
		std::list<CppScriptSession> _listOfScriptSessions;
		std::set<std::string> _globalVariables;
		bool _bErrorEncountered;

	public:
		CppCompilerEnvironment(const std::string& sCppProjectDirectory);
		~CppCompilerEnvironment();

		bool pushFilename(const std::string& sFilename);

		std::string getRadical() const;
		std::string getMainRadical() const;
		static std::string filename2Module(const std::string& sFilename);
		static std::string getRadical(const std::string& sFile);
		static std::string getRelativePath(const std::string& sFile);
		static std::string convertTemplateKey(const std::string& sKey);
		static std::string convertToCppVariable(const std::string& sVariable);

		std::string getIncludeParentScript() const;
		inline const std::list<std::string>& getProjectModules() const { return _listOfProjectModules; }
		ScpStream& getHeader() const;
		ScpStream& getMainHeader() const;
		ScpStream& getBody() const;
		inline const std::string& getIndentation() const { return getCurrentScriptSession()._sIndentation; }
		inline void setIndentation(const std::string& sIndentation) { _listOfScriptSessions.front()._sIndentation = sIndentation; }
		void incrementIndentation();
		void pushVariableScope();
		inline const CppScriptSession& getCurrentScriptSession() const { return _listOfScriptSessions.front(); }
		inline const GrfFunction* getCurrentFunction() const { return _pCurrentFunction; }
		inline void setCurrentFunction(const GrfFunction* pFunction) { _pCurrentFunction = pFunction; }
		inline const BNFClause* getCurrentClause() const { return _pCurrentClause; }
		inline void setCurrentClause(const BNFClause* pClause) { _pCurrentClause = pClause; }
		inline int getPointerToDeclarations() const { return _iPointerToDeclarations; }
		inline void setPointerToDeclarations(int iPointerToDeclarations) { _iPointerToDeclarations = iPointerToDeclarations; }
		inline int getBNFStepperCursor() const { return _iBNFStepperCursor; }
		inline void setBNFStepperCursor(int iCursor) { _iBNFStepperCursor = iCursor; }
		void pushLastRepeatCursor(int iCursor);
		int getLastRepeatCursor(bool bUsed = false);
		bool popLastRepeatCursor();
		inline const std::set<std::string>& getGlobalVariables() const { return _globalVariables; }

		std::string newInlineScriptFilename();
		void newFunction(const GrfFunction* pFunction);
		void newClause(const BNFClause* pClause);
		std::string getFunctionModule(const std::string& sFunction) const;
		std::string getClauseModule(const std::string& sClause) const;
		bool addVariable(const std::string& sVariable);
		bool existVariable(const std::string& sVariable) const;
		void addGlobalVariable(const std::string& sVariable);
		void setClauseReturnValue(const std::string& sClauseName);
		bool isClauseReturnValue(const std::string& sVariableName) const;
		void hasEvaluatedExpressionInScope(bool bEvaluated);
		bool hasEvaluatedExpressionInScope() const;
		int newSwitch();
		int newCursor();
		void popVariableScope();
		void decrementIndentation();
		inline bool bracketsToNextBlock() const { return _bBracketsToNextBlock; }
		inline void bracketsToNextBlock(bool bBrackets) { _bBracketsToNextBlock = bBrackets; }
		inline bool carriageReturnAfterBlock() const { return _bCarriageReturnAfterBlock; }
		inline void carriageReturnAfterBlock(bool bCarriage) { _bCarriageReturnAfterBlock = bCarriage; }
		void pushForeach(GrfForeach* pForeach);
		GrfForeach* getLastForeach() const;
		void popForeach();
		void catchFilename(UtlException& exception);
		void popFilename();

		void optimizeSources();
		void translateToTargetLanguage(const std::string& sTargetLanguage);

	private:
		CppCompilerEnvironment();
		CppCompilerEnvironment(const CppCompilerEnvironment&);
		CppCompilerEnvironment& operator =(const CppCompilerEnvironment&);
		bool operator ==(const CppCompilerEnvironment&);
		bool operator !=(const CppCompilerEnvironment&);

		void generateDynamicPackages();
		void generateDSP();
		void generateMakefile();
	};


	class CppNewVariableScopeEnvironment {
	private:
		const CppScriptSession& _session;
		std::list<CppVariableScope*> _stackOfScopes;

	public:
		CppNewVariableScopeEnvironment(CppCompilerEnvironment& env);
		~CppNewVariableScopeEnvironment();
	};

}

#endif
