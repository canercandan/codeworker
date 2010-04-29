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

#ifndef _GrfBlock_h_
#define _GrfBlock_h_

#include <vector>
#include <list>
#include <map>

#include "ExprScriptFunction.h"
#include "GrfCommand.h"

namespace CodeWorker {
	class GrfFunction;
	class DtaScript;
	class DtaClass;
	class DtaTargetLanguageTypeSpecifier;

	class GrfBlock : public GrfCommand {
	protected:
		DtaScript* _pScript;
		std::vector<GrfCommand*> _commands;
		std::list<GrfCommand*>* _functions;
		std::list<DtaClass*>* _classes;
		std::map<std::string, EXPRESSION_TYPE>* _variables;

	public:
		GrfBlock(GrfBlock* pParent) : GrfCommand(pParent), _pScript(NULL), _functions(NULL), _classes(NULL), _variables(NULL) {}
		virtual ~GrfBlock();

		DtaScript* getScript() const;
		inline void setScript(DtaScript* pScript) { _pScript = pScript; }
		inline bool hasLocalVariables() const { return (_variables != NULL); }

		virtual void applyRecursively(APPLY_ON_COMMAND_FUNCTION apply);

		void clear();
		void add(GrfCommand* pCommand);
		bool removeCommand(GrfCommand* pCommand);
		void moveCommands(GrfBlock& destinationBlock);
		inline int getNbCommands() const { return _commands.size(); }
		inline const std::vector<GrfCommand*>& getCommands() const { return _commands; }
		inline GrfCommand* getCommand(int i) const { return _commands[i]; }
		void setCommand(int i, GrfCommand* pCommand);

		GrfFunction* addFunction(const std::string& sFunctionName, const std::string& sTemplate, bool bGenericKey);
		GrfFunction* addFunction(GrfFunction* pFunction);
		bool removeFunction(const std::string& sFunctionName, const std::string& sTemplateInstantiation);
		bool removeGenericTemplateFunction(const std::string& sFunctionName);
		GrfFunction* getFunction(const std::string& sFunctionName, const std::string& sTemplate, bool bGenericKey) const;
		void moveFunctions(GrfBlock& destinationBlock);

		bool addLocalVariable(const std::string& sVarName, EXPRESSION_TYPE varType);
		virtual bool addBNFLocalVariable(const std::string& sVarName, EXPRESSION_TYPE varType);
		bool removeVariable(const std::string& sVarName);
		virtual EXPRESSION_TYPE getLocalVariable(const std::string& sVarName) const;
		virtual EXPRESSION_TYPE getVariable(const std::string& sVarName) const;
		void moveVariables(GrfBlock& destinationBlock);

		DtaClass* addClass(const std::string& sClassName);
		bool removeClass(const std::string& sClassName);
		DtaClass* getClass(const std::string& sClassName) const;
		void moveClasses(GrfBlock& destinationBlock);
		DtaTargetLanguageTypeSpecifier* addTargetLanguageTypeSpecifier(const std::string& sTypeSpec);
		DtaTargetLanguageTypeSpecifier* getTargetLanguageTypeSpecifier(const std::string& sTypeSpec);

		void registerLocalVariable(DtaScriptVariable* pLocalVariable);
		void removeLocalVariables();

		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual void compileCppFunctions(CppCompilerEnvironment& theCompilerEnvironment) const;
		virtual void compileCppBNFSequence(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility);
		SEQUENCE_INTERRUPTION_LIST executeAt(DtaScriptVariable& visibility, int iPosition);
		SEQUENCE_INTERRUPTION_LIST executeInto(DtaScriptVariable& visibility, int iStart, int iEnd);

		void compileCppBetween(CppCompilerEnvironment& theCompilerEnvironment, int iStart, int iEnd) const;
		void compileCppAt(CppCompilerEnvironment& theCompilerEnvironment, int iStart) const;
	};
}

#endif
