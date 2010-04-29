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

#ifndef _ASTCommand_h_
#define _ASTCommand_h_

namespace CodeWorker {
	class ScpStream;

	class ASTCommandEnvironment {
	};


	class ASTCommand {
		public:
			virtual ~ASTCommand();
			virtual void execute(ASTCommandEnvironment& env) = 0;
		protected:
			ExprScriptExpression* parseExpression(ScpStream& script);
			ExprScriptVariable* parseVariableExpression(ScpStream& script);
	};

	class ASTThisCommand : public ASTCommand {
		private:
			ExprScriptVariable* _pVariable;

		public:
			ASTThisCommand(ScpStream& script);
			virtual ~ASTThisCommand();
			virtual void execute(ASTCommandEnvironment& env);
	};

	class ASTValueCommand : public ASTCommand {
		private:
			ExprScriptVariable* _pVariable;

		public:
			ASTValueCommand(ScpStream& script);
			virtual ~ASTValueCommand();
			virtual void execute(ASTCommandEnvironment& env);
	};

	class ASTRefCommand : public ASTCommand {
		private:
			ExprScriptVariable* _pVariable;

		public:
			ASTRefCommand(ScpStream& script);
			virtual ~ASTRefCommand();
			virtual void execute(ASTCommandEnvironment& env);
	};

	class ASTSlideCommand : public ASTCommand {
		private:
			ExprScriptVariable* _pVariable;
			std::string _sAttribute;

		public:
			ASTSlideCommand(ScpStream& script);
			virtual ~ASTSlideCommand();
			virtual void execute(ASTCommandEnvironment& env);
	};
}

#endif
