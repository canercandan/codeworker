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

#include "JNI-utils.h"
#include "JNIParseTree.h"


/*
* Class:     org_codeworker_jni_ParseTree
* Method:    getName
* Signature: ()Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_codeworker_jni_ParseTree_getName(JNIEnv *pEnv, jobject jObject) {
	jstring pResult;
	GET_PARSETREE_HANDLE(Object);
	const char* tcValue = pObjectInstance->getName();
	if (tcValue == NULL) pResult = NULL;
	else pResult = pEnv->NewStringUTF(tcValue);
	return pResult;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    getValue
* Signature: ()Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_codeworker_jni_ParseTree_getValue(JNIEnv *pEnv, jobject jObject) {
	jstring pResult;
	GET_PARSETREE_HANDLE(Object);
	const char* tcValue = pObjectInstance->getValue();
	if (tcValue == NULL) pResult = NULL;
	else pResult = pEnv->NewStringUTF(tcValue);
	return pResult;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    getStringValue
* Signature: ()Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_codeworker_jni_ParseTree_getStringValue(JNIEnv *pEnv, jobject jObject) {
	jstring pResult;
	GET_PARSETREE_HANDLE(Object);
	const char* tcValue = pObjectInstance->getValue();
	if (tcValue == NULL) pResult = pEnv->NewStringUTF("");
	else pResult = pEnv->NewStringUTF(tcValue);
	return pResult;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    setValue
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_ParseTree_setValue(JNIEnv *pEnv, jobject jObject, jstring jValue) {
	GET_PARSETREE_HANDLE(Object);
	GET_STRING(Value);
	pObjectInstance->setValue(tcValue);
	RELEASE_STRING(Value);
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    getReference
* Signature: ()Lorg/codeworker/ParseTree;
*/
JNIEXPORT jobject JNICALL Java_org_codeworker_jni_ParseTree_getReference(JNIEnv *pEnv, jobject jObject) {
	GET_PARSETREE_HANDLE(Object);
	jobject result = createWrapper(pEnv, pObjectClass, pObjectInstance->getReferencedVariable());
	return result;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    getArray
* Signature: ()[Lorg/codeworker/ParseTree;
*/
JNIEXPORT jobjectArray JNICALL Java_org_codeworker_jni_ParseTree_getArray(JNIEnv *pEnv, jobject jObject) {
	jobjectArray pResult;
	GET_PARSETREE_HANDLE(Object);
	const std::list<CodeWorker::DtaScriptVariable*>* pArray = pObjectInstance->getArray();
	if (pArray == NULL) {
		pResult = NULL;
	} else {
		pResult = pEnv->NewObjectArray(pObjectInstance->getArraySize(), pObjectClass, NULL);
		int i = 0;
		for (std::list<CodeWorker::DtaScriptVariable*>::const_iterator it = pArray->begin(); it != pArray->end(); ++it) {
			pEnv->SetObjectArrayElement(pResult, i++, createWrapper(pEnv, pObjectClass, *it));
		}
	}
	return pResult;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    getAttributeNames
* Signature: ()[Ljava/lang/String;
*/
JNIEXPORT jobjectArray JNICALL Java_org_codeworker_jni_ParseTree_getAttributeNames(JNIEnv *pEnv, jobject jObject) {
	GET_PARSETREE_HANDLE(Object);
	CodeWorker::DtaScriptVariableList* pAttributes = pObjectInstance->getAttributes();
	if (pAttributes == NULL) return NULL;
	std::vector<const char*> tsAttributes;
	while (pAttributes != NULL) {
		CodeWorker::DtaScriptVariable* pNode = pAttributes->getNode();
		tsAttributes.push_back(pNode->getName());
		pAttributes = pAttributes->getNext();
	}
	jobjectArray pResult = pEnv->NewObjectArray((int) tsAttributes.size(), pEnv->FindClass("java/lang/String"), NULL);
	for (unsigned int i = 0; i < tsAttributes.size(); ++i) {
		jstring pEntry = pEnv->NewStringUTF(tsAttributes[i]);
		pEnv->SetObjectArrayElement(pResult, i, pEntry);
	}
	return pResult;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    getNode
* Signature: (Ljava/lang/String;)Lorg/codeworker/ParseTree;
*/
JNIEXPORT jobject JNICALL Java_org_codeworker_jni_ParseTree_getNode(JNIEnv *pEnv, jobject jObject, jstring jAttribute) {
	GET_PARSETREE_HANDLE(Object);
	GET_STRING(Attribute);
	jobject  result = createWrapper(pEnv, pObjectClass, pObjectInstance->getNode(tcAttribute));
	RELEASE_STRING(Attribute);
	return result;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    insertNode
* Signature: (Ljava/lang/String;)Lorg/codeworker/ParseTree;
*/
JNIEXPORT jobject JNICALL Java_org_codeworker_jni_ParseTree_insertNode(JNIEnv *pEnv, jobject jObject, jstring jAttribute) {
	GET_PARSETREE_HANDLE(Object);
	GET_STRING(Attribute);
	jobject  result = createWrapper(pEnv, pObjectClass, pObjectInstance->insertNode(tcAttribute));
	RELEASE_STRING(Attribute);
	return result;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    insertItem
* Signature: (Ljava/lang/String;Lorg/codeworker/ParseTree;)Lorg/codeworker/ParseTree;
*/
JNIEXPORT jobject JNICALL Java_org_codeworker_jni_ParseTree_addItem(JNIEnv *pEnv, jobject jObject, jstring jKey) {
	GET_PARSETREE_HANDLE(Object);
	GET_STRING(Key);
	jobject  result = createWrapper(pEnv, pObjectClass, pObjectInstance->addElement(tcKey));
	RELEASE_STRING(Key);
	return result;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    init
* Signature: ()[B
*/
JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_ParseTree_init(JNIEnv *pEnv, jobject) {
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::DtaScriptVariable*));
	CodeWorker::DtaScriptVariable* cppInstance = new CodeWorker::DtaScriptVariable;
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(CodeWorker::DtaScriptVariable*), (jbyte*) &cppInstance);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_ParseTree
* Method:    destroy
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_ParseTree_destroy(JNIEnv *pEnv, jobject jObject) {
	GET_PARSETREE_HANDLE(Object);
	delete pObjectInstance;
}
