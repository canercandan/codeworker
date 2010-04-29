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
#pragma warning (disable : 4786)
#endif

#include "CGRuntime.h"
#include "ScpStream.h"
#include "DtaProject.h"
#include "JNIExternalHandling.h"

namespace CodeWorker {
	JNIExternalHandling::JNIExternalHandling(JNIEnv& theEnv, jobject pJNIObject) : _theEnv(theEnv), _pJNIObject(pJNIObject) {
#ifdef CODEWORKER_JNI
		_pClass = _theEnv.GetObjectClass(_pJNIObject);
		_pTraceLineMethod = _theEnv.GetMethodID(_pClass, "traceLine", "(Ljava/lang/String;)V");
		_pTraceTextMethod = _theEnv.GetMethodID(_pClass, "traceText", "(Ljava/lang/String;)V");
#endif
	}

	JNIExternalHandling::~JNIExternalHandling() {
	}

	jstring JNIExternalHandling::executeScript(jobjectArray listOfCommands) {
#ifdef CODEWORKER_JNI
		DtaProject::getInstance().reset();
		jsize iLength = _theEnv.GetArrayLength(listOfCommands);
		char** tsCommands = new char*[iLength + 1];
		tsCommands[0] = "CodeWorker library";
		int i;
		for (i = 0; i < iLength; i++) {
			jstring pText = (jstring) _theEnv.GetObjectArrayElement(listOfCommands, i);
			const char *tcText = _theEnv.GetStringUTFChars(pText, 0);
			tsCommands[i + 1] = (char*) tcText;
		}

		std::string sErrorMessage;
		try {
			if (!CGRuntime::executeScript(iLength + 1, tsCommands, NULL)) {
				sErrorMessage = "Bad command line passed to 'CodeWorker' interpreter: shouldn't occur, please call the authors";
			}
		} catch(UtlException& exception) {
			sErrorMessage = exception.getMessage();
		} catch(std::exception& exception) {
			sErrorMessage = exception.what();
		} catch(...) {
			sErrorMessage = "unexpected exception encountered while running CodeWorker interpreter";
		}

		for (i = 0; i < iLength; i++) {
			jstring pText = (jstring) _theEnv.GetObjectArrayElement(listOfCommands, i);
			_theEnv.ReleaseStringUTFChars(pText, tsCommands[i + 1]);
		}
		delete [] tsCommands;

		if (sErrorMessage.empty()) return NULL;
		return _theEnv.NewStringUTF(sErrorMessage.c_str());
#else
		return NULL; // Use -D CODEWORKER_JNI to take advantage of the Java bridge.
#endif
	}

	std::string JNIExternalHandling::inputLine(bool bEcho) {
#ifdef CODEWORKER_JNI
		std::string sLine;
		if (_pInputLineMethod != 0) {
			jclass pBooleanClass = _theEnv.FindClass("java/lang/Boolean");
			jmethodID pBooleanConstructor = _theEnv.GetMethodID(pBooleanClass, "<init>", "(B)V");
			jobject pEcho = _theEnv.NewObject(pBooleanClass, pBooleanConstructor, bEcho);
			jstring pLine = (jstring) _theEnv.CallObjectMethod(_pJNIObject, _pInputLineMethod, pEcho);
			const char *tcText = _theEnv.GetStringUTFChars(pLine, 0);
			sLine = tcText;
			_theEnv.ReleaseStringUTFChars(pLine, tcText);
		}
		return sLine;
#else
		return "Use -D CODEWORKER_JNI to take advantage of the Java bridge.";
#endif
	}

	std::string JNIExternalHandling::inputKey(bool bEcho) {
		return "JNIExternalHandling::inputKey() NOT IMPLEMENTED YET";
	}

	void JNIExternalHandling::traceLine(const std::string& sLine) {
#ifdef CODEWORKER_JNI
		if (_pTraceLineMethod == 0) return;
		_theEnv.CallVoidMethod(_pJNIObject, _pTraceLineMethod, _theEnv.NewStringUTF(sLine.c_str()));
#endif
	}

	void JNIExternalHandling::traceText(const std::string& sText) {
#ifdef CODEWORKER_JNI
		if (_pTraceTextMethod == 0) return;
		_theEnv.CallVoidMethod(_pJNIObject, _pTraceTextMethod, _theEnv.NewStringUTF(sText.c_str()));
#endif
	}
}
