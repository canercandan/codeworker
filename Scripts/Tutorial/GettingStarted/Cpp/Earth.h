#ifndef _Earth_h_
#define _Earth_h_

#include "Planet.h"
#include <vector>
#include <string>

// this line separates the two insertion points, so as to distinguish them!

class Earth : public Planet {
	private:
		std::vector<std::string> _tsCountryNames;

	public:
		Earth();
		~Earth();

		// accessors:
		inline std::vector<std::string> getCountryNames() const { return _tsCountryNames; }
		inline void setCountryNames(std::vector<std::string> tsCountryNames) { _tsCountryNames = tsCountryNames; }

		// methods:

	private:
		Earth(const Earth&);
		Earth& operator =(const Earth&);
};

#endif
