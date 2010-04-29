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
#pragma warning(disable : 4786)
#endif

#include "ScpStream.h"

#include "CGRuntime.h"
#include "NetSocket.h"
#include "CGExternalHandling.h"

namespace CodeWorker {
	CGExternalHandling::CGExternalHandling() {
		_pOldExternalHandling = CGRuntime::getExternalHandling();
		CGRuntime::setExternalHandling(this);
	}

	CGExternalHandling::~CGExternalHandling() {
		CGRuntime::setExternalHandling(_pOldExternalHandling);
	}


	CGQuietOutput::~CGQuietOutput() {}

	std::string CGQuietOutput::inputLine(bool bEcho) {
		char tcText[16384];
		if (CGRuntime::getExternalHandling() != this) return CGRuntime::getExternalHandling()->inputLine(bEcho);
		if (getOldExternalHandling() != NULL) return getOldExternalHandling()->inputLine(bEcho);
#ifndef WIN32
		closeKeyboard();
#endif
		std::cin.getline(tcText, 16383);
#ifndef WIN32
		initKeyboard();
#endif
		return tcText;
	}

	std::string CGQuietOutput::inputKey(bool bEcho) {
		if (CGRuntime::getExternalHandling() != this) return CGRuntime::getExternalHandling()->inputKey(bEcho);
		if (getOldExternalHandling() != NULL) return getOldExternalHandling()->inputKey(bEcho);
		int iKey = std::cin.peek();
		return "";
	}

	void CGQuietOutput::traceLine(const std::string& sLine) {
		_sOutput += sLine + CGRuntime::endl();
	}

	void CGQuietOutput::traceText(const std::string& sText) {
		_sOutput += sText;
	}


	CGStandardInputOutput::CGStandardInputOutput(const std::string& sText) : _bInputOwner(true), _pStandardOutput(NULL), _bOutputOwner(false) {
		_pStandardInput = new ScpStream;
		(*_pStandardInput) << sText;
	}

	CGStandardInputOutput::~CGStandardInputOutput() {
		if (_bInputOwner) delete _pStandardInput;
		if (_bOutputOwner) delete _pStandardOutput;
	}

	std::string CGStandardInputOutput::inputLine(bool bEcho) {
		if (_pStandardInput != NULL) {
			std::string sLine;
			_pStandardInput->readLine(sLine);
			if (bEcho) traceLine(sLine);
			return sLine;
		}
		char tcText[16384];
		if (CGRuntime::getExternalHandling() != this) return CGRuntime::getExternalHandling()->inputLine(bEcho);
		if (getOldExternalHandling() != NULL) return getOldExternalHandling()->inputLine(bEcho);
#ifndef WIN32
		closeKeyboard();
#endif
		std::cin.getline(tcText, 16383);
#ifndef WIN32
		initKeyboard();
#endif
		return tcText;
	}

	std::string CGStandardInputOutput::inputKey(bool bEcho) {
		if (_pStandardInput != NULL) {
			std::string sLine;
			int iChar = _pStandardInput->readChar();
			char tcText[] = {0, 0};
			if (iChar > 0) tcText[0] = (char) iChar;
			if (bEcho) traceText(tcText);
			return tcText;
		}
		if (CGRuntime::getExternalHandling() != this) return CGRuntime::getExternalHandling()->inputKey(bEcho);
		if (getOldExternalHandling() != NULL) return getOldExternalHandling()->inputKey(bEcho);
		int iKey = std::cin.peek();
		return "";
	}

	void CGStandardInputOutput::traceLine(const std::string& sLine) {
		if (_pStandardOutput != NULL) {
			(*_pStandardOutput) << sLine << ScpStream::ENDL;
		} else {
			if (CGRuntime::getExternalHandling() != this) CGRuntime::getExternalHandling()->traceLine(sLine);
			else if (getOldExternalHandling() != NULL) getOldExternalHandling()->traceLine(sLine);
			else std::cout << sLine << std::endl;
		}
	}

	void CGStandardInputOutput::traceText(const std::string& sText) {
		if (_pStandardOutput != NULL) {
			(*_pStandardOutput) << sText;
		} else {
			if (CGRuntime::getExternalHandling() != this) CGRuntime::getExternalHandling()->traceText(sText);
			else if (getOldExternalHandling() != NULL) getOldExternalHandling()->traceText(sText);
			else std::cout << sText << std::flush;
		}
	}


	CGFileStandardInputOutput::CGFileStandardInputOutput(const std::string& sInputFilename, const std::string& sOutputFilename) {
		if (!sInputFilename.empty()) {
			_pStandardInput = new ScpStream(sInputFilename, ScpStream::IN | ScpStream::PATH);
			_bInputOwner = true;
		}
		if (!sOutputFilename.empty()) {
			_pStandardOutput = new ScpStream();
			_pStandardOutput->setFilename(sOutputFilename);
			_bOutputOwner = true;
		}
	}

	CGFileStandardInputOutput::~CGFileStandardInputOutput() {
		if (_pStandardOutput != NULL) {
			_pStandardOutput->saveIntoFile(_pStandardOutput->getFilename(), true);
		}
	}


	CGRemoteDebugInputOutput::CGRemoteDebugInputOutput(const std::string& sHost, int iPort) {
		if (sHost.empty()) {
			// server application
			hServerSocket_ = NetSocket::createINETServerSocket(iPort, 5);
			hSocket_ = NetSocket::acceptSocket(hServerSocket_);
		} else {
			// client application
			hServerSocket_ = -1;
			hSocket_ = NetSocket::createINETClientSocket(sHost.c_str(), iPort);
		}
	}

	CGRemoteDebugInputOutput::~CGRemoteDebugInputOutput() {
		NetSocket::closeSocket(hSocket_);
		if (hServerSocket_ >= 0) NetSocket::closeSocket(hServerSocket_);
	}

	std::string CGRemoteDebugInputOutput::inputLine(bool bEcho) {
		if (!NetSocket::sendToSocket(hSocket_, "I", 1)) {
			throw UtlException("Remote debug protocol error: unable to send a 'inputLine()' header properly");
		}
		long iLength;
		if (!NetSocket::receiveExactlyFromSocket(hSocket_, (char*) &iLength, sizeof(long))) {
			throw UtlException("Remote debug protocol error: incorrect number of bytes received");
		}
		iLength = NetSocket::nToHl(iLength);
		char* tcInput = new char[iLength + 1];
		if (!NetSocket::receiveExactlyFromSocket(hSocket_, tcInput, iLength)) {
			delete tcInput;
			throw UtlException("Remote debug protocol error: incorrect number of bytes received");
		}
		tcInput[iLength] = '\0';
		std::string sResult = tcInput;
		delete tcInput;
		return sResult;
	}

	std::string CGRemoteDebugInputOutput::inputKey(bool bEcho) { return ""; }

	void CGRemoteDebugInputOutput::traceLine(const std::string& sLine) {
		if (!NetSocket::sendToSocket(hSocket_, "T", 1)) {
			throw UtlException("Remote debug protocol error: unable to send a 'traceLine()' header properly");
		}
		long iLength = NetSocket::hToNl(sLine.size() + 1);
		if (!NetSocket::sendToSocket(hSocket_, (char*) &iLength, sizeof(long))) {
			throw UtlException("Remote debug protocol error: unable to send a 'traceLine()' properly");
		}
		if (!NetSocket::sendToSocket(hSocket_, sLine.c_str(), sLine.size())) {
			throw UtlException("Remote debug protocol error: unable to send a 'traceLine()' properly");
		}
		if (!NetSocket::sendToSocket(hSocket_, "\n", 1)) {
			throw UtlException("Remote debug protocol error: unable to send a 'traceLine()' properly");
		}
	}

	void CGRemoteDebugInputOutput::traceText(const std::string& sText) {
		if (!NetSocket::sendToSocket(hSocket_, "T", 1)) {
			throw UtlException("Remote debug protocol error: unable to send a 'traceText()' header properly");
		}
		long iLength = NetSocket::hToNl(sText.size());
		if (!NetSocket::sendToSocket(hSocket_, (char*) &iLength, sizeof(long))) {
			throw UtlException("Remote debug protocol error: unable to send a 'traceText()' properly");
		}
		if (!NetSocket::sendToSocket(hSocket_, sText.c_str(), sText.size())) {
			throw UtlException("Remote debug protocol error: unable to send a 'traceText()' properly");
		}
	}
}
