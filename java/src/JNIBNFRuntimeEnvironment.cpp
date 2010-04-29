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

#include "CGRuntime.h"

#include "JNI-utils.h"
#include "JNIBNFRuntimeEnvironment.h"


/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    implicitCopy
* Signature: ()Z
*/
JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_implicitCopy(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	return (jboolean) pObjectInstance->implicitCopy();
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    pushIgnoreMode
* Signature: (Lorg/codeworker/BNFRuntimeIgnore;ILorg/codeworker/IEXECUTE_CLAUSE;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_pushIgnoreMode(JNIEnv *pEnv, jobject jObject, jobject jIgnoreMode, jint iNewIgnoreMode, jobject jNewClause) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	GET_CWOBJECT_HANDLE(IgnoreMode, CGBNFRuntimeIgnore);
	GET_CWOBJECT_HANDLE(NewClause, EXECUTE_CLAUSE);
	pObjectInstance->pushIgnoreMode(*pIgnoreModeInstance, iNewIgnoreMode, pNewClauseInstance);
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    popIgnoreMode
* Signature: (Lorg/codeworker/BNFRuntimeIgnore;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_popIgnoreMode(JNIEnv *pEnv, jobject jObject, jobject jIgnoreMode) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	GET_CWOBJECT_HANDLE(IgnoreMode, CGBNFRuntimeIgnore);
	pObjectInstance->popIgnoreMode(*pIgnoreModeInstance);
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    pushImplicitCopy
* Signature: (Lorg/codeworker/BNFRuntimeTransformationMode;Z)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_pushImplicitCopy(JNIEnv *pEnv, jobject jObject, jobject jTransformationMode, jboolean bNewImplicitCopy) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	GET_CWOBJECT_HANDLE(TransformationMode, CGBNFRuntimeTransformationMode);
	pObjectInstance->pushImplicitCopy(*pTransformationModeInstance, (bNewImplicitCopy != 0));
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    popImplicitCopy
* Signature: (Lorg/codeworker/BNFRuntimeTransformationMode;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_popImplicitCopy(JNIEnv *pEnv, jobject jObject, jobject jTransformationMode) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	GET_CWOBJECT_HANDLE(TransformationMode, CGBNFRuntimeTransformationMode);
	pObjectInstance->popImplicitCopy(*pTransformationModeInstance);
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    addClauseSignature
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_addClauseSignature(JNIEnv *pEnv, jobject jObject, jstring jSignature) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	GET_STRING(Signature);
	pObjectInstance->addClauseSignature(tcSignature);
	RELEASE_STRING(Signature);
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    activateMatchingAreas
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_activateMatchingAreas(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	pObjectInstance->activateMatchingAreas();
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    storeMatchingAreas
* Signature: (Lorg/codeworker/IParseTree;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_storeMatchingAreas(JNIEnv *pEnv, jobject jObject, jobject jStorage) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	GET_PARSETREE_HANDLE(Storage);
	CodeWorker::CppParsingTree_var pStorage(pStorageInstance);
	pObjectInstance->storeMatchingAreas(pStorage);
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    skipEmptyChars
* Signature: ()I
*/
JNIEXPORT jint JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_skipEmptyChars(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	return pObjectInstance->skipEmptyChars();
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    writeBinaryData
* Signature: ([B)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_writeBinaryData___3B(JNIEnv *pEnv, jobject jObject, jbyteArray jText) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	jbyte* tcText = pEnv->GetByteArrayElements(jText, NULL);
	int iLength = pEnv->GetArrayLength(jText);
	pObjectInstance->writeBinaryData((const char*) tcText, iLength);
	pEnv->ReleaseByteArrayElements(jText, tcText, 0);
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    writeBinaryData
* Signature: (B)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_writeBinaryData__B(JNIEnv *pEnv, jobject jObject, jbyte cChar) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	pObjectInstance->writeBinaryData((unsigned char) cChar);
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    init
* Signature: (Lorg/codeworker/IEXECUTE_CLAUSE;IZ)[B
*/
JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_init(JNIEnv *pEnv, jobject, jobject jClause, jint iIgnoreMode, jboolean bImplicitCopy) {
	GET_CWOBJECT_HANDLE(Clause, EXECUTE_CLAUSE);
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::CGBNFRuntimeEnvironment*));
	CodeWorker::CGBNFRuntimeEnvironment* cppInstance = new CodeWorker::CGBNFRuntimeEnvironment(pClauseInstance, iIgnoreMode, (bImplicitCopy != 0));
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(CodeWorker::CGBNFRuntimeEnvironment*), (jbyte*) &cppInstance);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_BNFRuntimeEnvironment
* Method:    destroy
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_destroy(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeEnvironment);
	delete pObjectInstance;
}
