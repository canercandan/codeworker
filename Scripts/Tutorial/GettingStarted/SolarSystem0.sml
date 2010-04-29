//note: a class is declared with keyword \textbf{class}
class Planet {
//note: declaration of attributes in a syntax close to C++ or JAVA
	double diameter;
//note: declaration of methods in a syntax close to C++ or JAVA
	double getDistanceToSun(int day, int month, int year);
}

//note: a class may inherit from an other ; the syntax looks like C++, see \textbf{':'}
class Earth : Planet {
//note: an attribute may be an array ; the syntax looks like JAVA
	string[] countryNames;
}

class SolarSystem {
//note: an attribute may be an object or an array of objects,
//note: and an object may be an aggregation (meaning that it belongs to the instance),
	aggregate Planet[] planets;
}
