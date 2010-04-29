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

#ifndef _GrfCommand_h_
#define _GrfCommand_h_

namespace CodeWorker {
	class ScpStream;

	class UtlException;
	class DtaScriptVariable;
	class GrfExecutionContext;
	class GrfCommand;
	class GrfBlock;
	class CppCompilerEnvironment;
	class UtlTimer;

	class DtaVisitor;
	class DtaVisitorEnvironment;

	#ifndef SEQUENCE_INTERRUPTION_DECLARATION
	#define SEQUENCE_INTERRUPTION_DECLARATION
	enum SEQUENCE_INTERRUPTION_LIST { NO_INTERRUPTION, CONTINUE_INTERRUPTION, RETURN_INTERRUPTION, BREAK_INTERRUPTION, EXIT_INTERRUPTION, THROW_INTERRUPTION };
	#endif

	typedef void (*APPLY_ON_COMMAND_FUNCTION)(GrfCommand*);

	class GrfCommand {
	private:
		static GrfExecutionContext* _pExecutionContext;

		GrfBlock* _pParent;

	private:
		// These attributes are used only by subclasses of 'GrfExecutionContext'
		unsigned int _iCounter;
		UtlTimer* _pTimer;

		friend class GrfExecutionContext;

	protected:
		const char* _sParsingFilePtr;
		int _iFileLocation;
		friend class GrfScriptBlock;

	public:
		GrfCommand(GrfBlock* pParent = NULL);
		virtual ~GrfCommand();

		virtual void accept(DtaVisitor& visitor, DtaVisitorEnvironment& env);

		virtual const char* getFunctionName() const;
		virtual bool isAPredefinedFunction() const;
		virtual bool isABNFCommand() const;

		inline void setParent(GrfBlock* pBlock) { _pParent = pBlock; }
		inline GrfBlock* getParent() const { return _pParent; }
		static inline GrfExecutionContext* getCurrentExecutionContext() { return _pExecutionContext; }

		virtual void applyRecursively(APPLY_ON_COMMAND_FUNCTION apply);

		void setParsingInformation(const char* sName, ScpStream& stream);

		virtual SEQUENCE_INTERRUPTION_LIST execute(DtaScriptVariable& visibility);

		virtual std::string toString() const;
		virtual void compileCpp(CppCompilerEnvironment& theCompilerEnvironment) const;

	protected:
		static inline void setCurrentExecutionContext(GrfExecutionContext* pContext) { _pExecutionContext = pContext; }

		virtual void callBeforeExecutionCBK(DtaScriptVariable& visibility);
		virtual void callRecursiveBeforeExecutionCBK(GrfExecutionContext* pContext, DtaScriptVariable& visibility);
		virtual void callAfterExecutionCBK(DtaScriptVariable& visibility);
		virtual void callAfterExceptionCBK(DtaScriptVariable& visibility, UtlException& exception);
		virtual SEQUENCE_INTERRUPTION_LIST executeInternal(DtaScriptVariable& visibility) = 0;

	private:
		inline void clearCounter() { _iCounter = 0; }
		void clearTimer();
		void startTimer();
		void stopTimer();
		long getTimeInMillis() const;

		friend class GrfQuantifyExecution;
	};
}

#endif
