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
#include "JNIEXECUTE_FUNCTION.h"

class JNIEXECUTE_FUNCTION : public CodeWorker::EXECUTE_FUNCTION {
	private:
		jobject _jScript;
		JNIEnv* _pEnv;

	public:
		inline JNIEXECUTE_FUNCTION(JNIEnv* pEnv, jobject jScript) : _pEnv(pEnv) {
			_jScript = pEnv->NewGlobalRef(jScript);
		}
		~JNIEXECUTE_FUNCTION() {}

		virtual void run() {
			jclass pClass = _pEnv->GetObjectClass(_jScript);
			jmethodID pMethod = _pEnv->GetMethodID(pClass, "run", "()V");
			_pEnv->CallVoidMethod(_jScript, pMethod);
		}
};

/*
* Class:     org_codeworker_jni_EXECUTE_FUNCTION
* Method:    run
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_EXECUTE_1FUNCTION_run(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, EXECUTE_FUNCTION);
	std::string sError;
	try {
		pObjectInstance->run();
	} catch(std::exception& exception) {
		sError = exception.what();
	}
	if (!sError.empty()) pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), sError.c_str());
}


/*
* Class:     org_codeworker_jni_EXECUTE_FUNCTION
* Method:    init
* Signature: ()[B
*/
JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_EXECUTE_1FUNCTION_init(JNIEnv *pEnv, jobject jObject) {
	jbyteArray pInstance = pEnv->NewByteArray(sizeof(CodeWorker::EXECUTE_FUNCTION*));
	JNIEXECUTE_FUNCTION* cppInstance = new JNIEXECUTE_FUNCTION(pEnv, jObject);
	pEnv->SetByteArrayRegion(pInstance, 0, sizeof(JNIEXECUTE_FUNCTION*), (jbyte*) &cppInstance);
	return pInstance;
}

/*
* Class:     org_codeworker_jni_EXECUTE_FUNCTION
* Method:    destroy
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_codeworker_jni_EXECUTE_1FUNCTION_destroy(JNIEnv *pEnv, jobject jObject) {
	GET_CWOBJECT_HANDLE(Object, EXECUTE_FUNCTION);
	delete pObjectInstance;
}
