/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2004 Cédric Lemaire

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
#ifndef _DOTNET_h_
#define _DOTNET_h_


#using <mscorlib.dll>
#include <windows.h>
#include "_vcclrit.h"

/**[description]
[title]CodeWorker in .NET[/title]

[chapter]Introduction[/chapter]

An assembly is now available for CodeWorker on the .NET platform. This enables calling parse and generation tasks
directly from C# or VB.NET. Hence, the .NET application can handle the parse tree, iterating node arrays,
accessing to subtrees (the attributes) and so on. Moreover, the developer has all native functions of the
CodeWorker scripting language at his disposal.

Many thanks to Alan Cyment for his huge patience in waiting for this .NET version of CodeWorker.

[chapter]Calling CodeWorker from .NET[/chapter]

There are some points to know before using the CodeWorker assembly in a .NET software.

[section]Assemblies[/section]
Once you have downloaded CodeWorker, you'll find the directory "cs" just below the main directory of the setup. "cs"
stands for "C#" and concerns all about CodeWorker in .NET. The Debug and release assemblies are respectively in
"cs/lib/Debug/CodeWorker.NET.dll" and "cs/lib/release/CodeWorker.NET.dll".

From here, you have to register the assembly in the "References" heading of your project (we assume here that you
have a minimal knowledge of the .NET platform).

[section]Activating the assembly[/section]

Before the first call to the CodeWorker API, the software must initialize the assembly. Here is an example of
what you should write at the beginning of your application:
[code]
// initialization in C#
CodeWorker.Main.initialize();
[/code]

Before leaving the application, CodeWorker should terminate properly. Add the following line at the end:
[code]
// initialization in C#
CodeWorker.Main.initialize();
[/code]

So, a software using CodeWorker has generally the following form:
[code]
namespace my_project
  public class My_Application {
    public static void Main() {
      // initialization of CodeWorker
      CodeWorker.Main.initialize();
      try {
        // insert the main code here
      } catch(System.Exception e) {
        System.Console.WriteLine(e);
      }
      // termination of CodeWorker
      CodeWorker.Main.terminate();
    }
  }
}
[/code]

[section]Example in C#[/section]

The following C# application behaves like a CodeWorker's leader script. It asks for the parsing of a file,
then it generates an HTML file by exploring the parse tree. A very classical process in Design-Specific Modeling.
[code]
// ... skipping the beginning of Main()

// compile a BNF and parse a DSL
CodeWorker.ParseTree tree = new CodeWorker.ParseTree();
CodeWorker.CompiledBNFScript BNF = new CodeWorker.CompiledBNFScript();
BNF.buildFromFile("My_DSL_Parser.cwp");
theBNF.parse(tree, "requirements.dsl");

// generate the documentation in HTML
CodeWorker.CompiledTemplateScript genDoc = new CodeWorker.CompiledTemplateScript();
genDoc.buildFromFile("docHTML.cwp");
genDoc.generate(tree, "doc.html");

// to finish, display of the whole parse
// tree, up to depth = 4
CodeWorker.Runtime.traceObject(tree, 4);

// ... skipping the end of Main()
[/code]

The class "CodeWorker.CompiledBNFScript" allows the precompilation of an extended-BNF script and its execution. The
execution requires a "CodeWorker.ParseTree" object as the context to populate.

The class "CodeWorker.CompiledTemplateScript" precompiles a template-based script and generates an output file,
traversing the parse tree previously populated by the BNF script.

At the end, the parse tree is displayed, up to a depth of 4, using a classical function of the scripting
language: "traceObject()". All functions of the scripting language are accessible via the class "CodeWorker.Runtime".

You'll find more examples in "cs/tests".

[chapter].NET API of CodeWorker[/chapter]

[/description]
**/

namespace CodeWorker {
	/**[description]
[section]CodeWorker.ParseTree[/section]

This class represents a CodeWorker variable, able to contain an association table, to have some attributes (branches through subtrees),
to be worth a string value or to point to another parse tree.

Example of declaration:
[code]
CodeWorker.ParseTree tree = new CodeWorker.ParseTree();
[/code]

The equivalent declaration in CodeWorker is:
[code]
local tree;
[/code]
	[/description]
	**/
	public __gc class ParseTree {
		public:
			CodeWorker::DtaScriptVariable __nogc* cppInstance_;

		private:
			bool owner_;

		public:
			/**[description]
[subsection]public ParseTree()[/subsection]

This constructor creates an internal parse tree, which will be deleted once the garbage collector will free this
instance. Note that this parse tree will have no name (the property "name" will return null).

			[/description]
			**/
			ParseTree();

			/**[description]
[subsection]public ParseTree(ParseTree)[/subsection]

This constructor points to another parse tree, but will never delete the internal parse tree it refers to. It is a
kind of copy-by-reference.

Example:
[code]
CodeWorker.ParseTree secondTree = new CodeWorker.ParseTree(firstTree);
[/code]

The equivalent declaration in CodeWorker is:
[code]
localref secondTree = firstTree;
[/code]
			[/description]
			**/
			ParseTree(ParseTree* pCopy);
			~ParseTree();

			/**[description]
[subsection]public string name {get}[/subsection]

This property returns the name of the node (null if the node was declared on the stack of the .NET application).

			[/description]
			**/
			__property System::String* get_name();

			/**[description]
[subsection]public string text {get, set}[/subsection]

This property contains the string value attached to the node, which may be null.

Example:
[code]
tree.text = "pink elephant";
System.Console.WriteLine(tree.text);
[/code]

The equivalent in CodeWorker is:
[code]
tree = "pink elephant";
traceLine(tree);
[/code]
			[/description]
			**/
			__property System::String* get_text();
			__property void set_text(System::String* pValue);

			/**[description]
[subsection]public ParseTree reference {get, set}[/subsection]

This property is assigned if the node points to another node.

Example:
[code]
secondTree.reference = firstTree;
[/code]

The equivalent in CodeWorker is:
[code]
ref secondTree = firstTree;
[/code]
			[/description]
			**/
			__property ParseTree* get_reference();
			__property void set_reference(ParseTree* pRef);

			/**[description]
[subsection]public ParseTree[] array {get}[/subsection]

This property returns the association table attached to the node. If there is no table, it returns null.

Example:
[code]
if (tree.array != null) {
	for (int j = 0; j < tree.array.Length(0); ++j) {
		CodeWorker.Runtime.traceObject(tree.array[j]);
	}
}
[/code]

The equivalent in CodeWorker is:
[code]
foreach j in tree {
	traceObject(j);
}
[/code]
			[/description]
			**/
			__property ParseTree* get_array()[];

			/**[description]
[subsection]public string[] attributeNames {get}[/subsection]

This property returns all attribute names (branches through subtrees) of the node. This function introspects the
node.

Example:
[code]
String[] list = tree.attributeNames;
if (list != null) {
	for (int j = 0; j < list.Length(0); ++j) {
		System.Console.WriteLine(list[j]);
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
			[/description]
			**/
			__property System::String* get_attributeNames()[];

			/**[description]
[subsection]public ParseTree getNode(string attr)[/subsection]

This function returns the subtree attached to an attribute of the node. If the attribute doesn't exist,
it returns null.

Example:
[code]
CodeWorker.ParseTree nextNode = tree.getNode("expression");
if ((nextNode != null) && (nextNode.text != null)) {
	System.Console.WriteLine(nextNode.text);
}
[/code]

The equivalent in CodeWorker is:
[code]
if tree.expression {
	traceLine(tree.expression);
}
[/code]
			[/description]
			**/
			ParseTree* getNode(System::String* pAttribute);

	/**[description]
[subsection]public ParseTree insertNode(string attr)[/subsection]

This function inserts a new attribute to the node and returns the subtree newly created. If the attribute
already exists, it returns the attached subtree.

Example:
[code]
tree.insertNode("expression").text = "a + b";
[/code]

The equivalent in CodeWorker is:
[code]
insert tree.expression = "a + b";
[/code]
	[/description]
	**/
			ParseTree* insertNode(System::String* pAttribute);

			ParseTree* addItem(System::String* pKey);
	};


	/**[description]
[section]CodeWorker.CompiledCommonScript[/section]

This class represents a CodeWorker common script, so called because it doesn't process parse tasks and it doesn't
generate outputs, like a leader script.

It encapsulates a precompiled common script, which can be executed at any time, without requiring a new
compilation of the script.

Do not forget to build the precompiled script before executing it.

Example:
[code]
CodeWorker.ParseTree theContext = new CodeWorker.ParseTree();
// ... [skipping]
CodeWorker.CompiledCommonScript script = new CodeWorker.CompiledCommonScript();
// precompilation of the common script
script.buildFromFile("my_script.cws");
// execution of the script
script.execute(theContext);
[/code]
	[/description]
	**/
	public __gc class CompiledCommonScript {
		private:
			CodeWorker::CGCompiledCommonScript __nogc* _pScript;

		public:
			/**[description]
[subsection]public CompiledCommonScript()[/subsection]

The constructor of a common script precompilator.

			[/description]
			**/
			CompiledCommonScript();

			virtual ~CompiledCommonScript();

			/**[description]
[subsection]void buildFromFile(string filename)[/subsection]

It precompiles a common script coming from a file.

			[/description]
			**/
			void buildFromFile(System::String* pFilename);

			/**[description]
[subsection]void buildFromString(string text)[/subsection]

It precompiles a common script stored in the string argument "text".

			[/description]
			**/
			void buildFromString(System::String* pText);

			/**[description]
[subsection]void execute(ParseTree context)[/subsection]

It executes the precompiled common script, passing the argument "context" as 'this'.

			[/description]
			**/
			void execute(ParseTree* pContext);
	};


	/**[description]
[section]CodeWorker.CompiledBNFScript[/section]

This class represents a CodeWorker extended-BNF script.

It encapsulates a precompiled BNF script, which can be executed at any time, without requiring a new
compilation of the script.

Do not forget to build the precompiled script before executing it.

Example:
[code]
CodeWorker.ParseTree theContext = new CodeWorker.ParseTree();
// ... [skipping]
CodeWorker.CompiledBNFScript script = new CodeWorker.CompiledBNFScript();
// precompilation of the common script
script.buildFromFile("my_grammar.cwp");
// execution of the script
script.parse(theContext, "my_DSL.dsl");
[/code]
	[/description]
	**/
	public __gc class CompiledBNFScript {
		private:
			CodeWorker::CGCompiledBNFScript __nogc* _pScript;

		public:
			/**[description]
[subsection]public CompiledBNFScript()[/subsection]

The constructor of an extended-BNF script precompilator.

			[/description]
			**/
			CompiledBNFScript();
			virtual ~CompiledBNFScript();

			/**[description]
[subsection]void buildFromFile(string filename)[/subsection]

It precompiles an extended-BNF script coming from a file.

			[/description]
			**/
			void buildFromFile(System::String* pFilename);

			/**[description]
[subsection]void buildFromString(string text)[/subsection]

It precompiles an extended-BNF script stored in the string argument "text".

			[/description]
			**/
			void buildFromString(System::String* pText);

			/**[description]
[subsection]void parse(ParseTree context, string parsedFile)[/subsection]

The precompiled BNF script parses a file, passing the argument "context" as 'this'.

			[/description]
			**/
			void parse(ParseTree* pContext, System::String* sParsedFile);

			/**[description]
[subsection]void parseString(ParseTree context, string text)[/subsection]

The precompiled BNF script parses a string, passing the argument "context" as 'this'.

			[/description]
			**/
			void parseString(ParseTree* pContext, System::String* sText);
	};


	/**[description]
[section]CodeWorker.CompiledTemplateScript[/section]

This class represents a CodeWorker template script.

It encapsulates a template-based script, which can be executed at any time, without requiring a new
compilation of the script.

Do not forget to build the precompiled script before executing it.

Example:
[code]
CodeWorker.ParseTree theContext = new CodeWorker.ParseTree();
// ... [skipping]
CodeWorker.CompiledTemplateScript script = new CodeWorker.CompiledTemplateScript();
// precompilation of the common script
script.buildFromFile("my_script.cwt");
// execution of the script
script.generate(theContext);
[/code]
	[/description]
	**/
	public __gc class CompiledTemplateScript {
		private:
			CodeWorker::CGCompiledTemplateScript __nogc* _pScript;

		public:
			/**[description]
[subsection]public CompiledTemplateScript()[/subsection]

The constructor of a template-based script precompilator.

			[/description]
			**/
			CompiledTemplateScript();

			virtual ~CompiledTemplateScript();

			/**[description]
[subsection]void buildFromFile(string filename)[/subsection]

It precompiles a template-based script coming from a file.

			[/description]
			**/
			void buildFromFile(System::String* pFilename);

			/**[description]
[subsection]void buildFromString(string text)[/subsection]

It precompiles a template-based script stored in the string argument "text".

			[/description]
			**/
			void buildFromString(System::String* pText);

			/**[description]
[subsection]void generate(ParseTree context, string outputFile)[/subsection]

The precompiled template-based script generates a file, passing the argument "context" as 'this'.

			[/description]
			**/
			void generate(ParseTree* pContext, System::String* sOutputFile);

			/**[description]
[subsection]string generateString(ParseTree context, string text)[/subsection]

The precompiled template-based script generates an output, which is returned as a string. The string argument
"text" contains the precedent version of this output (preserved areas, for instance).

It passes the argument "context" as 'this'.

			[/description]
			**/
			System::String*generateString(ParseTree* pContext, System::String* sOutputText);

			/**[description]
[subsection]void expand(ParseTree context, string outputFile)[/subsection]

The precompiled template-based script expands a file, passing the argument "context" as 'this'.

			[/description]
			**/
			void expand(ParseTree* pContext, System::String* sOutputFile);
	};


	/**[description]
[section]CodeWorker.CompiledTranslationScript[/section]

This class represents a CodeWorker translation script.

It encapsulates a translation script, which can be executed at any time, without requiring a new
compilation of the script.

Do not forget to build the precompiled script before executing it.

Example:
[code]
CodeWorker.ParseTree theContext = new CodeWorker.ParseTree();
// ... [skipping]
CodeWorker.CompiledTranslationScript script = new CodeWorker.CompiledTranslationScript();
// precompilation of the common script
script.buildFromFile("my_script.cwp");
// execution of the script
script.translate(theContext, "my_source.txt", "my_target.txt");
[/code]
	[/description]
	**/
	public __gc class CompiledTranslationScript {
		private:
			CodeWorker::CGCompiledTranslationScript __nogc* _pScript;

		public:
			/**[description]
[subsection]public CompiledTranslationScript()[/subsection]

The constructor of a translation script precompilator.

			[/description]
			**/
			CompiledTranslationScript();

			virtual ~CompiledTranslationScript();

			/**[description]
[subsection]void buildFromFile(string filename)[/subsection]

It precompiles a translation script coming from a file.

			[/description]
			**/
			void buildFromFile(System::String* pFilename);

			/**[description]
[subsection]void buildFromString(string text)[/subsection]

It precompiles a translation script stored in the string argument "text".

			[/description]
			**/
			void buildFromString(System::String* pText);

			/**[description]
[subsection]void translate(ParseTree context, string parsedFile, string outputFile)[/subsection]

The precompiled translation script translates the file "parsedFile" to another file "outputFile", passing the
argument "context" as 'this'.

			[/description]
			**/
			void translate(ParseTree* pContext, System::String* sParsedFile, System::String* sOutputFile);

			/**[description]
[subsection]string generateString(ParseTree context, stringBuilder text)[/subsection]

The precompiled template-based script translates the content of the string parameter "inputText" and returns
the result as a string.

It passes the argument "context" as 'this'.

			[/description]
			**/
			System::String* translateString(ParseTree* pContext, System::String* sInputText);
	};

	public __gc class BooleanRef {
		private:
			bool bValue_;

		public:
			BooleanRef() { bValue_ = false; }
			BooleanRef(bool bValue) { bValue_ = bValue; }

			__property bool get_value() { return bValue_; }
			__property void set_value(bool bValue) { bValue_ = bValue; }
	};

	public __gc class DoubleRef {
		private:
			double dValue_;

		public:
			DoubleRef() { dValue_ = 0.0; }
			DoubleRef(double dValue) { dValue_ = dValue; }

			__property double get_value() { return dValue_; }
			__property void set_value(double dValue) { dValue_ = dValue; }
	};

	/**[description]
[section]CodeWorker.Runtime[/section]

This class collects all functions existing in the scripting language of CodeWorker, declared as static methods. Please
look up the reference manual of CodeWorker for more information about their behaviour.

	[/description]
	**/
	public __gc class Runtime {
		public:
//##markup##"C++ header"
//##begin##"C++ header"
			static void appendFile(System::String* sFilename, System::String* sContent);
			static void clearVariable(ParseTree* pNode);
			static void compileToCpp(System::String* sScriptFileName, System::String* sProjectDirectory, System::String* sCodeWorkerDirectory);
			static void copyFile(System::String* sSourceFileName, System::String* sDestinationFileName);
			static void copyGenerableFile(System::String* sSourceFileName, System::String* sDestinationFileName);
			static void copySmartDirectory(System::String* sSourceDirectory, System::String* sDestinationPath);
			static void cutString(System::String* sText, System::String* sSeparator, ParseTree* slList);
			static void environTable(ParseTree* pTable);
			static void extendExecutedScript(System::String* sScriptContent);
			static void insertElementAt(ParseTree* pList, System::String* sKey, int iPosition);
			static void invertArray(ParseTree* pArray);
			static void listAllGeneratedFiles(ParseTree* pFiles);
			static void loadProject(System::String* sXMLorTXTFileName, ParseTree* pNodeToLoad);
			static void openLogFile(System::String* sFilename);
			static void produceHTML(System::String* sScriptFileName, System::String* sHTMLFileName);
			static void putEnv(System::String* sName, System::String* sValue);
			static void randomSeed(int iSeed);
			static void removeAllElements(ParseTree* pVariable);
			static void removeElement(ParseTree* pVariable, System::String* sKey);
			static void removeFirstElement(ParseTree* pList);
			static void removeLastElement(ParseTree* pList);
			static void removeRecursive(ParseTree* pVariable, System::String* sAttribute);
			static void removeVariable(ParseTree* pNode);
			static void saveBinaryToFile(System::String* sFilename, System::String* sContent);
			static void saveProject(System::String* sXMLorTXTFileName, ParseTree* pNodeToSave);
			static void saveProjectTypes(System::String* sXMLFileName);
			static void saveToFile(System::String* sFilename, System::String* sContent);
			static void setCommentBegin(System::String* sCommentBegin);
			static void setCommentEnd(System::String* sCommentEnd);
			static void setGenerationHeader(System::String* sComment);
			static void setIncludePath(System::String* sPath);
			static void setNow(System::String* sConstantDateTime);
			static void setProperty(System::String* sDefine, System::String* sValue);
			static void setTextMode(System::String* sTextMode);
			static void setVersion(System::String* sVersion);
			static void setWriteMode(System::String* sMode);
			static void setWorkingPath(System::String* sPath);
			static void sleep(int iMillis);
			static void slideNodeContent(ParseTree* pOrgNode, System::String* xDestNode);
			static void sortArray(ParseTree* pArray);
			static void traceEngine();
			static void traceLine(System::String* sLine);
			static void traceObject(ParseTree* pObject, int iDepth);
			static void traceText(System::String* sText);
			static void attachInputToSocket(int iSocket);
			static void detachInputFromSocket(int iSocket);
			static void goBack();
			static void setInputLocation(int iLocation);
			static void allFloatingLocations(ParseTree* pList);
			static void attachOutputToSocket(int iSocket);
			static void detachOutputFromSocket(int iSocket);
			static void incrementIndentLevel(int iLevel);
			static void insertText(int iLocation, System::String* sText);
			static void insertTextOnce(int iLocation, System::String* sText);
			static void insertTextToFloatingLocation(System::String* sLocation, System::String* sText);
			static void insertTextOnceToFloatingLocation(System::String* sLocation, System::String* sText);
			static void overwritePortion(int iLocation, System::String* sText, int iSize);
			static void populateProtectedArea(System::String* sProtectedAreaName, System::String* sContent);
			static void resizeOutputStream(int iNewSize);
			static void setFloatingLocation(System::String* sKey, int iLocation);
			static void setOutputLocation(int iLocation);
			static void setProtectedArea(System::String* sProtectedAreaName);
			static void writeBytes(System::String* sBytes);
			static void writeText(System::String* sText);
			static void writeTextOnce(System::String* sText);
			static void closeSocket(int iSocket);
			static bool flushOutputToSocket(int iSocket);
			static int acceptSocket(int iServerSocket);
			static double add(double dLeft, double dRight);
			static System::String* addToDate(System::String* sDate, System::String* sFormat, System::String* sShifting);
			static System::String* byteToChar(System::String* sByte);
			static unsigned long bytesToLong(System::String* sBytes);
			static unsigned short bytesToShort(System::String* sBytes);
			static System::String* canonizePath(System::String* sPath);
			static bool changeDirectory(System::String* sPath);
			static int changeFileTime(System::String* sFilename, System::String* sAccessTime, System::String* sModificationTime);
			static System::String* charAt(System::String* sText, int iIndex);
			static System::String* charToByte(System::String* sChar);
			static int charToInt(System::String* sChar);
			static bool chmod(System::String* sFilename, System::String* sMode);
			static int ceil(double dNumber);
			static int compareDate(System::String* sDate1, System::String* sDate2);
			static System::String* completeDate(System::String* sDate, System::String* sFormat);
			static System::String* completeLeftSpaces(System::String* sText, int iLength);
			static System::String* completeRightSpaces(System::String* sText, int iLength);
			static System::String* composeAdaLikeString(System::String* sText);
			static System::String* composeCLikeString(System::String* sText);
			static System::String* composeHTMLLikeString(System::String* sText);
			static System::String* composeSQLLikeString(System::String* sText);
			static System::String* computeMD5(System::String* sText);
			static bool copySmartFile(System::String* sSourceFileName, System::String* sDestinationFileName);
			static System::String* coreString(System::String* sText, int iPos, int iLastRemoved);
			static int countStringOccurences(System::String* sString, System::String* sText);
			static bool createDirectory(System::String* sPath);
			static int createINETClientSocket(System::String* sRemoteAddress, int iPort);
			static int createINETServerSocket(int iPort, int iBackLog);
			static bool createIterator(ParseTree* pI, ParseTree* pList);
			static bool createReverseIterator(ParseTree* pI, ParseTree* pList);
			static bool createVirtualFile(System::String* sHandle, System::String* sContent);
			static System::String* createVirtualTemporaryFile(System::String* sContent);
			static System::String* decodeURL(System::String* sURL);
			static double decrement(DoubleRef* dNumber);
			static bool deleteFile(System::String* sFilename);
			static bool deleteVirtualFile(System::String* sHandle);
			static double div(double dDividend, double dDivisor);
			static bool duplicateIterator(ParseTree* pOldIt, ParseTree* pNewIt);
			static System::String* encodeURL(System::String* sURL);
			static System::String* endl();
			static bool endString(System::String* sText, System::String* sEnd);
			static bool equal(double dLeft, double dRight);
			static bool equalsIgnoreCase(System::String* sLeft, System::String* sRight);
			static bool equalTrees(ParseTree* pFirstTree, ParseTree* pSecondTree);
			static System::String* executeStringQuiet(ParseTree* pThis, System::String* sCommand);
			static bool existDirectory(System::String* sPath);
			static bool existEnv(System::String* sVariable);
			static bool existFile(System::String* sFileName);
			static bool existVirtualFile(System::String* sHandle);
			static bool existVariable(ParseTree* pVariable);
			static double exp(double dX);
			static bool exploreDirectory(ParseTree* pDirectory, System::String* sPath, bool bSubfolders);
			static System::String* extractGenerationHeader(System::String* sFilename, System::Text::StringBuilder* sGenerator, System::Text::StringBuilder* sVersion, System::Text::StringBuilder* sDate);
			static System::String* fileCreation(System::String* sFilename);
			static System::String* fileLastAccess(System::String* sFilename);
			static System::String* fileLastModification(System::String* sFilename);
			static int fileLines(System::String* sFilename);
			static System::String* fileMode(System::String* sFilename);
			static int fileSize(System::String* sFilename);
			static bool findElement(System::String* sValue, ParseTree* pVariable);
			static int findFirstChar(System::String* sText, System::String* sSomeChars);
			static int findFirstSubstringIntoKeys(System::String* sSubstring, ParseTree* pArray);
			static int findLastString(System::String* sText, System::String* sFind);
			static int findNextString(System::String* sText, System::String* sFind, int iPosition);
			static int findNextSubstringIntoKeys(System::String* sSubstring, ParseTree* pArray, int iNext);
			static int findString(System::String* sText, System::String* sFind);
			static int floor(double dNumber);
			static System::String* formatDate(System::String* sDate, System::String* sFormat);
			static int getArraySize(ParseTree* pVariable);
			static System::String* getCommentBegin();
			static System::String* getCommentEnd();
			static System::String* getCurrentDirectory();
			static System::String* getEnv(System::String* sVariable);
			static System::String* getGenerationHeader();
			static System::String* getHTTPRequest(System::String* sURL, ParseTree* pHTTPSession, ParseTree* pArguments);
			static System::String* getIncludePath();
			static double getLastDelay();
			static System::String* getNow();
			static System::String* getProperty(System::String* sDefine);
			static System::String* getShortFilename(System::String* sPathFilename);
			static System::String* getTextMode();
			static int getVariableAttributes(ParseTree* pVariable, ParseTree* pList);
			static System::String* getVersion();
			static System::String* getWorkingPath();
			static System::String* getWriteMode();
			static int hexaToDecimal(System::String* sHexaNumber);
			static System::String* hostToNetworkLong(System::String* sBytes);
			static System::String* hostToNetworkShort(System::String* sBytes);
			static double increment(DoubleRef* dNumber);
			static bool indentFile(System::String* sFile, System::String* sMode);
			static bool inf(double dLeft, double dRight);
			static System::String* inputKey(bool bEcho);
			static System::String* inputLine(bool bEcho, System::String* sPrompt);
			static bool isEmpty(ParseTree* pArray);
			static bool isIdentifier(System::String* sIdentifier);
			static bool isNegative(double dNumber);
			static bool isNumeric(System::String* sNumber);
			static bool isPositive(double dNumber);
			static System::String* joinStrings(ParseTree* pList, System::String* sSeparator);
			static System::String* leftString(System::String* sText, int iLength);
			static int lengthString(System::String* sText);
			static System::String* loadBinaryFile(System::String* sFile, int iLength);
			static System::String* loadFile(System::String* sFile, int iLength);
			static System::String* loadVirtualFile(System::String* sHandle);
			static double log(double dX);
			static System::String* longToBytes(unsigned long ulLong);
			static System::String* midString(System::String* sText, int iPos, int iLength);
			static int mod(int iDividend, int iDivisor);
			static double mult(double dLeft, double dRight);
			static System::String* networkLongToHost(System::String* sBytes);
			static System::String* networkShortToHost(System::String* sBytes);
			static int octalToDecimal(System::String* sOctalNumber);
			static System::String* pathFromPackage(System::String* sPackage);
			static System::String* postHTTPRequest(System::String* sURL, ParseTree* pHTTPSession, ParseTree* pArguments);
			static double pow(double dX, double dY);
			static int randomInteger();
			static System::String* receiveBinaryFromSocket(int iSocket, int iLength);
			static System::String* receiveFromSocket(int iSocket, BooleanRef* bIsText);
			static System::String* receiveTextFromSocket(int iSocket, int iLength);
			static System::String* relativePath(System::String* sPath, System::String* sReference);
			static bool removeDirectory(System::String* sPath);
			static bool removeGenerationTagsHandler(System::String* sKey);
			static System::String* repeatString(System::String* sText, int iOccurrences);
			static System::String* replaceString(System::String* sOld, System::String* sNew, System::String* sText);
			static System::String* replaceTabulations(System::String* sText, int iTab);
			static System::String* resolveFilePath(System::String* sFilename);
			static System::String* rightString(System::String* sText, int iLength);
			static System::String* rsubString(System::String* sText, int iPos);
			static bool scanDirectories(ParseTree* pDirectory, System::String* sPath, System::String* sPattern);
			static bool scanFiles(ParseTree* pFiles, System::String* sPath, System::String* sPattern, bool bSubfolders);
			static bool sendBinaryToSocket(int iSocket, System::String* sBytes);
			static System::String* sendHTTPRequest(System::String* sURL, ParseTree* pHTTPSession);
			static bool sendTextToSocket(int iSocket, System::String* sText);
			static bool selectGenerationTagsHandler(System::String* sKey);
			static System::String* shortToBytes(unsigned short ulShort);
			static double sqrt(double dX);
			static bool startString(System::String* sText, System::String* sStart);
			static double sub(double dLeft, double dRight);
			static System::String* subString(System::String* sText, int iPos);
			static bool sup(double dLeft, double dRight);
			static System::String* system(System::String* sCommand);
			static System::String* toLowerString(System::String* sText);
			static System::String* toUpperString(System::String* sText);
			static int trimLeft(System::Text::StringBuilder* sString);
			static int trimRight(System::Text::StringBuilder* sString);
			static int trim(System::Text::StringBuilder* sString);
			static System::String* truncateAfterString(ParseTree* pVariable, System::String* sText);
			static System::String* truncateBeforeString(ParseTree* pVariable, System::String* sText);
			static System::String* UUID();
			static int countInputCols();
			static int countInputLines();
			static System::String* getInputFilename();
			static System::String* getLastReadChars(int iLength);
			static int getInputLocation();
			static bool lookAhead(System::String* sText);
			static System::String* peekChar();
			static bool readAdaString(System::Text::StringBuilder* sText);
			static System::String* readByte();
			static System::String* readBytes(int iLength);
			static System::String* readCChar();
			static System::String* readChar();
			static int readCharAsInt();
			static System::String* readChars(int iLength);
			static System::String* readIdentifier();
			static bool readIfEqualTo(System::String* sText);
			static bool readIfEqualToIgnoreCase(System::String* sText);
			static bool readIfEqualToIdentifier(System::String* sIdentifier);
			static bool readLine(System::Text::StringBuilder* sText);
			static bool readNextText(System::String* sText);
			static bool readNumber(DoubleRef* dNumber);
			static bool readPythonString(System::Text::StringBuilder* sText);
			static bool readString(System::Text::StringBuilder* sText);
			static System::String* readUptoJustOneChar(System::String* sOneAmongChars);
			static System::String* readWord();
			static bool skipBlanks();
			static bool skipSpaces();
			static bool skipEmptyCpp();
			static bool skipEmptyCppExceptDoxygen();
			static bool skipEmptyHTML();
			static bool skipEmptyLaTeX();
			static int countOutputCols();
			static int countOutputLines();
			static bool decrementIndentLevel(int iLevel);
			static bool equalLastWrittenChars(System::String* sText);
			static bool existFloatingLocation(System::String* sKey, bool bParent);
			static int getFloatingLocation(System::String* sKey);
			static System::String* getLastWrittenChars(int iNbChars);
			static System::String* getMarkupKey();
			static System::String* getMarkupValue();
			static System::String* getOutputFilename();
			static int getOutputLocation();
			static System::String* getProtectedArea(System::String* sProtection);
			static int getProtectedAreaKeys(ParseTree* pKeys);
			static bool indentText(System::String* sMode);
			static bool newFloatingLocation(System::String* sKey);
			static int remainingProtectedAreas(ParseTree* pKeys);
			static int removeFloatingLocation(System::String* sKey);
			static bool removeProtectedArea(System::String* sProtectedAreaName);
//##end##"C++ header"
	};
}

/**[description]
[chapter]Conclusion[/chapter]

The .NET assembly of CodeWorker allows the developer to drive easily DSL parsing and code generation from the
.NET platform. The parse tree can be traversed and decorated directly in any .NET programming language, rather
than only in the scripting language of CodeWorker.
   [/description]
**/

#endif
