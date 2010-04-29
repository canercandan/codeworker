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

#ifndef _JNIExternalHandling_h_
#define _JNIExternalHandling_h_

#include <string>

#ifdef CODEWORKER_JNI
#	include "jni.h"
#else
	typedef void* JNIEnv;
	typedef void* jobject;
	typedef void* jclass;
	typedef void* jmethodID;
	typedef void* jobjectArray;
	typedef void* jstring;
#endif

#include "CGExternalHandling.h"


namespace CodeWorker {
	class JNIExternalHandling : public CGExternalHandling {
	private:
		JNIEnv& _theEnv;
		jobject _pJNIObject;

		jclass _pClass;
		jmethodID _pInputLineMethod;
		jmethodID _pTraceLineMethod;
		jmethodID _pTraceTextMethod;

	public:
//note: the constructor of the C++ class \samp{JNIExternalHandling} that diverts the messages
//note: sent to the console by \CodeWorker\ . It must be called into the function of a C++
//note: JNI module. The two parameters are always passed by JAVA when calling a native
//note: interface.
//note: \begin{tableiii}{l|l|l}{0.6}{Parameter}{Type}{Description}
//note:		\lineiii{theEnv}{JNIEnv\&}{the environment variable for JNI}
//note:		\lineiii{pJNIObject}{jobject}{a reference to the JAVA instance whose class name
//note: 		is chosen by the developer freely, and called \textit{mypackage\_MyClass} below}
//note: \end{tableiii}
//note: The constructor only serves to launch properly \samp{executeScript()} on the instance,
//note: and to call functions that catch messages intended to the display.
		JNIExternalHandling(JNIEnv& theEnv, jobject pJNIObject);
		virtual ~JNIExternalHandling();

//note: the method \samp{executeScript()} expects a command line to execute by \CodeWorker\ 
//note: and it returns an error message if something failed, or and empty string if success.
//note: The method must be called into a C++ JNI module that first calls the constructor seen
//note: before:\\ 
//note: \texttt{/*\\ 
//note: * Class:     \textit{mypackage\_MyClass}\\ 
//note: * Method:    \textit{myMethod}\\ 
//note: * Signature: ([Ljava/lang/String;)Ljava/lang/String;\\ 
//note: * \begin{tableiii}{l|l|l}{0.6}{Parameter}{Type}{Description} \lineiii{theEnv}{JNIEnv\&}{the environment variable for JNI} \lineiii{pJNIObject}{jobject}{a reference to the JAVA instance} \lineiii{listOfCommands}{String[]}{the command line that \CodeWorker\ must execute} \end{tableiii}
//note: */\\ 
//note: JNIEXPORT jstring JNICALL Java\_\textit{mypackage\_MyClass\_myMethod}\\ 
//note: (JNIEnv *pEnv, jobject pJNIObject, jobjectArray listOfCommands)\\ 
//note: \{\\ 
//note: 	JNIExternalHandling theExecution(*pEnv, pJNIObject);\\ 
//note: 	return theExecution.executeScript(listOfCommands);\\ 
//note: \}}
		jstring executeScript(jobjectArray listOfCommands);

		virtual std::string inputKey(bool bEcho);
//note: the callback method \samp{inputLine()} is waiting for a line that the user will put
//note: into the standard input, the keyboard generally.
//note: The method must be implemented into the JAVA class \textit{mypackage\_MyClass}
//note: under the prototype: \samp{public String inputLine(boolean bEcho)}, as seen further.
		virtual std::string inputLine(bool bEcho);
//note: the callback method \samp{traceLine()} passes the message intended to the console to the argument
//note: \samp{sLine}. The method must be implemented into the JAVA class \textit{mypackage\_MyClass}
//note: under the prototype: \samp{public void traceLine(String sLine)}, as seen further.
		virtual void traceLine(const std::string& sLine);
//note: the callback method \samp{traceText()} passes the message intended to the console to the argument
//note: \samp{sText}. The method must be implemented into JAVA class \textit{mypackage\_MyClass}
//note: under the prototype: \samp{public void traceText(String sText)}. An example of source
//note: code that illustrates the JAVA part of the JNI may be:\\ 
//note: \texttt{package \textit{mypackage};\\ 
//note: ...\\ 
//note: public class \textit{MyClass} \{\\ 
//note: \makebox[0.8cm][r]{}...\\ 
//note: \makebox[0.8cm][r]{}public native String \textit{myMethod}(String[] listOfCommands);\\ 
//note: \\ 
//note: \makebox[0.8cm][r]{}public String inputLine(boolean bEcho) \{\\ 
//note: \makebox[1.6cm][r]{}byte[] tbLine = new byte[512];\\ 
//note: \makebox[1.6cm][r]{}System.in.read(tbLine);\\ 
//note: \makebox[1.6cm][r]{}return new String(tbLine);\\ 
//note: \makebox[0.8cm][r]{}\}\\ 
//note: \\ 
//note: \makebox[0.8cm][r]{}public void traceLine(String sLine) \{\\ 
//note: \makebox[1.6cm][r]{}System.out.println(sLine);\\ 
//note: \makebox[0.8cm][r]{}\}\\ 
//note: \\ 
//note: \makebox[0.8cm][r]{}public void traceText(String sText) \{\\ 
//note: \makebox[1.6cm][r]{}System.out.print(sText);\\ 
//note: \makebox[0.8cm][r]{}\}\\ 
//note: \}}
//note:
//note: Before calling the native method for the first time, don't forget to load your library:\\ 
//note: \texttt{try \{\\
//note: \makebox[0.8cm][r]{}System.loadLibrary("\textit{MyLibrary}");\\ 
//note: \} catch(Exception exception) \{\\ 
//note: \makebox[0.8cm][r]{}System.out.println("Unable to load the library: '" + exception.toString() + "'");\\ 
//note: \}}
//note:
//note: The following source code is an example of implementation for calling the native method,
//note: to put into a method of \textit{MyClass}:\\
//note: \texttt{String sErrorMessage = null;\\ 
//note: try \{\\ 
//note: \makebox[0.8cm][r]{}sErrorMessage = \textit{myMethod}(_tsCommands);\\ 
//note: \} catch(UnsatisfiedLinkError exception) \{\\ 
//note: \makebox[0.8cm][r]{}System.out.println("Unable to link to function '\textit{myMethod}(String[])' into the library: '" + exception.toString() + "'");\\ 
//note: \} catch(Exception exception) \{\\ 
//note: \makebox[0.8cm][r]{}System.out.println("Unexpected exception while calling function '\textit{myMethod}(String[])' into the library: '" + exception.toString() + "'");\\ 
//note: \}}
		virtual void traceText(const std::string& sText);
	};
}

#endif
