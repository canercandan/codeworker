//note: Visual C++-specific pragma must be added to prevent from intempestive warnings
//note: about template class instantiation of \samp{std::vector<\textit{T}>} in DEBUG mode!
#ifdef WIN32
#pragma warning(disable : 4786)
#endif

//##protect##"include files"
//##protect##"include files"

#include "Planet.h"

Planet::Planet() : _dDiameter(0.0) {
}

Planet::~Planet() {
}

double Planet::getDistanceToSun(int iDay, int iMonth, int iYear) {
//##protect##"getDistanceToSun.int.int.int"
//##protect##"getDistanceToSun.int.int.int"
}
