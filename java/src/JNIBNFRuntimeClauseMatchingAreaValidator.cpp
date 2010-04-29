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

#include <set>

#include "CGRuntime.h"

#include "JNI-utils.h"
#include "JNIBNFRuntimeClauseMatchingAreaValidator.h"


/*
* Class:     org_codeworker_jni_BNFRuntimeClauseMatchingAreaValidator
* Method:    init
* Signature: (Ljava/lang/String;Lorg/codeworker/IBNFRuntimeEnvironment;)[B
*/
JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_BNFRuntimeClauseMatchingAreaValidator_init(JNIEnv *pEnv, jobject, jstring jClause, jobject jRuntimeEnv) {
	static std::set<std::string> clauseNames;
	GET_STRING(Clause);
	GET_CWOBJECT_HANDLE(RuntimeEnv, CGBNFRuntimeEnvironment);
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::CGBNFRuntimeClauseMatchingAreaValidator*));
	const char* ptcClause;
	std::set<std::string>::const_iterator cursor = clauseNames.find(tcClause);
	if (cursor == clauseNames.end()) {
		clauseNames.insert(tcClause);
		cursor = clauseNames.find(tcClause);
	}
	ptcClause = cursor->c_str();
	CodeWorker::CGBNFRuntimeClauseMatchingAreaValidator* cppInstance = new CodeWorker::CGBNFRuntimeClauseMatchingAreaValidator(ptcClause, pRuntimeEnvInstance);
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(CodeWorker::CGBNFRuntimeClauseMatchingAreaValidator*), (jbyte*) &cppInstance);
	RELEASE_STRING(Clause);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_BNFRuntimeClauseMatchingAreaValidator
* Method:    purgeChildsAfterPosition
* Signature: (I)V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeClauseMatchingAreaValidator_purgeChildsAfterPosition(JNIEnv *pEnv, jobject jObject, jint iPosition) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeClauseMatchingAreaValidator);
	pObjectInstance->purgeChildsAfterPosition(iPosition);
}

/*
* Class:     org_codeworker_jni_BNFRuntimeClauseMatchingAreaValidator
* Method:    validate
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeClauseMatchingAreaValidator_validate(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeClauseMatchingAreaValidator);
	pObjectInstance->validate();
}

/*
* Class:     org_codeworker_jni_BNFRuntimeClauseMatchingAreaValidator
* Method:    terminate
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeClauseMatchingAreaValidator_terminate(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeClauseMatchingAreaValidator);
	delete pObjectInstance;
}
