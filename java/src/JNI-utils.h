#ifndef _JNI_utils_h_
#define _JNI_utils_h_

#include <jni.h>

namespace CodeWorker {
	class DtaScriptVariable;
}

#define GET_PARSETREE_HANDLE(radical) \
	jclass p##radical##Class;\
	CodeWorker::DtaScriptVariable* p##radical##Instance;\
	if (j##radical == NULL) {\
		p##radical##Class = NULL;\
		p##radical##Instance = NULL;\
	} else {\
		p##radical##Class = pEnv->GetObjectClass(j##radical);\
		jfieldID p##radical##HandleID = pEnv->GetFieldID(p##radical##Class, "cppInstance_", "[B");\
		jbyteArray p##radical##Handle = (jbyteArray) pEnv->GetObjectField(j##radical, p##radical##HandleID);\
		jbyte** tc##radical##Handle = (jbyte**) pEnv->GetByteArrayElements(p##radical##Handle, NULL);\
		p##radical##Instance = (CodeWorker::DtaScriptVariable*) *tc##radical##Handle;\
		pEnv->ReleaseByteArrayElements(p##radical##Handle, (jbyte*) tc##radical##Handle, 0);\
	}

#define GET_CWOBJECT_HANDLE(radical, object_type) \
	jclass p##radical##Class;\
	CodeWorker::object_type* p##radical##Instance;\
	if (j##radical == NULL) {\
		p##radical##Class = NULL;\
		p##radical##Instance = NULL;\
	} else {\
		p##radical##Class = pEnv->GetObjectClass(j##radical);\
		jfieldID p##radical##HandleID = pEnv->GetFieldID(p##radical##Class, "cppInstance_", "[B");\
		jbyteArray p##radical##Handle = (jbyteArray) pEnv->GetObjectField(j##radical, p##radical##HandleID);\
		jbyte** tc##radical##Handle = (jbyte**) pEnv->GetByteArrayElements(p##radical##Handle, NULL);\
		p##radical##Instance = (CodeWorker::object_type*) *tc##radical##Handle;\
		pEnv->ReleaseByteArrayElements(p##radical##Handle, (jbyte*) tc##radical##Handle, 0);\
	}


#define GET_STRING(radical) const char* tc##radical = pEnv->GetStringUTFChars(j##radical, NULL)
#define RELEASE_STRING(radical) pEnv->ReleaseStringUTFChars(j##radical, tc##radical)


jobject createWrapper(JNIEnv *pEnv, jclass pClass, CodeWorker::DtaScriptVariable* pNextNode);

#endif
