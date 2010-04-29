//------------------------------------------------------------
//                Intrusive code in a source
//
//
// Type:
//   CodeWorker -expand switch.cwt switch.cpp
//
// After running the script and under Windows/VC++ 6.0, launch
// "switch.dsp" to compile this expanded source.
//------------------------------------------------------------

#include <string>

//##markup##"enum PET_TYPE"
//##data##
//cat
//dog
//snake
//##data##


int main(int, char**) {
	std::string sText = "Customer";
//##markup##"switch(sText)"
//##data##
//Product
//Customer
//Figurine
//##data##
	// Before code expansion, this comment stands just before
	// the trailing '##data##' tag.
	// To run this test once again, delete lines the generator
	// has injected between '##begin##' and '##end##' tags.
	return 0;
}
