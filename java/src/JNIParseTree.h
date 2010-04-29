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

#ifndef _JNIParseTree_h_
#define _JNIParseTree_h_

#include <jni.h>

/**[description]
[title]CodeWorker in Java[/title]

[chapter]Introduction[/chapter]

A Java interface is now available for using CodeWorker in Java applications. This enables calling parse and
generation tasks directly from Java. Hence, the Java application can handle the parse tree, iterating node arrays,
accessing to subtrees (the attributes) and so on. Moreover, the developer has all native functions of the
CodeWorker scripting language at his disposal.

[chapter]Calling CodeWorker from Java[/chapter]

There are some points to know before using CodeWorker in a Java software.

[section]JNI library and the "org.codeworker.jni" package[/section]
Once you have downloaded CodeWorker, you'll find the directory "java" just below the main directory of the
setup. This directory concerns all about CodeWorker in Java. The JNI library is directly available under the "java"
directory, in "java/JNICodeWorker.dll" (Windows only; for other platforms, you must build the dynamic library). The
Java classes implementing the bridge with the native interface are distributed in "java/org/codeworker/jni".

From here, you have to put the JNI library somewhere in your Java library paths (we assume here that you
have a minimal knowledge of the Java platform).

[section]Building the JNI library[/section]

[subsection]Windows platform[/subsection]

Under Windows, you should find easily the binary of the JNI library to download on the
[url=http://www.codeworker.org/Download.html]official Web site of CodeWorker[/url]. However, you can recompile
the JNI library.

The Visual C++ 7.1 project is located in "java/CodeWorker.JNI.vcproj". The include path to your JNI C++ header
files ("YOUR_JAVA_DISTRIBUTION/include") must be changed in the properties of the project. Once this small
adjustment done, you just have to build the project.

Note that the Debug configuration generates the JNI library "java/JNICodeWorkerD.dll".

[subsection]Other platforms[/subsection]

All sources of the JNI library are located in "java/src". You could write a little Makefile, which specifies an
include path to your JNI C++ header files ("YOUR_JAVA_DISTRIBUTION/include"), compiles the sources and links with
"libcodeworker.a" (the static library of CodeWorker).

[section]Activating the JNI library[/section]

Before the first call to the CodeWorker API, the software must load the JNI library. Here is an example of
what you could write at the beginning of your application:
[code]
// load the JNI library of CodeWorker
try {
	System.loadLibrary("JNICodeWorker");
} catch(Exception exception) {
	System.out.println("Unable to load the library: '" + exception.toString() + "'");
}
[/code]

[section]Example[/section]

The following Java application behaves like a CodeWorker's leader script. It asks for the parsing of a file,
then it generates an HTML file by exploring the parse tree. A very classical process in Design-Specific Modeling.
[code]
// ... skipping the beginning of main()

// compile a BNF and parse a DSL
org.codeworker.jni.ParseTree tree = new org.codeworker.jni.ParseTree();
org.codeworker.jni.CompiledBNFScript BNF = new org.codeworker.jni.CompiledBNFScript();
BNF.buildFromFile("My_DSL_Parser.cwp");
theBNF.parse(tree, "requirements.dsl");

// generate the documentation in HTML
org.codeworker.jni.CompiledTemplateScript genDoc = new org.codeworker.jni.CompiledTemplateScript();
genDoc.buildFromFile("docHTML.cwp");
genDoc.generate(tree, "doc.html");

// to finish, display of the whole parse
// tree, up to depth = 4
org.codeworker.jni.Runtime.traceObject(tree, 4);

// ... skipping the end of Main()
[/code]

The class "org.codeworker.jni.CompiledBNFScript" allows the precompilation of an extended-BNF script and its execution. The
execution requires a "org.codeworker.jni.ParseTree" object as the context to populate.

The class "org.codeworker.jni.CompiledTemplateScript" precompiles a template-based script and generates an output file,
traversing the parse tree previously populated by the BNF script.

At the end, the parse tree is displayed, up to a depth of 4, using a classical function of the scripting
language: "traceObject()". All functions of the scripting language are accessible via the class "org.codeworker.jni.Runtime".

You'll find more examples in "java/org/codeworker/jni/tests".

[chapter]The Java API of CodeWorker[/chapter]

[section]org.codeworker.jni.ParseTree[/section]

This class represents a CodeWorker variable, able to contain an association table, to have some attributes (branches through subtrees),
to be worth a string value or to point to another parse tree.

Example of declaration:
[code]
org.codeworker.jni.ParseTree tree = new org.codeworker.jni.ParseTree();
[/code]

The equivalent declaration in CodeWorker is:
[code]
local tree;
[/code]

[subsection]public ParseTree()[/subsection]

This constructor creates an internal parse tree, which will be deleted once the garbage collector will free this
instance. Note that this parse tree will have no name (the property "getName()" will return null).

[subsection]public ParseTree(ParseTree)[/subsection]

This constructor points to another parse tree, but will never delete the internal parse tree it refers to. It is a
kind of copy-by-reference.

Example:
[code]
org.codeworker.jni.ParseTree secondTree = new org.codeworker.jni.ParseTree(firstTree);
[/code]

The equivalent declaration in CodeWorker is:
[code]
localref secondTree = firstTree;
[/code]
[subsection]public String getName()[/subsection]

This property returns the name of the node (null if the node was declared on the stack of the Java application).

[subsection]public String get[String]Value() / void setValue(String)[/subsection]

This property contains the string value attached to the node, which may be null.
Note that getValue() may return a null string (no value attached to the node or empty string),
while getStringValue() always returns a string instance, even if the node doesn't have a value.

Example:
[code]
tree.setValue("pink elephant");
System.out.println(tree.getValue());
[/code]

The equivalent in CodeWorker is:
[code]
tree = "pink elephant";
traceLine(tree);
[/code]

[subsection]public ParseTree getReference()[/subsection]

This property is assigned if the node points to another node.

Example:
[code]
secondTree = firstTree.getReference();
[/code]

The equivalent in CodeWorker is:
[code]
ref secondTree = firstTree;
[/code]

[subsection]public ParseTree[] getArray()[/subsection]

This property returns the association table attached to the node. If there is no table, it returns null.

Example:
[code]
org.codeworker.jni.ParseTree[] nodeArray = tree.getArray();
if (nodeArray != null) {
	for (int j = 0; j < nodeArray.length; ++j) {
		org.codeworker.jni.Runtime.traceObject(nodeArray[j]);
	}
}
[/code]

The equivalent in CodeWorker is:
[code]
foreach j in tree {
	traceObject(j);
}
[/code]

[subsection]public String[] getAttributeNames()[/subsection]

This property returns all attribute names (branches through subtrees) of the node. This function introspects the
node.

Example:
[code]
String[] list = tree.getAttributeNames();
if (list != null) {
	for (int j = 0; j < list.length; ++j) {
		System.out.println(list[j]);
	}
}
[/code]

The equivalent in CodeWorker is:
[code]
local list;
getVariableAttributes(tree, list);
foreach j in list {
	traceLine(j);
}
[/code]

[subsection]public ParseTree getNode(String attr)[/subsection]

This function returns the subtree attached to an attribute of the node. If the attribute doesn't exist,
it returns null.

Example:
[code]
org.codeworker.jni.ParseTree nextNode = tree.getNode("expression");
if ((nextNode != null) && (nextNode.getValue() != null)) {
	System.out.println(nextNode.getValue());
}
[/code]

The equivalent in CodeWorker is:
[code]
if tree.expression {
	traceLine(tree.expression);
}
[/code]

[subsection]public ParseTree insertNode(String attr)[/subsection]

This function inserts a new attribute to the node and returns the subtree newly created. If the attribute
already exists, it returns the attached subtree.

Example:
[code]
tree.insertNode("expression").setValue("a + b");
[/code]

The equivalent in CodeWorker is:
[code]
insert tree.expression = "a + b";
[/code]

[/description]
**/

/**[description="java/src/JNICompiledCommonScript.h"/]**/
/**[description="java/src/JNICompiledBNFScript.h"/]**/
/**[description="java/src/JNICompiledTemplateScript.h"]**/
/**[description="java/src/JNICompiledTranslationScript.h"]**/
/**[description="java/src/JNIRuntime.h"]**/

/**[description]
[chapter]Conclusion[/chapter]

The Java package of CodeWorker allows the developer to drive easily DSL parsing and code generation from the
Java platform. The parse tree can be traversed and decorated directly in Java, rather
than only in the scripting language of CodeWorker.
   [/description]
**/

extern "C" {
	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    getName
	* Signature: ()Ljava/lang/String;
	*/
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_ParseTree_getName(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    getValue
	* Signature: ()Ljava/lang/String;
	*/
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_ParseTree_getValue(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    getStringValue
	* Signature: ()Ljava/lang/String;
	*/
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_ParseTree_getStringValue(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    setValue
	* Signature: (Ljava/lang/String;)V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_ParseTree_setValue(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    getReference
	* Signature: ()Lorg/codeworker/ParseTree;
	*/
	JNIEXPORT jobject JNICALL Java_org_codeworker_jni_ParseTree_getReference(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    getArray
	* Signature: ()[Lorg/codeworker/ParseTree;
	*/
	JNIEXPORT jobjectArray JNICALL Java_org_codeworker_jni_ParseTree_getArray(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    getAttributeNames
	* Signature: ()[Ljava/lang/String;
	*/
	JNIEXPORT jobjectArray JNICALL Java_org_codeworker_jni_ParseTree_getAttributeNames(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    getNode
	* Signature: (Ljava/lang/String;)Lorg/codeworker/ParseTree;
	*/
	JNIEXPORT jobject JNICALL Java_org_codeworker_jni_ParseTree_getNode(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    insertNode
	* Signature: (Ljava/lang/String;)Lorg/codeworker/ParseTree;
	*/
	JNIEXPORT jobject JNICALL Java_org_codeworker_jni_ParseTree_insertNode(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    addItem
	* Signature: (Ljava/lang/String;)Lorg/codeworker/ParseTree;
	*/
	JNIEXPORT jobject JNICALL Java_org_codeworker_jni_ParseTree_addItem(JNIEnv *, jobject, jstring);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    init
	* Signature: ()[B
	*/
	JNIEXPORT jbyteArray JNICALL Java_org_codeworker_jni_ParseTree_init(JNIEnv *, jobject);

	/*
	* Class:     org_codeworker_jni_ParseTree
	* Method:    destroy
	* Signature: ()V
	*/
	JNIEXPORT void JNICALL Java_org_codeworker_jni_ParseTree_destroy(JNIEnv *, jobject);
}

#endif
