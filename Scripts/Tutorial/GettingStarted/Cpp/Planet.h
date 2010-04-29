#ifndef _Planet_h_
#define _Planet_h_

#include <string>

// this line separates the two insertion points, so as to distinguish them!

class Planet {
	private:
		double _dDiameter;

	public:
		Planet();
		~Planet();

		// accessors:
		inline double getDiameter() const { return _dDiameter; }
		inline void setDiameter(double dDiameter) { _dDiameter = dDiameter; }

		// methods:
		virtual double getDistanceToSun(int iDay, int iMonth, int iYear);

	private:
		Planet(const Planet&);
		Planet& operator =(const Planet&);
};

#endif
