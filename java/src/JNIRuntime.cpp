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

#include "ScpStream.h"

#include "CGRuntime.h"
#include "ExprScriptVariable.h"
#include "GrfBlock.h"
#include "DtaScript.h"

#include "JNI-utils.h"
#include "JNIRuntime.h"


JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_registerScript(JNIEnv *pEnv, jclass, jstring jKey, jobject jScript) {
	GET_STRING(Key);
	GET_CWOBJECT_HANDLE(Script, EXECUTE_FUNCTION);
	CodeWorker::CGRuntime::registerScript(tcKey, pScriptInstance);
	RELEASE_STRING(Key);
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_entryPoint(JNIEnv *pEnv, jclass, jobjectArray jArgs, jobject jScript) {
#define CW_FINALLY \
					for (int j = 0; j < iSize; j++) {\
						jstring pText = (jstring) pEnv->GetObjectArrayElement(jArgs, j);\
						pEnv->ReleaseStringUTFChars(pText, tsArgs[j]);\
					}\
					delete [] tsArgs
	int iResult;
	int iSize = pEnv->GetArrayLength(jArgs);
	char** tsArgs = new char*[iSize];
	for (int i = 0; i < iSize; i++) {
		jstring pText = (jstring) pEnv->GetObjectArrayElement(jArgs, i);
		tsArgs[i] = (char*) pEnv->GetStringUTFChars(pText, 0);
	}
	GET_CWOBJECT_HANDLE(Script, EXECUTE_FUNCTION);
	try {
		iResult = CodeWorker::CGRuntime::entryPoint(iSize, tsArgs, pScriptInstance);
		CW_FINALLY;
	} catch(std::exception& except) {
		CW_FINALLY;
		pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), except.what());
	}
#undef CW_FINALLY
	return iResult;
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_parseAsBNF(JNIEnv *pEnv, jclass, jobject jScript, jobject jContext, jstring jInputFile) {
	GET_CWOBJECT_HANDLE(Script, EXECUTE_FUNCTION);
	GET_PARSETREE_HANDLE(Context);
	GET_STRING(InputFile);
	std::string sInputFile = tcInputFile;
	RELEASE_STRING(InputFile);
	try {
		CodeWorker::CGRuntime::parseAsBNF(pScriptInstance, pContextInstance, sInputFile);
	} catch(std::exception& except) {
		pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), except.what());
	}
}

JNIEXPORT jobject JNICALL Java_org_codeworker_jni_Runtime_getThisTree(JNIEnv *pEnv, jclass) {
	jclass pClass = pEnv->FindClass("Lorg/codeworker/jni/ParseTree;");
	return createWrapper(pEnv, pClass, &CodeWorker::CGRuntime::getThisInternalNode());
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_throwBNFExecutionError(JNIEnv *pEnv, jclass, jstring jMessage) {
	GET_STRING(Message);
	std::string sMessage = tcMessage;
	RELEASE_STRING(Message);
	try {
		CodeWorker::CGRuntime::throwBNFExecutionError(sMessage);
	} catch(std::exception& except) {
		pEnv->ThrowNew(pEnv->FindClass("org/codeworker/Exception"), except.what());
	}
}

//##markup##"functions"
//##begin##"functions"
JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_appendFile(JNIEnv *pEnv, jclass, jstring jFilename, jstring jContent) {
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string sContent;
	GET_STRING(Content);
	sContent = tcContent;
	CodeWorker::CGRuntime::appendFile(sFilename, sContent);
	RELEASE_STRING(Content);
	RELEASE_STRING(Filename);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_clearVariable(JNIEnv *pEnv, jclass, jobject jNode) {
	CodeWorker::DtaScriptVariable* pNode;
	GET_PARSETREE_HANDLE(Node);
	pNode = pNodeInstance;
	CodeWorker::CGRuntime::clearVariable(pNode);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_compileToCpp(JNIEnv *pEnv, jclass, jstring jScriptFileName, jstring jProjectDirectory, jstring jCodeWorkerDirectory) {
	std::string sScriptFileName;
	GET_STRING(ScriptFileName);
	sScriptFileName = tcScriptFileName;
	std::string sProjectDirectory;
	GET_STRING(ProjectDirectory);
	sProjectDirectory = tcProjectDirectory;
	std::string sCodeWorkerDirectory;
	GET_STRING(CodeWorkerDirectory);
	sCodeWorkerDirectory = tcCodeWorkerDirectory;
	CodeWorker::CGRuntime::compileToCpp(sScriptFileName, sProjectDirectory, sCodeWorkerDirectory);
	RELEASE_STRING(CodeWorkerDirectory);
	RELEASE_STRING(ProjectDirectory);
	RELEASE_STRING(ScriptFileName);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_copyFile(JNIEnv *pEnv, jclass, jstring jSourceFileName, jstring jDestinationFileName) {
	std::string sSourceFileName;
	GET_STRING(SourceFileName);
	sSourceFileName = tcSourceFileName;
	std::string sDestinationFileName;
	GET_STRING(DestinationFileName);
	sDestinationFileName = tcDestinationFileName;
	CodeWorker::CGRuntime::copyFile(sSourceFileName, sDestinationFileName);
	RELEASE_STRING(DestinationFileName);
	RELEASE_STRING(SourceFileName);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_copyGenerableFile(JNIEnv *pEnv, jclass, jstring jSourceFileName, jstring jDestinationFileName) {
	std::string sSourceFileName;
	GET_STRING(SourceFileName);
	sSourceFileName = tcSourceFileName;
	std::string sDestinationFileName;
	GET_STRING(DestinationFileName);
	sDestinationFileName = tcDestinationFileName;
	CodeWorker::CGRuntime::copyGenerableFile(sSourceFileName, sDestinationFileName);
	RELEASE_STRING(DestinationFileName);
	RELEASE_STRING(SourceFileName);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_copySmartDirectory(JNIEnv *pEnv, jclass, jstring jSourceDirectory, jstring jDestinationPath) {
	std::string sSourceDirectory;
	GET_STRING(SourceDirectory);
	sSourceDirectory = tcSourceDirectory;
	std::string sDestinationPath;
	GET_STRING(DestinationPath);
	sDestinationPath = tcDestinationPath;
	CodeWorker::CGRuntime::copySmartDirectory(sSourceDirectory, sDestinationPath);
	RELEASE_STRING(DestinationPath);
	RELEASE_STRING(SourceDirectory);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_cutString(JNIEnv *pEnv, jclass, jstring jText, jstring jSeparator, jobject jList) {
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string sSeparator;
	GET_STRING(Separator);
	sSeparator = tcSeparator;
	std::list<std::string> slList;
	// NOT HANDLED YET!
	CodeWorker::CGRuntime::cutString(sText, sSeparator, slList);
	RELEASE_STRING(Separator);
	RELEASE_STRING(Text);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_environTable(JNIEnv *pEnv, jclass, jobject jTable) {
	CodeWorker::DtaScriptVariable* pTable;
	GET_PARSETREE_HANDLE(Table);
	pTable = pTableInstance;
	CodeWorker::CGRuntime::environTable(pTable);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_extendExecutedScript(JNIEnv *pEnv, jclass, jstring jScriptContent) {
	std::string sScriptContent;
	GET_STRING(ScriptContent);
	sScriptContent = tcScriptContent;
	CodeWorker::CGRuntime::extendExecutedScript(sScriptContent);
	RELEASE_STRING(ScriptContent);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertElementAt(JNIEnv *pEnv, jclass, jobject jList, jstring jKey, jint jPosition) {
	CodeWorker::DtaScriptVariable* pList;
	GET_PARSETREE_HANDLE(List);
	pList = pListInstance;
	std::string sKey;
	GET_STRING(Key);
	sKey = tcKey;
	int iPosition;
	iPosition = jPosition;
	CodeWorker::CGRuntime::insertElementAt(pList, sKey, iPosition);
	RELEASE_STRING(Key);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_invertArray(JNIEnv *pEnv, jclass, jobject jArray) {
	CodeWorker::DtaScriptVariable* pArray;
	GET_PARSETREE_HANDLE(Array);
	pArray = pArrayInstance;
	CodeWorker::CGRuntime::invertArray(pArray);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_listAllGeneratedFiles(JNIEnv *pEnv, jclass, jobject jFiles) {
	CodeWorker::DtaScriptVariable* pFiles;
	GET_PARSETREE_HANDLE(Files);
	pFiles = pFilesInstance;
	CodeWorker::CGRuntime::listAllGeneratedFiles(pFiles);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_loadProject(JNIEnv *pEnv, jclass, jstring jXMLorTXTFileName, jobject jNodeToLoad) {
	std::string sXMLorTXTFileName;
	GET_STRING(XMLorTXTFileName);
	sXMLorTXTFileName = tcXMLorTXTFileName;
	CodeWorker::DtaScriptVariable* pNodeToLoad;
	GET_PARSETREE_HANDLE(NodeToLoad);
	pNodeToLoad = pNodeToLoadInstance;
	CodeWorker::CGRuntime::loadProject(sXMLorTXTFileName, pNodeToLoad);
	RELEASE_STRING(XMLorTXTFileName);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_openLogFile(JNIEnv *pEnv, jclass, jstring jFilename) {
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	CodeWorker::CGRuntime::openLogFile(sFilename);
	RELEASE_STRING(Filename);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_produceHTML(JNIEnv *pEnv, jclass, jstring jScriptFileName, jstring jHTMLFileName) {
	std::string sScriptFileName;
	GET_STRING(ScriptFileName);
	sScriptFileName = tcScriptFileName;
	std::string sHTMLFileName;
	GET_STRING(HTMLFileName);
	sHTMLFileName = tcHTMLFileName;
	CodeWorker::CGRuntime::produceHTML(sScriptFileName, sHTMLFileName);
	RELEASE_STRING(HTMLFileName);
	RELEASE_STRING(ScriptFileName);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_putEnv(JNIEnv *pEnv, jclass, jstring jName, jstring jValue) {
	std::string sName;
	GET_STRING(Name);
	sName = tcName;
	std::string sValue;
	GET_STRING(Value);
	sValue = tcValue;
	CodeWorker::CGRuntime::putEnv(sName, sValue);
	RELEASE_STRING(Value);
	RELEASE_STRING(Name);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_randomSeed(JNIEnv *pEnv, jclass, jint jSeed) {
	int iSeed;
	iSeed = jSeed;
	CodeWorker::CGRuntime::randomSeed(iSeed);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeAllElements(JNIEnv *pEnv, jclass, jobject jVariable) {
	CodeWorker::DtaScriptVariable* pVariable;
	GET_PARSETREE_HANDLE(Variable);
	pVariable = pVariableInstance;
	CodeWorker::CGRuntime::removeAllElements(pVariable);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeElement(JNIEnv *pEnv, jclass, jobject jVariable, jstring jKey) {
	CodeWorker::DtaScriptVariable* pVariable;
	GET_PARSETREE_HANDLE(Variable);
	pVariable = pVariableInstance;
	std::string sKey;
	GET_STRING(Key);
	sKey = tcKey;
	CodeWorker::CGRuntime::removeElement(pVariable, sKey);
	RELEASE_STRING(Key);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeFirstElement(JNIEnv *pEnv, jclass, jobject jList) {
	CodeWorker::DtaScriptVariable* pList;
	GET_PARSETREE_HANDLE(List);
	pList = pListInstance;
	CodeWorker::CGRuntime::removeFirstElement(pList);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeLastElement(JNIEnv *pEnv, jclass, jobject jList) {
	CodeWorker::DtaScriptVariable* pList;
	GET_PARSETREE_HANDLE(List);
	pList = pListInstance;
	CodeWorker::CGRuntime::removeLastElement(pList);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeRecursive(JNIEnv *pEnv, jclass, jobject jVariable, jstring jAttribute) {
	CodeWorker::DtaScriptVariable* pVariable;
	GET_PARSETREE_HANDLE(Variable);
	pVariable = pVariableInstance;
	std::string sAttribute;
	GET_STRING(Attribute);
	sAttribute = tcAttribute;
	CodeWorker::CGRuntime::removeRecursive(pVariable, sAttribute);
	RELEASE_STRING(Attribute);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeVariable(JNIEnv *pEnv, jclass, jobject jNode) {
	CodeWorker::DtaScriptVariable* pNode;
	GET_PARSETREE_HANDLE(Node);
	pNode = pNodeInstance;
	CodeWorker::CGRuntime::removeVariable(pNode);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_saveBinaryToFile(JNIEnv *pEnv, jclass, jstring jFilename, jstring jContent) {
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string sContent;
	GET_STRING(Content);
	sContent = tcContent;
	CodeWorker::CGRuntime::saveBinaryToFile(sFilename, sContent);
	RELEASE_STRING(Content);
	RELEASE_STRING(Filename);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_saveProject(JNIEnv *pEnv, jclass, jstring jXMLorTXTFileName, jobject jNodeToSave) {
	std::string sXMLorTXTFileName;
	GET_STRING(XMLorTXTFileName);
	sXMLorTXTFileName = tcXMLorTXTFileName;
	CodeWorker::DtaScriptVariable* pNodeToSave;
	GET_PARSETREE_HANDLE(NodeToSave);
	pNodeToSave = pNodeToSaveInstance;
	CodeWorker::CGRuntime::saveProject(sXMLorTXTFileName, pNodeToSave);
	RELEASE_STRING(XMLorTXTFileName);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_saveProjectTypes(JNIEnv *pEnv, jclass, jstring jXMLFileName) {
	std::string sXMLFileName;
	GET_STRING(XMLFileName);
	sXMLFileName = tcXMLFileName;
	CodeWorker::CGRuntime::saveProjectTypes(sXMLFileName);
	RELEASE_STRING(XMLFileName);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_saveToFile(JNIEnv *pEnv, jclass, jstring jFilename, jstring jContent) {
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string sContent;
	GET_STRING(Content);
	sContent = tcContent;
	CodeWorker::CGRuntime::saveToFile(sFilename, sContent);
	RELEASE_STRING(Content);
	RELEASE_STRING(Filename);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setCommentBegin(JNIEnv *pEnv, jclass, jstring jCommentBegin) {
	std::string sCommentBegin;
	GET_STRING(CommentBegin);
	sCommentBegin = tcCommentBegin;
	CodeWorker::CGRuntime::setCommentBegin(sCommentBegin);
	RELEASE_STRING(CommentBegin);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setCommentEnd(JNIEnv *pEnv, jclass, jstring jCommentEnd) {
	std::string sCommentEnd;
	GET_STRING(CommentEnd);
	sCommentEnd = tcCommentEnd;
	CodeWorker::CGRuntime::setCommentEnd(sCommentEnd);
	RELEASE_STRING(CommentEnd);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setGenerationHeader(JNIEnv *pEnv, jclass, jstring jComment) {
	std::string sComment;
	GET_STRING(Comment);
	sComment = tcComment;
	CodeWorker::CGRuntime::setGenerationHeader(sComment);
	RELEASE_STRING(Comment);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setIncludePath(JNIEnv *pEnv, jclass, jstring jPath) {
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	CodeWorker::CGRuntime::setIncludePath(sPath);
	RELEASE_STRING(Path);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setNow(JNIEnv *pEnv, jclass, jstring jConstantDateTime) {
	std::string sConstantDateTime;
	GET_STRING(ConstantDateTime);
	sConstantDateTime = tcConstantDateTime;
	CodeWorker::CGRuntime::setNow(sConstantDateTime);
	RELEASE_STRING(ConstantDateTime);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setProperty(JNIEnv *pEnv, jclass, jstring jDefine, jstring jValue) {
	std::string sDefine;
	GET_STRING(Define);
	sDefine = tcDefine;
	std::string sValue;
	GET_STRING(Value);
	sValue = tcValue;
	CodeWorker::CGRuntime::setProperty(sDefine, sValue);
	RELEASE_STRING(Value);
	RELEASE_STRING(Define);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setTextMode(JNIEnv *pEnv, jclass, jstring jTextMode) {
	std::string sTextMode;
	GET_STRING(TextMode);
	sTextMode = tcTextMode;
	CodeWorker::CGRuntime::setTextMode(sTextMode);
	RELEASE_STRING(TextMode);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setVersion(JNIEnv *pEnv, jclass, jstring jVersion) {
	std::string sVersion;
	GET_STRING(Version);
	sVersion = tcVersion;
	CodeWorker::CGRuntime::setVersion(sVersion);
	RELEASE_STRING(Version);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setWriteMode(JNIEnv *pEnv, jclass, jstring jMode) {
	std::string sMode;
	GET_STRING(Mode);
	sMode = tcMode;
	CodeWorker::CGRuntime::setWriteMode(sMode);
	RELEASE_STRING(Mode);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setWorkingPath(JNIEnv *pEnv, jclass, jstring jPath) {
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	CodeWorker::CGRuntime::setWorkingPath(sPath);
	RELEASE_STRING(Path);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_sleep(JNIEnv *pEnv, jclass, jint jMillis) {
	int iMillis;
	iMillis = jMillis;
	CodeWorker::CGRuntime::sleep(iMillis);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_slideNodeContent(JNIEnv *pEnv, jclass, jobject jOrgNode, jstring jDestNode) {
	CodeWorker::DtaScriptVariable* pOrgNode;
	GET_PARSETREE_HANDLE(OrgNode);
	pOrgNode = pOrgNodeInstance;
	CodeWorker::ExprScriptVariable* exprdestNode;
	{
		GET_STRING(DestNode);
		CodeWorker::DtaScript theEmptyScript(NULL);
		CodeWorker::GrfBlock* pBlock = NULL;
		CodeWorker::GrfBlock& myNullBlock = *pBlock;
		CodeWorker::ScpStream myStream(tcDestNode);
		exprdestNode = theEmptyScript.parseVariableExpression(myNullBlock, myStream);
		RELEASE_STRING(DestNode);
	}
	std::auto_ptr<CodeWorker::ExprScriptVariable> treexprdestNode(exprdestNode);
	CodeWorker::ExprScriptVariable& xDestNode = *treexprdestNode;
	CodeWorker::CGRuntime::slideNodeContent(pOrgNode, xDestNode);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_sortArray(JNIEnv *pEnv, jclass, jobject jArray) {
	CodeWorker::DtaScriptVariable* pArray;
	GET_PARSETREE_HANDLE(Array);
	pArray = pArrayInstance;
	CodeWorker::CGRuntime::sortArray(pArray);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_traceEngine(JNIEnv *pEnv, jclass) {
	CodeWorker::CGRuntime::traceEngine();
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_traceLine(JNIEnv *pEnv, jclass, jstring jLine) {
	std::string sLine;
	GET_STRING(Line);
	sLine = tcLine;
	CodeWorker::CGRuntime::traceLine(sLine);
	RELEASE_STRING(Line);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_traceObject(JNIEnv *pEnv, jclass, jobject jObject, jint jDepth) {
	CodeWorker::DtaScriptVariable* pObject;
	GET_PARSETREE_HANDLE(Object);
	pObject = pObjectInstance;
	int iDepth;
	iDepth = jDepth;
	CodeWorker::CGRuntime::traceObject(pObject, iDepth);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_traceText(JNIEnv *pEnv, jclass, jstring jText) {
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	CodeWorker::CGRuntime::traceText(sText);
	RELEASE_STRING(Text);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_attachInputToSocket(JNIEnv *pEnv, jclass, jint jSocket) {
	int iSocket;
	iSocket = jSocket;
	CodeWorker::CGRuntime::attachInputToSocket(iSocket);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_detachInputFromSocket(JNIEnv *pEnv, jclass, jint jSocket) {
	int iSocket;
	iSocket = jSocket;
	CodeWorker::CGRuntime::detachInputFromSocket(iSocket);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_goBack(JNIEnv *pEnv, jclass) {
	CodeWorker::CGRuntime::goBack();
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setInputLocation(JNIEnv *pEnv, jclass, jint jLocation) {
	int iLocation;
	iLocation = jLocation;
	CodeWorker::CGRuntime::setInputLocation(iLocation);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_allFloatingLocations(JNIEnv *pEnv, jclass, jobject jList) {
	CodeWorker::DtaScriptVariable* pList;
	GET_PARSETREE_HANDLE(List);
	pList = pListInstance;
	CodeWorker::CGRuntime::allFloatingLocations(pList);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_attachOutputToSocket(JNIEnv *pEnv, jclass, jint jSocket) {
	int iSocket;
	iSocket = jSocket;
	CodeWorker::CGRuntime::attachOutputToSocket(iSocket);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_detachOutputFromSocket(JNIEnv *pEnv, jclass, jint jSocket) {
	int iSocket;
	iSocket = jSocket;
	CodeWorker::CGRuntime::detachOutputFromSocket(iSocket);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_incrementIndentLevel(JNIEnv *pEnv, jclass, jint jLevel) {
	int iLevel;
	iLevel = jLevel;
	CodeWorker::CGRuntime::incrementIndentLevel(iLevel);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertText(JNIEnv *pEnv, jclass, jint jLocation, jstring jText) {
	int iLocation;
	iLocation = jLocation;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	CodeWorker::CGRuntime::insertText(iLocation, sText);
	RELEASE_STRING(Text);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertTextOnce(JNIEnv *pEnv, jclass, jint jLocation, jstring jText) {
	int iLocation;
	iLocation = jLocation;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	CodeWorker::CGRuntime::insertTextOnce(iLocation, sText);
	RELEASE_STRING(Text);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertTextToFloatingLocation(JNIEnv *pEnv, jclass, jstring jLocation, jstring jText) {
	std::string sLocation;
	GET_STRING(Location);
	sLocation = tcLocation;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	CodeWorker::CGRuntime::insertTextToFloatingLocation(sLocation, sText);
	RELEASE_STRING(Text);
	RELEASE_STRING(Location);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertTextOnceToFloatingLocation(JNIEnv *pEnv, jclass, jstring jLocation, jstring jText) {
	std::string sLocation;
	GET_STRING(Location);
	sLocation = tcLocation;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	CodeWorker::CGRuntime::insertTextOnceToFloatingLocation(sLocation, sText);
	RELEASE_STRING(Text);
	RELEASE_STRING(Location);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_overwritePortion(JNIEnv *pEnv, jclass, jint jLocation, jstring jText, jint jSize) {
	int iLocation;
	iLocation = jLocation;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iSize;
	iSize = jSize;
	CodeWorker::CGRuntime::overwritePortion(iLocation, sText, iSize);
	RELEASE_STRING(Text);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_populateProtectedArea(JNIEnv *pEnv, jclass, jstring jProtectedAreaName, jstring jContent) {
	std::string sProtectedAreaName;
	GET_STRING(ProtectedAreaName);
	sProtectedAreaName = tcProtectedAreaName;
	std::string sContent;
	GET_STRING(Content);
	sContent = tcContent;
	CodeWorker::CGRuntime::populateProtectedArea(sProtectedAreaName, sContent);
	RELEASE_STRING(Content);
	RELEASE_STRING(ProtectedAreaName);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_resizeOutputStream(JNIEnv *pEnv, jclass, jint jNewSize) {
	int iNewSize;
	iNewSize = jNewSize;
	CodeWorker::CGRuntime::resizeOutputStream(iNewSize);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setFloatingLocation(JNIEnv *pEnv, jclass, jstring jKey, jint jLocation) {
	std::string sKey;
	GET_STRING(Key);
	sKey = tcKey;
	int iLocation;
	iLocation = jLocation;
	CodeWorker::CGRuntime::setFloatingLocation(sKey, iLocation);
	RELEASE_STRING(Key);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setOutputLocation(JNIEnv *pEnv, jclass, jint jLocation) {
	int iLocation;
	iLocation = jLocation;
	CodeWorker::CGRuntime::setOutputLocation(iLocation);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setProtectedArea(JNIEnv *pEnv, jclass, jstring jProtectedAreaName) {
	std::string sProtectedAreaName;
	GET_STRING(ProtectedAreaName);
	sProtectedAreaName = tcProtectedAreaName;
	CodeWorker::CGRuntime::setProtectedArea(sProtectedAreaName);
	RELEASE_STRING(ProtectedAreaName);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_writeBytes(JNIEnv *pEnv, jclass, jstring jBytes) {
	std::string sBytes;
	GET_STRING(Bytes);
	sBytes = tcBytes;
	CodeWorker::CGRuntime::writeBytes(sBytes);
	RELEASE_STRING(Bytes);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_writeText(JNIEnv *pEnv, jclass, jstring jText) {
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	CodeWorker::CGRuntime::writeText(sText);
	RELEASE_STRING(Text);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_writeTextOnce(JNIEnv *pEnv, jclass, jstring jText) {
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	CodeWorker::CGRuntime::writeTextOnce(sText);
	RELEASE_STRING(Text);
}

JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_closeSocket(JNIEnv *pEnv, jclass, jint jSocket) {
	int iSocket;
	iSocket = jSocket;
	CodeWorker::CGRuntime::closeSocket(iSocket);
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_flushOutputToSocket(JNIEnv *pEnv, jclass, jint jSocket) {
	jboolean result;
	int iSocket;
	iSocket = jSocket;
	bool cppResult = CodeWorker::CGRuntime::flushOutputToSocket(iSocket);
	result = cppResult;
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_acceptSocket(JNIEnv *pEnv, jclass, jint jServerSocket) {
	jint result;
	int iServerSocket;
	iServerSocket = jServerSocket;
	int cppResult = CodeWorker::CGRuntime::acceptSocket(iServerSocket);
	result = cppResult;
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_add(JNIEnv *pEnv, jclass, jdouble jLeft, jdouble jRight) {
	jdouble result;
	double dLeft;
	dLeft = jLeft;
	double dRight;
	dRight = jRight;
	double cppResult = CodeWorker::CGRuntime::add(dLeft, dRight);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_addToDate(JNIEnv *pEnv, jclass, jstring jDate, jstring jFormat, jstring jShifting) {
	jstring result;
	std::string sDate;
	GET_STRING(Date);
	sDate = tcDate;
	std::string sFormat;
	GET_STRING(Format);
	sFormat = tcFormat;
	std::string sShifting;
	GET_STRING(Shifting);
	sShifting = tcShifting;
	std::string cppResult = CodeWorker::CGRuntime::addToDate(sDate, sFormat, sShifting);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Shifting);
	RELEASE_STRING(Format);
	RELEASE_STRING(Date);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_byteToChar(JNIEnv *pEnv, jclass, jstring jByte) {
	jstring result;
	std::string sByte;
	GET_STRING(Byte);
	sByte = tcByte;
	std::string cppResult = CodeWorker::CGRuntime::byteToChar(sByte);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Byte);
	return result;
}

JNIEXPORT jlong JNICALL Java_org_codeworker_jni_Runtime_bytesToLong(JNIEnv *pEnv, jclass, jstring jBytes) {
	jlong result;
	std::string sBytes;
	GET_STRING(Bytes);
	sBytes = tcBytes;
	unsigned long cppResult = CodeWorker::CGRuntime::bytesToLong(sBytes);
	result = (jlong) cppResult;
	RELEASE_STRING(Bytes);
	return result;
}

JNIEXPORT jshort JNICALL Java_org_codeworker_jni_Runtime_bytesToShort(JNIEnv *pEnv, jclass, jstring jBytes) {
	jshort result;
	std::string sBytes;
	GET_STRING(Bytes);
	sBytes = tcBytes;
	unsigned short cppResult = CodeWorker::CGRuntime::bytesToShort(sBytes);
	result = (jshort) cppResult;
	RELEASE_STRING(Bytes);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_canonizePath(JNIEnv *pEnv, jclass, jstring jPath) {
	jstring result;
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	std::string cppResult = CodeWorker::CGRuntime::canonizePath(sPath);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Path);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_changeDirectory(JNIEnv *pEnv, jclass, jstring jPath) {
	jboolean result;
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	bool cppResult = CodeWorker::CGRuntime::changeDirectory(sPath);
	result = cppResult;
	RELEASE_STRING(Path);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_changeFileTime(JNIEnv *pEnv, jclass, jstring jFilename, jstring jAccessTime, jstring jModificationTime) {
	jint result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string sAccessTime;
	GET_STRING(AccessTime);
	sAccessTime = tcAccessTime;
	std::string sModificationTime;
	GET_STRING(ModificationTime);
	sModificationTime = tcModificationTime;
	int cppResult = CodeWorker::CGRuntime::changeFileTime(sFilename, sAccessTime, sModificationTime);
	result = cppResult;
	RELEASE_STRING(ModificationTime);
	RELEASE_STRING(AccessTime);
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_charAt(JNIEnv *pEnv, jclass, jstring jText, jint jIndex) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iIndex;
	iIndex = jIndex;
	std::string cppResult = CodeWorker::CGRuntime::charAt(sText, iIndex);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_charToByte(JNIEnv *pEnv, jclass, jstring jChar) {
	jstring result;
	std::string sChar;
	GET_STRING(Char);
	sChar = tcChar;
	std::string cppResult = CodeWorker::CGRuntime::charToByte(sChar);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Char);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_charToInt(JNIEnv *pEnv, jclass, jstring jChar) {
	jint result;
	std::string sChar;
	GET_STRING(Char);
	sChar = tcChar;
	int cppResult = CodeWorker::CGRuntime::charToInt(sChar);
	result = cppResult;
	RELEASE_STRING(Char);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_chmod(JNIEnv *pEnv, jclass, jstring jFilename, jstring jMode) {
	jboolean result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string sMode;
	GET_STRING(Mode);
	sMode = tcMode;
	bool cppResult = CodeWorker::CGRuntime::chmod(sFilename, sMode);
	result = cppResult;
	RELEASE_STRING(Mode);
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_ceil(JNIEnv *pEnv, jclass, jdouble jNumber) {
	jint result;
	double dNumber;
	dNumber = jNumber;
	int cppResult = CodeWorker::CGRuntime::ceil(dNumber);
	result = cppResult;
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_compareDate(JNIEnv *pEnv, jclass, jstring jDate1, jstring jDate2) {
	jint result;
	std::string sDate1;
	GET_STRING(Date1);
	sDate1 = tcDate1;
	std::string sDate2;
	GET_STRING(Date2);
	sDate2 = tcDate2;
	int cppResult = CodeWorker::CGRuntime::compareDate(sDate1, sDate2);
	result = cppResult;
	RELEASE_STRING(Date2);
	RELEASE_STRING(Date1);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_completeDate(JNIEnv *pEnv, jclass, jstring jDate, jstring jFormat) {
	jstring result;
	std::string sDate;
	GET_STRING(Date);
	sDate = tcDate;
	std::string sFormat;
	GET_STRING(Format);
	sFormat = tcFormat;
	std::string cppResult = CodeWorker::CGRuntime::completeDate(sDate, sFormat);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Format);
	RELEASE_STRING(Date);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_completeLeftSpaces(JNIEnv *pEnv, jclass, jstring jText, jint jLength) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::completeLeftSpaces(sText, iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_completeRightSpaces(JNIEnv *pEnv, jclass, jstring jText, jint jLength) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::completeRightSpaces(sText, iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_composeAdaLikeString(JNIEnv *pEnv, jclass, jstring jText) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::composeAdaLikeString(sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_composeCLikeString(JNIEnv *pEnv, jclass, jstring jText) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::composeCLikeString(sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_composeHTMLLikeString(JNIEnv *pEnv, jclass, jstring jText) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::composeHTMLLikeString(sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_composeSQLLikeString(JNIEnv *pEnv, jclass, jstring jText) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::composeSQLLikeString(sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_computeMD5(JNIEnv *pEnv, jclass, jstring jText) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::computeMD5(sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_copySmartFile(JNIEnv *pEnv, jclass, jstring jSourceFileName, jstring jDestinationFileName) {
	jboolean result;
	std::string sSourceFileName;
	GET_STRING(SourceFileName);
	sSourceFileName = tcSourceFileName;
	std::string sDestinationFileName;
	GET_STRING(DestinationFileName);
	sDestinationFileName = tcDestinationFileName;
	bool cppResult = CodeWorker::CGRuntime::copySmartFile(sSourceFileName, sDestinationFileName);
	result = cppResult;
	RELEASE_STRING(DestinationFileName);
	RELEASE_STRING(SourceFileName);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_coreString(JNIEnv *pEnv, jclass, jstring jText, jint jPos, jint jLastRemoved) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iPos;
	iPos = jPos;
	int iLastRemoved;
	iLastRemoved = jLastRemoved;
	std::string cppResult = CodeWorker::CGRuntime::coreString(sText, iPos, iLastRemoved);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countStringOccurences(JNIEnv *pEnv, jclass, jstring jString, jstring jText) {
	jint result;
	std::string sString;
	GET_STRING(String);
	sString = tcString;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int cppResult = CodeWorker::CGRuntime::countStringOccurences(sString, sText);
	result = cppResult;
	RELEASE_STRING(Text);
	RELEASE_STRING(String);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_createDirectory(JNIEnv *pEnv, jclass, jstring jPath) {
	jboolean result;
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	bool cppResult = CodeWorker::CGRuntime::createDirectory(sPath);
	result = cppResult;
	RELEASE_STRING(Path);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_createINETClientSocket(JNIEnv *pEnv, jclass, jstring jRemoteAddress, jint jPort) {
	jint result;
	std::string sRemoteAddress;
	GET_STRING(RemoteAddress);
	sRemoteAddress = tcRemoteAddress;
	int iPort;
	iPort = jPort;
	int cppResult = CodeWorker::CGRuntime::createINETClientSocket(sRemoteAddress, iPort);
	result = cppResult;
	RELEASE_STRING(RemoteAddress);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_createINETServerSocket(JNIEnv *pEnv, jclass, jint jPort, jint jBackLog) {
	jint result;
	int iPort;
	iPort = jPort;
	int iBackLog;
	iBackLog = jBackLog;
	int cppResult = CodeWorker::CGRuntime::createINETServerSocket(iPort, iBackLog);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_createIterator(JNIEnv *pEnv, jclass, jobject jI, jobject jList) {
	jboolean result;
	CodeWorker::DtaScriptVariable* pI;
	GET_PARSETREE_HANDLE(I);
	pI = pIInstance;
	CodeWorker::DtaScriptVariable* pList;
	GET_PARSETREE_HANDLE(List);
	pList = pListInstance;
	bool cppResult = CodeWorker::CGRuntime::createIterator(pI, pList);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_createReverseIterator(JNIEnv *pEnv, jclass, jobject jI, jobject jList) {
	jboolean result;
	CodeWorker::DtaScriptVariable* pI;
	GET_PARSETREE_HANDLE(I);
	pI = pIInstance;
	CodeWorker::DtaScriptVariable* pList;
	GET_PARSETREE_HANDLE(List);
	pList = pListInstance;
	bool cppResult = CodeWorker::CGRuntime::createReverseIterator(pI, pList);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_createVirtualFile(JNIEnv *pEnv, jclass, jstring jHandle, jstring jContent) {
	jboolean result;
	std::string sHandle;
	GET_STRING(Handle);
	sHandle = tcHandle;
	std::string sContent;
	GET_STRING(Content);
	sContent = tcContent;
	bool cppResult = CodeWorker::CGRuntime::createVirtualFile(sHandle, sContent);
	result = cppResult;
	RELEASE_STRING(Content);
	RELEASE_STRING(Handle);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_createVirtualTemporaryFile(JNIEnv *pEnv, jclass, jstring jContent) {
	jstring result;
	std::string sContent;
	GET_STRING(Content);
	sContent = tcContent;
	std::string cppResult = CodeWorker::CGRuntime::createVirtualTemporaryFile(sContent);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Content);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_decodeURL(JNIEnv *pEnv, jclass, jstring jURL) {
	jstring result;
	std::string sURL;
	GET_STRING(URL);
	sURL = tcURL;
	std::string cppResult = CodeWorker::CGRuntime::decodeURL(sURL);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(URL);
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_decrement(JNIEnv *pEnv, jclass, jobject jNumber) {
	jdouble result;
	double dNumber;
	// NOT HANDLED YET!
	double cppResult = CodeWorker::CGRuntime::decrement(dNumber);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_deleteFile(JNIEnv *pEnv, jclass, jstring jFilename) {
	jboolean result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	bool cppResult = CodeWorker::CGRuntime::deleteFile(sFilename);
	result = cppResult;
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_deleteVirtualFile(JNIEnv *pEnv, jclass, jstring jHandle) {
	jboolean result;
	std::string sHandle;
	GET_STRING(Handle);
	sHandle = tcHandle;
	bool cppResult = CodeWorker::CGRuntime::deleteVirtualFile(sHandle);
	result = cppResult;
	RELEASE_STRING(Handle);
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_div(JNIEnv *pEnv, jclass, jdouble jDividend, jdouble jDivisor) {
	jdouble result;
	double dDividend;
	dDividend = jDividend;
	double dDivisor;
	dDivisor = jDivisor;
	double cppResult = CodeWorker::CGRuntime::div(dDividend, dDivisor);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_duplicateIterator(JNIEnv *pEnv, jclass, jobject jOldIt, jobject jNewIt) {
	jboolean result;
	CodeWorker::DtaScriptVariable* pOldIt;
	GET_PARSETREE_HANDLE(OldIt);
	pOldIt = pOldItInstance;
	CodeWorker::DtaScriptVariable* pNewIt;
	GET_PARSETREE_HANDLE(NewIt);
	pNewIt = pNewItInstance;
	bool cppResult = CodeWorker::CGRuntime::duplicateIterator(pOldIt, pNewIt);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_encodeURL(JNIEnv *pEnv, jclass, jstring jURL) {
	jstring result;
	std::string sURL;
	GET_STRING(URL);
	sURL = tcURL;
	std::string cppResult = CodeWorker::CGRuntime::encodeURL(sURL);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(URL);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_endl(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::endl();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_endString(JNIEnv *pEnv, jclass, jstring jText, jstring jEnd) {
	jboolean result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string sEnd;
	GET_STRING(End);
	sEnd = tcEnd;
	bool cppResult = CodeWorker::CGRuntime::endString(sText, sEnd);
	result = cppResult;
	RELEASE_STRING(End);
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_equal(JNIEnv *pEnv, jclass, jdouble jLeft, jdouble jRight) {
	jboolean result;
	double dLeft;
	dLeft = jLeft;
	double dRight;
	dRight = jRight;
	bool cppResult = CodeWorker::CGRuntime::equal(dLeft, dRight);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_equalsIgnoreCase(JNIEnv *pEnv, jclass, jstring jLeft, jstring jRight) {
	jboolean result;
	std::string sLeft;
	GET_STRING(Left);
	sLeft = tcLeft;
	std::string sRight;
	GET_STRING(Right);
	sRight = tcRight;
	bool cppResult = CodeWorker::CGRuntime::equalsIgnoreCase(sLeft, sRight);
	result = cppResult;
	RELEASE_STRING(Right);
	RELEASE_STRING(Left);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_equalTrees(JNIEnv *pEnv, jclass, jobject jFirstTree, jobject jSecondTree) {
	jboolean result;
	CodeWorker::DtaScriptVariable* pFirstTree;
	GET_PARSETREE_HANDLE(FirstTree);
	pFirstTree = pFirstTreeInstance;
	CodeWorker::DtaScriptVariable* pSecondTree;
	GET_PARSETREE_HANDLE(SecondTree);
	pSecondTree = pSecondTreeInstance;
	bool cppResult = CodeWorker::CGRuntime::equalTrees(pFirstTree, pSecondTree);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_executeStringQuiet(JNIEnv *pEnv, jclass, jobject jThis, jstring jCommand) {
	jstring result;
	CodeWorker::DtaScriptVariable* pThis;
	GET_PARSETREE_HANDLE(This);
	pThis = pThisInstance;
	std::string sCommand;
	GET_STRING(Command);
	sCommand = tcCommand;
	std::string cppResult = CodeWorker::CGRuntime::executeStringQuiet(pThis, sCommand);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Command);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existDirectory(JNIEnv *pEnv, jclass, jstring jPath) {
	jboolean result;
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	bool cppResult = CodeWorker::CGRuntime::existDirectory(sPath);
	result = cppResult;
	RELEASE_STRING(Path);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existEnv(JNIEnv *pEnv, jclass, jstring jVariable) {
	jboolean result;
	std::string sVariable;
	GET_STRING(Variable);
	sVariable = tcVariable;
	bool cppResult = CodeWorker::CGRuntime::existEnv(sVariable);
	result = cppResult;
	RELEASE_STRING(Variable);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existFile(JNIEnv *pEnv, jclass, jstring jFileName) {
	jboolean result;
	std::string sFileName;
	GET_STRING(FileName);
	sFileName = tcFileName;
	bool cppResult = CodeWorker::CGRuntime::existFile(sFileName);
	result = cppResult;
	RELEASE_STRING(FileName);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existVirtualFile(JNIEnv *pEnv, jclass, jstring jHandle) {
	jboolean result;
	std::string sHandle;
	GET_STRING(Handle);
	sHandle = tcHandle;
	bool cppResult = CodeWorker::CGRuntime::existVirtualFile(sHandle);
	result = cppResult;
	RELEASE_STRING(Handle);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existVariable(JNIEnv *pEnv, jclass, jobject jVariable) {
	jboolean result;
	CodeWorker::DtaScriptVariable* pVariable;
	GET_PARSETREE_HANDLE(Variable);
	pVariable = pVariableInstance;
	bool cppResult = CodeWorker::CGRuntime::existVariable(pVariable);
	result = cppResult;
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_exp(JNIEnv *pEnv, jclass, jdouble jX) {
	jdouble result;
	double dX;
	dX = jX;
	double cppResult = CodeWorker::CGRuntime::exp(dX);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_exploreDirectory(JNIEnv *pEnv, jclass, jobject jDirectory, jstring jPath, jboolean jSubfolders) {
	jboolean result;
	CodeWorker::DtaScriptVariable* pDirectory;
	GET_PARSETREE_HANDLE(Directory);
	pDirectory = pDirectoryInstance;
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	bool bSubfolders;
	bSubfolders = (jSubfolders != '\0');
	bool cppResult = CodeWorker::CGRuntime::exploreDirectory(pDirectory, sPath, bSubfolders);
	result = cppResult;
	RELEASE_STRING(Path);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_extractGenerationHeader(JNIEnv *pEnv, jclass, jstring jFilename, jobject jGenerator, jobject jVersion, jobject jDate) {
	jstring result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string sGenerator;
	// NOT HANDLED YET!
	std::string sVersion;
	// NOT HANDLED YET!
	std::string sDate;
	// NOT HANDLED YET!
	std::string cppResult = CodeWorker::CGRuntime::extractGenerationHeader(sFilename, sGenerator, sVersion, sDate);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_fileCreation(JNIEnv *pEnv, jclass, jstring jFilename) {
	jstring result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string cppResult = CodeWorker::CGRuntime::fileCreation(sFilename);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_fileLastAccess(JNIEnv *pEnv, jclass, jstring jFilename) {
	jstring result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string cppResult = CodeWorker::CGRuntime::fileLastAccess(sFilename);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_fileLastModification(JNIEnv *pEnv, jclass, jstring jFilename) {
	jstring result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string cppResult = CodeWorker::CGRuntime::fileLastModification(sFilename);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_fileLines(JNIEnv *pEnv, jclass, jstring jFilename) {
	jint result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	int cppResult = CodeWorker::CGRuntime::fileLines(sFilename);
	result = cppResult;
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_fileMode(JNIEnv *pEnv, jclass, jstring jFilename) {
	jstring result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string cppResult = CodeWorker::CGRuntime::fileMode(sFilename);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_fileSize(JNIEnv *pEnv, jclass, jstring jFilename) {
	jint result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	int cppResult = CodeWorker::CGRuntime::fileSize(sFilename);
	result = cppResult;
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_findElement(JNIEnv *pEnv, jclass, jstring jValue, jobject jVariable) {
	jboolean result;
	std::string sValue;
	GET_STRING(Value);
	sValue = tcValue;
	CodeWorker::DtaScriptVariable* pVariable;
	GET_PARSETREE_HANDLE(Variable);
	pVariable = pVariableInstance;
	bool cppResult = CodeWorker::CGRuntime::findElement(sValue, pVariable);
	result = cppResult;
	RELEASE_STRING(Value);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findFirstChar(JNIEnv *pEnv, jclass, jstring jText, jstring jSomeChars) {
	jint result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string sSomeChars;
	GET_STRING(SomeChars);
	sSomeChars = tcSomeChars;
	int cppResult = CodeWorker::CGRuntime::findFirstChar(sText, sSomeChars);
	result = cppResult;
	RELEASE_STRING(SomeChars);
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findFirstSubstringIntoKeys(JNIEnv *pEnv, jclass, jstring jSubstring, jobject jArray) {
	jint result;
	std::string sSubstring;
	GET_STRING(Substring);
	sSubstring = tcSubstring;
	CodeWorker::DtaScriptVariable* pArray;
	GET_PARSETREE_HANDLE(Array);
	pArray = pArrayInstance;
	int cppResult = CodeWorker::CGRuntime::findFirstSubstringIntoKeys(sSubstring, pArray);
	result = cppResult;
	RELEASE_STRING(Substring);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findLastString(JNIEnv *pEnv, jclass, jstring jText, jstring jFind) {
	jint result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string sFind;
	GET_STRING(Find);
	sFind = tcFind;
	int cppResult = CodeWorker::CGRuntime::findLastString(sText, sFind);
	result = cppResult;
	RELEASE_STRING(Find);
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findNextString(JNIEnv *pEnv, jclass, jstring jText, jstring jFind, jint jPosition) {
	jint result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string sFind;
	GET_STRING(Find);
	sFind = tcFind;
	int iPosition;
	iPosition = jPosition;
	int cppResult = CodeWorker::CGRuntime::findNextString(sText, sFind, iPosition);
	result = cppResult;
	RELEASE_STRING(Find);
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findNextSubstringIntoKeys(JNIEnv *pEnv, jclass, jstring jSubstring, jobject jArray, jint jNext) {
	jint result;
	std::string sSubstring;
	GET_STRING(Substring);
	sSubstring = tcSubstring;
	CodeWorker::DtaScriptVariable* pArray;
	GET_PARSETREE_HANDLE(Array);
	pArray = pArrayInstance;
	int iNext;
	iNext = jNext;
	int cppResult = CodeWorker::CGRuntime::findNextSubstringIntoKeys(sSubstring, pArray, iNext);
	result = cppResult;
	RELEASE_STRING(Substring);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findString(JNIEnv *pEnv, jclass, jstring jText, jstring jFind) {
	jint result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string sFind;
	GET_STRING(Find);
	sFind = tcFind;
	int cppResult = CodeWorker::CGRuntime::findString(sText, sFind);
	result = cppResult;
	RELEASE_STRING(Find);
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_floor(JNIEnv *pEnv, jclass, jdouble jNumber) {
	jint result;
	double dNumber;
	dNumber = jNumber;
	int cppResult = CodeWorker::CGRuntime::floor(dNumber);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_formatDate(JNIEnv *pEnv, jclass, jstring jDate, jstring jFormat) {
	jstring result;
	std::string sDate;
	GET_STRING(Date);
	sDate = tcDate;
	std::string sFormat;
	GET_STRING(Format);
	sFormat = tcFormat;
	std::string cppResult = CodeWorker::CGRuntime::formatDate(sDate, sFormat);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Format);
	RELEASE_STRING(Date);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getArraySize(JNIEnv *pEnv, jclass, jobject jVariable) {
	jint result;
	CodeWorker::DtaScriptVariable* pVariable;
	GET_PARSETREE_HANDLE(Variable);
	pVariable = pVariableInstance;
	int cppResult = CodeWorker::CGRuntime::getArraySize(pVariable);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getCommentBegin(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getCommentBegin();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getCommentEnd(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getCommentEnd();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getCurrentDirectory(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getCurrentDirectory();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getEnv(JNIEnv *pEnv, jclass, jstring jVariable) {
	jstring result;
	std::string sVariable;
	GET_STRING(Variable);
	sVariable = tcVariable;
	std::string cppResult = CodeWorker::CGRuntime::getEnv(sVariable);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Variable);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getGenerationHeader(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getGenerationHeader();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getHTTPRequest(JNIEnv *pEnv, jclass, jstring jURL, jobject jHTTPSession, jobject jArguments) {
	jstring result;
	std::string sURL;
	GET_STRING(URL);
	sURL = tcURL;
	CodeWorker::DtaScriptVariable* pHTTPSession;
	GET_PARSETREE_HANDLE(HTTPSession);
	pHTTPSession = pHTTPSessionInstance;
	CodeWorker::DtaScriptVariable* pArguments;
	GET_PARSETREE_HANDLE(Arguments);
	pArguments = pArgumentsInstance;
	std::string cppResult = CodeWorker::CGRuntime::getHTTPRequest(sURL, pHTTPSession, pArguments);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(URL);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getIncludePath(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getIncludePath();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_getLastDelay(JNIEnv *pEnv, jclass) {
	jdouble result;
	double cppResult = CodeWorker::CGRuntime::getLastDelay();
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getNow(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getNow();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getProperty(JNIEnv *pEnv, jclass, jstring jDefine) {
	jstring result;
	std::string sDefine;
	GET_STRING(Define);
	sDefine = tcDefine;
	std::string cppResult = CodeWorker::CGRuntime::getProperty(sDefine);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Define);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getShortFilename(JNIEnv *pEnv, jclass, jstring jPathFilename) {
	jstring result;
	std::string sPathFilename;
	GET_STRING(PathFilename);
	sPathFilename = tcPathFilename;
	std::string cppResult = CodeWorker::CGRuntime::getShortFilename(sPathFilename);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(PathFilename);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getTextMode(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getTextMode();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getVariableAttributes(JNIEnv *pEnv, jclass, jobject jVariable, jobject jList) {
	jint result;
	CodeWorker::DtaScriptVariable* pVariable;
	GET_PARSETREE_HANDLE(Variable);
	pVariable = pVariableInstance;
	CodeWorker::DtaScriptVariable* pList;
	GET_PARSETREE_HANDLE(List);
	pList = pListInstance;
	int cppResult = CodeWorker::CGRuntime::getVariableAttributes(pVariable, pList);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getVersion(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getVersion();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getWorkingPath(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getWorkingPath();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getWriteMode(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getWriteMode();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_hexaToDecimal(JNIEnv *pEnv, jclass, jstring jHexaNumber) {
	jint result;
	std::string sHexaNumber;
	GET_STRING(HexaNumber);
	sHexaNumber = tcHexaNumber;
	int cppResult = CodeWorker::CGRuntime::hexaToDecimal(sHexaNumber);
	result = cppResult;
	RELEASE_STRING(HexaNumber);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_hostToNetworkLong(JNIEnv *pEnv, jclass, jstring jBytes) {
	jstring result;
	std::string sBytes;
	GET_STRING(Bytes);
	sBytes = tcBytes;
	std::string cppResult = CodeWorker::CGRuntime::hostToNetworkLong(sBytes);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Bytes);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_hostToNetworkShort(JNIEnv *pEnv, jclass, jstring jBytes) {
	jstring result;
	std::string sBytes;
	GET_STRING(Bytes);
	sBytes = tcBytes;
	std::string cppResult = CodeWorker::CGRuntime::hostToNetworkShort(sBytes);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Bytes);
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_increment(JNIEnv *pEnv, jclass, jobject jNumber) {
	jdouble result;
	double dNumber;
	// NOT HANDLED YET!
	double cppResult = CodeWorker::CGRuntime::increment(dNumber);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_indentFile(JNIEnv *pEnv, jclass, jstring jFile, jstring jMode) {
	jboolean result;
	std::string sFile;
	GET_STRING(File);
	sFile = tcFile;
	std::string sMode;
	GET_STRING(Mode);
	sMode = tcMode;
	bool cppResult = CodeWorker::CGRuntime::indentFile(sFile, sMode);
	result = cppResult;
	RELEASE_STRING(Mode);
	RELEASE_STRING(File);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_inf(JNIEnv *pEnv, jclass, jdouble jLeft, jdouble jRight) {
	jboolean result;
	double dLeft;
	dLeft = jLeft;
	double dRight;
	dRight = jRight;
	bool cppResult = CodeWorker::CGRuntime::inf(dLeft, dRight);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_inputKey(JNIEnv *pEnv, jclass, jboolean jEcho) {
	jstring result;
	bool bEcho;
	bEcho = (jEcho != '\0');
	std::string cppResult = CodeWorker::CGRuntime::inputKey(bEcho);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_inputLine(JNIEnv *pEnv, jclass, jboolean jEcho, jstring jPrompt) {
	jstring result;
	bool bEcho;
	bEcho = (jEcho != '\0');
	std::string sPrompt;
	GET_STRING(Prompt);
	sPrompt = tcPrompt;
	std::string cppResult = CodeWorker::CGRuntime::inputLine(bEcho, sPrompt);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Prompt);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isEmpty(JNIEnv *pEnv, jclass, jobject jArray) {
	jboolean result;
	CodeWorker::DtaScriptVariable* pArray;
	GET_PARSETREE_HANDLE(Array);
	pArray = pArrayInstance;
	bool cppResult = CodeWorker::CGRuntime::isEmpty(pArray);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isIdentifier(JNIEnv *pEnv, jclass, jstring jIdentifier) {
	jboolean result;
	std::string sIdentifier;
	GET_STRING(Identifier);
	sIdentifier = tcIdentifier;
	bool cppResult = CodeWorker::CGRuntime::isIdentifier(sIdentifier);
	result = cppResult;
	RELEASE_STRING(Identifier);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isNegative(JNIEnv *pEnv, jclass, jdouble jNumber) {
	jboolean result;
	double dNumber;
	dNumber = jNumber;
	bool cppResult = CodeWorker::CGRuntime::isNegative(dNumber);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isNumeric(JNIEnv *pEnv, jclass, jstring jNumber) {
	jboolean result;
	std::string sNumber;
	GET_STRING(Number);
	sNumber = tcNumber;
	bool cppResult = CodeWorker::CGRuntime::isNumeric(sNumber);
	result = cppResult;
	RELEASE_STRING(Number);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isPositive(JNIEnv *pEnv, jclass, jdouble jNumber) {
	jboolean result;
	double dNumber;
	dNumber = jNumber;
	bool cppResult = CodeWorker::CGRuntime::isPositive(dNumber);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_joinStrings(JNIEnv *pEnv, jclass, jobject jList, jstring jSeparator) {
	jstring result;
	CodeWorker::DtaScriptVariable* pList;
	GET_PARSETREE_HANDLE(List);
	pList = pListInstance;
	std::string sSeparator;
	GET_STRING(Separator);
	sSeparator = tcSeparator;
	std::string cppResult = CodeWorker::CGRuntime::joinStrings(pList, sSeparator);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Separator);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_leftString(JNIEnv *pEnv, jclass, jstring jText, jint jLength) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::leftString(sText, iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_lengthString(JNIEnv *pEnv, jclass, jstring jText) {
	jint result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int cppResult = CodeWorker::CGRuntime::lengthString(sText);
	result = cppResult;
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_loadBinaryFile(JNIEnv *pEnv, jclass, jstring jFile, jint jLength) {
	jstring result;
	std::string sFile;
	GET_STRING(File);
	sFile = tcFile;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::loadBinaryFile(sFile, iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(File);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_loadFile(JNIEnv *pEnv, jclass, jstring jFile, jint jLength) {
	jstring result;
	std::string sFile;
	GET_STRING(File);
	sFile = tcFile;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::loadFile(sFile, iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(File);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_loadVirtualFile(JNIEnv *pEnv, jclass, jstring jHandle) {
	jstring result;
	std::string sHandle;
	GET_STRING(Handle);
	sHandle = tcHandle;
	std::string cppResult = CodeWorker::CGRuntime::loadVirtualFile(sHandle);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Handle);
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_log(JNIEnv *pEnv, jclass, jdouble jX) {
	jdouble result;
	double dX;
	dX = jX;
	double cppResult = CodeWorker::CGRuntime::log(dX);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_longToBytes(JNIEnv *pEnv, jclass, jlong jLong) {
	jstring result;
	unsigned long ulLong;
	ulLong = (unsigned long) jLong;
	std::string cppResult = CodeWorker::CGRuntime::longToBytes(ulLong);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_midString(JNIEnv *pEnv, jclass, jstring jText, jint jPos, jint jLength) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iPos;
	iPos = jPos;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::midString(sText, iPos, iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_mod(JNIEnv *pEnv, jclass, jint jDividend, jint jDivisor) {
	jint result;
	int iDividend;
	iDividend = jDividend;
	int iDivisor;
	iDivisor = jDivisor;
	int cppResult = CodeWorker::CGRuntime::mod(iDividend, iDivisor);
	result = cppResult;
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_mult(JNIEnv *pEnv, jclass, jdouble jLeft, jdouble jRight) {
	jdouble result;
	double dLeft;
	dLeft = jLeft;
	double dRight;
	dRight = jRight;
	double cppResult = CodeWorker::CGRuntime::mult(dLeft, dRight);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_networkLongToHost(JNIEnv *pEnv, jclass, jstring jBytes) {
	jstring result;
	std::string sBytes;
	GET_STRING(Bytes);
	sBytes = tcBytes;
	std::string cppResult = CodeWorker::CGRuntime::networkLongToHost(sBytes);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Bytes);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_networkShortToHost(JNIEnv *pEnv, jclass, jstring jBytes) {
	jstring result;
	std::string sBytes;
	GET_STRING(Bytes);
	sBytes = tcBytes;
	std::string cppResult = CodeWorker::CGRuntime::networkShortToHost(sBytes);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Bytes);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_octalToDecimal(JNIEnv *pEnv, jclass, jstring jOctalNumber) {
	jint result;
	std::string sOctalNumber;
	GET_STRING(OctalNumber);
	sOctalNumber = tcOctalNumber;
	int cppResult = CodeWorker::CGRuntime::octalToDecimal(sOctalNumber);
	result = cppResult;
	RELEASE_STRING(OctalNumber);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_pathFromPackage(JNIEnv *pEnv, jclass, jstring jPackage) {
	jstring result;
	std::string sPackage;
	GET_STRING(Package);
	sPackage = tcPackage;
	std::string cppResult = CodeWorker::CGRuntime::pathFromPackage(sPackage);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Package);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_postHTTPRequest(JNIEnv *pEnv, jclass, jstring jURL, jobject jHTTPSession, jobject jArguments) {
	jstring result;
	std::string sURL;
	GET_STRING(URL);
	sURL = tcURL;
	CodeWorker::DtaScriptVariable* pHTTPSession;
	GET_PARSETREE_HANDLE(HTTPSession);
	pHTTPSession = pHTTPSessionInstance;
	CodeWorker::DtaScriptVariable* pArguments;
	GET_PARSETREE_HANDLE(Arguments);
	pArguments = pArgumentsInstance;
	std::string cppResult = CodeWorker::CGRuntime::postHTTPRequest(sURL, pHTTPSession, pArguments);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(URL);
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_pow(JNIEnv *pEnv, jclass, jdouble jX, jdouble jY) {
	jdouble result;
	double dX;
	dX = jX;
	double dY;
	dY = jY;
	double cppResult = CodeWorker::CGRuntime::pow(dX, dY);
	result = cppResult;
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_randomInteger(JNIEnv *pEnv, jclass) {
	jint result;
	int cppResult = CodeWorker::CGRuntime::randomInteger();
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_receiveBinaryFromSocket(JNIEnv *pEnv, jclass, jint jSocket, jint jLength) {
	jstring result;
	int iSocket;
	iSocket = jSocket;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::receiveBinaryFromSocket(iSocket, iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_receiveFromSocket(JNIEnv *pEnv, jclass, jint jSocket, jobject jIsText) {
	jstring result;
	int iSocket;
	iSocket = jSocket;
	bool bIsText;
	// NOT HANDLED YET!
	std::string cppResult = CodeWorker::CGRuntime::receiveFromSocket(iSocket, bIsText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_receiveTextFromSocket(JNIEnv *pEnv, jclass, jint jSocket, jint jLength) {
	jstring result;
	int iSocket;
	iSocket = jSocket;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::receiveTextFromSocket(iSocket, iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_relativePath(JNIEnv *pEnv, jclass, jstring jPath, jstring jReference) {
	jstring result;
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	std::string sReference;
	GET_STRING(Reference);
	sReference = tcReference;
	std::string cppResult = CodeWorker::CGRuntime::relativePath(sPath, sReference);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Reference);
	RELEASE_STRING(Path);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_removeDirectory(JNIEnv *pEnv, jclass, jstring jPath) {
	jboolean result;
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	bool cppResult = CodeWorker::CGRuntime::removeDirectory(sPath);
	result = cppResult;
	RELEASE_STRING(Path);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_removeGenerationTagsHandler(JNIEnv *pEnv, jclass, jstring jKey) {
	jboolean result;
	std::string sKey;
	GET_STRING(Key);
	sKey = tcKey;
	bool cppResult = CodeWorker::CGRuntime::removeGenerationTagsHandler(sKey);
	result = cppResult;
	RELEASE_STRING(Key);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_repeatString(JNIEnv *pEnv, jclass, jstring jText, jint jOccurrences) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iOccurrences;
	iOccurrences = jOccurrences;
	std::string cppResult = CodeWorker::CGRuntime::repeatString(sText, iOccurrences);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_replaceString(JNIEnv *pEnv, jclass, jstring jOld, jstring jNew, jstring jText) {
	jstring result;
	std::string sOld;
	GET_STRING(Old);
	sOld = tcOld;
	std::string sNew;
	GET_STRING(New);
	sNew = tcNew;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::replaceString(sOld, sNew, sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	RELEASE_STRING(New);
	RELEASE_STRING(Old);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_replaceTabulations(JNIEnv *pEnv, jclass, jstring jText, jint jTab) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iTab;
	iTab = jTab;
	std::string cppResult = CodeWorker::CGRuntime::replaceTabulations(sText, iTab);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_resolveFilePath(JNIEnv *pEnv, jclass, jstring jFilename) {
	jstring result;
	std::string sFilename;
	GET_STRING(Filename);
	sFilename = tcFilename;
	std::string cppResult = CodeWorker::CGRuntime::resolveFilePath(sFilename);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Filename);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_rightString(JNIEnv *pEnv, jclass, jstring jText, jint jLength) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::rightString(sText, iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_rsubString(JNIEnv *pEnv, jclass, jstring jText, jint jPos) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iPos;
	iPos = jPos;
	std::string cppResult = CodeWorker::CGRuntime::rsubString(sText, iPos);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_scanDirectories(JNIEnv *pEnv, jclass, jobject jDirectory, jstring jPath, jstring jPattern) {
	jboolean result;
	CodeWorker::DtaScriptVariable* pDirectory;
	GET_PARSETREE_HANDLE(Directory);
	pDirectory = pDirectoryInstance;
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	std::string sPattern;
	GET_STRING(Pattern);
	sPattern = tcPattern;
	bool cppResult = CodeWorker::CGRuntime::scanDirectories(pDirectory, sPath, sPattern);
	result = cppResult;
	RELEASE_STRING(Pattern);
	RELEASE_STRING(Path);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_scanFiles(JNIEnv *pEnv, jclass, jobject jFiles, jstring jPath, jstring jPattern, jboolean jSubfolders) {
	jboolean result;
	CodeWorker::DtaScriptVariable* pFiles;
	GET_PARSETREE_HANDLE(Files);
	pFiles = pFilesInstance;
	std::string sPath;
	GET_STRING(Path);
	sPath = tcPath;
	std::string sPattern;
	GET_STRING(Pattern);
	sPattern = tcPattern;
	bool bSubfolders;
	bSubfolders = (jSubfolders != '\0');
	bool cppResult = CodeWorker::CGRuntime::scanFiles(pFiles, sPath, sPattern, bSubfolders);
	result = cppResult;
	RELEASE_STRING(Pattern);
	RELEASE_STRING(Path);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_sendBinaryToSocket(JNIEnv *pEnv, jclass, jint jSocket, jstring jBytes) {
	jboolean result;
	int iSocket;
	iSocket = jSocket;
	std::string sBytes;
	GET_STRING(Bytes);
	sBytes = tcBytes;
	bool cppResult = CodeWorker::CGRuntime::sendBinaryToSocket(iSocket, sBytes);
	result = cppResult;
	RELEASE_STRING(Bytes);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_sendHTTPRequest(JNIEnv *pEnv, jclass, jstring jURL, jobject jHTTPSession) {
	jstring result;
	std::string sURL;
	GET_STRING(URL);
	sURL = tcURL;
	CodeWorker::DtaScriptVariable* pHTTPSession;
	GET_PARSETREE_HANDLE(HTTPSession);
	pHTTPSession = pHTTPSessionInstance;
	std::string cppResult = CodeWorker::CGRuntime::sendHTTPRequest(sURL, pHTTPSession);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(URL);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_sendTextToSocket(JNIEnv *pEnv, jclass, jint jSocket, jstring jText) {
	jboolean result;
	int iSocket;
	iSocket = jSocket;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	bool cppResult = CodeWorker::CGRuntime::sendTextToSocket(iSocket, sText);
	result = cppResult;
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_selectGenerationTagsHandler(JNIEnv *pEnv, jclass, jstring jKey) {
	jboolean result;
	std::string sKey;
	GET_STRING(Key);
	sKey = tcKey;
	bool cppResult = CodeWorker::CGRuntime::selectGenerationTagsHandler(sKey);
	result = cppResult;
	RELEASE_STRING(Key);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_shortToBytes(JNIEnv *pEnv, jclass, jshort jShort) {
	jstring result;
	unsigned short ulShort;
	ulShort = (unsigned short) jShort;
	std::string cppResult = CodeWorker::CGRuntime::shortToBytes(ulShort);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_sqrt(JNIEnv *pEnv, jclass, jdouble jX) {
	jdouble result;
	double dX;
	dX = jX;
	double cppResult = CodeWorker::CGRuntime::sqrt(dX);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_startString(JNIEnv *pEnv, jclass, jstring jText, jstring jStart) {
	jboolean result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string sStart;
	GET_STRING(Start);
	sStart = tcStart;
	bool cppResult = CodeWorker::CGRuntime::startString(sText, sStart);
	result = cppResult;
	RELEASE_STRING(Start);
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_sub(JNIEnv *pEnv, jclass, jdouble jLeft, jdouble jRight) {
	jdouble result;
	double dLeft;
	dLeft = jLeft;
	double dRight;
	dRight = jRight;
	double cppResult = CodeWorker::CGRuntime::sub(dLeft, dRight);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_subString(JNIEnv *pEnv, jclass, jstring jText, jint jPos) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	int iPos;
	iPos = jPos;
	std::string cppResult = CodeWorker::CGRuntime::subString(sText, iPos);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_sup(JNIEnv *pEnv, jclass, jdouble jLeft, jdouble jRight) {
	jboolean result;
	double dLeft;
	dLeft = jLeft;
	double dRight;
	dRight = jRight;
	bool cppResult = CodeWorker::CGRuntime::sup(dLeft, dRight);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_system(JNIEnv *pEnv, jclass, jstring jCommand) {
	jstring result;
	std::string sCommand;
	GET_STRING(Command);
	sCommand = tcCommand;
	std::string cppResult = CodeWorker::CGRuntime::system(sCommand);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Command);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_toLowerString(JNIEnv *pEnv, jclass, jstring jText) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::toLowerString(sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_toUpperString(JNIEnv *pEnv, jclass, jstring jText) {
	jstring result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::toUpperString(sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_trimLeft(JNIEnv *pEnv, jclass, jobject jString) {
	jint result;
	std::string sString;
	// NOT HANDLED YET!
	int cppResult = CodeWorker::CGRuntime::trimLeft(sString);
	result = cppResult;
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_trimRight(JNIEnv *pEnv, jclass, jobject jString) {
	jint result;
	std::string sString;
	// NOT HANDLED YET!
	int cppResult = CodeWorker::CGRuntime::trimRight(sString);
	result = cppResult;
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_trim(JNIEnv *pEnv, jclass, jobject jString) {
	jint result;
	std::string sString;
	// NOT HANDLED YET!
	int cppResult = CodeWorker::CGRuntime::trim(sString);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_truncateAfterString(JNIEnv *pEnv, jclass, jobject jVariable, jstring jText) {
	jstring result;
	CodeWorker::DtaScriptVariable* pVariable;
	GET_PARSETREE_HANDLE(Variable);
	pVariable = pVariableInstance;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::truncateAfterString(pVariable, sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_truncateBeforeString(JNIEnv *pEnv, jclass, jobject jVariable, jstring jText) {
	jstring result;
	CodeWorker::DtaScriptVariable* pVariable;
	GET_PARSETREE_HANDLE(Variable);
	pVariable = pVariableInstance;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	std::string cppResult = CodeWorker::CGRuntime::truncateBeforeString(pVariable, sText);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_UUID(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::UUID();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countInputCols(JNIEnv *pEnv, jclass) {
	jint result;
	int cppResult = CodeWorker::CGRuntime::countInputCols();
	result = cppResult;
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countInputLines(JNIEnv *pEnv, jclass) {
	jint result;
	int cppResult = CodeWorker::CGRuntime::countInputLines();
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getInputFilename(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getInputFilename();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getLastReadChars(JNIEnv *pEnv, jclass, jint jLength) {
	jstring result;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::getLastReadChars(iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getInputLocation(JNIEnv *pEnv, jclass) {
	jint result;
	int cppResult = CodeWorker::CGRuntime::getInputLocation();
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_lookAhead(JNIEnv *pEnv, jclass, jstring jText) {
	jboolean result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	bool cppResult = CodeWorker::CGRuntime::lookAhead(sText);
	result = cppResult;
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_peekChar(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::peekChar();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readAdaString(JNIEnv *pEnv, jclass, jobject jText) {
	jboolean result;
	std::string sText;
	// NOT HANDLED YET!
	bool cppResult = CodeWorker::CGRuntime::readAdaString(sText);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readByte(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::readByte();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readBytes(JNIEnv *pEnv, jclass, jint jLength) {
	jstring result;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::readBytes(iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readCChar(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::readCChar();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readChar(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::readChar();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_readCharAsInt(JNIEnv *pEnv, jclass) {
	jint result;
	int cppResult = CodeWorker::CGRuntime::readCharAsInt();
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readChars(JNIEnv *pEnv, jclass, jint jLength) {
	jstring result;
	int iLength;
	iLength = jLength;
	std::string cppResult = CodeWorker::CGRuntime::readChars(iLength);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readIdentifier(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::readIdentifier();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readIfEqualTo(JNIEnv *pEnv, jclass, jstring jText) {
	jboolean result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	bool cppResult = CodeWorker::CGRuntime::readIfEqualTo(sText);
	result = cppResult;
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readIfEqualToIgnoreCase(JNIEnv *pEnv, jclass, jstring jText) {
	jboolean result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	bool cppResult = CodeWorker::CGRuntime::readIfEqualToIgnoreCase(sText);
	result = cppResult;
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readIfEqualToIdentifier(JNIEnv *pEnv, jclass, jstring jIdentifier) {
	jboolean result;
	std::string sIdentifier;
	GET_STRING(Identifier);
	sIdentifier = tcIdentifier;
	bool cppResult = CodeWorker::CGRuntime::readIfEqualToIdentifier(sIdentifier);
	result = cppResult;
	RELEASE_STRING(Identifier);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readLine(JNIEnv *pEnv, jclass, jobject jText) {
	jboolean result;
	std::string sText;
	// NOT HANDLED YET!
	bool cppResult = CodeWorker::CGRuntime::readLine(sText);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readNextText(JNIEnv *pEnv, jclass, jstring jText) {
	jboolean result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	bool cppResult = CodeWorker::CGRuntime::readNextText(sText);
	result = cppResult;
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readNumber(JNIEnv *pEnv, jclass, jobject jNumber) {
	jboolean result;
	double dNumber;
	// NOT HANDLED YET!
	bool cppResult = CodeWorker::CGRuntime::readNumber(dNumber);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readPythonString(JNIEnv *pEnv, jclass, jobject jText) {
	jboolean result;
	std::string sText;
	// NOT HANDLED YET!
	bool cppResult = CodeWorker::CGRuntime::readPythonString(sText);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readString(JNIEnv *pEnv, jclass, jobject jText) {
	jboolean result;
	std::string sText;
	// NOT HANDLED YET!
	bool cppResult = CodeWorker::CGRuntime::readString(sText);
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readUptoJustOneChar(JNIEnv *pEnv, jclass, jstring jOneAmongChars) {
	jstring result;
	std::string sOneAmongChars;
	GET_STRING(OneAmongChars);
	sOneAmongChars = tcOneAmongChars;
	std::string cppResult = CodeWorker::CGRuntime::readUptoJustOneChar(sOneAmongChars);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(OneAmongChars);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readWord(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::readWord();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipBlanks(JNIEnv *pEnv, jclass) {
	jboolean result;
	bool cppResult = CodeWorker::CGRuntime::skipBlanks();
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipSpaces(JNIEnv *pEnv, jclass) {
	jboolean result;
	bool cppResult = CodeWorker::CGRuntime::skipSpaces();
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipEmptyCpp(JNIEnv *pEnv, jclass) {
	jboolean result;
	bool cppResult = CodeWorker::CGRuntime::skipEmptyCpp();
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipEmptyCppExceptDoxygen(JNIEnv *pEnv, jclass) {
	jboolean result;
	bool cppResult = CodeWorker::CGRuntime::skipEmptyCppExceptDoxygen();
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipEmptyHTML(JNIEnv *pEnv, jclass) {
	jboolean result;
	bool cppResult = CodeWorker::CGRuntime::skipEmptyHTML();
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipEmptyLaTeX(JNIEnv *pEnv, jclass) {
	jboolean result;
	bool cppResult = CodeWorker::CGRuntime::skipEmptyLaTeX();
	result = cppResult;
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countOutputCols(JNIEnv *pEnv, jclass) {
	jint result;
	int cppResult = CodeWorker::CGRuntime::countOutputCols();
	result = cppResult;
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countOutputLines(JNIEnv *pEnv, jclass) {
	jint result;
	int cppResult = CodeWorker::CGRuntime::countOutputLines();
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_decrementIndentLevel(JNIEnv *pEnv, jclass, jint jLevel) {
	jboolean result;
	int iLevel;
	iLevel = jLevel;
	bool cppResult = CodeWorker::CGRuntime::decrementIndentLevel(iLevel);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_equalLastWrittenChars(JNIEnv *pEnv, jclass, jstring jText) {
	jboolean result;
	std::string sText;
	GET_STRING(Text);
	sText = tcText;
	bool cppResult = CodeWorker::CGRuntime::equalLastWrittenChars(sText);
	result = cppResult;
	RELEASE_STRING(Text);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existFloatingLocation(JNIEnv *pEnv, jclass, jstring jKey, jboolean jParent) {
	jboolean result;
	std::string sKey;
	GET_STRING(Key);
	sKey = tcKey;
	bool bParent;
	bParent = (jParent != '\0');
	bool cppResult = CodeWorker::CGRuntime::existFloatingLocation(sKey, bParent);
	result = cppResult;
	RELEASE_STRING(Key);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getFloatingLocation(JNIEnv *pEnv, jclass, jstring jKey) {
	jint result;
	std::string sKey;
	GET_STRING(Key);
	sKey = tcKey;
	int cppResult = CodeWorker::CGRuntime::getFloatingLocation(sKey);
	result = cppResult;
	RELEASE_STRING(Key);
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getLastWrittenChars(JNIEnv *pEnv, jclass, jint jNbChars) {
	jstring result;
	int iNbChars;
	iNbChars = jNbChars;
	std::string cppResult = CodeWorker::CGRuntime::getLastWrittenChars(iNbChars);
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getMarkupKey(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getMarkupKey();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getMarkupValue(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getMarkupValue();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getOutputFilename(JNIEnv *pEnv, jclass) {
	jstring result;
	std::string cppResult = CodeWorker::CGRuntime::getOutputFilename();
	result = pEnv->NewStringUTF(cppResult.c_str());
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getOutputLocation(JNIEnv *pEnv, jclass) {
	jint result;
	int cppResult = CodeWorker::CGRuntime::getOutputLocation();
	result = cppResult;
	return result;
}

JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getProtectedArea(JNIEnv *pEnv, jclass, jstring jProtection) {
	jstring result;
	std::string sProtection;
	GET_STRING(Protection);
	sProtection = tcProtection;
	std::string cppResult = CodeWorker::CGRuntime::getProtectedArea(sProtection);
	result = pEnv->NewStringUTF(cppResult.c_str());
	RELEASE_STRING(Protection);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getProtectedAreaKeys(JNIEnv *pEnv, jclass, jobject jKeys) {
	jint result;
	CodeWorker::DtaScriptVariable* pKeys;
	GET_PARSETREE_HANDLE(Keys);
	pKeys = pKeysInstance;
	int cppResult = CodeWorker::CGRuntime::getProtectedAreaKeys(pKeys);
	result = cppResult;
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_indentText(JNIEnv *pEnv, jclass, jstring jMode) {
	jboolean result;
	std::string sMode;
	GET_STRING(Mode);
	sMode = tcMode;
	bool cppResult = CodeWorker::CGRuntime::indentText(sMode);
	result = cppResult;
	RELEASE_STRING(Mode);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_newFloatingLocation(JNIEnv *pEnv, jclass, jstring jKey) {
	jboolean result;
	std::string sKey;
	GET_STRING(Key);
	sKey = tcKey;
	bool cppResult = CodeWorker::CGRuntime::newFloatingLocation(sKey);
	result = cppResult;
	RELEASE_STRING(Key);
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_remainingProtectedAreas(JNIEnv *pEnv, jclass, jobject jKeys) {
	jint result;
	CodeWorker::DtaScriptVariable* pKeys;
	GET_PARSETREE_HANDLE(Keys);
	pKeys = pKeysInstance;
	int cppResult = CodeWorker::CGRuntime::remainingProtectedAreas(pKeys);
	result = cppResult;
	return result;
}

JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_removeFloatingLocation(JNIEnv *pEnv, jclass, jstring jKey) {
	jint result;
	std::string sKey;
	GET_STRING(Key);
	sKey = tcKey;
	int cppResult = CodeWorker::CGRuntime::removeFloatingLocation(sKey);
	result = cppResult;
	RELEASE_STRING(Key);
	return result;
}

JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_removeProtectedArea(JNIEnv *pEnv, jclass, jstring jProtectedAreaName) {
	jboolean result;
	std::string sProtectedAreaName;
	GET_STRING(ProtectedAreaName);
	sProtectedAreaName = tcProtectedAreaName;
	bool cppResult = CodeWorker::CGRuntime::removeProtectedArea(sProtectedAreaName);
	result = cppResult;
	RELEASE_STRING(ProtectedAreaName);
	return result;
}

//##end##"functions"
