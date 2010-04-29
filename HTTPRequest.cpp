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

#include <stdio.h>

#ifdef WIN32
#	pragma warning(disable : 4503)
#	pragma warning(disable : 4786)
#	include <rpc.h>
#endif

#ifdef CODEWORKER_CURL_LIB
enum CURLcode {
  CURLE_OK = 0,
  CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
  CURLE_FAILED_INIT,             /* 2 */
  CURLE_URL_MALFORMAT,           /* 3 */
  CURLE_URL_MALFORMAT_USER,      /* 4 */
  CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
  CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
  CURLE_COULDNT_CONNECT,         /* 7 */
  CURLE_FTP_WEIRD_SERVER_REPLY,  /* 8 */
  CURLE_FTP_ACCESS_DENIED,       /* 9 */
  CURLE_FTP_USER_PASSWORD_INCORRECT, /* 10 */
  CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
  CURLE_FTP_WEIRD_USER_REPLY,    /* 12 */
  CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
  CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
  CURLE_FTP_CANT_GET_HOST,       /* 15 */
  CURLE_FTP_CANT_RECONNECT,      /* 16 */
  CURLE_FTP_COULDNT_SET_BINARY,  /* 17 */
  CURLE_PARTIAL_FILE,            /* 18 */
  CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
  CURLE_FTP_WRITE_ERROR,         /* 20 */
  CURLE_FTP_QUOTE_ERROR,         /* 21 */
  CURLE_HTTP_RETURNED_ERROR,     /* 22 */
  CURLE_WRITE_ERROR,             /* 23 */
  CURLE_MALFORMAT_USER,          /* 24 - user name is illegally specified */
  CURLE_FTP_COULDNT_STOR_FILE,   /* 25 - failed FTP upload */
  CURLE_READ_ERROR,              /* 26 - could open/read from file */
  CURLE_OUT_OF_MEMORY,           /* 27 */
  CURLE_OPERATION_TIMEOUTED,     /* 28 - the timeout time was reached */
  CURLE_FTP_COULDNT_SET_ASCII,   /* 29 - TYPE A failed */
  CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
  CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
  CURLE_FTP_COULDNT_GET_SIZE,    /* 32 - the SIZE command failed */
  CURLE_HTTP_RANGE_ERROR,        /* 33 - RANGE "command" didn't work */
  CURLE_HTTP_POST_ERROR,         /* 34 */
  CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
  CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
  CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
  CURLE_LDAP_CANNOT_BIND,        /* 38 */
  CURLE_LDAP_SEARCH_FAILED,      /* 39 */
  CURLE_LIBRARY_NOT_FOUND,       /* 40 */
  CURLE_FUNCTION_NOT_FOUND,      /* 41 */
  CURLE_ABORTED_BY_CALLBACK,     /* 42 */
  CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
  CURLE_BAD_CALLING_ORDER,       /* 44 */
  CURLE_HTTP_PORT_FAILED,        /* 45 - HTTP Interface operation failed */
  CURLE_BAD_PASSWORD_ENTERED,    /* 46 - my_getpass() returns fail */
  CURLE_TOO_MANY_REDIRECTS ,     /* 47 - catch endless re-direct loops */
  CURLE_UNKNOWN_TELNET_OPTION,   /* 48 - User specified an unknown option */
  CURLE_TELNET_OPTION_SYNTAX ,   /* 49 - Malformed telnet option */
  CURLE_OBSOLETE,	         /* 50 - removed after 7.7.3 */
  CURLE_SSL_PEER_CERTIFICATE,    /* 51 - peer's certificate wasn't ok */
  CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
  CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
  CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                    default */
  CURLE_SEND_ERROR,              /* 55 - failed sending network data */
  CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
  CURLE_SHARE_IN_USE,            /* 57 - share is in use */
  CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
  CURLE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
  CURLE_SSL_CACERT,              /* 60 - problem with the CA cert (path?) */
  CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized transfer encoding */

  CURL_LAST /* never use! */
};

/* long may be 32 or 64 bits, but we should never depend on anything else
   but 32 */
#define CURLOPTTYPE_LONG          0
#define CURLOPTTYPE_OBJECTPOINT   10000
#define CURLOPTTYPE_FUNCTIONPOINT 20000

#define CINIT(name,type,number) CURLOPT_ ## name = CURLOPTTYPE_ ## type + number

enum CURLoption {
  CINIT(NOTHING, LONG, 0), /********* the first one is unused ************/
  
  /* This is the FILE * or void * the regular output should be written to. */
  CINIT(FILE, OBJECTPOINT, 1),

  /* The full URL to get/put */
  CINIT(URL,  OBJECTPOINT, 2),

  /* Port number to connect to, if other than default. Specify the CONF_PORT
     flag in the CURLOPT_FLAGS to activate this */
  CINIT(PORT, LONG, 3),

  /* Name of proxy to use. Specify the CONF_PROXY flag in the CURLOPT_FLAGS to
     activate this */
  CINIT(PROXY, OBJECTPOINT, 4),
  
  /* Name and password to use when fetching. Specify the CONF_USERPWD flag in
     the CURLOPT_FLAGS to activate this */
  CINIT(USERPWD, OBJECTPOINT, 5),

  /* Name and password to use with Proxy. Specify the CONF_PROXYUSERPWD 
     flag in the CURLOPT_FLAGS to activate this */
  CINIT(PROXYUSERPWD, OBJECTPOINT, 6),

  /* Range to get, specified as an ASCII string. Specify the CONF_RANGE flag
     in the CURLOPT_FLAGS to activate this */
  CINIT(RANGE, OBJECTPOINT, 7),

  /* not used */

  /* Specified file stream to upload from (use as input): */
  CINIT(INFILE, OBJECTPOINT, 9),

  /* Buffer to receive error messages in, must be at least CURL_ERROR_SIZE
   * bytes big. If this is not used, error messages go to stderr instead: */
  CINIT(ERRORBUFFER, OBJECTPOINT, 10),

  /* Function that will be called to store the output (instead of fwrite). The
   * parameters will use fwrite() syntax, make sure to follow them. */
  CINIT(WRITEFUNCTION, FUNCTIONPOINT, 11),

  /* Function that will be called to read the input (instead of fread). The
   * parameters will use fread() syntax, make sure to follow them. */
  CINIT(READFUNCTION, FUNCTIONPOINT, 12),

  /* Time-out the read operation after this amount of seconds */
  CINIT(TIMEOUT, LONG, 13),

  /* If the CURLOPT_INFILE is used, this can be used to inform libcurl about
   * how large the file being sent really is. That allows better error
   * checking and better verifies that the upload was succcessful. -1 means
   * unknown size. */
  CINIT(INFILESIZE, LONG, 14),

  /* POST input fields. */
  CINIT(POSTFIELDS, OBJECTPOINT, 15),

  /* Set the referer page (needed by some CGIs) */
  CINIT(REFERER, OBJECTPOINT, 16),

  /* Set the FTP PORT string (interface name, named or numerical IP address)
     Use i.e '-' to use default address. */
  CINIT(FTPPORT, OBJECTPOINT, 17),

  /* Set the User-Agent string (examined by some CGIs) */
  CINIT(USERAGENT, OBJECTPOINT, 18),

  /* If the download receives less than "low speed limit" bytes/second
   * during "low speed time" seconds, the operations is aborted.
   * You could i.e if you have a pretty high speed connection, abort if
   * it is less than 2000 bytes/sec during 20 seconds.   
   */

  /* Set the "low speed limit" */
  CINIT(LOW_SPEED_LIMIT, LONG , 19),

  /* Set the "low speed time" */
  CINIT(LOW_SPEED_TIME, LONG, 20),

  /* Set the continuation offset */
  CINIT(RESUME_FROM, LONG, 21),

  /* Set cookie in request: */
  CINIT(COOKIE, OBJECTPOINT, 22),

  /* This points to a linked list of headers, struct curl_slist kind */
  CINIT(HTTPHEADER, OBJECTPOINT, 23),

  /* This points to a linked list of post entries, struct HttpPost */
  CINIT(HTTPPOST, OBJECTPOINT, 24),

  /* name of the file keeping your private SSL-certificate */
  CINIT(SSLCERT, OBJECTPOINT, 25),

  /* password for the SSL-private key, keep this for compatibility */
  CINIT(SSLCERTPASSWD, OBJECTPOINT, 26),
  /* password for the SSL private key */
  CINIT(SSLKEYPASSWD, OBJECTPOINT, 26),
  
  /* send TYPE parameter? */
  CINIT(CRLF, LONG, 27),

  /* send linked-list of QUOTE commands */
  CINIT(QUOTE, OBJECTPOINT, 28),

  /* send FILE * or void * to store headers to, if you use a callback it
     is simply passed to the callback unmodified */
  CINIT(WRITEHEADER, OBJECTPOINT, 29),

  /* point to a file to read the initial cookies from, also enables
     "cookie awareness" */
  CINIT(COOKIEFILE, OBJECTPOINT, 31),

  /* What version to specifly try to use.
     See CURL_SSLVERSION defines below. */
  CINIT(SSLVERSION, LONG, 32),

  /* What kind of HTTP time condition to use, see defines */
  CINIT(TIMECONDITION, LONG, 33),

  /* Time to use with the above condition. Specified in number of seconds
     since 1 Jan 1970 */
  CINIT(TIMEVALUE, LONG, 34),

  /* HTTP request, for odd commands like DELETE, TRACE and others */
  /* OBSOLETE DEFINE, left for tradition only */
  CINIT(HTTPREQUEST, OBJECTPOINT, 35),

  /* Custom request, for customizing the get command like
     HTTP: DELETE, TRACE and others
     FTP: to use a different list command
     */
  CINIT(CUSTOMREQUEST, OBJECTPOINT, 36),

  /* HTTP request, for odd commands like DELETE, TRACE and others */
  CINIT(STDERR, OBJECTPOINT, 37),

  /* 38 is not used */

  /* send linked-list of post-transfer QUOTE commands */
  CINIT(POSTQUOTE, OBJECTPOINT, 39),

  /* Pass a pointer to string of the output using full variable-replacement
     as described elsewhere. */
  CINIT(WRITEINFO, OBJECTPOINT, 40),

  /* Previous FLAG bits */
  CINIT(VERBOSE, LONG, 41),      /* talk a lot */
  CINIT(HEADER, LONG, 42),       /* throw the header out too */
  CINIT(NOPROGRESS, LONG, 43),   /* shut off the progress meter */
  CINIT(NOBODY, LONG, 44),       /* use HEAD to get http document */
  CINIT(FAILONERROR, LONG, 45),  /* no output on http error codes >= 300 */
  CINIT(UPLOAD, LONG, 46),       /* this is an upload */
  CINIT(POST, LONG, 47),         /* HTTP POST method */
  CINIT(FTPLISTONLY, LONG, 48),  /* Use NLST when listing ftp dir */

  CINIT(FTPAPPEND, LONG, 50),    /* Append instead of overwrite on upload! */

  /* Specify whether to read the user+password from the .netrc or the URL.
   * This must be one of the CURL_NETRC_* enums below. */
  CINIT(NETRC, LONG, 51),

  CINIT(FOLLOWLOCATION, LONG, 52),  /* use Location: Luke! */

  /* This FTPASCII name is now obsolete, to be removed, use the TRANSFERTEXT
     instead. It goes for more protocols than just ftp... */
  CINIT(FTPASCII, LONG, 53),     /* use TYPE A for transfer */

  CINIT(TRANSFERTEXT, LONG, 53), /* transfer data in text/ASCII format */
  CINIT(PUT, LONG, 54),          /* PUT the input file */

  CINIT(MUTE, LONG, 55),         /* OBSOLETE OPTION, removed in 7.8 */

  /* Function that will be called instead of the internal progress display
   * function. This function should be defined as the curl_progress_callback
   * prototype defines. */
  CINIT(PROGRESSFUNCTION, FUNCTIONPOINT, 56),

  /* Data passed to the progress callback */
  CINIT(PROGRESSDATA, OBJECTPOINT, 57),

  /* We want the referer field set automatically when following locations */
  CINIT(AUTOREFERER, LONG, 58),

  /* Port of the proxy, can be set in the proxy string as well with:
     "[host]:[port]" */
  CINIT(PROXYPORT, LONG, 59),

  /* size of the POST input data, if strlen() is not good to use */
  CINIT(POSTFIELDSIZE, LONG, 60),

  /* tunnel non-http operations through a HTTP proxy */
  CINIT(HTTPPROXYTUNNEL, LONG, 61),

  /* Set the interface string to use as outgoing network interface */
  CINIT(INTERFACE, OBJECTPOINT, 62),

  /* Set the krb4 security level, this also enables krb4 awareness.  This is a
   * string, 'clear', 'safe', 'confidential' or 'private'.  If the string is
   * set but doesn't match one of these, 'private' will be used.  */
  CINIT(KRB4LEVEL, OBJECTPOINT, 63),

  /* Set if we should verify the peer in ssl handshake, set 1 to verify. */
  CINIT(SSL_VERIFYPEER, LONG, 64),
  
  /* The CApath or CAfile used to validate the peer certificate
     this option is used only if SSL_VERIFYPEER is true */
  CINIT(CAINFO, OBJECTPOINT, 65),

  /* Function pointer to replace the internal password prompt */
  CINIT(PASSWDFUNCTION, FUNCTIONPOINT, 66),

  /* Custom pointer that gets passed as first argument to the password
     function */
  CINIT(PASSWDDATA, OBJECTPOINT, 67),
  
  /* Maximum number of http redirects to follow */
  CINIT(MAXREDIRS, LONG, 68),

  /* Pass a pointer to a time_t to get a possible date of the requested
     document! Pass a NULL to shut it off. */
  CINIT(FILETIME, OBJECTPOINT, 69),

  /* This points to a linked list of telnet options */
  CINIT(TELNETOPTIONS, OBJECTPOINT, 70),

  /* Max amount of cached alive connections */
  CINIT(MAXCONNECTS, LONG, 71),

  /* What policy to use when closing connections when the cache is filled
     up */
  CINIT(CLOSEPOLICY, LONG, 72),

  /* Callback to use when CURLCLOSEPOLICY_CALLBACK is set */
  CINIT(CLOSEFUNCTION, FUNCTIONPOINT, 73),

  /* Set to explicitly use a new connection for the upcoming transfer.
     Do not use this unless you're absolutely sure of this, as it makes the
     operation slower and is less friendly for the network. */
  CINIT(FRESH_CONNECT, LONG, 74),

  /* Set to explicitly forbid the upcoming transfer's connection to be re-used
     when done. Do not use this unless you're absolutely sure of this, as it
     makes the operation slower and is less friendly for the network. */
  CINIT(FORBID_REUSE, LONG, 75),

  /* Set to a file name that contains random data for libcurl to use to
     seed the random engine when doing SSL connects. */
  CINIT(RANDOM_FILE, OBJECTPOINT, 76),

  /* Set to the Entropy Gathering Daemon socket pathname */
  CINIT(EGDSOCKET, OBJECTPOINT, 77),

  /* Time-out connect operations after this amount of seconds, if connects
     are OK within this time, then fine... This only aborts the connect
     phase. [Only works on unix-style/SIGALRM operating systems] */
  CINIT(CONNECTTIMEOUT, LONG, 78),

  /* Function that will be called to store headers (instead of fwrite). The
   * parameters will use fwrite() syntax, make sure to follow them. */
  CINIT(HEADERFUNCTION, FUNCTIONPOINT, 79),

  /* Set this to force the HTTP request to get back to GET. Only really usable
     if POST, PUT or a custom request have been used first.
   */
  CINIT(HTTPGET, LONG, 80),

  /* Set if we should verify the Common name from the peer certificate in ssl
   * handshake, set 1 to check existence, 2 to ensure that it matches the
   * provided hostname. */
  CINIT(SSL_VERIFYHOST, LONG, 81),

  /* Specify which file name to write all known cookies in after completed
     operation. Set file name to "-" (dash) to make it go to stdout. */
  CINIT(COOKIEJAR, OBJECTPOINT, 82),

  /* Specify which SSL ciphers to use */
  CINIT(SSL_CIPHER_LIST, OBJECTPOINT, 83),

  /* Specify which HTTP version to use! This must be set to one of the
     CURL_HTTP_VERSION* enums set below. */
  CINIT(HTTP_VERSION, LONG, 84),

  /* Specificly switch on or off the FTP engine's use of the EPSV command. By
     default, that one will always be attempted before the more traditional
     PASV command. */     
  CINIT(FTP_USE_EPSV, LONG, 85),

  /* type of the file keeping your SSL-certificate ("DER", "PEM", "ENG") */
  CINIT(SSLCERTTYPE, OBJECTPOINT, 86),

  /* name of the file keeping your private SSL-key */
  CINIT(SSLKEY, OBJECTPOINT, 87),

  /* type of the file keeping your private SSL-key ("DER", "PEM", "ENG") */
  CINIT(SSLKEYTYPE, OBJECTPOINT, 88),

  /* crypto engine for the SSL-sub system */
  CINIT(SSLENGINE, OBJECTPOINT, 89),

  /* set the crypto engine for the SSL-sub system as default
     the param has no meaning...
   */
  CINIT(SSLENGINE_DEFAULT, LONG, 90),

  /* Non-zero value means to use the global dns cache */
  CINIT(DNS_USE_GLOBAL_CACHE, LONG, 91),

  /* DNS cache timeout */
  CINIT(DNS_CACHE_TIMEOUT, LONG, 92),

  /* send linked-list of pre-transfer QUOTE commands (Wesley Laxton)*/
  CINIT(PREQUOTE, OBJECTPOINT, 93),

  /* set the debug function */
  CINIT(DEBUGFUNCTION, FUNCTIONPOINT, 94),

  /* set the data for the debug function */
  CINIT(DEBUGDATA, OBJECTPOINT, 95),

  /* mark this as start of a cookie session */
  CINIT(COOKIESESSION, LONG, 96),

  /* The CApath directory used to validate the peer certificate
     this option is used only if SSL_VERIFYPEER is true */
  CINIT(CAPATH, OBJECTPOINT, 97),

  /* Instruct libcurl to use a smaller receive buffer */
  CINIT(BUFFERSIZE, LONG, 98),

  /* Instruct libcurl to not use any signal/alarm handlers, even when using
     timeouts. This option is useful for multi-threaded applications.
     See libcurl-the-guide for more background information. */
  CINIT(NOSIGNAL, LONG, 99),
  
  /* Provide a CURLShare for mutexing non-ts data */
  CINIT(SHARE, OBJECTPOINT, 100),

  /* indicates type of proxy. accepted values are CURLPROXY_HTTP (default),
     CURLPROXY_SOCKS4 and CURLPROXY_SOCKS5. */
  CINIT(PROXYTYPE, LONG, 101),

  /* Set the Accept-Encoding string. Use this to tell a server you would like
     the response to be compressed. */
  CINIT(ENCODING, OBJECTPOINT, 102),
 
  /* Set pointer to private data */
  CINIT(PRIVATE, OBJECTPOINT, 103),

  /* Set aliases for HTTP 200 in the HTTP Response header */
  CINIT(HTTP200ALIASES, OBJECTPOINT, 104),

  /* Continue to send authentication (user+password) when following locations,
     even when hostname changed. This can potentionally send off the name
     and password to whatever host the server decides. */
  CINIT(UNRESTRICTED_AUTH, LONG, 105),

  /* Specificly switch on or off the FTP engine's use of the EPRT command ( it
     also disables the LPRT attempt). By default, those ones will always be
     attempted before the good old traditional PORT command. */     
  CINIT(FTP_USE_EPRT, LONG, 106),

  CURLOPT_LASTENTRY /* the last unused */
};

typedef void* CURL;

typedef char* (*CURL_ESCAPE)(const char*, int);
typedef void (*CURL_FREE)(void*);
typedef CURL* (*CURL_EASY_INIT)(void);
typedef CURLcode (*CURL_EASY_SETOPT)(CURL*, CURLoption, ...);
typedef CURLcode (*CURL_EASY_PERFORM)(CURL*);
typedef void (*CURL_EASY_CLEANUP)(CURL*);

CURL_ESCAPE curl_escape = NULL;
CURL_FREE curl_free = NULL;
CURL_EASY_INIT curl_easy_init = NULL;
CURL_EASY_SETOPT curl_easy_setopt = NULL;
CURL_EASY_PERFORM curl_easy_perform = NULL;
CURL_EASY_CLEANUP curl_easy_cleanup = NULL;

#include "DynPackage.h"
#endif

#include "UtlException.h"
#include "CGRuntime.h"

#include "DtaScriptVariable.h"
#include "HTTPRequest.h"


namespace CodeWorker {

#ifdef CODEWORKER_CURL_LIB
	struct CurlOutputDataSection {
		char* _tcSection;
		int   _iLength;
		CurlOutputDataSection* next;

		CurlOutputDataSection(const char* tcSection, int iLength) : _iLength(iLength), next(NULL) {
			_tcSection = new char[_iLength];
			memcpy(_tcSection, tcSection, _iLength);
		}
		~CurlOutputDataSection() {
			delete [] _tcSection;
			delete next;
		}
	};

	struct CurlOutputData {
		bool bText;
		CurlOutputDataSection* first;
		CurlOutputDataSection* last;
		std::map<std::string, std::list<std::string> > headers;

		CurlOutputData(const std::string& sURL) : bText(false), first(NULL), last(NULL) {
			std::string::size_type iIndex = sURL.find_last_of('.');
			if (iIndex != std::string::npos) {
				std::string sExtension = sURL.substr(iIndex);
				bText = (sExtension == ".html" || sExtension == ".htm" || sExtension == ".txt");
			}
		}
		~CurlOutputData() { delete first; }

		int length() const {
			int iLength = 0;
			CurlOutputDataSection* pCurrent = first;
			while (pCurrent != NULL) {
				iLength += pCurrent->_iLength;
				pCurrent = pCurrent->next;
			}
			return iLength;
		}

		char* array() const {
			if (first == NULL) return NULL;
			char* tcArray = new char[length()];
			CurlOutputDataSection* pCurrent = first;
			int iLength = 0;
			while (pCurrent != NULL) {
				memcpy(tcArray + iLength, pCurrent->_tcSection, pCurrent->_iLength);
				iLength += pCurrent->_iLength;
				pCurrent = pCurrent->next;
			}
			return tcArray;
		}

		bool isBinary() const {
			CurlOutputDataSection* pCurrent = first;
			while (pCurrent != NULL) {
				char* tcSection = pCurrent->_tcSection;
				for (int i = 0; i < pCurrent->_iLength; ++i) {
					unsigned char a = (unsigned char) *tcSection;
					if ((a == 255) || (a <= 1)) {
						if ((a == 0) && bText) {
							// something stupid: it arrives that a '\0' character
							// is put into a ".html" file: the browser ignores it,
							// so, we'll do the same
							int iSize = pCurrent->_iLength - i - 1;
							if (iSize > 0) {
								memmove(pCurrent->_tcSection + i, pCurrent->_tcSection + i + 1, iSize);
							}
							pCurrent->_iLength--;
							--i;
							continue;
						} else {
							return true;
						}
					}
					tcSection++;
				}
				pCurrent = pCurrent->next;
			}
			return false;
		}
	};

	size_t writeHeaderLineFunction(void* ptr, size_t iSize, size_t iNmemb, void* pCurlOutput) {
		unsigned int iLength = iSize*iNmemb;
		CurlOutputData* pData = (CurlOutputData*) pCurlOutput;
		const char* tcHeader = (const char*) ptr;
		unsigned int iIndex = 0;
		while ((iIndex < iLength) && (tcHeader[iIndex] != ':')) iIndex++;
		std::string sHeader = std::string(tcHeader, iIndex);
		if (iIndex < iLength) {
			do {
				iIndex++;
			} while ((iIndex < iLength) && (tcHeader[iIndex] == ' '));
		}
		if (iIndex < iLength) {
			std::string sValue = std::string(tcHeader + iIndex, iLength - iIndex);
			std::string::size_type iEnd = sValue.find_first_of("\r\n");
			if (iEnd != std::string::npos) sValue = sValue.substr(0, iEnd);
			pData->headers[sHeader].push_back(sValue);
		} else {
			std::string::size_type iEnd = sHeader.find_first_of("\r\n");
			if (iEnd != std::string::npos) sHeader = sHeader.substr(0, iEnd);
			if (!sHeader.empty()) {
				pData->headers[sHeader].push_back("");
			}
		}
		return iLength;
	}

	size_t writeCurlFunction(void* ptr, size_t iSize, size_t iNmemb, void* pCurlOutput) {
		int iLength = iSize*iNmemb;
		CurlOutputData* pData = (CurlOutputData*) pCurlOutput;
		CurlOutputDataSection* pSection = new CurlOutputDataSection((const char*) ptr, iLength);
		if (pData->last == NULL) pData->first = pSection;
		else pData->last->next = pSection;
		pData->last = pSection;
		return iLength;
	}

	std::string getCurlErrorCode(CURLcode aCode) {
		switch(aCode) {
			case CURLE_OK: return "";
			case CURLE_UNSUPPORTED_PROTOCOL: return "Unsupported protocol";
			case CURLE_FAILED_INIT: return "Failed init";
			case CURLE_URL_MALFORMAT: return "URL malformated";
			case CURLE_URL_MALFORMAT_USER: return "URL malformated - user";
			case CURLE_COULDNT_RESOLVE_PROXY: return "Couldn't resolve proxy";
			case CURLE_COULDNT_RESOLVE_HOST: return "Couldn't resolve host";
			case CURLE_COULDNT_CONNECT: return "Couldn't connect";
			case CURLE_FTP_WEIRD_SERVER_REPLY: return "FTP weird server reply";
			case CURLE_FTP_ACCESS_DENIED: return "FTP access denied";
			case CURLE_FTP_USER_PASSWORD_INCORRECT: return "FTP password incorrect";
			case CURLE_FTP_WEIRD_PASS_REPLY: return "FTP weird password reply";
			case CURLE_FTP_WEIRD_USER_REPLY: return "FTP weird user reply";
			case CURLE_FTP_WEIRD_PASV_REPLY: return "FTP weird PASV reply";
			case CURLE_FTP_WEIRD_227_FORMAT: return "FTP weird 227 format";
			case CURLE_FTP_CANT_GET_HOST: return "FTP can't get host";
			case CURLE_FTP_CANT_RECONNECT: return "FTP can't reconnect";
			case CURLE_FTP_COULDNT_SET_BINARY: return "FTP can't set binary";
			case CURLE_PARTIAL_FILE: return "FTP partial file";
			case CURLE_FTP_COULDNT_RETR_FILE: return "FTP couldn't retrieve file";
			case CURLE_FTP_WRITE_ERROR: return "FTP write error";
			case CURLE_FTP_QUOTE_ERROR: return "FTP quote error";
			case CURLE_HTTP_RETURNED_ERROR: return "FTP returned error";
			case CURLE_WRITE_ERROR: return "FTP write error";
			case CURLE_MALFORMAT_USER: return "User name is illegally specified";
			case CURLE_FTP_COULDNT_STOR_FILE: return "Failed FTP upload";
			case CURLE_READ_ERROR: return "Couldn't open/read from file";
			case CURLE_OUT_OF_MEMORY: return "Out of memory";
			case CURLE_OPERATION_TIMEOUTED: return "The timeout time was reached";
			case CURLE_FTP_COULDNT_SET_ASCII: return "FTP couldn't set ASCII";
			case CURLE_FTP_PORT_FAILED: return "FTP PORT operation failed";
			case CURLE_FTP_COULDNT_USE_REST: return "FTP couldn't use REST";
			case CURLE_FTP_COULDNT_GET_SIZE: return "FTP couldn't get size";
			case CURLE_HTTP_RANGE_ERROR: return "HTTP RANGE 'command' didn't work";
			case CURLE_HTTP_POST_ERROR: return "HTTP POST error";
			case CURLE_SSL_CONNECT_ERROR: return "HTTP SSL connection error";
			case CURLE_BAD_DOWNLOAD_RESUME: return "Bad download resume";
			case CURLE_FILE_COULDNT_READ_FILE: return "Couldn't read file";
			case CURLE_LDAP_CANNOT_BIND: return "LDAP cannot bind";
			case CURLE_LDAP_SEARCH_FAILED: return "LDAP search failed";
			case CURLE_LIBRARY_NOT_FOUND: return "Library not found";
			case CURLE_FUNCTION_NOT_FOUND: return "Function not found";
			case CURLE_ABORTED_BY_CALLBACK: return "Aborted by callback";
			case CURLE_BAD_FUNCTION_ARGUMENT: return "Bad function argument";
			case CURLE_BAD_CALLING_ORDER: return "Bad calling order";
			case CURLE_HTTP_PORT_FAILED: return "HTTP Interface operation failed";
			case CURLE_BAD_PASSWORD_ENTERED: return "Bad password entered";
			case CURLE_TOO_MANY_REDIRECTS: return "Catch endless re-direct loops";
			case CURLE_UNKNOWN_TELNET_OPTION: return "Unknown TELNET option";
			case CURLE_TELNET_OPTION_SYNTAX: return "Malformed TELNET option";
			case CURLE_OBSOLETE: return "Obsolete cURL error (50), removed after version 7.7.3";
			case CURLE_SSL_PEER_CERTIFICATE: return "SSL peer's certificate wasn't ok";
			case CURLE_GOT_NOTHING: return "Got nothing";
			case CURLE_SSL_ENGINE_NOTFOUND: return "SSL crypto engine not found";
			case CURLE_SSL_ENGINE_SETFAILED: return "Cannot set SSL crypto engine as default";
			case CURLE_SEND_ERROR: return "Failed sending network data";
			case CURLE_RECV_ERROR: return "Failure in receiving network data";
			case CURLE_SHARE_IN_USE: return "Share is in use";
			case CURLE_SSL_CERTPROBLEM: return "Problem with the local certificate";
			case CURLE_SSL_CIPHER: return "Couldn't use specified cipher";
			case CURLE_SSL_CACERT: return "Problem with the CA cert (path?)";
			case CURLE_BAD_CONTENT_ENCODING: return "Unrecognized transfer encoding";
		}
		char tcMessage[80];
		sprintf(tcMessage, "internal error: unrecognized 'libcURL' error code (%d)", (int) aCode);
		throw UtlException(tcMessage);
	}

	CURL* fillCurlSession(DtaScriptVariable* pSession, CurlOutputData& theCurlOutput) {
		CURL* pCURL = curl_easy_init();
		if (pCURL != NULL) {
			DtaScriptVariable* pAgent = (pSession != NULL) ? pSession->getNode("agent") : NULL;
			const char* tcAgent = ((pAgent != NULL) ? pAgent->getValue() : NULL);
			curl_easy_setopt(pCURL, CURLOPT_USERAGENT, ((tcAgent != NULL) ? tcAgent : "CodeWorker"));
			DtaScriptVariable* pReferer = (pSession != NULL) ? pSession->getNode("referer") : NULL;
			if (pReferer != NULL) {
				const char* tcReferer = pReferer->getValue();
				curl_easy_setopt(pCURL, CURLOPT_REFERER, ((tcReferer == NULL) ? "" : tcReferer));
			}
			DtaScriptVariable* pProxy = (pSession != NULL) ? pSession->getNode("proxy") : NULL;
			if (pProxy != NULL) {
				DtaScriptVariable* pHost = pProxy->getNode("host");
				DtaScriptVariable* pPort = pProxy->getNode("port");
				if ((pHost == NULL) || (pPort == NULL)) {
					throw UtlException("'proxy.host' and 'proxy.port' expected into the HTTP session");
				}
				const char* tcHost = pHost->getValue();
				if (tcHost == NULL) {
					throw UtlException("non empty 'proxy.host' expected into the HTTP session");
				}
				curl_easy_setopt(pCURL, CURLOPT_PROXY, tcHost);
				curl_easy_setopt(pCURL, CURLOPT_PROXYPORT, pPort->getIntValue());
				DtaScriptVariable* pUser = pProxy->getNode("userpwd");
				if (pUser != NULL) {
					const char* tcUserPwd = pUser->getValue();
					if (strchr(tcUserPwd, ':') == NULL) {
						throw UtlException("'proxy.userpwd' must write the user and the password like 'user:pwd'");
					}
					curl_easy_setopt(pCURL, CURLOPT_PROXYUSERPWD, tcUserPwd);
				}
			}
			DtaScriptVariable* pCookies = (pSession != NULL) ? pSession->getNode("cookies") : NULL;
			if (pCookies != NULL) {
				std::string sCookies;
				const std::list<DtaScriptVariable*>* cookies = pCookies->getArray();
				if (cookies != NULL) {
					for (std::list<DtaScriptVariable*>::const_iterator i = cookies->begin(); i != cookies->end(); i++) {
						DtaScriptVariable* pName = (*i)->getNode("name");
						if (pName == NULL) throw UtlException("attribute 'name' of a cookie expected into the HTTP session");
						const char* tcName = pName->getValue();
						if (tcName == NULL) throw UtlException("attribute 'name' of a cookie should be populated into the HTTP session");
						std::string sValue;
						DtaScriptVariable* pValue = (*i)->getNode("value");
						if (pValue != NULL) {
							const char* tcValue = pValue->getValue();
							if (tcValue != NULL) sValue = tcValue;
						}
						if (!sCookies.empty()) sCookies += "; ";
						sCookies += std::string(tcName) + "=" + sValue;
					}
				}
				if (!sCookies.empty()) {
					curl_easy_setopt(pCURL, CURLOPT_COOKIE, sCookies.c_str());
				}
			}
			curl_easy_setopt(pCURL, CURLOPT_FILE, &theCurlOutput);
			curl_easy_setopt(pCURL, CURLOPT_WRITEHEADER, &theCurlOutput);
			curl_easy_setopt(pCURL, CURLOPT_HEADERFUNCTION, writeHeaderLineFunction);
			curl_easy_setopt(pCURL, CURLOPT_WRITEFUNCTION, writeCurlFunction);
		}
		return pCURL;
	}

	void populateCookie(DtaScriptVariable* pSession, const std::string& sCookie) {
		std::string::size_type iIndex = sCookie.find_first_of("=;");
		if (iIndex == std::string::npos) return;
		std::string sName = sCookie.substr(0, iIndex);
		std::string sValue;
		std::string::size_type iNext;
		if (sCookie[iIndex++] == '=') {
			iNext  = sCookie.find(';', iIndex);
			if (iNext == std::string::npos) {
				sValue = sCookie.substr(iIndex);
				iIndex = std::string::npos;
			} else {
				sValue = sCookie.substr(iIndex, iNext - iIndex);
				iIndex = iNext + 1;
			}
		}
		DtaScriptVariable* pCookie = pSession->insertNode("cookies")->getArrayElement(sName);
		if (pCookie == NULL) {
			pCookie = pSession->insertNode("cookies")->addElement(sName);
		}
		pCookie->insertNode("name")->setValue(sName.c_str());
		pCookie->insertNode("value")->setValue(sValue.c_str());
		while (iIndex != std::string::npos) {
			iIndex = sCookie.find_first_not_of(' ', iIndex);
			if (iIndex == std::string::npos) return;
			iNext = sCookie.find_first_of("=;", iIndex);
			if (iNext == std::string::npos) {
				sName = sCookie.substr(iIndex);
				sValue = "";
				iIndex = std::string::npos;
			} else {
				sName = sCookie.substr(iIndex, iNext - iIndex);
				iIndex = iNext + 1;
				iNext = sCookie.find(';', iIndex);
				if (iNext == std::string::npos) {
					sValue = sCookie.substr(iIndex);
					iIndex = std::string::npos;
				} else {
					sValue = sCookie.substr(iIndex, iNext - iIndex);
					iIndex = iNext + 1;
				}
			}
			if (!sName.empty()) {
				pCookie->insertNode(sName.c_str())->setValue(sValue.c_str());
			}
		}
	}

	std::string handleCurlOutput(DtaScriptVariable* pSession, CurlOutputData& theCurlOutput) {
		// do not initialize before the execution of 'isBinary()'!
		char* tcArray;
		int iLength;
		if (theCurlOutput.isBinary()) {
			tcArray = theCurlOutput.array();
			iLength = theCurlOutput.length();
			pSession->setValueAtVariable("binary_data", "true");
			char* tcResult = new char[iLength * 2];
			char* pResult = tcResult;
			for (int i = 0; i < iLength; i++) {
				unsigned char a = (unsigned char) tcArray[i];
				*pResult = CGRuntime::_tcHexa[a >> 4];
				pResult++;
				*pResult = CGRuntime::_tcHexa[a & '\x0F'];
				pResult++;
			}
			delete [] tcArray;
			tcArray = tcResult;
			iLength += iLength;
		} else {
			tcArray = theCurlOutput.array();
			iLength = theCurlOutput.length();
			pSession->remove("binary_data");
		}
		if (!theCurlOutput.headers.empty()) {
			DtaScriptVariable* pHeaders = pSession->insertNode("header_lines");
			pHeaders->clearArray();
			for (std::map<std::string, std::list<std::string> >::const_iterator i = theCurlOutput.headers.begin(); i != theCurlOutput.headers.end(); ++i) {
				DtaScriptVariable* pLine = pHeaders->addElement(i->first);
				bool bCookie = (i->first == "Set-Cookie");
				for (std::list<std::string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
					pLine->pushItem(*j);
					if (bCookie) populateCookie(pSession, *j);
				}
			}
		} else {
			pSession->remove("header_lines");
		}
		std::string sResult = std::string(tcArray, iLength);
		delete [] tcArray;
		return sResult;
	}

#endif

	HTTPRequest::HTTPRequest() {
#ifdef CODEWORKER_CURL_LIB
		if (curl_escape == NULL) {
#ifdef WIN32
			std::string sLibrary("libcurl.dll");
#else
			std::string sLibrary("libcurl.so");
#endif
			LIBRARY_HANDLE hHandle = DynPackage::loadLibrary(sLibrary);
			if (hHandle == NULL) {
				throw UtlException("libcURL error: unable to locate the dynamic library \"" + sLibrary + "\"");
			}
			curl_escape = (CURL_ESCAPE) DynPackage::findFunction(hHandle, "curl_escape");
			if (curl_escape == NULL) throw UtlException("libcURL error: unable to find function 'curl_escape' in dynamic library \"" + sLibrary + "\"");
			curl_free = (CURL_FREE) DynPackage::findFunction(hHandle, "curl_free");
			if (curl_free == NULL) throw UtlException("libcURL error: unable to find function 'curl_free' in dynamic library \"" + sLibrary + "\"");
			curl_easy_init = (CURL_EASY_INIT) DynPackage::findFunction(hHandle, "curl_easy_init");
			if (curl_easy_init == NULL) throw UtlException("libcURL error: unable to find function 'curl_easy_init' in dynamic library \"" + sLibrary + "\"");
			curl_easy_setopt = (CURL_EASY_SETOPT) DynPackage::findFunction(hHandle, "curl_easy_setopt");
			if (curl_easy_setopt == NULL) throw UtlException("libcURL error: unable to find function 'curl_easy_setopt' in dynamic library \"" + sLibrary + "\"");
			curl_easy_perform = (CURL_EASY_PERFORM) DynPackage::findFunction(hHandle, "curl_easy_perform");
			if (curl_easy_perform == NULL) throw UtlException("libcURL error: unable to find function 'curl_easy_perform' in dynamic library \"" + sLibrary + "\"");
			curl_easy_cleanup = (CURL_EASY_CLEANUP) DynPackage::findFunction(hHandle, "curl_easy_cleanup");
			if (curl_easy_cleanup == NULL) throw UtlException("libcURL error: unable to find function 'curl_easy_cleanup' in dynamic library \"" + sLibrary + "\"");
		}
#endif
	}

	std::string HTTPRequest::get(const std::string& sURL, DtaScriptVariable* pSession, DtaScriptVariable* pArguments) {
#ifdef CODEWORKER_CURL_LIB
		CurlOutputData theCurlOutput(sURL);
		CURL* pCURL = fillCurlSession(pSession, theCurlOutput);
		if (pCURL != NULL) {
			std::string sGetURL = sURL;
			if (pArguments != NULL) {
				const std::list<DtaScriptVariable*>* arguments = pArguments->getArray();
				if (arguments != NULL) {
					for (std::list<DtaScriptVariable*>::const_iterator i = arguments->begin(); i != arguments->end(); i++) {
						if (i == arguments->begin()) sGetURL += "?";
						else sGetURL += "&";
						char* tcName = curl_escape((*i)->getName(), ::strlen((*i)->getName()));
						sGetURL += tcName;
						sGetURL += "=";
						const char* tcValue = (*i)->getValue();
						if ((tcValue != NULL) && (*tcValue != '\0')) {
							char* tcGetValue = curl_escape(tcValue, (*i)->getValueLength());
							sGetURL += tcGetValue;
							curl_free(tcGetValue);
						}
						curl_free(tcName);
					}
				}
			}
			curl_easy_setopt(pCURL, CURLOPT_URL, sGetURL.c_str());
			CURLcode res = curl_easy_perform(pCURL);
			curl_easy_cleanup(pCURL);
			std::string sErrorCode = getCurlErrorCode(res);
			if (!sErrorCode.empty()) throw UtlException("libcURL error: " + sErrorCode);
		}
		return handleCurlOutput(pSession, theCurlOutput);
#else
		throw UtlException("HTTPRequest::get(" + sURL + ") cannot work, because 'CodeWorker' doesn't link libCURL!");
#endif
	}

	std::string HTTPRequest::post(const std::string& sURL, DtaScriptVariable* pSession, DtaScriptVariable* pArguments) {
#ifdef CODEWORKER_CURL_LIB
		CurlOutputData theCurlOutput(sURL);
		CURL* pCURL = fillCurlSession(pSession, theCurlOutput);
		if (pCURL != NULL) {
			std::string sPostURL;
			if (pArguments != NULL) {
				const std::list<DtaScriptVariable*>* arguments = pArguments->getArray();
				if (arguments != NULL) {
					for (std::list<DtaScriptVariable*>::const_iterator i = arguments->begin(); i != arguments->end(); i++) {
						if (i != arguments->begin()) sPostURL += "&";
						char* tcName = curl_escape((*i)->getName(), ::strlen((*i)->getName()));
						sPostURL += tcName;
						sPostURL += "=";
						const char* tcValue = (*i)->getValue();
						if ((tcValue != NULL) && (*tcValue != '\0')) {
							char* tcPostValue = curl_escape(tcValue, (*i)->getValueLength());
							sPostURL += tcPostValue;
							curl_free(tcPostValue);
						}
						curl_free(tcName);
					}
				}
			}
			curl_easy_setopt(pCURL, CURLOPT_POSTFIELDS, sPostURL.c_str());
			curl_easy_setopt(pCURL, CURLOPT_URL, sURL.c_str());
			CURLcode res = curl_easy_perform(pCURL);
			curl_easy_cleanup(pCURL);
			std::string sErrorCode = getCurlErrorCode(res);
			if (!sErrorCode.empty()) throw UtlException("libcURL error: " + sErrorCode);
		}
		return handleCurlOutput(pSession, theCurlOutput);
#else
		throw UtlException("HTTPRequest::post(" + sURL + ") cannot work, because 'CodeWorker' doesn't link libCURL!");
#endif
	}

	std::string HTTPRequest::send(const std::string& sURL, DtaScriptVariable* pSession) {
#ifdef CODEWORKER_CURL_LIB
		CurlOutputData theCurlOutput(sURL);
		CURL* pCURL = fillCurlSession(pSession, theCurlOutput);
		if (pCURL != NULL) {
			curl_easy_setopt(pCURL, CURLOPT_URL, sURL.c_str());
			CURLcode res = curl_easy_perform(pCURL);
			curl_easy_cleanup(pCURL);
			std::string sErrorCode = getCurlErrorCode(res);
			if (!sErrorCode.empty()) throw UtlException("libcURL error: " + sErrorCode);
		}
		return handleCurlOutput(pSession, theCurlOutput);
#else
		throw UtlException("HTTPRequest::send(" + sURL + ") cannot work, because 'CodeWorker' doesn't link libCURL!");
#endif
	}

	std::string HTTPRequest::UUID() {
#ifdef WIN32
		::UUID aUuid;

		RPC_STATUS aCreateStatus = ::UuidCreate(&aUuid);
		if (aCreateStatus != RPC_S_OK) throw UtlException("Cannot create a unique Windows UUID");
		unsigned char * aStringUuid;
		RPC_STATUS aConversionStatus = ::UuidToString(&aUuid, &aStringUuid);
		if (aConversionStatus != RPC_S_OK) throw UtlException("Cannot convert a unique Windows UUID to a string");
		std::string ssUuid((char *)aStringUuid); 
		RPC_STATUS aDeallocationStatus = ::RpcStringFree(&aStringUuid);
		if (aDeallocationStatus != RPC_S_OK) throw UtlException("Cannot deallocate UUID memory");
		return ssUuid;
#else
		throw UtlException("UUID() not implemented yet. Use 'uuid.h' and 'libuuid.so' under Linux");
#endif
	}
}
