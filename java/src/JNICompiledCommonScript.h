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

#ifndef _JNICompiledCommonScript_h_
#define _JNICompiledCommonScript_h_

#include <jni.h>

/**[description]
[section]org.codeworker.jni.CompiledCommonScript[/section]

This class represents a CodeWorker common script, so called because it doesn't process parse tasks and it doesn't
generate outputs, like a leader script.

It encapsulates a precompiled common script, which can be executed at any time, without requiring a new
compilation of the script.

Do not forget to build the precompiled script before executing it.

Example:
[code]
org.codeworker.jni.ParseTree theContext = new org.codeworker.jni.ParseTree();
// ... [skipping]
org.codeworker.jni.CompiledCommonScript script = new org.codeworker.jni.CompiledCommonScript();
// precompilation of the common script
script.buildFromFile("my_script.cws");
// execution of the script
script.execute(theContext);
[/code]

[subsection]public CompiledCommonScript()[/subsection]

The constructor of a common script precompilator.

[subsection]void buildFromFile(string filename)[/subsection]

It precompiles a common script coming from a file.

[subsection]void buildFromString(string text)[/subsection]

It precompiles a common script stored in the string argument "text".

Example: precompilation of a script that displays the content of the attribute 'message', belonging to the context.
[code]
org.codeworker.jni.CompiledCommonScript script = new org.codeworker.jni.CompiledCommonScript();
script.buildFromString("traceLine(this.message);");
[/code]

[subsection]void execute(ParseTree context)[/subsection]

It executes the precompiled common script, passing the argument "context" as 'this'.

Example:
[code]
org.codeworker.jni.ParseTree theContext = new org.codeworker.jni.ParseTree();
// ... skipping creation and precompilation of the script
script.execute(theContext);
[/code]

[/description]
**/
extern "C" {
	/*
	* Class:     org_codeworker_jni_CompiledCommonScript
	* Method:    buildFromString
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledCommonScript_buildFromString(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledCommonScript
	* Method:    buildFromFile
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledCommonScript_buildFromFile(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledCommonScript
	* Method:    execute
	* Signature: (Lorg/codeworker/ParseTree;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledCommonScript_execute(JNIEnv *, jobject, jobject);

	/*
	* Class:     org_codeworker_jni_CompiledCommonScript
	* Method:    init
	* Signature: ()[B
	*/
	JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_CompiledCommonScript_init(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_CompiledCommonScript
	* Method:    destroy
	* Signature: ()V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledCommonScript_destroy(JNIEnv *, jobject);
}

#endif
