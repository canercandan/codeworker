;; Author: Laurent Riesterer


(defvar gen-font-lock-keywords
  (eval-when-compile
    (list
     '("\\<\\(local\\|localref\\|insert\\|ref\\|set\\|pushItem\\|merge\\|setall\\)\\>" . font-lock-type-face)

     '("\\<\\(do\\|while\\|foreach\\|forfile\\|select\\|in\\|reverse\\|sorted\\|no_case\\|if\\|else\\|switch\\|try\\|catch\\|case\\|start\\|default\\|declare\\|external\\|finally\\|return\\|break\\|continue\\|debug\\|delay\\|error\\|quantify\\)\\>" . font-lock-keyword-face)
     '("\\<\\(function\\)\\>[ \t]*\\(\\sw+\\)"
       (1 font-lock-keyword-face) (2 font-lock-function-name-face nil t))
;##markup##"procedures"
;##script##
;@     '("\\<\\(@
;foreach i in project.procedureList {
;    if !first(i) {@|@}
;    writeText(i.name + "\\\\");
;}
;@)\\>" . font-lock-variable-name-face)
;@
;##script##
;##begin##"procedures"
     '("\\<\\(appendFile\\|autoexpand\\|clearVariable\\|compileToCpp\\|copyFile\\|copyGenerableFile\\|copySmartDirectory\\|cutString\\|environTable\\|error\\|executeString\\|expand\\|extendExecutedScript\\|generate\\|generateString\\|insertElementAt\\|invertArray\\|listAllGeneratedFiles\\|loadProject\\|openLogFile\\|parseAsBNF\\|parseStringAsBNF\\|parseFree\\|produceHTML\\|putEnv\\|randomSeed\\|removeAllElements\\|removeElement\\|removeFirstElement\\|removeLastElement\\|removeRecursive\\|removeVariable\\|saveBinaryToFile\\|saveProject\\|saveProjectTypes\\|saveToFile\\|setCommentBegin\\|setCommentEnd\\|setGenerationHeader\\|setIncludePath\\|setNow\\|setProperty\\|setTextMode\\|setVersion\\|setWriteMode\\|setWorkingPath\\|sleep\\|slideNodeContent\\|sortArray\\|traceEngine\\|traceLine\\|traceObject\\|traceStack\\|traceText\\|translate\\|attachInputToSocket\\|detachInputFromSocket\\|goBack\\|setInputLocation\\|allFloatingLocations\\|attachOutputToSocket\\|detachOutputFromSocket\\|incrementIndentLevel\\|insertText\\|insertTextOnce\\|insertTextToFloatingLocation\\|insertTextOnceToFloatingLocation\\|overwritePortion\\|populateProtectedArea\\|resizeOutputStream\\|setFloatingLocation\\|setOutputLocation\\|setProtectedArea\\|writeBytes\\|writeText\\|writeTextOnce\\|closeSocket\\)\\>" . font-lock-variable-name-face)
;##end##"procedures"
;##markup##"functions"
;##script##
;@     '("\\<\\(@
;foreach i in project.functionList {
;    if !first(i) {@|@}
;    writeText(i.name + "\\\\");
;}
;@)\\>" . font-lock-variable-name-face)
;@
;##script##
;##begin##"functions"
     '("\\<\\(flushOutputToSocket\\|acceptSocket\\|add\\|addGenerationTagsHandler\\|addToDate\\|byteToChar\\|bytesToLong\\|bytesToShort\\|canonizePath\\|changeDirectory\\|changeFileTime\\|charAt\\|charToByte\\|charToInt\\|chmod\\|ceil\\|compareDate\\|completeDate\\|completeLeftSpaces\\|completeRightSpaces\\|composeAdaLikeString\\|composeCLikeString\\|composeHTMLLikeString\\|composeSQLLikeString\\|computeMD5\\|copySmartFile\\|coreString\\|countStringOccurences\\|createDirectory\\|createINETClientSocket\\|createINETServerSocket\\|createIterator\\|createReverseIterator\\|createVirtualFile\\|createVirtualTemporaryFile\\|decodeURL\\|decrement\\|deleteFile\\|deleteVirtualFile\\|div\\|duplicateIterator\\|encodeURL\\|endl\\|endString\\|equal\\|equalsIgnoreCase\\|equalTrees\\|executeStringQuiet\\|existDirectory\\|existEnv\\|existFile\\|existVirtualFile\\|existVariable\\|exp\\|exploreDirectory\\|extractGenerationHeader\\|fileCreation\\|fileLastAccess\\|fileLastModification\\|fileLines\\|fileMode\\|fileSize\\|findElement\\|findFirstChar\\|findFirstSubstringIntoKeys\\|findLastString\\|findNextString\\|findNextSubstringIntoKeys\\|findString\\|first\\|floor\\|formatDate\\|getArraySize\\|getCommentBegin\\|getCommentEnd\\|getCurrentDirectory\\|getEnv\\|getGenerationHeader\\|getHTTPRequest\\|getIncludePath\\|getLastDelay\\|getNow\\|getProperty\\|getShortFilename\\|getTextMode\\|getVariableAttributes\\|getVersion\\|getWorkingPath\\|getWriteMode\\|hexaToDecimal\\|hostToNetworkLong\\|hostToNetworkShort\\|increment\\|indentFile\\|index\\|inf\\|inputKey\\|inputLine\\|isEmpty\\|isIdentifier\\|isNegative\\|isNumeric\\|isPositive\\|joinStrings\\|key\\|last\\|leftString\\|lengthString\\|loadBinaryFile\\|loadFile\\|loadVirtualFile\\|log\\|longToBytes\\|midString\\|mod\\|mult\\|networkLongToHost\\|networkShortToHost\\|next\\|not\\|octalToDecimal\\|parseFreeQuiet\\|pathFromPackage\\|postHTTPRequest\\|pow\\|prec\\|randomInteger\\|receiveBinaryFromSocket\\|receiveFromSocket\\|receiveTextFromSocket\\|relativePath\\|removeDirectory\\|removeGenerationTagsHandler\\|repeatString\\|replaceString\\|replaceTabulations\\|resolveFilePath\\|rightString\\|rsubString\\|scanDirectories\\|scanFiles\\|sendBinaryToSocket\\|sendHTTPRequest\\|sendTextToSocket\\|selectGenerationTagsHandler\\|shortToBytes\\|sqrt\\|startString\\|sub\\|subString\\|sup\\|system\\|toLowerString\\|toUpperString\\|translateString\\|trimLeft\\|trimRight\\|trim\\|truncateAfterString\\|truncateBeforeString\\|UUID\\|countInputCols\\|countInputLines\\|getInputFilename\\|getLastReadChars\\|getInputLocation\\|lookAhead\\|peekChar\\|readAdaString\\|readByte\\|readBytes\\|readCChar\\|readChar\\|readCharAsInt\\|readChars\\|readIdentifier\\|readIfEqualTo\\|readIfEqualToIgnoreCase\\|readIfEqualToIdentifier\\|readLine\\|readNextText\\|readNumber\\|readPythonString\\|readString\\|readUptoJustOneChar\\|readWord\\|skipBlanks\\|skipSpaces\\|skipEmptyCpp\\|skipEmptyCppExceptDoxygen\\|skipEmptyHTML\\|skipEmptyLaTeX\\|countOutputCols\\|countOutputLines\\|decrementIndentLevel\\|equalLastWrittenChars\\|existFloatingLocation\\|getFloatingLocation\\|getLastWrittenChars\\|getMarkupKey\\|getMarkupValue\\|getOutputFilename\\|getOutputLocation\\|getProtectedArea\\|getProtectedAreaKeys\\|indentText\\|newFloatingLocation\\|remainingProtectedAreas\\|removeFloatingLocation\\|removeProtectedArea\\)\\>" . font-lock-variable-name-face)
;##end##"functions"

     '("\\(\\sw+\\)[ \t]*:" . font-lock-constant-face)

     '("^\\(#[ \t]*\\sw+\\)\\>[ \t]*\\(\\sw+\\)?"
       (1 font-lock-builtin-face)
       (2 font-lock-function-name-face nil t))

    ))
  "*Rules for highlighting GEN (CodeWorker) code.")

(defvar gen-mode-syntax-table nil
  "Syntax table in use in Gen-mode buffers.")
(if gen-mode-syntax-table
    ()
  (setq gen-mode-syntax-table (make-syntax-table))
  (modify-syntax-entry ?%  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?&  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?+  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?-  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?.  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?:  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?!  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?$  "_" gen-mode-syntax-table) ; FIXME use "'"?
  (modify-syntax-entry ?~  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?<  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?=  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?>  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?|  "_" gen-mode-syntax-table)
  (modify-syntax-entry ?\( "()" gen-mode-syntax-table)
  (modify-syntax-entry ?\) ")(" gen-mode-syntax-table)
  (modify-syntax-entry ?\; "." gen-mode-syntax-table)
  (modify-syntax-entry ?\n "> b" gen-mode-syntax-table)
  (modify-syntax-entry ?\f "> b" gen-mode-syntax-table)
  (modify-syntax-entry ?/  ". 124b" gen-mode-syntax-table)
  (modify-syntax-entry ?*  ". 23" gen-mode-syntax-table)
  (modify-syntax-entry ?@  "\"" gen-mode-syntax-table)
  )


(defvar gen-font-lock-syntactic-keywords
  '(
;    ("\\(\\s\$\\)\\S\$*\\(\\s\$\\)" (1 "< b") (2 "> b"))
;    ("^\\(@\\)\\(.\\|\n\\)*\\(@\\)$" (1 ".") (3 "."))
;    ("\\(@\\)\\(.\\|\n\\)*?\\(@\\)" (1 "< b") (3 "> b"))
     ("\\`@" 0 ".")
    ))

(defun gen-font-lock-syntactic-face-function (state)
  (if (nth 3 state)
      (if (eq (nth 3 state) ?@) 
	  font-lock-gen1-face
	font-lock-string-face)
    font-lock-comment-face))

(defvar gen-imenu-generic-expression
  '(
    (nil "^\\s-*\\(function\\)\\s-+\\(\\(\\s_\\|\\sw\\|\\s.\\)+\\)" 2)
    )
  "Imenu generic expression for gen-mode.  See `imenu-generic-expression'.")


(defun gen-mode ()
  "Major mode for editing GEN (CodeWorker)."
  (interactive)
  (let ((switches nil)
	s)
    (kill-all-local-variables)
    (setq major-mode 'gen-mode)
    (setq mode-name "GEN")
    (set-syntax-table gen-mode-syntax-table)

    (set (make-local-variable 'comment-start) "//")
    (set (make-local-variable 'comment-start-skip) "// *")
;    (set (make-local-variable 'font-lock-comment-face)
;                 'php-comment-face)
    (set (make-local-variable 'font-lock-defaults)
	 '(gen-font-lock-keywords nil t (("_." . "w"))
				  beginning-of-buffer
				  (font-lock-syntactic-keywords
				   . gen-font-lock-syntactic-keywords)
				  (font-lock-syntactic-face-function
				   . gen-font-lock-syntactic-face-function)))
    (set (make-local-variable 'imenu-generic-expression)
	 gen-imenu-generic-expression)
    (setq tab-width 4)
    (run-hooks 'gen-mode-hook)
))


(provide 'gen-mode)
