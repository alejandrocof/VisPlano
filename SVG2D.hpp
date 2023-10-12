#ifndef SVG2D_HPP
#define SVG2D_HPP

#include <iostream>
#include <fstream>
#include<iomanip>
#include "Shape.hpp"

using namespace std;

class Shape;

class SVG2D{
	ofstream file;
	string nameFile;
	int width,height;

	
  public:
  Transform2 *TXY;
  SVG2D(const string& name, const int& width, const int& height, const string& id="");
  SVG2D(Transform2 &TXY, const string& name, const string& id="");
  ~SVG2D();
  void add(const Shape& ss);
  void setTransform(Transform2 &TXY){
	  this->TXY=&TXY;
  }
};


#endif // SVG2D_HPP
