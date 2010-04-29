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

#ifndef _JNICompiledTranslationScript_h_
#define _JNICompiledTranslationScript_h_

/**[description]
[section]org.codeworker.jni.CompiledTranslationScript[/section]

This class represents a CodeWorker translation script.

It encapsulates a translation script, which can be executed at any time, without requiring a new
compilation of the script.

Do not forget to build the precompiled script before executing it.

Example:
[code]
org.codeworker.jni.ParseTree theContext = new org.codeworker.jni.ParseTree();
// ... [skipping]
org.codeworker.jni.CompiledTranslationScript script = new org.codeworker.jni.CompiledTranslationScript();
// precompilation of the common script
script.buildFromFile("my_script.cwp");
// execution of the script
script.translate(theContext, "my_source.txt", "my_target.txt");
[/code]

[subsection]public CompiledTranslationScript()[/subsection]

The constructor of a translation script precompilator.

[subsection]void buildFromFile(string filename)[/subsection]

It precompiles a translation script coming from a file.

[subsection]void buildFromString(string text)[/subsection]

It precompiles a translation script stored in the string argument "text".

Example: precompilation of a script that extracts identifiers from an input and writes them on the flow to the
output. Following a straightforward process, It does something similar to the merge of examples given for the
BNF and template scripts above.
[code]
org.codeworker.jni.CompiledTranslationScript script = new org.codeworker.jni.CompiledTranslationScript();
script.buildFromString("translator ::= =>{%>list of values:\n<%}[->[#readIdentifier:id => {%>- \"<%id%>\"\n<%}]]*;");
[/code]

[subsection]void translate(ParseTree context, string parsedFile, string outputFile)[/subsection]

The precompiled translation script translates the file "parsedFile" to another file "outputFile", passing the
argument "context" as 'this'.

[subsection]string generateString(ParseTree context, stringBuilder text)[/subsection]

The precompiled template-based script translates the content of the string parameter "inputText" and returns
the result as a string.

It passes the argument "context" as 'this'.

[/description]
**/

#include <jni.h>

extern "C" {
	/*
	* Class:     org_codeworker_jni_CompiledTranslationScript
	* Method:    buildFromString
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTranslationScript_buildFromString(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledTranslationScript
	* Method:    buildFromFile
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTranslationScript_buildFromFile(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledTranslationScript
	* Method:    translate
	* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTranslationScript_translate(JNIEnv *, jobject, jobject, jstring, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledTranslationScript
	* Method:    translateString
	* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)Ljava/lang/String;
	*/
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_CompiledTranslationScript_translateString(JNIEnv *, jobject, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledTranslationScript
	* Method:    init
	* Signature: ()[B
	*/
	JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_CompiledTranslationScript_init(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_CompiledTranslationScript
	* Method:    destroy
	* Signature: ()V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledTranslationScript_destroy(JNIEnv *, jobject);
}

#endif
