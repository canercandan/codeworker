#include "JNI-utils.h"

jobject createWrapper(JNIEnv *pEnv, jclass pClass, CodeWorker::DtaScriptVariable* pNextNode) {
	if (pNextNode == NULL) return NULL;
	jmethodID pMethod = pEnv->GetMethodID(pClass, "<init>", "([B)V");
	jbyteArray pArray = pEnv->NewByteArray(sizeof(CodeWorker::DtaScriptVariable*));
	pEnv->SetByteArrayRegion(pArray, 0, sizeof(CodeWorker::DtaScriptVariable*), (jbyte*) &pNextNode);
	return pEnv->NewObject(pClass, pMethod, pArray);
}
