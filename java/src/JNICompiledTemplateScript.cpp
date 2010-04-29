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
#include "JNICompiledTemplateScript.h"


/*
* Class:     org_codeworker_jni_CompiledTemplateScript
* Method:    buildFromString
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_buildFromString(JNIEnv *pEnv, jobject jObject, jstring jText) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTemplateScript);
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
* Class:     org_codeworker_jni_CompiledTemplateScript
* Method:    buildFromFile
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_buildFromFile(JNIEnv *pEnv, jobject jObject, jstring jFile) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTemplateScript);
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
* Class:     org_codeworker_jni_CompiledTemplateScript
* Method:    generate
* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_generate(JNIEnv *pEnv, jobject jObject, jobject jThis, jstring jFile) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTemplateScript);
	GET_STRING(File);
	GET_PARSETREE_HANDLE(This);
	std::string sError;
	try {
		pObjectInstance->generate(pThisInstance, tcFile);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	RELEASE_STRING(File);
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
}

/*
* Class:     org_codeworker_jni_CompiledTemplateScript
* Method:    generateString
* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_codeworker_jni_CompiledTemplateScript_generateString(JNIEnv *pEnv, jobject jObject, jobject jThis, jstring jText) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTemplateScript);
	GET_STRING(Text);
	GET_PARSETREE_HANDLE(This);
	std::string sError;
	std::string sText = tcText;
	try {
		pObjectInstance->generateString(pThisInstance, sText);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	RELEASE_STRING(Text);
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
	return pEnv->NewStringUTF(sText.c_str());
}

/*
* Class:     org_codeworker_jni_CompiledTemplateScript
* Method:    expand
* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_expand(JNIEnv *pEnv, jobject jObject, jobject jThis, jstring jFile) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTemplateScript);
	GET_STRING(File);
	GET_PARSETREE_HANDLE(This);
	std::string sError;
	try {
		pObjectInstance->expand(pThisInstance, tcFile);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	RELEASE_STRING(File);
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
}

/*
* Class:     org_codeworker_jni_CompiledTemplateScript
* Method:    init
* Signature: ()[B
*/
JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_CompiledTemplateScript_init(JNIEnv *pEnv, jobject) {
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::CGCompiledTemplateScript*));
	CodeWorker::CGCompiledTemplateScript* cppInstance = new CodeWorker::CGCompiledTemplateScript;
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(CodeWorker::CGCompiledTemplateScript*), (jbyte*) &cppInstance);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_CompiledTemplateScript
* Method:    destroy
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_destroy(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTemplateScript);
	delete pObjectInstance;
}
