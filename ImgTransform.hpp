#ifndef IMG_TRANSFORML_HPP
#define IMG_TRANSFORML_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include "Coord.hpp"

using namespace std;

class ImgTransform{

public:
	ImgTransform(const double _xmin, const double _xmax,
	const double _ymin, const double _ymax,
	int _nx, int _ny){
		xmin = _xmin;
		xmax = _xmax;
		ymin = _ymin;
		ymax = _ymax;
		nx = _nx;
		ny = _ny;
		p0=Coord( _xmin, _ymin);
		//p_d=Coord( _xmax-_xmin, _ymax-_ymin);
		dx = (_xmax - _xmin)/_nx;
		dy = (_ymax - _ymin)/_ny;
		
		/*
		cout<<"***** Tamaño ( "<<nx<<" x "<<ny<<" )"<<endl;
		cout<<"***** Coordenada1 ( "<<xmin<<", "<<ymin<<" )"<<endl;
		cout<<"***** Coordenada2 ( "<<xmax<<", "<<ymax<<" )"<<endl;
		cout<<"***** dx, dy ( "<<dx<<", "<<dy<<" )"<<endl;
		cout<<"***** p0 ( "<<p0.lon<<", "<<p0.lat<<" )"<<endl;
		*/
	}

	ImgTransform(){}
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int nx;
	int ny;
	double dx,dy;
	Coord p0;
	

	double x(const int i, const int j);
	double y(const int i, const int j);
	Coord xy(const float i, const float j);
};


inline double ImgTransform::x(const int i, const int j){
	return 0; }

inline double ImgTransform::y(const int i, const int j){
	return 0; }
	
inline Coord ImgTransform::xy(const float i, const float j){
	return p0 + Coord( i*dx, j*dy );
}
	
using ImgT = ImgTransform;
#endif // IMG_TRANSFORML_HPP

