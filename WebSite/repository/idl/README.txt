README.txt
==========

The NRG SDK contains the CORBA classes, which are used transparantly
to HOSA applications if the SDK is used. The Java source code and 
documentation of these CORBA classes are not needed by developers 
using the SDK and are not part of the SDK installation file, thus
minimizing its size. 

Developers that wish to use the CORBA classes directly (or create/
update corba_xx.jar files) may want to generate the Java source code 
and its documentation. First, an IDL compiler should be used to create
Java source files. Next, the JavaDoc tool can be used to generate
documentation for the resulting classes.

Below, some examples are given on how to invoke these tools. 
It is assumed that the current working directory is hosasdk\corba\. 


Creating the Java source (idlj)
-------------------------------

The following example uses the Sun IDL compiler (version 3.0) 
to create Java source code based on the IDL-files. 
It has to be repeated for all *.idl files in idl\. 
The resulting source code is output to source\:

	idlj -keep -emitAll -i idl -fall -td source idl\xyz.idl

For details on the command-line arguments, enter idlj without any arguments.


Creating the Java source (idl2java)
-----------------------------------

The following example uses the Visibroker IDL compiler (version 5.0) 
to create Java source code based on the IDL-files. 
It has to be repeated for all *.idl files in idl\. 
The resulting source code is output to source\:

	idl2java -boa -back_compat_mapping -root_dir source\ idl\xyz.idl

For details on the command-line arguments, enter idl2java without any arguments.


Compiling the Java source
-------------------------

The following example uses the Sun Javac tool to compile the Java
source code.
The resulting class files are output to classes\ (which must be created
before the javac tool is invoked):

First, create a file input_files.txt that contains all Java-source files 
that have to be compiled.

Windows users can invoke the following command:

	dir /s /b source\*.java > input_files.txt

Similarly, Linux/UNIX users can invoke the following command:

	ls -r source/*.java > input_files.txt

Next, invoke the following command to create the class files:

	javac -d classes @input_files.txt

For details on the command-line arguments, enter javac without any arguments.

To create a Java archive containing the resulting classes, use the Sun Jar tool.
The following command creates a Jar file named lib\my_corba_sun.jar:

	jar cf lib\my_corba_sun.jar -C classes .

For details on the command-line arguments, enter jar without any arguments.

