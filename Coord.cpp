#include "Coord.hpp"

Coord operator+(Coord const& c1, Coord const& c2){
	return Coord( c1.lon+c2.lon, c1.lat+c2.lat );
}

Coord operator-(Coord const& c1, Coord const& c2){
	return Coord( c1.lon-c2.lon, c1.lat-c2.lat );
}

Coord operator*(double const& k, Coord const& c){
	return Coord( k*c.lon, k*c.lat );
}

Coord operator*(Coord const& c, double const& k){
	return Coord( k*c.lon, k*c.lat );
}
