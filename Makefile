#------------------------------------------------------------------------------
#                     Makefile of "CodeWorker" under Linux
#
#
#
# Add the following preprocessor definitions to take advantage of all
# functionalities:
#    * -D CODEWORKER_GNU_READLINE: you can use the C library GNU Readline
#      to improve the console mode in CodeWorker, whille a debugging
#      session, for instance.
#    * -D CODEWORKER_CURL_LIB: the C library "libcurl" is used for URL transfers
#      if you wish to get HTTP pages from the Web. Sources are available at
#      "http://curl.haxx.se/libcurl/", version 7.10.5 or newer.
#      Note: add an include path to the root directory of "curl".
#      Functions interested by this library:
#        - sendHTTPRequest(...),
#        - getHTTPRequest (...),
#        - postHTTPRequest(...),
#    * -D CODEWORKER_LIBUUID: not yet available. Will be used by the function
#      UUID().
#
# If a syntax error occurs into "CGRuntime.cpp" (symbol 'environ' not recognized)
# try -D __USE_GNU.
# If the error doesn't disappear, please find the declaration
# 'extern char **environ;' amongst your system includes and send me an email
# (codeworker@free.fr).
#
# Under Linux, functions for loading dynamic libraries + finding symbols into,
# require the flag '-ldl'.
# Under Cygwin, remove '-ldl'.
# Under other UNIX systems, the flag might be different.
#
#------------------------------------------------------------------------------

##-----------------------------------------------------------------------------
## Generally valid for 'gcc'
##-----------------------------------------------------------------------------

INCDIRS	= -I.
CXXFLAGS	= -O2 -g $(INCDIRS)
LFLAGS	= -g -lm -ldl
CC	= g++

##-----------------------------------------------------------------------------
## If you want to take advantage of GNU Readline, uncomment the following
## 2 lines and add the include path to this library
##-----------------------------------------------------------------------------
#LFLAGS	+= -lreadline -lcurses
#CC	+= -DCODEWORKER_GNU_READLINE

##-----------------------------------------------------------------------------
## Under HP, using g++:
##   - remove the precedent 'LFLAGS'
##   - uncomment 'LFLAGS' below
##-----------------------------------------------------------------------------
#LFLAGS        = -g -lm -L/usr/local/lib

##-----------------------------------------------------------------------------
## Under SUN Solaris, using CC:
##   - remove the precedent 'CXXFLAGS' and 'LFLAGS' and 'CC',
##   - uncomment 'SUNFLAGS'+'SUNLDFLAGS'+'CXXFLAGS'+'LFLAGS'+'CXX'+'CC' below
##   - you may have to remove:
##     * '-D_RWCONFIG_32_NativeStdLib_POSIXThrLib_Static_Release' from SUNFLAGS
##     * '-ltls7532-_NativeStdLib_POSIXThrLib_Static_Release' from SUNLDFLAGS
##-----------------------------------------------------------------------------
#SUNFLAGS        = -D_RWCONFIG_32_NativeStdLib_POSIXThrLib_Static_Release \
#                  -mt -DRW_MULTI_THREAD -D_REENTRANT \
#                  -xtarget=ultra -xarch=v8plusa
#SUNLDFLAGS      = -ltls7532-_NativeStdLib_POSIXThrLib_Static_Release     \
#                  -lposix4 -lthread -lsocket -lnsl
#CXXFLAGS      = -O2 -g $(INCDIRS) $(SUNFLAGS)
#LFLAGS                = -g -lm -ldl $(SUNLDFLAGS)
#CXX             = CC
#CC              = CC


###markup##"OBJECTS"
###begin##"OBJECTS"
OBJECTS = ASTCommand.o BNFAndOrJunction.o BNFBreak.o BNFCharBoundaries.o\
		  BNFCharLitteral.o BNFCheck.o BNFClause.o BNFClauseCall.o\
		  BNFComplementary.o BNFConjunction.o BNFDisjunction.o\
		  BNFEndOfFile.o BNFEndOfLine.o BNFFindToken.o BNFForeach.o\
		  BNFIgnore.o BNFInsert.o BNFMoveAhead.o BNFMultiplicity.o\
		  BNFNextStep.o BNFNot.o BNFPushItem.o BNFRatchet.o\
		  BNFReadAdaString.o BNFReadByte.o BNFReadBytes.o BNFReadCChar.o\
		  BNFReadChar.o BNFReadChars.o BNFReadCompleteIdentifier.o\
		  BNFReadCString.o BNFReadIdentifier.o BNFReadInteger.o\
		  BNFReadNumeric.o BNFReadPythonString.o BNFReadText.o\
		  BNFReadToken.o BNFReadUptoIgnore.o BNFScanWindow.o\
		  BNFSkipIgnore.o BNFStepintoHook.o BNFStepoutHook.o\
		  BNFStepper.o BNFString.o BNFTransformationMode.o BNFTryCatch.o\
		  CGCompiler.o CGExternalHandling.o CGRuntime.o\
		  CppCompilerEnvironment.o CppParsingTree.o DtaArrayIterator.o\
		  DtaASTNavigator.o DtaAttributeType.o DtaBNFScript.o DtaClass.o\
		  DtaCommandScript.o DtaConsoleScript.o DtaDesignScript.o\
		  DtaPatternScript.o DtaProject.o DtaProtectedAreasBag.o\
		  DtaScript.o DtaScriptFactory.o DtaScriptVariable.o\
		  DtaSharpTagsHandler.o DtaTranslateScript.o DtaVisitor.o\
		  DynPackage.o ExprScriptBinaryExpression.o\
		  ExprScriptExpression.o ExprScriptFunction.o ExprScriptMotif.o\
		  ExprScriptVariable.o ExternalValueNode.o GrfAlienInstruction.o\
		  GrfAllFloatingLocations.o GrfAppendedFile.o GrfAppendFile.o\
		  GrfAspectAdvice.o GrfAttachInputToSocket.o\
		  GrfAttachOutputToSocket.o GrfAutoexpand.o GrfBlock.o\
		  GrfBreak.o GrfClearVariable.o GrfCloseSocket.o GrfCommand.o\
		  GrfCompileToCpp.o GrfContinue.o GrfCopyFile.o\
		  GrfCopyGenerableFile.o GrfCopySmartDirectory.o GrfCutString.o\
		  GrfDebugExecution.o GrfDelay.o GrfDetachInputFromSocket.o\
		  GrfDetachOutputFromSocket.o GrfDo.o GrfEnvironTable.o\
		  GrfError.o GrfExecuteString.o GrfExecutionContext.o GrfExit.o\
		  GrfExpand.o GrfExtendExecutedScript.o GrfFileAsStandardInput.o\
		  GrfForeach.o GrfFunction.o GrfGenerate.o GrfGeneratedFile.o\
		  GrfGeneratedString.o GrfGenerateString.o GrfGlobalVariable.o\
		  GrfGoBack.o GrfIfThenElse.o GrfIncrementIndentLevel.o\
		  GrfInsertAssignment.o GrfInsertElementAt.o GrfInsertText.o\
		  GrfInsertTextOnce.o GrfInsertTextOnceToFloatingLocation.o\
		  GrfInsertTextToFloatingLocation.o GrfInvertArray.o\
		  GrfJointPoint.o GrfJointPointCall.o GrfListAllGeneratedFiles.o\
		  GrfLoadProject.o GrfLocalReference.o GrfLocalVariable.o\
		  GrfMerge.o GrfNewProject.o GrfNop.o GrfOpenLogFile.o\
		  GrfOverwritePortion.o GrfParseAsBNF.o GrfParsedFile.o\
		  GrfParsedString.o GrfParseFree.o GrfParseStringAsBNF.o\
		  GrfPopulateProtectedArea.o GrfProduceHTML.o GrfPushItem.o\
		  GrfPutEnv.o GrfQuantifyExecution.o GrfQuiet.o GrfRandomSeed.o\
		  GrfReadonlyHook.o GrfReference.o GrfRemoveAllElements.o\
		  GrfRemoveElement.o GrfRemoveFirstElement.o\
		  GrfRemoveLastElement.o GrfRemoveRecursive.o\
		  GrfRemoveVariable.o GrfResizeOutputStream.o GrfReturn.o\
		  GrfSaveBinaryToFile.o GrfSaveProject.o GrfSaveProjectTypes.o\
		  GrfSaveToFile.o GrfSelect.o GrfSetAll.o GrfSetAssignment.o\
		  GrfSetCommentBegin.o GrfSetCommentEnd.o\
		  GrfSetFloatingLocation.o GrfSetGenerationHeader.o\
		  GrfSetIncludePath.o GrfSetInputLocation.o GrfSetNow.o\
		  GrfSetOutputLocation.o GrfSetProperty.o GrfSetProtectedArea.o\
		  GrfSetTextMode.o GrfSetVersion.o GrfSetWorkingPath.o\
		  GrfSetWriteMode.o GrfSleep.o GrfSlideNodeContent.o\
		  GrfSortArray.o GrfStringAsStandardInput.o GrfSwitch.o\
		  GrfText.o GrfTraceEngine.o GrfTraceLine.o GrfTraceObject.o\
		  GrfTraceStack.o GrfTraceText.o GrfTranslate.o GrfTryCatch.o\
		  GrfWhile.o GrfWriteBytes.o GrfWritefileHook.o GrfWriteText.o\
		  GrfWriteTextOnce.o HTTPRequest.o JNIExternalHandling.o md5.o\
		  NetSocket.o ScpStream.o UtlDate.o UtlDirectory.o\
		  UtlException.o UtlString.o UtlTimer.o UtlTrace.o\
		  UtlXMLStream.o Workspace.o
###end##"OBJECTS"


all: build lib

build: codeworker

codeworker: $(OBJECTS)
	$(CC) -c generator.cpp
	$(CC) -o codeworker $(OBJECTS) generator.o $(LFLAGS) 

lib: $(OBJECTS)
	ar -r libcodeworker.a $(OBJECTS)

clean:
	-rm -f *.o *.a CodeWorker codeworker core
