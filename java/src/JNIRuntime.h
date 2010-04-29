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

#ifndef _JNIRuntime_h_
#define _JNIRuntime_h_

#include <jni.h>

extern "C" {
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_registerScript(JNIEnv *, jclass, jstring sKey, jobject pScript);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_entryPoint(JNIEnv *, jclass, jobjectArray tsArgs, jobject pScript);
	JNIEXPORT jobject JNICALL Java_org_codeworker_jni_Runtime_getThisTree(JNIEnv *, jclass);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_throwBNFExecutionError(JNIEnv *, jclass, jstring sMessage);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_parseAsBNF(JNIEnv *, jclass, jobject pScript, jobject pContext, jstring sInputFile);
//##markup##"functions"
//##begin##"functions"
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_appendFile(JNIEnv *, jclass, jstring sFilename, jstring sContent);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_clearVariable(JNIEnv *, jclass, jobject pNode);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_compileToCpp(JNIEnv *, jclass, jstring sScriptFileName, jstring sProjectDirectory, jstring sCodeWorkerDirectory);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_copyFile(JNIEnv *, jclass, jstring sSourceFileName, jstring sDestinationFileName);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_copyGenerableFile(JNIEnv *, jclass, jstring sSourceFileName, jstring sDestinationFileName);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_copySmartDirectory(JNIEnv *, jclass, jstring sSourceDirectory, jstring sDestinationPath);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_cutString(JNIEnv *, jclass, jstring sText, jstring sSeparator, jobject slList);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_environTable(JNIEnv *, jclass, jobject pTable);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_extendExecutedScript(JNIEnv *, jclass, jstring sScriptContent);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertElementAt(JNIEnv *, jclass, jobject pList, jstring sKey, jint iPosition);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_invertArray(JNIEnv *, jclass, jobject pArray);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_listAllGeneratedFiles(JNIEnv *, jclass, jobject pFiles);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_loadProject(JNIEnv *, jclass, jstring sXMLorTXTFileName, jobject pNodeToLoad);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_openLogFile(JNIEnv *, jclass, jstring sFilename);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_produceHTML(JNIEnv *, jclass, jstring sScriptFileName, jstring sHTMLFileName);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_putEnv(JNIEnv *, jclass, jstring sName, jstring sValue);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_randomSeed(JNIEnv *, jclass, jint iSeed);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeAllElements(JNIEnv *, jclass, jobject pVariable);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeElement(JNIEnv *, jclass, jobject pVariable, jstring sKey);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeFirstElement(JNIEnv *, jclass, jobject pList);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeLastElement(JNIEnv *, jclass, jobject pList);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeRecursive(JNIEnv *, jclass, jobject pVariable, jstring sAttribute);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_removeVariable(JNIEnv *, jclass, jobject pNode);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_saveBinaryToFile(JNIEnv *, jclass, jstring sFilename, jstring sContent);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_saveProject(JNIEnv *, jclass, jstring sXMLorTXTFileName, jobject pNodeToSave);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_saveProjectTypes(JNIEnv *, jclass, jstring sXMLFileName);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_saveToFile(JNIEnv *, jclass, jstring sFilename, jstring sContent);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setCommentBegin(JNIEnv *, jclass, jstring sCommentBegin);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setCommentEnd(JNIEnv *, jclass, jstring sCommentEnd);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setGenerationHeader(JNIEnv *, jclass, jstring sComment);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setIncludePath(JNIEnv *, jclass, jstring sPath);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setNow(JNIEnv *, jclass, jstring sConstantDateTime);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setProperty(JNIEnv *, jclass, jstring sDefine, jstring sValue);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setTextMode(JNIEnv *, jclass, jstring sTextMode);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setVersion(JNIEnv *, jclass, jstring sVersion);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setWriteMode(JNIEnv *, jclass, jstring sMode);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setWorkingPath(JNIEnv *, jclass, jstring sPath);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_sleep(JNIEnv *, jclass, jint iMillis);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_slideNodeContent(JNIEnv *, jclass, jobject pOrgNode, jstring xDestNode);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_sortArray(JNIEnv *, jclass, jobject pArray);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_traceEngine(JNIEnv *, jclass);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_traceLine(JNIEnv *, jclass, jstring sLine);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_traceObject(JNIEnv *, jclass, jobject pObject, jint iDepth);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_traceText(JNIEnv *, jclass, jstring sText);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_attachInputToSocket(JNIEnv *, jclass, jint iSocket);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_detachInputFromSocket(JNIEnv *, jclass, jint iSocket);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_goBack(JNIEnv *, jclass);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setInputLocation(JNIEnv *, jclass, jint iLocation);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_allFloatingLocations(JNIEnv *, jclass, jobject pList);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_attachOutputToSocket(JNIEnv *, jclass, jint iSocket);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_detachOutputFromSocket(JNIEnv *, jclass, jint iSocket);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_incrementIndentLevel(JNIEnv *, jclass, jint iLevel);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertText(JNIEnv *, jclass, jint iLocation, jstring sText);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertTextOnce(JNIEnv *, jclass, jint iLocation, jstring sText);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertTextToFloatingLocation(JNIEnv *, jclass, jstring sLocation, jstring sText);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_insertTextOnceToFloatingLocation(JNIEnv *, jclass, jstring sLocation, jstring sText);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_overwritePortion(JNIEnv *, jclass, jint iLocation, jstring sText, jint iSize);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_populateProtectedArea(JNIEnv *, jclass, jstring sProtectedAreaName, jstring sContent);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_resizeOutputStream(JNIEnv *, jclass, jint iNewSize);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setFloatingLocation(JNIEnv *, jclass, jstring sKey, jint iLocation);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setOutputLocation(JNIEnv *, jclass, jint iLocation);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_setProtectedArea(JNIEnv *, jclass, jstring sProtectedAreaName);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_writeBytes(JNIEnv *, jclass, jstring sBytes);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_writeText(JNIEnv *, jclass, jstring sText);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_writeTextOnce(JNIEnv *, jclass, jstring sText);
	JNIEXPORT void JNICALL Java_org_codeworker_jni_Runtime_closeSocket(JNIEnv *, jclass, jint iSocket);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_flushOutputToSocket(JNIEnv *, jclass, jint iSocket);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_acceptSocket(JNIEnv *, jclass, jint iServerSocket);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_add(JNIEnv *, jclass, jdouble dLeft, jdouble dRight);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_addToDate(JNIEnv *, jclass, jstring sDate, jstring sFormat, jstring sShifting);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_byteToChar(JNIEnv *, jclass, jstring sByte);
	JNIEXPORT jlong JNICALL Java_org_codeworker_jni_Runtime_bytesToLong(JNIEnv *, jclass, jstring sBytes);
	JNIEXPORT jshort JNICALL Java_org_codeworker_jni_Runtime_bytesToShort(JNIEnv *, jclass, jstring sBytes);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_canonizePath(JNIEnv *, jclass, jstring sPath);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_changeDirectory(JNIEnv *, jclass, jstring sPath);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_changeFileTime(JNIEnv *, jclass, jstring sFilename, jstring sAccessTime, jstring sModificationTime);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_charAt(JNIEnv *, jclass, jstring sText, jint iIndex);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_charToByte(JNIEnv *, jclass, jstring sChar);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_charToInt(JNIEnv *, jclass, jstring sChar);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_chmod(JNIEnv *, jclass, jstring sFilename, jstring sMode);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_ceil(JNIEnv *, jclass, jdouble dNumber);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_compareDate(JNIEnv *, jclass, jstring sDate1, jstring sDate2);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_completeDate(JNIEnv *, jclass, jstring sDate, jstring sFormat);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_completeLeftSpaces(JNIEnv *, jclass, jstring sText, jint iLength);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_completeRightSpaces(JNIEnv *, jclass, jstring sText, jint iLength);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_composeAdaLikeString(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_composeCLikeString(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_composeHTMLLikeString(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_composeSQLLikeString(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_computeMD5(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_copySmartFile(JNIEnv *, jclass, jstring sSourceFileName, jstring sDestinationFileName);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_coreString(JNIEnv *, jclass, jstring sText, jint iPos, jint iLastRemoved);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countStringOccurences(JNIEnv *, jclass, jstring sString, jstring sText);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_createDirectory(JNIEnv *, jclass, jstring sPath);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_createINETClientSocket(JNIEnv *, jclass, jstring sRemoteAddress, jint iPort);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_createINETServerSocket(JNIEnv *, jclass, jint iPort, jint iBackLog);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_createIterator(JNIEnv *, jclass, jobject pI, jobject pList);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_createReverseIterator(JNIEnv *, jclass, jobject pI, jobject pList);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_createVirtualFile(JNIEnv *, jclass, jstring sHandle, jstring sContent);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_createVirtualTemporaryFile(JNIEnv *, jclass, jstring sContent);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_decodeURL(JNIEnv *, jclass, jstring sURL);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_decrement(JNIEnv *, jclass, jobject dNumber);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_deleteFile(JNIEnv *, jclass, jstring sFilename);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_deleteVirtualFile(JNIEnv *, jclass, jstring sHandle);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_div(JNIEnv *, jclass, jdouble dDividend, jdouble dDivisor);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_duplicateIterator(JNIEnv *, jclass, jobject pOldIt, jobject pNewIt);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_encodeURL(JNIEnv *, jclass, jstring sURL);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_endl(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_endString(JNIEnv *, jclass, jstring sText, jstring sEnd);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_equal(JNIEnv *, jclass, jdouble dLeft, jdouble dRight);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_equalsIgnoreCase(JNIEnv *, jclass, jstring sLeft, jstring sRight);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_equalTrees(JNIEnv *, jclass, jobject pFirstTree, jobject pSecondTree);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_executeStringQuiet(JNIEnv *, jclass, jobject pThis, jstring sCommand);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existDirectory(JNIEnv *, jclass, jstring sPath);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existEnv(JNIEnv *, jclass, jstring sVariable);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existFile(JNIEnv *, jclass, jstring sFileName);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existVirtualFile(JNIEnv *, jclass, jstring sHandle);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existVariable(JNIEnv *, jclass, jobject pVariable);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_exp(JNIEnv *, jclass, jdouble dX);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_exploreDirectory(JNIEnv *, jclass, jobject pDirectory, jstring sPath, jboolean bSubfolders);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_extractGenerationHeader(JNIEnv *, jclass, jstring sFilename, jobject sGenerator, jobject sVersion, jobject sDate);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_fileCreation(JNIEnv *, jclass, jstring sFilename);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_fileLastAccess(JNIEnv *, jclass, jstring sFilename);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_fileLastModification(JNIEnv *, jclass, jstring sFilename);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_fileLines(JNIEnv *, jclass, jstring sFilename);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_fileMode(JNIEnv *, jclass, jstring sFilename);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_fileSize(JNIEnv *, jclass, jstring sFilename);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_findElement(JNIEnv *, jclass, jstring sValue, jobject pVariable);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findFirstChar(JNIEnv *, jclass, jstring sText, jstring sSomeChars);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findFirstSubstringIntoKeys(JNIEnv *, jclass, jstring sSubstring, jobject pArray);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findLastString(JNIEnv *, jclass, jstring sText, jstring sFind);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findNextString(JNIEnv *, jclass, jstring sText, jstring sFind, jint iPosition);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findNextSubstringIntoKeys(JNIEnv *, jclass, jstring sSubstring, jobject pArray, jint iNext);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_findString(JNIEnv *, jclass, jstring sText, jstring sFind);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_floor(JNIEnv *, jclass, jdouble dNumber);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_formatDate(JNIEnv *, jclass, jstring sDate, jstring sFormat);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getArraySize(JNIEnv *, jclass, jobject pVariable);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getCommentBegin(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getCommentEnd(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getCurrentDirectory(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getEnv(JNIEnv *, jclass, jstring sVariable);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getGenerationHeader(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getHTTPRequest(JNIEnv *, jclass, jstring sURL, jobject pHTTPSession, jobject pArguments);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getIncludePath(JNIEnv *, jclass);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_getLastDelay(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getNow(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getProperty(JNIEnv *, jclass, jstring sDefine);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getShortFilename(JNIEnv *, jclass, jstring sPathFilename);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getTextMode(JNIEnv *, jclass);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getVariableAttributes(JNIEnv *, jclass, jobject pVariable, jobject pList);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getVersion(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getWorkingPath(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getWriteMode(JNIEnv *, jclass);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_hexaToDecimal(JNIEnv *, jclass, jstring sHexaNumber);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_hostToNetworkLong(JNIEnv *, jclass, jstring sBytes);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_hostToNetworkShort(JNIEnv *, jclass, jstring sBytes);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_increment(JNIEnv *, jclass, jobject dNumber);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_indentFile(JNIEnv *, jclass, jstring sFile, jstring sMode);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_inf(JNIEnv *, jclass, jdouble dLeft, jdouble dRight);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_inputKey(JNIEnv *, jclass, jboolean bEcho);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_inputLine(JNIEnv *, jclass, jboolean bEcho, jstring sPrompt);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isEmpty(JNIEnv *, jclass, jobject pArray);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isIdentifier(JNIEnv *, jclass, jstring sIdentifier);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isNegative(JNIEnv *, jclass, jdouble dNumber);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isNumeric(JNIEnv *, jclass, jstring sNumber);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_isPositive(JNIEnv *, jclass, jdouble dNumber);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_joinStrings(JNIEnv *, jclass, jobject pList, jstring sSeparator);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_leftString(JNIEnv *, jclass, jstring sText, jint iLength);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_lengthString(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_loadBinaryFile(JNIEnv *, jclass, jstring sFile, jint iLength);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_loadFile(JNIEnv *, jclass, jstring sFile, jint iLength);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_loadVirtualFile(JNIEnv *, jclass, jstring sHandle);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_log(JNIEnv *, jclass, jdouble dX);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_longToBytes(JNIEnv *, jclass, jlong ulLong);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_midString(JNIEnv *, jclass, jstring sText, jint iPos, jint iLength);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_mod(JNIEnv *, jclass, jint iDividend, jint iDivisor);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_mult(JNIEnv *, jclass, jdouble dLeft, jdouble dRight);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_networkLongToHost(JNIEnv *, jclass, jstring sBytes);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_networkShortToHost(JNIEnv *, jclass, jstring sBytes);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_octalToDecimal(JNIEnv *, jclass, jstring sOctalNumber);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_pathFromPackage(JNIEnv *, jclass, jstring sPackage);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_postHTTPRequest(JNIEnv *, jclass, jstring sURL, jobject pHTTPSession, jobject pArguments);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_pow(JNIEnv *, jclass, jdouble dX, jdouble dY);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_randomInteger(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_receiveBinaryFromSocket(JNIEnv *, jclass, jint iSocket, jint iLength);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_receiveFromSocket(JNIEnv *, jclass, jint iSocket, jobject bIsText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_receiveTextFromSocket(JNIEnv *, jclass, jint iSocket, jint iLength);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_relativePath(JNIEnv *, jclass, jstring sPath, jstring sReference);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_removeDirectory(JNIEnv *, jclass, jstring sPath);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_removeGenerationTagsHandler(JNIEnv *, jclass, jstring sKey);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_repeatString(JNIEnv *, jclass, jstring sText, jint iOccurrences);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_replaceString(JNIEnv *, jclass, jstring sOld, jstring sNew, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_replaceTabulations(JNIEnv *, jclass, jstring sText, jint iTab);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_resolveFilePath(JNIEnv *, jclass, jstring sFilename);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_rightString(JNIEnv *, jclass, jstring sText, jint iLength);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_rsubString(JNIEnv *, jclass, jstring sText, jint iPos);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_scanDirectories(JNIEnv *, jclass, jobject pDirectory, jstring sPath, jstring sPattern);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_scanFiles(JNIEnv *, jclass, jobject pFiles, jstring sPath, jstring sPattern, jboolean bSubfolders);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_sendBinaryToSocket(JNIEnv *, jclass, jint iSocket, jstring sBytes);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_sendHTTPRequest(JNIEnv *, jclass, jstring sURL, jobject pHTTPSession);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_sendTextToSocket(JNIEnv *, jclass, jint iSocket, jstring sText);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_selectGenerationTagsHandler(JNIEnv *, jclass, jstring sKey);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_shortToBytes(JNIEnv *, jclass, jshort ulShort);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_sqrt(JNIEnv *, jclass, jdouble dX);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_startString(JNIEnv *, jclass, jstring sText, jstring sStart);
	JNIEXPORT jdouble JNICALL Java_org_codeworker_jni_Runtime_sub(JNIEnv *, jclass, jdouble dLeft, jdouble dRight);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_subString(JNIEnv *, jclass, jstring sText, jint iPos);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_sup(JNIEnv *, jclass, jdouble dLeft, jdouble dRight);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_system(JNIEnv *, jclass, jstring sCommand);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_toLowerString(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_toUpperString(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_trimLeft(JNIEnv *, jclass, jobject sString);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_trimRight(JNIEnv *, jclass, jobject sString);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_trim(JNIEnv *, jclass, jobject sString);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_truncateAfterString(JNIEnv *, jclass, jobject pVariable, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_truncateBeforeString(JNIEnv *, jclass, jobject pVariable, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_UUID(JNIEnv *, jclass);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countInputCols(JNIEnv *, jclass);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countInputLines(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getInputFilename(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getLastReadChars(JNIEnv *, jclass, jint iLength);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getInputLocation(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_lookAhead(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_peekChar(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readAdaString(JNIEnv *, jclass, jobject sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readByte(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readBytes(JNIEnv *, jclass, jint iLength);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readCChar(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readChar(JNIEnv *, jclass);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_readCharAsInt(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readChars(JNIEnv *, jclass, jint iLength);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readIdentifier(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readIfEqualTo(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readIfEqualToIgnoreCase(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readIfEqualToIdentifier(JNIEnv *, jclass, jstring sIdentifier);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readLine(JNIEnv *, jclass, jobject sText);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readNextText(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readNumber(JNIEnv *, jclass, jobject dNumber);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readPythonString(JNIEnv *, jclass, jobject sText);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_readString(JNIEnv *, jclass, jobject sText);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readUptoJustOneChar(JNIEnv *, jclass, jstring sOneAmongChars);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_readWord(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipBlanks(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipSpaces(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipEmptyCpp(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipEmptyCppExceptDoxygen(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipEmptyHTML(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_skipEmptyLaTeX(JNIEnv *, jclass);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countOutputCols(JNIEnv *, jclass);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_countOutputLines(JNIEnv *, jclass);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_decrementIndentLevel(JNIEnv *, jclass, jint iLevel);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_equalLastWrittenChars(JNIEnv *, jclass, jstring sText);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_existFloatingLocation(JNIEnv *, jclass, jstring sKey, jboolean bParent);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getFloatingLocation(JNIEnv *, jclass, jstring sKey);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getLastWrittenChars(JNIEnv *, jclass, jint iNbChars);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getMarkupKey(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getMarkupValue(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getOutputFilename(JNIEnv *, jclass);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getOutputLocation(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_codeworker_jni_Runtime_getProtectedArea(JNIEnv *, jclass, jstring sProtection);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_getProtectedAreaKeys(JNIEnv *, jclass, jobject pKeys);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_indentText(JNIEnv *, jclass, jstring sMode);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_newFloatingLocation(JNIEnv *, jclass, jstring sKey);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_remainingProtectedAreas(JNIEnv *, jclass, jobject pKeys);
	JNIEXPORT jint JNICALL Java_org_codeworker_jni_Runtime_removeFloatingLocation(JNIEnv *, jclass, jstring sKey);
	JNIEXPORT jboolean JNICALL Java_org_codeworker_jni_Runtime_removeProtectedArea(JNIEnv *, jclass, jstring sProtectedAreaName);
//##end##"functions"
}
#endif
