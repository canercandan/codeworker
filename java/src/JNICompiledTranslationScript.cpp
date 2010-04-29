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
#include "JNICompiledTranslationScript.h"


/*
* Class:     org_codeworker_jni_CompiledTranslationScript
* Method:    buildFromString
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTranslationScript_buildFromString(JNIEnv *pEnv, jobject jObject, jstring jText) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTranslationScript);
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
* Class:     org_codeworker_jni_CompiledTranslationScript
* Method:    buildFromFile
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTranslationScript_buildFromFile(JNIEnv *pEnv, jobject jObject, jstring jFile) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTranslationScript);
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
* Class:     org_codeworker_jni_CompiledTranslationScript
* Method:    translate
* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTranslationScript_translate(JNIEnv *pEnv, jobject jObject, jobject jThis, jstring jInput, jstring jOutput) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTranslationScript);
	GET_STRING(Input);
	GET_STRING(Output);
	GET_PARSETREE_HANDLE(This);
	std::string sError;
	try {
		pObjectInstance->translate(pThisInstance, tcInput, tcOutput);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	RELEASE_STRING(Output);
	RELEASE_STRING(Input);
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
}

/*
* Class:     org_codeworker_jni_CompiledTranslationScript
* Method:    translateString
* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_codeworker_jni_CompiledTranslationScript_translateString(JNIEnv *pEnv, jobject jObject, jobject jThis, jstring jInputText) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTranslationScript);
	GET_STRING(InputText);
	GET_PARSETREE_HANDLE(This);
	std::string sError;
	std::string sOutputText;
	try {
		pObjectInstance->translateString(pThisInstance, tcInputText, sOutputText);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	RELEASE_STRING(InputText);
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
	return pEnv->NewStringUTF(sOutputText.c_str());
}

/*
* Class:     org_codeworker_jni_CompiledTranslationScript
* Method:    init
* Signature: ()[B
*/
JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_CompiledTranslationScript_init(JNIEnv *pEnv, jobject) {
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::CGCompiledTranslationScript*));
	CodeWorker::CGCompiledTranslationScript* cppInstance = new CodeWorker::CGCompiledTranslationScript;
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(CodeWorker::CGCompiledTranslationScript*), (jbyte*) &cppInstance);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_CompiledTranslationScript
* Method:    destroy
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTranslationScript_destroy(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGCompiledTranslationScript);
	delete pObjectInstance;
}
