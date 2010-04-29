# Microsoft Developer Studio Project File - Name="CodeWorker" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CodeWorker - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CodeWorker.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CodeWorker.mak" CFG="CodeWorker - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CodeWorker - Win32-STLport Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CodeWorker - Win32-STLport Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "CodeWorker - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CodeWorker - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CodeWorker - Win32-STLport Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseSTLport"
# PROP BASE Intermediate_Dir "ReleaseSTLport"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseSTLport"
# PROP Intermediate_Dir "ReleaseSTLport"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /GR /GX /O2 /I "bin/STLPort-4.5.3/stlport" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "CodeWorker - Win32-STLport Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugSTLport"
# PROP BASE Intermediate_Dir "DebugSTLport"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugSTLport"
# PROP Intermediate_Dir "DebugSTLport"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MDd /W3 /Gm /GR /GX /Zi /Od /I "bin/STLPort-4.5.3/stlport" /D "WIN32" /D "_MBCS" /D "_LIB" /D "LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"DebugSTLport\CodeWorkerD.lib"

!ELSEIF  "$(CFG)" == "CodeWorker - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /GR /GX /O2 /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "CodeWorker - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MDd /W3 /Gm /GR /GX /Zi /Od /D "WIN32" /D "_MBCS" /D "_LIB" /D "LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\CodeWorkerD.lib"

!ENDIF 

# Begin Target

# Name "CodeWorker - Win32-STLport Release"
# Name "CodeWorker - Win32-STLport Debug"
# Name "CodeWorker - Win32 Release"
# Name "CodeWorker - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BNFAndOrJunction.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFBreak.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFCharBoundaries.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFCharLitteral.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFCheck.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFClause.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFClauseCall.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFComplementary.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFConjunction.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFDisjunction.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFEndOfFile.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFFindToken.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFIgnore.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFInsert.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFMoveAhead.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFMultiplicity.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFNextStep.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFNot.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFPushItem.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFRatchet.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadAdaString.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadByte.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadBytes.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadCChar.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadChar.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadChars.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadCompleteIdentifier.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadCString.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadIdentifier.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadInteger.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadNumeric.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadText.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadToken.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFReadUptoIgnore.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFScanWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFSkipIgnore.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFStepintoHook.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFStepoutHook.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFStepper.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFString.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFTransformationMode.cpp
# End Source File
# Begin Source File

SOURCE=.\BNFTryCatch.cpp
# End Source File
# Begin Source File

SOURCE=.\CGCompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\CGExternalHandling.cpp
# End Source File
# Begin Source File

SOURCE=.\CGRuntime.cpp
# End Source File
# Begin Source File

SOURCE=.\CppCompilerEnvironment.cpp
# End Source File
# Begin Source File

SOURCE=.\CppParsingTree.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaArrayIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaAttributeType.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaBNFScript.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaClass.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaCommandScript.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaConsoleScript.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaDesignScript.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaPatternScript.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaProject.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaProtectedAreasBag.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaScript.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaScriptFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaScriptVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaSharpTagsHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\DtaTranslateScript.cpp
# End Source File
# Begin Source File

SOURCE=.\DynPackage.cpp
# End Source File
# Begin Source File

SOURCE=.\ExprScriptBinaryExpression.cpp
# End Source File
# Begin Source File

SOURCE=.\ExprScriptExpression.cpp
# End Source File
# Begin Source File

SOURCE=.\ExprScriptFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\ExprScriptMotif.cpp
# End Source File
# Begin Source File

SOURCE=.\ExprScriptVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\ExternalValueNode.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfAlienInstruction.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfAllFloatingLocations.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfAppendedFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfAppendFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfAttachInputToSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfAttachOutputToSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfAutoexpand.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfBreak.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfClearVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfCloseSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfCompileToCpp.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfContinue.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfCopyFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfCopyGenerableFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfCopySmartDirectory.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfCutString.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfDebugExecution.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfDelay.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfDetachInputFromSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfDetachOutputFromSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfDo.cpp

!IF  "$(CFG)" == "CodeWorker - Win32-STLport Release"

!ELSEIF  "$(CFG)" == "CodeWorker - Win32-STLport Debug"

!ELSEIF  "$(CFG)" == "CodeWorker - Win32 Release"

!ELSEIF  "$(CFG)" == "CodeWorker - Win32 Debug"

# ADD CPP /MD

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GrfEnvironTable.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfError.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfExecuteString.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfExecutionContext.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfExit.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfExpand.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfFileAsStandardInput.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfForeach.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfGenerate.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfGeneratedFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfGeneratedString.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfGenerateString.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfGlobalVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfGoBack.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfIfThenElse.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfIncrementIndentLevel.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfInsertAssignment.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfInsertElementAt.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfInsertText.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfInsertTextOnce.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfInsertTextOnceToFloatingLocation.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfInsertTextToFloatingLocation.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfInvertArray.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfListAllGeneratedFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfLocalReference.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfLocalVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfMerge.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfNewProject.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfNop.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfOpenLogFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfOverwritePortion.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfParseAsBNF.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfParsedFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfParsedString.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfParseFree.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfParseStringAsBNF.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfPopulateProtectedArea.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfProduceHTML.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfPushItem.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfPutEnv.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfQuantifyExecution.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfQuiet.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfRandomSeed.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfReadonlyHook.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfReference.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveAllElements.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveElement.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveFirstElement.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveLastElement.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveRecursive.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfResizeOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfReturn.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSaveBinaryToFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSaveProject.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSaveProjectTypes.cpp

!IF  "$(CFG)" == "CodeWorker - Win32-STLport Release"

!ELSEIF  "$(CFG)" == "CodeWorker - Win32-STLport Debug"

!ELSEIF  "$(CFG)" == "CodeWorker - Win32 Release"

!ELSEIF  "$(CFG)" == "CodeWorker - Win32 Debug"

# ADD CPP /MD

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GrfSaveToFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetAll.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetAssignment.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetCommentBegin.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetCommentEnd.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetFloatingLocation.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetGenerationHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetIncludePath.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetInputLocation.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetNow.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetOutputLocation.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetProperty.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetProtectedArea.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetTextMode.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetVersion.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetWorkingPath.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSetWriteMode.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSleep.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSlideNodeContent.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSortArray.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfStringAsStandardInput.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfSwitch.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfText.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfTraceEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfTraceLine.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfTraceObject.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfTraceStack.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfTraceText.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfTranslate.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfTryCatch.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfWhile.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfWriteBytes.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfWritefileHook.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfWriteText.cpp
# End Source File
# Begin Source File

SOURCE=.\GrfWriteTextOnce.cpp
# End Source File
# Begin Source File

SOURCE=.\HTTPRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\JNIExternalHandling.cpp
# End Source File
# Begin Source File

SOURCE=.\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\NetSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\ScpStream.cpp
# End Source File
# Begin Source File

SOURCE=.\UtlDate.cpp
# End Source File
# Begin Source File

SOURCE=.\UtlDirectory.cpp
# End Source File
# Begin Source File

SOURCE=.\UtlException.cpp
# End Source File
# Begin Source File

SOURCE=.\UtlString.cpp
# End Source File
# Begin Source File

SOURCE=.\UtlTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\UtlTrace.cpp
# End Source File
# Begin Source File

SOURCE=.\UtlXMLStream.cpp
# End Source File
# Begin Source File

SOURCE=.\Workspace.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BNFAndOrJunction.h
# End Source File
# Begin Source File

SOURCE=.\BNFBreak.h
# End Source File
# Begin Source File

SOURCE=.\BNFCharBoundaries.h
# End Source File
# Begin Source File

SOURCE=.\BNFCharLitteral.h
# End Source File
# Begin Source File

SOURCE=.\BNFCheck.h
# End Source File
# Begin Source File

SOURCE=.\BNFClause.h
# End Source File
# Begin Source File

SOURCE=.\BNFClauseCall.h
# End Source File
# Begin Source File

SOURCE=.\BNFComplementary.h
# End Source File
# Begin Source File

SOURCE=.\BNFConjunction.h
# End Source File
# Begin Source File

SOURCE=.\BNFDisjunction.h
# End Source File
# Begin Source File

SOURCE=.\BNFEndOfFile.h
# End Source File
# Begin Source File

SOURCE=.\BNFFindToken.h
# End Source File
# Begin Source File

SOURCE=.\BNFIgnore.h
# End Source File
# Begin Source File

SOURCE=.\BNFInsert.h
# End Source File
# Begin Source File

SOURCE=.\BNFMoveAhead.h
# End Source File
# Begin Source File

SOURCE=.\BNFMultiplicity.h
# End Source File
# Begin Source File

SOURCE=.\BNFNextStep.h
# End Source File
# Begin Source File

SOURCE=.\BNFNot.h
# End Source File
# Begin Source File

SOURCE=.\BNFPushItem.h
# End Source File
# Begin Source File

SOURCE=.\BNFRatchet.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadAdaString.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadBinaryLong.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadBinaryShort.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadByte.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadBytes.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadCChar.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadChar.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadChars.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadCompleteIdentifier.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadCString.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadIdentifier.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadInteger.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadNetworkLong.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadNetworkShort.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadNumeric.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadText.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadToken.h
# End Source File
# Begin Source File

SOURCE=.\BNFReadUptoIgnore.h
# End Source File
# Begin Source File

SOURCE=.\BNFScanWindow.h
# End Source File
# Begin Source File

SOURCE=.\BNFSkipIgnore.h
# End Source File
# Begin Source File

SOURCE=.\BNFStepintoHook.h
# End Source File
# Begin Source File

SOURCE=.\BNFStepoutHook.h
# End Source File
# Begin Source File

SOURCE=.\BNFStepper.h
# End Source File
# Begin Source File

SOURCE=.\BNFString.h
# End Source File
# Begin Source File

SOURCE=.\BNFTransformationMode.h
# End Source File
# Begin Source File

SOURCE=.\BNFTryCatch.h
# End Source File
# Begin Source File

SOURCE=.\CGCompiler.h
# End Source File
# Begin Source File

SOURCE=.\CGExternalHandling.h
# End Source File
# Begin Source File

SOURCE=.\CGRuntime.h
# End Source File
# Begin Source File

SOURCE=.\CppCompilerEnvironment.h
# End Source File
# Begin Source File

SOURCE=.\CppParsingTree.h
# End Source File
# Begin Source File

SOURCE=.\CW4dl.h
# End Source File
# Begin Source File

SOURCE=.\DtaArrayIterator.h
# End Source File
# Begin Source File

SOURCE=.\DtaAttributeType.h
# End Source File
# Begin Source File

SOURCE=.\DtaBNFScript.h
# End Source File
# Begin Source File

SOURCE=.\DtaClass.h
# End Source File
# Begin Source File

SOURCE=.\DtaCommandScript.h
# End Source File
# Begin Source File

SOURCE=.\DtaConsoleScript.h
# End Source File
# Begin Source File

SOURCE=.\DtaDesignScript.h
# End Source File
# Begin Source File

SOURCE=.\DtaPatternScript.h
# End Source File
# Begin Source File

SOURCE=.\DtaProject.h
# End Source File
# Begin Source File

SOURCE=.\DtaProtectedAreasBag.h
# End Source File
# Begin Source File

SOURCE=.\DtaScript.h
# End Source File
# Begin Source File

SOURCE=.\DtaScriptFactory.h
# End Source File
# Begin Source File

SOURCE=.\DtaScriptVariable.h
# End Source File
# Begin Source File

SOURCE=.\DtaSharpTagsHandler.h
# End Source File
# Begin Source File

SOURCE=.\DtaTranslateScript.h
# End Source File
# Begin Source File

SOURCE=.\DynPackage.h
# End Source File
# Begin Source File

SOURCE=.\ExprScriptBinaryExpression.h
# End Source File
# Begin Source File

SOURCE=.\ExprScriptExpression.h
# End Source File
# Begin Source File

SOURCE=.\ExprScriptFunction.h
# End Source File
# Begin Source File

SOURCE=.\ExprScriptMotif.h
# End Source File
# Begin Source File

SOURCE=.\ExprScriptVariable.h
# End Source File
# Begin Source File

SOURCE=.\ExternalValueNode.h
# End Source File
# Begin Source File

SOURCE=.\GrfAlienInstruction.h
# End Source File
# Begin Source File

SOURCE=.\GrfAllFloatingLocations.h
# End Source File
# Begin Source File

SOURCE=.\GrfAppendedFile.h
# End Source File
# Begin Source File

SOURCE=.\GrfAppendFile.h
# End Source File
# Begin Source File

SOURCE=.\GrfAttachInputToSocket.h
# End Source File
# Begin Source File

SOURCE=.\GrfAttachOutputToSocket.h
# End Source File
# Begin Source File

SOURCE=.\GrfAutoexpand.h
# End Source File
# Begin Source File

SOURCE=.\GrfBlock.h
# End Source File
# Begin Source File

SOURCE=.\GrfBreak.h
# End Source File
# Begin Source File

SOURCE=.\GrfClearVariable.h
# End Source File
# Begin Source File

SOURCE=.\GrfCloseSocket.h
# End Source File
# Begin Source File

SOURCE=.\GrfCommand.h
# End Source File
# Begin Source File

SOURCE=.\GrfCompileToCpp.h
# End Source File
# Begin Source File

SOURCE=.\GrfContinue.h
# End Source File
# Begin Source File

SOURCE=.\GrfCopyFile.h
# End Source File
# Begin Source File

SOURCE=.\GrfCopyGenerableFile.h
# End Source File
# Begin Source File

SOURCE=.\GrfCopySmartDirectory.h
# End Source File
# Begin Source File

SOURCE=.\GrfCutString.h
# End Source File
# Begin Source File

SOURCE=.\GrfDebugExecution.h
# End Source File
# Begin Source File

SOURCE=.\GrfDelay.h
# End Source File
# Begin Source File

SOURCE=.\GrfDetachInputFromSocket.h
# End Source File
# Begin Source File

SOURCE=.\GrfDetachOutputFromSocket.h
# End Source File
# Begin Source File

SOURCE=.\GrfDo.h
# End Source File
# Begin Source File

SOURCE=.\GrfEnvironTable.h
# End Source File
# Begin Source File

SOURCE=.\GrfError.h
# End Source File
# Begin Source File

SOURCE=.\GrfExecuteString.h
# End Source File
# Begin Source File

SOURCE=.\GrfExecutionContext.h
# End Source File
# Begin Source File

SOURCE=.\GrfExit.h
# End Source File
# Begin Source File

SOURCE=.\GrfExpand.h
# End Source File
# Begin Source File

SOURCE=.\GrfFileAsStandardInput.h
# End Source File
# Begin Source File

SOURCE=.\GrfForeach.h
# End Source File
# Begin Source File

SOURCE=.\GrfFunction.h
# End Source File
# Begin Source File

SOURCE=.\GrfGenerate.h
# End Source File
# Begin Source File

SOURCE=.\GrfGeneratedFile.h
# End Source File
# Begin Source File

SOURCE=.\GrfGeneratedString.h
# End Source File
# Begin Source File

SOURCE=.\GrfGenerateString.h
# End Source File
# Begin Source File

SOURCE=.\GrfGlobalVariable.h
# End Source File
# Begin Source File

SOURCE=.\GrfGoBack.h
# End Source File
# Begin Source File

SOURCE=.\GrfIfThenElse.h
# End Source File
# Begin Source File

SOURCE=.\GrfIncrementIndentLevel.h
# End Source File
# Begin Source File

SOURCE=.\GrfInsertAssignment.h
# End Source File
# Begin Source File

SOURCE=.\GrfInsertElementAt.h
# End Source File
# Begin Source File

SOURCE=.\GrfInsertText.h
# End Source File
# Begin Source File

SOURCE=.\GrfInsertTextOnce.h
# End Source File
# Begin Source File

SOURCE=.\GrfInsertTextOnceToFloatingLocation.h
# End Source File
# Begin Source File

SOURCE=.\GrfInsertTextToFloatingLocation.h
# End Source File
# Begin Source File

SOURCE=.\GrfInvertArray.h
# End Source File
# Begin Source File

SOURCE=.\GrfListAllGeneratedFiles.h
# End Source File
# Begin Source File

SOURCE=.\GrfLocalReference.h
# End Source File
# Begin Source File

SOURCE=.\GrfLocalVariable.h
# End Source File
# Begin Source File

SOURCE=.\GrfMerge.h
# End Source File
# Begin Source File

SOURCE=.\GrfNewProject.h
# End Source File
# Begin Source File

SOURCE=.\GrfNop.h
# End Source File
# Begin Source File

SOURCE=.\GrfOpenLogFile.h
# End Source File
# Begin Source File

SOURCE=.\GrfOverwritePortion.h
# End Source File
# Begin Source File

SOURCE=.\GrfParseAsBNF.h
# End Source File
# Begin Source File

SOURCE=.\GrfParsedFile.h
# End Source File
# Begin Source File

SOURCE=.\GrfParsedString.h
# End Source File
# Begin Source File

SOURCE=.\GrfParseFree.h
# End Source File
# Begin Source File

SOURCE=.\GrfParseStringAsBNF.h
# End Source File
# Begin Source File

SOURCE=.\GrfPopulateProtectedArea.h
# End Source File
# Begin Source File

SOURCE=.\GrfProduceHTML.h
# End Source File
# Begin Source File

SOURCE=.\GrfPushItem.h
# End Source File
# Begin Source File

SOURCE=.\GrfPutEnv.h
# End Source File
# Begin Source File

SOURCE=.\GrfQuantifyExecution.h
# End Source File
# Begin Source File

SOURCE=.\GrfQuiet.h
# End Source File
# Begin Source File

SOURCE=.\GrfRandomSeed.h
# End Source File
# Begin Source File

SOURCE=.\GrfReadonlyHook.h
# End Source File
# Begin Source File

SOURCE=.\GrfReference.h
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveAllElements.h
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveElement.h
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveFirstElement.h
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveLastElement.h
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveRecursive.h
# End Source File
# Begin Source File

SOURCE=.\GrfRemoveVariable.h
# End Source File
# Begin Source File

SOURCE=.\GrfResizeOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\GrfReturn.h
# End Source File
# Begin Source File

SOURCE=.\GrfSaveBinaryToFile.h
# End Source File
# Begin Source File

SOURCE=.\GrfSaveProject.h
# End Source File
# Begin Source File

SOURCE=.\GrfSaveProjectTypes.h
# End Source File
# Begin Source File

SOURCE=.\GrfSaveToFile.h
# End Source File
# Begin Source File

SOURCE=.\GrfSelect.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetAll.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetAssignment.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetCommentBegin.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetCommentEnd.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetFloatingLocation.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetGenerationHeader.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetIncludePath.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetInputLocation.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetNow.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetOutputLocation.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetProperty.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetProtectedArea.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetTextMode.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetVersion.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetWorkingPath.h
# End Source File
# Begin Source File

SOURCE=.\GrfSetWriteMode.h
# End Source File
# Begin Source File

SOURCE=.\GrfSleep.h
# End Source File
# Begin Source File

SOURCE=.\GrfSlideNodeContent.h
# End Source File
# Begin Source File

SOURCE=.\GrfSortArray.h
# End Source File
# Begin Source File

SOURCE=.\GrfStringAsStandardInput.h
# End Source File
# Begin Source File

SOURCE=.\GrfSwitch.h
# End Source File
# Begin Source File

SOURCE=.\GrfText.h
# End Source File
# Begin Source File

SOURCE=.\GrfTraceEngine.h
# End Source File
# Begin Source File

SOURCE=.\GrfTraceLine.h
# End Source File
# Begin Source File

SOURCE=.\GrfTraceObject.h
# End Source File
# Begin Source File

SOURCE=.\GrfTraceStack.h
# End Source File
# Begin Source File

SOURCE=.\GrfTraceText.h
# End Source File
# Begin Source File

SOURCE=.\GrfTranslate.h
# End Source File
# Begin Source File

SOURCE=.\GrfTryCatch.h
# End Source File
# Begin Source File

SOURCE=.\GrfWhile.h
# End Source File
# Begin Source File

SOURCE=.\GrfWriteBytes.h
# End Source File
# Begin Source File

SOURCE=.\GrfWritefileHook.h
# End Source File
# Begin Source File

SOURCE=.\GrfWriteText.h
# End Source File
# Begin Source File

SOURCE=.\GrfWriteTextOnce.h
# End Source File
# Begin Source File

SOURCE=.\HTTPRequest.h
# End Source File
# Begin Source File

SOURCE=.\JNIExternalHandling.h
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\NetSocket.h
# End Source File
# Begin Source File

SOURCE=.\ScpStream.h
# End Source File
# Begin Source File

SOURCE=.\UtlDate.h
# End Source File
# Begin Source File

SOURCE=.\UtlDirectory.h
# End Source File
# Begin Source File

SOURCE=.\UtlException.h
# End Source File
# Begin Source File

SOURCE=.\UtlString.h
# End Source File
# Begin Source File

SOURCE=.\UtlTimer.h
# End Source File
# Begin Source File

SOURCE=.\UtlTrace.h
# End Source File
# Begin Source File

SOURCE=.\UtlXMLStream.h
# End Source File
# Begin Source File

SOURCE=.\Workspace.h
# End Source File
# End Group
# End Target
# End Project
