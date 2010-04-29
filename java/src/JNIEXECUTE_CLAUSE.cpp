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
#include "JNIEXECUTE_CLAUSE.h"

class JNIEXECUTE_CLAUSE : public CodeWorker::EXECUTE_CLAUSE {
	private:
		jobject _jClause;
		JNIEnv* _pEnv;

	public:
		inline JNIEXECUTE_CLAUSE(JNIEnv* pEnv, jobject jClause) : _pEnv(pEnv) {
			_jClause = pEnv->NewGlobalRef(jClause);
		}
		~JNIEXECUTE_CLAUSE() {}

		virtual bool run(CodeWorker::CGBNFRuntimeEnvironment& runtimeEnv) {
			jclass pClass = _pEnv->GetObjectClass(_jClause);
			jmethodID pMethod = _pEnv->GetMethodID(pClass, "run", "(Lorg/codeworker/IBNFRuntimeEnvironment;)Z");
			throw CodeWorker::UtlException("JNIEXECUTE_CLAUSE::run() not handled yet");
		}
};

/*
* Class:     org_codeworker_jni_EXECUTE_CLAUSE
* Method:    run
* Signature: (Lorg/codeworker/IBNFRuntimeEnvironment;)Z
*/
JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_EXECUTE_1CLAUSE_run(JNIEnv *pEnv, jobject jObject, jobject jRuntimeEnv) {
	jboolean bResult;
	GET_CWOBJECT_HANDLE(Object, EXECUTE_CLAUSE);
	GET_CWOBJECT_HANDLE(RuntimeEnv, CGBNFRuntimeEnvironment);
	std::string sError;
	try {
		bResult = (jboolean) pObjectInstance->run(*pRuntimeEnvInstance);
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
	return bResult;
}


/*
* Class:     org_codeworker_jni_EXECUTE_CLAUSE
* Method:    init
* Signature: ()[B
*/
JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_EXECUTE_1CLAUSE_init(JNIEnv *pEnv, jobject jObject) {
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::EXECUTE_CLAUSE*));
	JNIEXECUTE_CLAUSE* cppInstance = new JNIEXECUTE_CLAUSE(pEnv, jObject);
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(JNIEXECUTE_CLAUSE*), (jbyte*) &cppInstance);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_EXECUTE_CLAUSE
* Method:    destroy
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_EXECUTE_1CLAUSE_destroy(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, EXECUTE_CLAUSE);
	delete pObjectInstance;
}
