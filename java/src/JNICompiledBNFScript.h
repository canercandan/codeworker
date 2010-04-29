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

#ifndef _JNICompiledBNFScript_h_
#define _JNICompiledBNFScript_h_

/**[description]
[section]org.codeworker.jni.CompiledBNFScript[/section]

This class represents a CodeWorker extended-BNF script.

It encapsulates a precompiled BNF script, which can be executed at any time, without requiring a new
compilation of the script.

Do not forget to build the precompiled script before executing it.

Example:
[code]
org.codeworker.jni.ParseTree theContext = new org.codeworker.jni.ParseTree();
// ... [skipping]
org.codeworker.jni.CompiledBNFScript script = new org.codeworker.jni.CompiledBNFScript();
// precompilation of the common script
script.buildFromFile("my_grammar.cwp");
// execution of the script
script.parse(theContext, "my_DSL.dsl");
[/code]

[subsection]public CompiledBNFScript()[/subsection]

The constructor of an extended-BNF script precompilator.

[subsection]void buildFromFile(string filename)[/subsection]

It precompiles an extended-BNF script coming from a file.

[subsection]void buildFromString(string text)[/subsection]

It precompiles an extended-BNF script stored in the string argument "text".

Example: precompilation of a script that extracts all identifiers of an input.
[code]
org.codeworker.jni.CompiledBNFScript script = new org.codeworker.jni.CompiledBNFScript();
script.buildFromString("grammar ::= [->[#readIdentifier:id => pushItem this = id;]]*;");
[/code]

[subsection]void parse(ParseTree context, string parsedFile)[/subsection]

The precompiled BNF script parses a file, passing the argument "context" as 'this'.

[subsection]void parseString(ParseTree context, string text)[/subsection]

The precompiled BNF script parses a string, passing the argument "context" as 'this'.

[/description]
**/

#include <jni.h>

extern "C" {
	/*
	* Class:     org_codeworker_jni_CompiledBNFScript
	* Method:    buildFromString
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_buildFromString(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledBNFScript
	* Method:    buildFromFile
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_buildFromFile(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledBNFScript
	* Method:    parse
	* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_parse(JNIEnv *, jobject, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledBNFScript
	* Method:    parseString
	* Signature: (Lorg/codeworker/ParseTree;Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_parseString(JNIEnv *, jobject, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_CompiledBNFScript
	* Method:    init
	* Signature: ()[B
	*/
	JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_CompiledBNFScript_init(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_CompiledBNFScript
	* Method:    destroy
	* Signature: ()V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_CompiledBNFScript_destroy(JNIEnv *, jobject);
}

#endif
