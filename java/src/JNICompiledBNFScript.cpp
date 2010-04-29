/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2005 Cédric Lemaire

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

#include "DtaScriptVariable.h"
#include "CGCompiler.h"

#include "JNI-utils.h"
#include "JNICompiledBNFScript.h"


/*
* Class:     org_codeworker_jni_CompiledBNFScript
* Method:    buildFromString
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_buildFromString(JNIEnv *pEnv, jobject jObject, jstring jText) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledBNFScript);
	GET_STRING(Text);
	std::string sError;
	try {
		pObjectInstance->buildFromString(tcText);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	RELEASE_STRING(Text);
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
}

/*
* Class:     org_codeworker_jni_CompiledBNFScript
* Method:    buildFromFile
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_buildFromFile(JNIEnv *pEnv, jobject jObject, jstring jFile) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledBNFScript);
	GET_STRING(File);
	std::string sError;
	try {
		pObjectInstance->buildFromFile(tcFile);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	RELEASE_STRING(File);
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
}

/*
* Class:     org_codeworker_jni_CompiledBNFScript
* Method:    parse
* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_parse(JNIEnv *pEnv, jobject jObject, jobject jThis, jstring jFile) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledBNFScript);
	GET_STRING(File);
	GET_PARSETREE_HANDLE(This);
	std::string sError;
	try {
		pObjectInstance->parse(pThisInstance, tcFile);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	RELEASE_STRING(File);
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
}

/*
* Class:     org_codeworker_jni_CompiledBNFScript
* Method:    parseString
* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_parseString(JNIEnv *pEnv, jobject jObject, jobject jThis, jstring jText) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledBNFScript);
	GET_STRING(Text);
	GET_PARSETREE_HANDLE(This);
	std::string sError;
	try {
		pObjectInstance->parseString(pThisInstance, tcText);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	RELEASE_STRING(Text);
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
}

/*
* Class:     org_codeworker_jni_CompiledBNFScript
* Method:    init
* Signature: ()[B
*/
JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_CompiledBNFScript_init(JNIEnv *pEnv, jobject) {
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::CGCompiledBNFScript*));
	CodeWorker::CGCompiledBNFScript* cppInstance = new CodeWorker::CGCompiledBNFScript;
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(CodeWorker::CGCompiledBNFScript*), (jbyte*) &cppInstance);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_CompiledBNFScript
* Method:    destroy
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_destroy(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledBNFScript);
	delete pObjectInstance;
}
