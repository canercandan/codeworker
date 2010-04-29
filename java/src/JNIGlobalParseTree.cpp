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

#include "DtaProject.h"

#include "JNI-utils.h"
#include "JNIGlobalParseTree.h"


/*
* Class:     org_codeworker_jni_GlobalParseTree
* Method:    initializeNative
* Signature: (Ljava/lang/String;)[B
*/
JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_GlobalParseTree_initializeNative(JNIEnv *pEnv, jobject pObject, jstring jName) {
	GET_STRING(Name);
	std::string sName = tcName;
	CodeWorker::DtaScriptVariable* cppInstance = CodeWorker::DtaProject::getInstance().getGlobalVariable(sName);
	if (cppInstance == NULL) {
		cppInstance = CodeWorker::DtaProject::getInstance().setGlobalVariable(sName);
	}
	RELEASE_STRING(Name);
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::DtaScriptVariable*));
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(CodeWorker::DtaScriptVariable*), (jbyte*) &cppInstance);
	return pInstance;
}
