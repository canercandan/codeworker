/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2005 C�dric Lemaire

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
#include "JNIBNFRuntimeTransformationMode.h"

/*
* Class:     org_codeworker_jni_BNFRuntimeTransformationMode
* Method:    init
* Signature: (Lorg/codeworker/IBNFRuntimeEnvironment;Z)[B
*/
jbyteArray JNICALL Java_org_codeworker_jni_BNFRuntimeTransformationMode_init(JNIEnv *pEnv, jobject, jobject jRuntimeEnv, jboolean bNewImplicitCopy) {
	GET_CWOBJECT_HANDLE(RuntimeEnv, CGBNFRuntimeEnvironment);
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::CGBNFRuntimeTransformationMode*));
	CodeWorker::CGBNFRuntimeTransformationMode* cppInstance = new CodeWorker::CGBNFRuntimeTransformationMode(*pRuntimeEnvInstance, (bNewImplicitCopy != 0));
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(CodeWorker::CGBNFRuntimeTransformationMode*), (jbyte*) &cppInstance);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_BNFRuntimeTransformationMode
* Method:    terminate
* Signature: ()V
*/
void JNICALL Java_org_codeworker_jni_BNFRuntimeTransformationMode_terminate(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, CGBNFRuntimeTransformationMode);
	delete pObjectInstance;
}
