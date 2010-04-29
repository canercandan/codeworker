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

#ifndef _Workspace_h_
#define _Workspace_h_

#include <string>
#include <list>

#include "DtaCommandScript.h"

namespace CodeWorker {
	class CGQuietOutput;

	class ParseCoverageRecordingIterator; // private class for 'final info' coverage recording

	class Workspace {
	private:
		DtaCommandScript _script;
		std::string _sScriptFileName;
		std::string _sGeneratedFileName;
		std::string _sParsedFileName;
		enum SCRIPT_MODE {
			NONE,
			COMPILE,
			SCRIPT,
			PARSE_BNF,
			GENERATE,
			EXPAND,
			AUTO_EXPAND,
			TRANSLATE,
			TARGET
		};
		SCRIPT_MODE _iScriptMode;
		bool _bConsole;
		bool _bDebugMode;
		bool _bTimeMeasure;
		std::string _sCppCompilation;
		std::string _sCppServerPage;
		std::string _sQuantifyFile;
		std::string _sStandardInput;
		std::string _sStandardOutput;
		bool _bQuietMode;
		CGQuietOutput* _pQuietCGIOutput;
		bool _bNoLogo;

		static std::string _sTraceFileName;
		static bool _bWarningOnUnknownVariables;
		static std::list<GrfFunction*> _listOfExternalFunctions;

		friend class ExecuteServerPageCppHeader;
		friend class ExecuteServerPageCppBody;

	public:
		Workspace();
		~Workspace();

		inline static const std::string& getTraceFileName() { return  _sTraceFileName; }
		static void setTraceFileName(const std::string& sTraceFileName);
		inline static bool warningOnUnknownVariables() { return _bWarningOnUnknownVariables; }
		inline static void warningOnUnknownVariables(bool bWarningOnUnknownVariables) { _bWarningOnUnknownVariables = bWarningOnUnknownVariables; }
		inline bool quietMode() const { return _bQuietMode; }
		inline bool isCGIMode() const { return (_pQuietCGIOutput != NULL); }
		inline bool noLogo() { return _bNoLogo; }


		inline static void addExternalFunction(GrfFunction* pFunction) { _listOfExternalFunctions.push_back(pFunction); }
		inline static const std::list<GrfFunction*>& getExternalFunction() { return _listOfExternalFunctions; }

		static void displayHelp();

		bool checkArguments(int iNargs, char** tsArgs, EXECUTE_FUNCTION* executeFunction);
		bool checkArgumentsFromFile(const std::string& sFilename, EXECUTE_FUNCTION* executeFunction);
		bool execute(EXECUTE_FUNCTION* executeFunction);

		int executionHasFailed();

	private:
		bool executeArguments(int iNargs, char** tsArgs, EXECUTE_FUNCTION* executeFunction);
		static std::string readArgument(char** tsArgs, int& i);
		static int readInteger(char* tcArgs, int& i);

		static void writeFinalInfoParseCoverage(ScpStream& stream, DtaScriptVariable* pCoverage);
		static bool writeFinalInfoParseCoverage(ScpStream& stream, ParseCoverageRecordingIterator& it);
		static void writeFinalInfoGenerateCoverage(ScpStream& stream, DtaScriptVariable* pCoverage);
	};


	class ExecuteServerPageCppHeader : public EXECUTE_FUNCTION {
	private:
		Workspace& workspace_;

	public:
		inline ExecuteServerPageCppHeader(Workspace& workspace) : workspace_(workspace) {}
		virtual ~ExecuteServerPageCppHeader();

		virtual void run();
	};


	class ExecuteServerPageCppBody : public EXECUTE_FUNCTION {
	private:
		Workspace& workspace_;

	public:
		inline ExecuteServerPageCppBody(Workspace& workspace) : workspace_(workspace) {}
		virtual ~ExecuteServerPageCppBody();

		virtual void run();
	};
}

#endif
