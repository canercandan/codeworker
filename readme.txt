//////////////////////////////////////////////////
//                   CodeWorker                 //
//                   ----------                 //
//////////////////////////////////////////////////

* You have downloaded CodeWorker_WINxxx.zip.
    Directories and files:
  ___
	|- "GettingStarted.bat" allows running all examples, which are proposed
	|  into the 'getting started' section of the documentation,
	|
	|- include\ :
	|		. "CW4dl.h" is the C++ header for building packages intended to
	|			extend the functionalities via dynamic libraries,
	|		. "CGRuntime.h", "CppParsingTree.h", "UtlException.h" allow the
	|			binding of CodeWorker in a C++ application.
	|
	|- bin\
	|		. "CodeWorker.exe" is the interpreter,
	|		. "CodeWorker.lib" is the library for binding CodeWorker in a
	|			C++ application,
	|		. "libcurl.dll" provides the cURL HTTP functionalities
	|
	|- Scripts\
	|		|_ Tutorial\
	|				|___ GettingStarted\ :	examples of the 'getting started'
	|										section are found here,


* You have downloaded the sources from CodeWorker_SRCxxx.zip.
    Directories and files:
  ___
	|- "Makefile": run 'make all' to compile CodeWorker under Linux,
	|- "CodeWorker.dsw": VC++ workspace to compile CodeWorker under Windows
	|- "*.cpp", "*.h": source code
	|
	|- Documentation/
	|		. "CodeWorker.tex" is the TeX source of the documentation
	|		. other files are satellite files
	|
	|- Generation/	puts together all scripts needed to generate the
	|				source code, the documentation and the web site of
	|				CodeWorker
	|		. "GeneratorDriver.cws" is the leader script for generation tasks,
	|		. "descriptor.txt" describes all functions and procedures
	|			available in CodeWorker, with their prototype and an example,
	|
	|- Scripts/
	|		|_ Tutorial/
	|				|___ GettingStarted/ :	examples of the 'getting started'
	|										section are found here,
	|
	|- WebSite/ the web site of CodeWorker, except downloads and the manual
	|		|
	|		|- "index.html": the entry point,
	|		|- "francais.html": same, but in French
	|		|
	|		|- examples/ all examples shown on the site,
	|		|
	|		|- repository/ the repository of CodeWorker's scripts
	|		|		|	. "CodeWorker_grammar.cwp": the complete grammar of the
	|		|		|		scripting language,
	|		|		|	. "CWscript2HTML.cwp": syntax highlighting + contextual
	|		|		|		help of CW scripts in HTML,
	|		|		|	. a lot of other scripts...


* FAQ
	o How to generate the whole HTML documentation locally on my computer?
		Answer:
			- type:
				CodeWorker	-I Generation -path <parent-directory-of-WebSite>
							-script GeneratorDriver.cws -D makeHTMLDoc
			- example: if the directory "WebSite/" is in the folder
				"CodeWorkerSources/", you have to type:
					CodeWorker	-I Generation -path CodeWorkerSources
								-script GeneratorDriver.cws -D makeHTMLDoc
