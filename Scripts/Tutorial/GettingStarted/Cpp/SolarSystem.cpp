//note: Visual C++-specific pragma must be added to prevent from intempestive warnings
//note: about template class instantiation of \samp{std::vector<\textit{T}>} in DEBUG mode!
#ifdef WIN32
#pragma warning(disable : 4786)
#endif

//##protect##"include files"
//##protect##"include files"

#include "SolarSystem.h"

SolarSystem::SolarSystem() {
}

SolarSystem::~SolarSystem() {
	for (std::vector<planets*>::const_iterator iteratePlanets = _tpPlanets.begin(); iteratePlanets != _tpPlanets.end(); ++iteratePlanets) {
		delete *iteratePlanets;
	}
}

