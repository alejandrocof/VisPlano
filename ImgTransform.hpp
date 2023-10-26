#ifndef IMG_TRANSFORML_HPP
#define IMG_TRANSFORML_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include "Coord.hpp"
#include "inputdata.hpp"
#include "Transform.hpp"
#include "ReadCompleteInversion.hpp"


using namespace std;
extern double latlon2m;
//cambiar de lugar
struct DisplaySettings{
	unsigned int imageWidth;
	unsigned int imageHeight;
	string nameFileCSV;
	unsigned int colLabelFileCSV;
	double epi_lon;
	double epi_lat;
	double VELXMIN;
	double VELXMAX;
	double theta;
};

class ImgTransform{

public:
	ImgTransform( inputData &infoData, const DisplaySettings &dsettings, dataSlip &slip){
	//ImgTransform( inputData &infoData, const DisplaySettings &dsettings){
		this->SX = infoData.SX();
		this->SY = infoData.SY();
		this->LX = infoData.lx();
		this->LY = infoData.ly();
		this->DH = infoData.DH();
		this->DX = this->LX/this->SX;
		this->DY = this->LY/this->SY;
		this->NXSC = ((double)infoData.NXSC()-infoData.NBGX())/infoData.NSKPX();
		this->NYSC = ((double)infoData.NYSC()-infoData.NBGY())/infoData.NSKPY();
		this->lxsc = this->NXSC*this->DH;
		this->lysc = this->NYSC*this->DH;
		this->DT = infoData.DT();
		this->lng_cx = slip.LON;
		this->lat_cy = slip.LAT;
		this->lcx = infoData.lx()/2.0;
		this->lcy = infoData.ly()/2.0;

		if(dsettings.epi_lon && dsettings.epi_lat){
			this->lng_nxsc = dsettings.epi_lon;
			this->lat_nysc = dsettings.epi_lat;
		}
		else{
			this->lng_nxsc = slip.data[0][slip.Nx-1].coord.lon;
			this->lat_nysc = slip.data[0][slip.Nx-1].coord.lat;
		}

		this->theta = 270.0-dsettings.theta;
		this->theta_rad=this->theta*M_PI/180.0;
		//this->theta = 44.0*M_PI/180.0;;//dsettings.theta*M_PI/180.0;
	}


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

	int SX;
	int SY;
	int LX;
	int LY;
	int DH;
	double NXSC;
	double NYSC;
	double DX;
	double DY;
	double lxsc;
	double lysc;
	double lng_nxsc;
	double lat_nysc;
	double theta;
	double theta_rad;
	double DT;
	double lng_cx;
	double lat_cy;
	double lcx;
	double lcy;
	

	double x(const int i, const int j);
	double y(const int i, const int j);
	Coord xy(const float i, const float j);
	Coord ij2latlng(const double i, const double j);
	Coord lxly2latlng(const double lx, const double ly);
};


inline double ImgTransform::x(const int i, const int j){
	return 0; }

inline double ImgTransform::y(const int i, const int j){
	return 0; }
	
inline Coord ImgTransform::xy(const float i, const float j){
	return p0 + Coord( i*dx, j*dy );
}

inline Coord ImgTransform::ij2latlng(const double i, const double j){
	return lxly2latlng( (i + 0.5)*this->DX, (j + 0.5)*this->DY );
}

//    lx y ly en metros
//LY .----------------.
//   |                |
//   |-------.(lx,ly) |
//   |       |        |
// 0 .----------------.
//   0                LX
inline Coord ImgTransform::lxly2latlng(const double lx, const double ly){
	double lng = ( lx - this->lcx )/latlon2m;
	double lat = ( ly - this->lcy )/latlon2m;
	double lngrot = lng*cos(this->theta_rad) - lat*sin(this->theta_rad);
	double latrot = lng*sin(this->theta_rad) + lat*cos(this->theta_rad);
	return Coord( this->lng_cx+lngrot, this->lat_cy+latrot);
	//return Coord( this->lng_nxsc, this->lat_nysc);
}
	
using ImgT = ImgTransform;
#endif // IMG_TRANSFORML_HPP

