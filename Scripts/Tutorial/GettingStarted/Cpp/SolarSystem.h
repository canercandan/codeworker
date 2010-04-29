#ifndef _SolarSystem_h_
#define _SolarSystem_h_

#include <vector>
#include <string>

// this line separates the two insertion points, so as to distinguish them!
class Planet;

class SolarSystem {
	private:
		std::vector<Planet*> _tpPlanets;

	public:
		SolarSystem();
		~SolarSystem();

		// accessors:
		inline std::vector<Planet*> getPlanets() const { return _tpPlanets; }
		inline void setPlanets(std::vector<Planet*> tpPlanets) { _tpPlanets = tpPlanets; }

		// methods:

	private:
		SolarSystem(const SolarSystem&);
		SolarSystem& operator =(const SolarSystem&);
};

#endif
