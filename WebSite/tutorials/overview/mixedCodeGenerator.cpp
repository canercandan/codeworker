//-------------------------------------------------------------
// Example of an 'mixed code generator' as proposed by
// Jack D. Herrington (http://www.codegeneration.net).
//
// Type:
//   CodeWorker -expand mixedCodeGenerator.cwt
//                      mixedCodeGenerator.cpp
//
// It illustrates the capability of CodeWorker to
// generate code by expansion of an existing file.
// CodeWorker recognizes specific markups, looking
// like:
//   <comment-begin> "##markup##" <key-as-string> <comment-end>
//-------------------------------------------------------------
#include "db.h" 

int main( int argc, char *argv[] ) 
{ 
	//##markup##"SQL select * from users"
	// To run this test once again, delete lines the generator
	// has injected between '##begin##' and '##end##' tags.
	return 0;
} 
