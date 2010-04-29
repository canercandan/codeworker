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
#include "JNIBNFRuntimeIgnore.h"

/*
* Class:     org_codeworker_jni_BNFRuntimeIgnore
* Method:    init
* Signature: (Lorg/codeworker/IBNFRuntimeEnvironment;ILorg/codeworker/IEXECUTE_CLAUSE;)[B
*/
jbyteArray JNICALL Java_org_codeworker_jni_BNFRuntimeIgnore_init(JNIEnv *pEnv, jobject, jobject jRuntimeEnv, jint iNewIgnoreMode, jobject jClause) {
	GET_CWOBJECT_HANDLE(RuntimeEnv, CGBNFRuntimeEnvironment);
	GET_CWOBJECT_HANDLE(Clause, EXECUTE_CLAUSE);
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::CGBNFRuntimeIgnore*));
	CodeWorker::CGBNFRuntimeIgnore* cppInstance = new CodeWorker::CGBNFRuntimeIgnore(*pRuntimeEnvInstance, iNewIgnoreMode, pClauseInstance);
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(CodeWorker::CGBNFRuntimeIgnore*), (jbyte*) &cppInstance);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_BNFRuntimeIgnore
* Method:    terminate
* Signature: ()V
*/
void JNICALL Java_org_codeworker_jni_BNFRuntimeIgnore_terminate(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeIgnore);
	delete pObjectInstance;
}
