#ifndef _NetSocket_h_
#define _NetSocket_h_

namespace CodeWorker {
	class ScpStream;

	class NetSocket {
		public:
			static int createINETClientSocket(const char* tcRemoteAddress, int iPort);
			static int createINETServerSocket(int iPort, int iBackLog);

			static int acceptSocket(int hServerSocket);

			static void attachInputToSocket(int hSocket);
			static void detachInputFromSocket(int hSocket);
			static void attachOutputToSocket(int hSocket);
			static bool flushOutputToSocket(int hSocket);
			static void detachOutputFromSocket(int hSocket);

			static int  receiveFromSocket(int hSocket,  char* tcBuffer, int iMaxSize);
			static bool receiveExactlyFromSocket(int hSocket,  char* tcBuffer, int iLength);
			static bool sendToSocket(int hSocket, const char* tcBuffer, int iLength);

			static void closeSocket(int hSocket);

			static unsigned long  nToHl(unsigned long  iLong);
			static unsigned short nToHs(unsigned short iShort);
			static unsigned long  hToNl(unsigned long  iLong);
			static unsigned short hToNs(unsigned short iShort);

		private:
			static void initializeSocketLibrary();
	};
}

#endif
