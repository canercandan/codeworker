/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2003 Cédric Lemaire

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
#	pragma warning(disable : 4786)
#endif

#ifdef WIN32
#	include <stdlib.h>
#	include <winsock.h>
#else
#	include <unistd.h>
#	include <sys/wait.h>
#	include <sys/socket.h>
#	include <sys/types.h>
#	include <netinet/in.h>
#	include <netdb.h>
#endif


#include "UtlException.h"
#include "CGRuntime.h"
#include "ScpStream.h"
#include "NetSocket.h"


namespace CodeWorker {
	bool socketReaderCBK(ScpStream& theStream, void* pData) {
		char tcBuffer[2048];
		int hSocket = (int) (long) pData;
		int iLength = recv(hSocket, tcBuffer, 2047, 0);
		if (iLength <= 0) return false;
		theStream.writeBinaryData(tcBuffer, iLength);
		return true;
	}

	bool socketWriterCBK(ScpStream& theStream, void* pData) {
		int hSocket = (int) (long) pData;
		int iLength = theStream.size();
		if (iLength <= 0) return false;
		return NetSocket::sendToSocket(hSocket, theStream.readBuffer(), iLength);
	}

	
	int NetSocket::createINETClientSocket(const char* tcRemoteAddress, int iPort) {
		initializeSocketLibrary();
		int iSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (iSocket < 0) throw UtlException("unable to open a INET stream socket");
		struct hostent* hostInfo = gethostbyname(tcRemoteAddress);
		if (hostInfo == NULL) {
			closeSocket(iSocket);
			throw UtlException("unable to get entry of host '" + std::string(tcRemoteAddress) + "'");
		}
		struct sockaddr_in sockAddr;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(iPort);
		sockAddr.sin_addr = *((struct in_addr *) hostInfo->h_addr);
		memset(&(sockAddr.sin_zero), '\0', 8);
		if (connect(iSocket, (struct sockaddr *) &sockAddr, sizeof(struct sockaddr)) < 0) {
			closeSocket(iSocket);
			throw UtlException("unable to connect a INET client to host '" + std::string(tcRemoteAddress) + "'");
		}
		return iSocket;
	}

	int NetSocket::createINETServerSocket(int iPort, int iBackLog) {
		initializeSocketLibrary();
		int iSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (iSocket < 0) throw UtlException("unable to open a INET stream socket");
		int iYes = 1;
        if (setsockopt(iSocket, SOL_SOCKET,SO_REUSEADDR, (const char*) &iYes, sizeof(int)) < 0) {
			closeSocket(iSocket);
			throw UtlException("unable to set option 'REUSE_ADDR' on a INET stream socket");
		}
		struct sockaddr_in sockAddr;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = INADDR_ANY;
		sockAddr.sin_port = htons(iPort);
		if (bind(iSocket, (struct sockaddr *) &sockAddr, sizeof sockAddr) < 0) {
			closeSocket(iSocket);
			throw UtlException("unable to bind a INET stream socket");
		}
		if (listen(iSocket, iBackLog) < 0) {
			closeSocket(iSocket);
			throw UtlException("unable to listen a INET stream socket");
		}
		return iSocket;
	}

	int NetSocket::acceptSocket(int hServerSocket) {
		int hClientSocket;
		while  ((hClientSocket = accept(hServerSocket, NULL, NULL)) == -1);
		return hClientSocket;
	}

	void NetSocket::attachInputToSocket(int hSocket) {
		if (CGRuntime::_pInputStream == NULL) throw UtlException("no input stream to attach to a socket");
		CGRuntime::_pInputStream->setStreamReaderCallback(socketReaderCBK, (void*) hSocket);
	}

	void NetSocket::detachInputFromSocket(int hSocket) {
		if (CGRuntime::_pInputStream == NULL) throw UtlException("no input stream to detach from a socket");
		CGRuntime::_pInputStream->setStreamReaderCallback(NULL);
	}

	void NetSocket::attachOutputToSocket(int hSocket) {
		if (CGRuntime::_pOutputStream == NULL) throw UtlException("no output stream to attach to a socket");
		CGRuntime::_pOutputStream->setStreamWriterCallback(socketWriterCBK, (void*) hSocket);
	}

	bool NetSocket::flushOutputToSocket(int hSocket) {
		if (CGRuntime::_pOutputStream == NULL) throw UtlException("no output stream to flush to a socket");
		int iLength = CGRuntime::_pOutputStream->size();
		if (iLength > 0) {
			if (!NetSocket::sendToSocket(hSocket, CGRuntime::_pOutputStream->readBuffer(), iLength)) return false;
			CGRuntime::_pOutputStream->setOutputLocation(0);
		}
		return true;
	}

	void NetSocket::detachOutputFromSocket(int hSocket) {
		if (CGRuntime::_pOutputStream == NULL) throw UtlException("no output stream to detach from a socket");
		CGRuntime::_pOutputStream->setStreamWriterCallback(NULL);
	}

	int  NetSocket::receiveFromSocket(int hSocket,  char* tcBuffer, int iMaxSize) {
		return recv(hSocket, tcBuffer, iMaxSize, 0);
	}

	bool NetSocket::receiveExactlyFromSocket(int hSocket,  char* tcBuffer, int iLength) {
		int iOffset = 0;
		do {
			int iNbRead = recv(hSocket, tcBuffer + iOffset, iLength, 0);
			if (iNbRead <= 0) return false;
			iOffset += iNbRead;
			iLength -= iNbRead;
		} while (iLength > 0);
		return true;
	}

	bool NetSocket::sendToSocket(int hSocket, const char* tcBuffer, int iLength) {
		int iNbChars = 0;
		int iOffset = 0;
		do {
			iOffset += iNbChars;
			iNbChars = send(hSocket, tcBuffer + iOffset, iLength - iOffset, 0);
			if (iNbChars < 0) return false;
		} while (iOffset + iNbChars < iLength);
		return true;
	}

	void NetSocket::closeSocket(int hSocket) {
#ifdef WIN32
		closesocket(hSocket);
#else
		close(hSocket);
#endif
	}

	void NetSocket::initializeSocketLibrary() {
#ifdef WIN32
		static bool bInitialized = false;
		if (!bInitialized) {
			bInitialized = true;
			WORD wVersionRequested = MAKEWORD(2, 0);
			WSADATA wsaData;
			if (WSAStartup(wVersionRequested, &wsaData) != 0) throw UtlException("unable to initialize the Windows Socket Library!");
			if ((LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 0)) {
				WSACleanup();
				throw UtlException("the Windows Socket Library doesn't support version 2.0 and higher!");
			}
		}
#endif
	}

	unsigned long  NetSocket::nToHl(unsigned long  iLong)  { return ntohl(iLong); }
	unsigned short NetSocket::nToHs(unsigned short iShort) { return ntohs(iShort); }
	unsigned long  NetSocket::hToNl(unsigned long  iLong)  { return htonl(iLong); }
	unsigned short NetSocket::hToNs(unsigned short iShort) { return htons(iShort); }
}
