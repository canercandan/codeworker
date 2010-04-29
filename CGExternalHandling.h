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

#ifndef _CGExternalHandling_h_
#define _CGExternalHandling_h_

#include <string>

namespace CodeWorker {
	class CGExternalHandling {
	private:
		CGExternalHandling* _pOldExternalHandling;

	public:
		CGExternalHandling();
		virtual ~CGExternalHandling();

		inline CGExternalHandling* getOldExternalHandling() const { return _pOldExternalHandling; }

		virtual std::string inputLine(bool bEcho) = 0;
		virtual std::string inputKey(bool bEcho) = 0;
		virtual void traceLine(const std::string& sLine) = 0;
		virtual void traceText(const std::string& sText) = 0;
	};

	class CGQuietOutput : public CGExternalHandling {
	private:
		std::string _sOutput;

	public:
		CGQuietOutput() {}
		virtual ~CGQuietOutput();

		inline const std::string& getOutput() const { return _sOutput; }

		virtual std::string inputLine(bool bEcho);
		virtual std::string inputKey(bool bEcho);
		virtual void traceLine(const std::string& sLine);
		virtual void traceText(const std::string& sText);
	};

	class ScpStream;

	class CGStandardInputOutput : public CGExternalHandling {
	protected:
		ScpStream* _pStandardInput;
		ScpStream* _pStandardOutput;
		bool _bInputOwner;
		bool _bOutputOwner;

	public:
		inline CGStandardInputOutput() : _pStandardInput(NULL), _bInputOwner(false), _pStandardOutput(NULL), _bOutputOwner(false) {}
		inline CGStandardInputOutput(ScpStream& theStandardInput, bool bOwner = false) : _pStandardInput(&theStandardInput), _pStandardOutput(NULL), _bInputOwner(bOwner), _bOutputOwner(false) {}
		inline CGStandardInputOutput(ScpStream& theStandardInput, ScpStream& theStandardOutput, bool bInputOwner = false, bool bOutputOwner = false) : _pStandardInput(&theStandardInput), _pStandardOutput(&theStandardOutput), _bInputOwner(bInputOwner), _bOutputOwner(bOutputOwner) {}
		CGStandardInputOutput(const std::string& sText);
		virtual ~CGStandardInputOutput();

		inline ScpStream& getStandardInput() const { return *_pStandardInput; }
		inline ScpStream& getStandardOutput() const { return *_pStandardOutput; }

		virtual std::string inputLine(bool bEcho);
		virtual std::string inputKey(bool bEcho);
		virtual void traceLine(const std::string& sLine);
		virtual void traceText(const std::string& sText);
	};

	class CGFileStandardInputOutput : public CGStandardInputOutput {
	public:
		CGFileStandardInputOutput(const std::string& sInputFilename, const std::string& sOutputFilename);
		virtual ~CGFileStandardInputOutput();
	};

	class CGRemoteDebugInputOutput : public CGExternalHandling {
	private:
		int hSocket_;
		int hServerSocket_;

	public:
		CGRemoteDebugInputOutput(const std::string& sHost, int iPort);
		virtual ~CGRemoteDebugInputOutput();

		virtual std::string inputLine(bool bEcho);
		virtual std::string inputKey(bool bEcho);
		virtual void traceLine(const std::string& sLine);
		virtual void traceText(const std::string& sText);
	};
}

#endif

