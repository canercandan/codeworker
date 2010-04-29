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

#ifndef _JNICompiledTemplateScript_h_
#define _JNICompiledTemplateScript_h_

/**[description]
[section]org.codeworker.jni.CompiledTemplateScript[/section]

This class represents a CodeWorker template script.

It encapsulates a template-based script, which can be executed at any time, without requiring a new
compilation of the script.

Do not forget to build the precompiled script before executing it.

Example:
[code]
org.codeworker.jni.ParseTree theContext = new org.codeworker.jni.ParseTree();
// ... [skipping]
org.codeworker.jni.CompiledTemplateScript script = new org.codeworker.jni.CompiledTemplateScript();
// precompilation of the common script
script.buildFromFile("my_script.cwt");
// execution of the script
script.generate(theContext);
[/code]

[subsection]public CompiledTemplateScript()[/subsection]

The constructor of a template-based script precompilator.

[subsection]void buildFromFile(string filename)[/subsection]

It precompiles a template-based script coming from a file.

[subsection]void buildFromString(string text)[/subsection]

It precompiles a template-based script stored in the string argument "text".

Example: precompilation of a script that writes all identifiers precedently pushed into the context.
[code]
org.codeworker.jni.CompiledTemplateScript script = new org.codeworker.jni.CompiledTemplateScript();
script.buildFromString("list of values:\n<%foreach i in this { %>\t- \"<%i%>\"\n<%}");
[/code]

[subsection]void generate(ParseTree context, string outputFile)[/subsection]

The precompiled template-based script generates a file, passing the argument "context" as 'this'.

[subsection]string generateString(ParseTree context, string text)[/subsection]

The precompiled template-based script generates an output, which is returned as a string. The string argument
"text" contains the precedent version of this output (preserved areas, for instance).

It passes the argument "context" as 'this'.

[subsection]void expand(ParseTree context, string outputFile)[/subsection]

The precompiled template-based script expands a file, passing the argument "context" as 'this'.

[/description]
**/

#include <jni.h>

extern "C" {
	/*
	* Class:     org_codeworker_jni_CompiledTemplateScript
	* Method:    buildFromString
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_buildFromString(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledTemplateScript
	* Method:    buildFromFile
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_buildFromFile(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledTemplateScript
	* Method:    generate
	* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_generate(JNIEnv *, jobject, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledTemplateScript
	* Method:    generateString
	* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)Ljava/lang/String;
	*/
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_CompiledTemplateScript_generateString(JNIEnv *, jobject, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledTemplateScript
	* Method:    expand
	* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_expand(JNIEnv *, jobject, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledTemplateScript
	* Method:    init
	* Signature: ()[B
	*/
	JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_CompiledTemplateScript_init(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_CompiledTemplateScript
	* Method:    destroy
	* Signature: ()V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTemplateScript_destroy(JNIEnv *, jobject);
}

#endif
