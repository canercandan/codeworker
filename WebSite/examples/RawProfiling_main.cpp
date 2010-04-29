#ifdef RAW_PROFILING
#	include "RawProfiling.h"
#endif

#include "RawProfiling_example1.h"


int main() {
	//      Towers of Hanoi
	//
	//  *   |   |      |   *   | 
	// ***  |   |  ->  |  ***  | 
	// -|---|---|-    -|---|---|-
	//  A   B   C      A   B   C
	//
	// origin peg is A
	// destination peg is B
	//
	TowersOfHanoi hanoi(12);
	hanoi.start();

#ifdef RAW_PROFILING
	RawProfiling::RawProfiling::writeToHTML("RawProfiling.html");
#endif
	return 0;
}
