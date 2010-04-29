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

#ifndef _JNIBNFRuntimeEnvironment_h_
#define _JNIBNFRuntimeEnvironment_h_

#include <jni.h>


extern "C" {
	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    implicitCopy
	* Signature: ()Z
	*/
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_implicitCopy(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    pushIgnoreMode
	* Signature: (Lorg/codeworker/BNFRuntimeIgnore;ILorg/codeworker/IEXECUTE_CLAUSE;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_pushIgnoreMode(JNIEnv *, jobject, jobject, jint, jobject);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    popIgnoreMode
	* Signature: (Lorg/codeworker/BNFRuntimeIgnore;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_popIgnoreMode(JNIEnv *, jobject, jobject);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    pushImplicitCopy
	* Signature: (Lorg/codeworker/BNFRuntimeTransformationMode;Z)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_pushImplicitCopy(JNIEnv *, jobject, jobject, jboolean);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    popImplicitCopy
	* Signature: (Lorg/codeworker/BNFRuntimeTransformationMode;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_popImplicitCopy(JNIEnv *, jobject, jobject);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    addClauseSignature
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_addClauseSignature(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    activateMatchingAreas
	* Signature: ()V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_activateMatchingAreas(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    storeMatchingAreas
	* Signature: (Lorg/codeworker/IParseTree;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_storeMatchingAreas(JNIEnv *, jobject, jobject);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    skipEmptyChars
	* Signature: ()I
	*/
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_skipEmptyChars(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    writeBinaryData
	* Signature: ([B)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_writeBinaryData___3B(JNIEnv *, jobject, jbyteArray);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    writeBinaryData
	* Signature: (B)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_writeBinaryData__B(JNIEnv *, jobject, jbyte);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    init
	* Signature: (Lorg/codeworker/IEXECUTE_CLAUSE;IZ)[B
	*/
	JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_init(JNIEnv *, jobject, jobject, jint, jboolean);

	/*
	* Class:     org_codeworker_jni_BNFRuntimeEnvironment
	* Method:    destroy
	* Signature: ()V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_BNFRuntimeEnvironment_destroy(JNIEnv *, jobject);
}

#endif
