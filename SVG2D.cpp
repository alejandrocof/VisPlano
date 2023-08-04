#include "SVG2D.hpp"

SVG2D::SVG2D(const string& name, const int& width, const int& height, const string& id) : nameFile{name}, width{width}, height{height} {
	file.open(nameFile);
	file << "<?xml version='1.0' encoding='UTF-8' standalone='no'?>"<<endl;
	file << "<svg id='"<<id<<"' width='"<<width<<"' height='"<<height<<"' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'>"<<endl;
}

SVG2D::~SVG2D(){
	file << "</svg>"<<endl;
	file.close();
}

void SVG2D::add(const Shape& ss){
  file<<ss;
}


