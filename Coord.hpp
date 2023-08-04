#ifndef COORD_HPP
#define COORD_HPP

#include <iostream>
using namespace std;

class Coord{

private:



public:

	double lat;
	double lon;
	
	Coord(double longitude, double latitude){
		//cout<<"Constructor(longitude="<< longitude <<", latitude="<<latitude<<")"<<endl;
		this->lat=latitude;
		this->lon=longitude;
	}

	Coord(){
		//cout<<"Constructor()"<<endl;
		lat=0.0;
		lon=0.0;
	}
	
	Coord( const Coord& c){
		//cout<<"Constructor(const Coord& c)"<<endl;
		lat=c.lat;
		lon=c.lon;
	}

	friend Coord operator+(Coord const& c1, Coord const& c2);
	friend Coord operator*(double const& k, Coord const& c);
	friend Coord operator*(Coord const& c, double const& k);

	//void swap(Coord&) throw();
/*
	Coord& operator=(const Coord &other)
	{
		return *this;
	}
	*/
};



#endif // COORD_HPP
