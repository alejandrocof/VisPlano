#ifndef TRANSFORML_HPP
#define TRANSFORML_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>

#include "NiceScale.hpp"

using namespace std;

//https://en.wikibooks.org/wiki/More_C++_Idioms/Named_Constructor
//https://isocpp.org/wiki/faq/ctors#named-parameter-idiom




class Transform{

  
  vector<double> v;
  
public:
  Transform(const double _width, const double _height,
            const double _west,  const double _east,
            const double _south, const double _north){
            width=_width;
            height=_height;
            west=_west;
            east=_east;
            south=_south;
            north=_north;
            textHeight=height/40.0;
            pointHeight=height/160.0;
            m_x=_width/(_east-_west);
            b_x=-_west*_width/(_east-_west);
            m_y= _height/(_north-_south);
            b_y= _north*_height/(_north-_south);
          }

  Transform(){}
  static double width;
  static double height;
  static double west;
  static double east;
  static double south;
  static double north;
  static double textHeight;
  static double pointHeight;
  
  static double m_x,b_x;
  static double m_y,b_y;
  
  double x(const double _x);
  double y(const double _y);
  double sx(const double _sx);
  double sy(const double _sy);
  double inv_sx(const double _sx);
  double inv_sy(const double _sy);
  
  bool withinGeoCoor(const double longitude, const double latitude);
  
};


void rot(const double angle,const double cx, const double cy, double& px, double& py);


inline double Transform::x(const double _x){
	return m_x*_x + b_x; }
  //return width*(_x-west)/(east-west); }
  
inline double Transform::y(const double _y){
	return -m_y*_y + b_y; }
  //return height*(north-_y)/(north-south); }
  
inline double Transform::sx(const double _sx){
	return m_x*_sx; }
  //return width*_sx/(east-west); }
  
inline double Transform::sy(const double _sy){
	return m_y*_sy; }
  //return height*_sy/(north-south); }

inline double Transform::inv_sx(const double _sx){
	return _sx/m_x; }
  //return _sx*(east-west)/width; }

inline double Transform::inv_sy(const double _sy){
	return _sy/m_y; }
  //return _sy*(north-south)/height; }

inline bool Transform::withinGeoCoor(const double longitude, const double latitude){
	return west<=longitude && longitude<=east && south<=latitude && latitude<=north; }
	
	
using T = Transform;


class Transform2{


  vector<double> v;

public:
  Transform2(const double _width, const double _height,
			const double _west,  const double _east,
			const double _south, const double _north,
			 NiceScale ns_lat, NiceScale we_long ){
	  this->width=_width;
	  this->height=_height;
	  this->west=_west;
	  this->east=_east;
	  this->south=_south;
	  this->north=_north;
	  this->textHeight=this->height/40.0;
	  this->pointHeight=this->height/160.0;
	  this->m_x=_width/(_east-_west);
	  this->b_x=-_west*_width/(_east-_west);
	  this->m_y= _height/(_north-_south);
	  this->b_y= _north*_height/(_north-_south);
	  this->ns_lat=ns_lat;
	  this->we_long=we_long;
	  cout<<"Transform2:"<<this->width<<" x "<<this->height<<endl;
		  }

  Transform2(){}
  double width;
  double height;
  double west;
  double east;
  double south;
  double north;
  double textHeight;
  double pointHeight;

  double m_x,b_x;
  double m_y,b_y;

  NiceScale ns_lat;
  NiceScale we_long;

  double x(const double _x);
  double y(const double _y);
  double sx(const double _sx);
  double sy(const double _sy);
  double inv_sx(const double _sx);
  double inv_sy(const double _sy);

  bool withinGeoCoor(const double longitude, const double latitude);

};


void rot(const double angle,const double cx, const double cy, double& px, double& py);


inline double Transform2::x(const double _x){
	return m_x*_x + b_x; }
  //return width*(_x-west)/(east-west); }

inline double Transform2::y(const double _y){
	return -m_y*_y + b_y; }
  //return height*(north-_y)/(north-south); }

inline double Transform2::sx(const double _sx){
	return m_x*_sx; }
  //return width*_sx/(east-west); }

inline double Transform2::sy(const double _sy){
	return m_y*_sy; }
  //return height*_sy/(north-south); }

inline double Transform2::inv_sx(const double _sx){
	return _sx/m_x; }
  //return _sx*(east-west)/width; }

inline double Transform2::inv_sy(const double _sy){
	return _sy/m_y; }
  //return _sy*(north-south)/height; }

inline bool Transform2::withinGeoCoor(const double longitude, const double latitude){
	return west<=longitude && longitude<=east && south<=latitude && latitude<=north; }



#endif // TRANSFORML_HPP

