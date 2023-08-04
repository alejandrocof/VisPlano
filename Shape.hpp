#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include "Transform.hpp"
#include "NiceScale.hpp"
#include "boundingBox.hpp"
#include "LoadShapeFile.hpp"
#include "lut.hpp"
#include <time.h>

using namespace std;

enum class ShapeName{Rectangle,Circle,Ellipse,Line,Polyline,Polygon,Path,Text,
  LinearGradient,Image,ColorMap,ColorBar,Point,Mesh,ShapeFile,Use, Mask};


class Shape{
public:
  Shape(){}
  
  Shape& Rectangle(const float& x, const float& y, const float& width, const float& height){
    clear();
    head<<"<rect x='"<<x<<"' y='"<<y<<"' width='"<<width<<"' height='"<<height<<"'";
    tail<<"/>"<<endl;
    _shp=ShapeName::Rectangle;
    return *this;
  }
  
  Shape& Circle(const float& x, const float& y, const float& radio){
    clear();
    head<<"<circle cx='"<<x<<"' cy='"<<y<<"' r='"<<radio<<"'";
    tail<<"/>"<<endl;
    _shp=ShapeName::Circle;
    return *this;
  }
  
  Shape& Ellipse(const float& x, const float& y, const float& rx, const float& ry){
    clear();
    head<<"<ellipse cx='"<<x<<"' cy='"<<y<<"' rx='"<<rx<<"' ry='"<<ry<<"'";
    tail<<"/>"<<endl;
    _shp=ShapeName::Ellipse;
    return *this;
  }
  
  Shape& Line(const float& x1, const float& y1, const float& x2, const float& y2){
    clear();
    head<<"<line x1='"<<x1<<"' y1='"<<y1<<"' x2='"<<x2<<"' y2='"<<y2<<"'";
    tail<<"/>"<<endl;
    _shp=ShapeName::Line;
    return *this;
    
  }
  
  Shape& Polygon(const vector<float>& p){
    clear();
    head<<"<polygon points='";
	for(int i=0; i<p.size()/2; i++)
      head<<" "<<p[2*i]<<","<<p[2*i+1];
    head<<"'";
    tail<<"/>"<<endl;
    _shp=ShapeName::Polygon;
    return *this;
  }
  
  Shape& Polyline(const vector<float>& p){
    clear();
    head<<"<polyline points='";
	for(int i=0; i<p.size()/2; i++)
      head<<" "<<p[2*i]<<","<<p[2*i+1];
    head<<"'";
    tail<<"/>"<<endl;
    _shp=ShapeName::Polyline;
    return *this;
  }
  
  Shape& Text(const float& x, const float& y, const string& str){
    clear();
    head<<"<text x='"<<x<<"' y='"<<y<<"'";
    tail<<">"<< str <<"</text>"<<endl;
    _shp=ShapeName::Text;
    return *this;
  }
  
  Shape& LinearGradient(const float& x1, const float& y1,
  const float& x2, const float& y2, const string& id){
    clear();
    head<<"<defs>"<<endl;
    head<<"<linearGradient id='"<<id<<"' x1='"<<x1<<"' y1='"<<y1<<"' x2='"<<x2<<"' y2='"<<x2<<"'>"<<endl;
    tail<<"</linearGradient>"<<endl;
    tail<<"</defs>"<<endl;
    _shp=ShapeName::LinearGradient;
    return *this;
  };
  
  Shape& Image(const float& x, const float& y, const float& width, const float& height, const string& name,const float& cx=0.0, const float& cy=0.0, const float& angle=0.0){
    clear();
    head<<"<image xlink:href='"<< name<<"'";
    head<<" x='"<< x-cx <<"' y='"<< y-cy<<"'";
    head<<" width='"<< width <<"' height='"<< height <<"'";
    head<<" transform='rotate("<< -angle <<","<< x <<","<< y <<")'";
    tail<<"/>"<<endl;
    _shp=ShapeName::Image;
    return *this;
  }
  
  Shape& ColorMap(const lut& colorMap, const string& name){
    clear();
    head<<"<defs>"<<endl;
    head<<"<linearGradient id='"<<name<<"' x1='0' y1='1' x2='0' y2='0'>"<<endl;
    int NColor=colorMap.N();
    for(int iColor=0;iColor<NColor;iColor++){
      body << "\t<stop offset='" << 100*iColor/(NColor-1) << "%' stop-color='rgb(";
      body << (int)colorMap.data()[3*iColor] << ",";
      body << (int)colorMap.data()[3*iColor+1] << ",";
      body << (int)colorMap.data()[3*iColor+2] << ")'/>" << endl;
    }

    tail<<"</linearGradient>"<<endl;
    tail<<"</defs>"<<endl;
    _shp=ShapeName::ColorMap;
    return *this;
  }
  
  Shape& ColorBar(const float& x, const float& y, const float& width, const float& height,const string& name, const float& min, const float& max, const string& title ){
    clear();
    head << "<rect x='" << x << "' y='" << y;
    head << "' width='"<< width <<"' height='"<<  height;
    head <<"' fill='url(#" << name << ")' />"<<endl;

    NiceScale ns(min,max);
    int N=ns.N();
    for(int iVal=0;iVal<N;iVal++){
      body << "<text x='" << x+width << "' y='" << (y + (1.0 - (float)iVal/(N-1))*height) <<"'";
      body << " fill='black' font-family='Times' font-size='"<<0.25*height<<"%'";
      body << " text-anchor='start' dy='.3em'>" << fixed << setprecision( ns.decimals() ) << std::setfill( '0' ) << ns.niceMin + (float)iVal*ns.tickSpacing << "</text>" << endl;
    }
    body<<"<text x='" << x << "' y='" << y + 0.5*height <<"'";
    body<<" fill='black' font-family='Times' font-size='"<<0.25*height<<"%'";
    body << " text-anchor='middle' dy='-0.3em'";
    body << " transform='rotate(-90,"<<  x <<","<< y + 0.5*height <<")'";
    body << ">" << title << "</text>" << endl;
    tail<<""<<endl;
                
    _shp=ShapeName::ColorMap;
    return *this;
  }
  
  Shape& Mesh(const float& _west, const float& _east,
    const float& _south, const float& _north){
    clear();
    head << "";
    NiceScale ns_lat(_south,_north,12);
    NiceScale we_long(_west,_east,12);
    
    int N=ns_lat.N();
    float x1=T().x(we_long.niceMin);
    float x2=T().x(we_long.niceMax);
    //float height=T().sy(ns_lat.niceMax-ns_lat.niceMin)/(5.0*N);
    float height=T().textHeight;
    for(int iVal=0;iVal<N;iVal++){
      //float y=T().y(ns_lat.niceMin + (1.0 - (float)iVal/(N-1))*(ns_lat.niceMax-ns_lat.niceMin));
      float y=T().y(ns_lat.niceMin + (double)iVal*ns_lat.tickSpacing);
      body << "<line";
      body << " x1='" << x1 <<"'";
      body << " y1='" << y  <<"'";
      body << " x2='" << x2 <<"'";
      body << " y2='" << y  <<"'";
      body<<" stroke='black' stroke-width='2'/>" << endl;
    }
    for(int iVal=0;iVal<N;iVal++){
      //float y=T().y(ns_lat.niceMin + (ns_lat.niceMax-ns_lat.niceMin)*(float)iVal/(N-1));
      float lat=ns_lat.niceMin + (double)iVal*ns_lat.tickSpacing;
      float y=T().y(lat);
      body << "<text x='" << x1-0.5*height << "' y='" << y <<"'";
      body << " fill='black' font-family='Times' font-size='"<<0.75*height<<"'";
      //body << " text-anchor='end' dy='.3em'>" << fixed << setprecision( ns_lat.decimals() ) << std::setfill( '0' ) << ns_lat.niceMin + (float)iVal*ns_lat.tickSpacing << "</text>" << endl;
      body << " text-anchor='end' dy='.3em'>" << fixed << setprecision( ns_lat.decimals() ) << std::setfill( '0' ) << lat << "</text>" << endl;
    }
    N=we_long.N();
    float y1=T().y(ns_lat.niceMin);
    float y2=T().y(ns_lat.niceMax);
    for(int iVal=0;iVal<N;iVal++){
      //float x=T().x(we_long.niceMin + (1.0 - (float)iVal/(N-1))*(we_long.niceMax-we_long.niceMin));
      float x=T().x(we_long.niceMin + (double)iVal*we_long.tickSpacing);
      body << "<line";
      body << " x1='" << x<<"'";
      body << " y1='" << y1 <<"'";
      body << " x2='" << x<<"'";
      body << " y2='" << y2 <<"'";
      body<<" stroke='black' stroke-width='2'/>" << endl;
    }
    for(int iVal=0;iVal<N;iVal++){
      //float x=T().x(we_long.niceMin + (we_long.niceMax-we_long.niceMin)*(float)iVal/(N-1));
      float lon=we_long.niceMin + (double)iVal*we_long.tickSpacing;
      float x=T().x(lon);
      body << "<text x='" << x << "' y='" << y1-0.5*height <<"'";
      body << " fill='black' font-family='Times' font-size='"<<0.75*height<<"'";
      //body << " text-anchor='middle' dy='1.5em'>" << fixed << setprecision( we_long.decimals() ) << std::setfill( '0' ) << we_long.niceMin + (float)iVal*we_long.tickSpacing << "</text>" << endl;
      body << " text-anchor='middle' dy='1.5em'>" << fixed << setprecision( we_long.decimals() ) << std::setfill( '0' ) << lon << "</text>" << endl;
    }
    
    float x=T().x(0.5*(we_long.niceMin + we_long.niceMax));
    float y=T().y(ns_lat.niceMin);
    body << "<text x='" << x << "' y='" << y <<"'";
    body << " fill='black' font-family='Times' font-size='"<<height<<"'";
    //body << " text-anchor='middle' dy='1.5em'>" << "Longitude º W" << "</text>" << endl;
    body << " text-anchor='middle' dy='1.5em'>" << "Longitude ºW" << "</text>" << endl;
    x=T().x(we_long.niceMin)-height;
    y=T().y(0.5*(ns_lat.niceMin+ns_lat.niceMax));
    body << "<text x='" << x << "' y='" << y <<"'";
    body << " fill='black' font-family='Times' font-size='"<<height<<"'";
    body << " text-anchor='middle' dy='-1.5em'";
    body << " transform='rotate(-90,"<<  x <<","<< y <<")'";
    //body << ">" << "Latitude º N" << "</text>" << endl;
    body << ">" << "Latitude ºN" << "</text>" << endl;
      
    tail<<""<<endl;
    _shp=ShapeName::ColorMap;
    return *this;
  }
  
  Shape& ShapeFile(const string& name, const int &col, const float &fontSize=1.0){
    clear();
    head<<"";
    body<<"";
    tail<<"";
    nameShapeFile=name;
    //float height=T().sy(_north-_south)/20;
      ifstream fileLines(name,std::ios::in);
      //srand (time(NULL));
      if(fileLines.is_open()){
        std::string sline;
        getline(fileLines,sline);
        cout<<"**************"<<endl;
        cout<<name<<endl;
        cout<<sline<<endl;
        cout<<"**************"<<endl;
        while(getline(fileLines,sline)){
          ReadLineShapeFile RL(sline);
          if(RL.PartiallyInsideTheBox){
          	body<<"<path d=\""<<RL.getPath()<<"\" "<<endl;
          	//body<<"fill='rgb("<<127+rand()%127<<","<<127+rand()%127<<","<<127+rand()%127<<")' ";
          	//body<<"stroke='black'";
          	body<<"fill-opacity='0' stroke='black'";
          	body<<"/>"<<endl;
          	//tail<<"<text x='" << RL.cx() << "' y='" << RL.cy() <<"' ";
          	tail<<"<text x='" << RL.xCentroid << "' y='" << RL.yCentroid <<"' ";
          	//tail<<"fill='rgb("<<80<<","<<80<<","<<80<<")' ";
          	tail<<"opacity='"<<0.8<<"' ";
          	tail<<"font-family='Times' font-size='"<<fontSize*T().textHeight<<"' ";
          	tail<<"text-anchor='middle'>" << RL.col[col] << "</text>" << endl;
          	//RL.polygonArea();
          }
        }
      }
      else{
        cout<<"Error: no fue posible abrir el archivo "<<name<<endl;
      }
      
    _shp=ShapeName::ShapeFile;
    return *this;
  }
  
  Shape& Use(const string& name,const string& id){
    clear();
    head<<"<use xlink:href='"<< name<<"#"<<id<<"'";
    body<<"";
    tail<<"/>"<<endl;
    _shp=ShapeName::Image;
    return *this;
  }
  
  Shape& Mask(const float& _west, const float& _east,
    const float& _south, const float& _north){
    clear();
    NiceScale ns_lat(_south,_north,12);
    NiceScale we_long(_west,_east,12);
    
    float x1=T().x(we_long.niceMin);
    float x2=T().x(we_long.niceMax);
    float y1=T().y(ns_lat.niceMin);
    float y2=T().y(ns_lat.niceMax);
    
    //   A             --->                   B
    //     (0,0)--------------------(3000,0)
    //       |                          |     
    //       |   (x1,y2)-----(x2,y2)    |     
    //       |    |               |     |     
    //       |    |               |     |     
    //       |    |               |     |     
    //       |    |               |     |     
    //       |   (x1,y1)-----(x2,y1)    |     
    //       |                          |     
    //    (0,2000)----------------(3000,2000)
    //   D              <---                  C
    
		head << "";    
    body<<"<path";
    body<<" d='";
    body<<" M"<<0<<" "<<0;
    body<<" h"<<T().width;
		body<<" v"<<T().height;
		body<<" h"<<-T().width;
		body<<" Z";
		body<<" M"<<x1<<" "<<y1;
    body<<" L"<<x2<<" "<<y1;
    body<<" L"<<x2<<" "<<y2;
    body<<" L"<<x1<<" "<<y2;
		body<<" Z'";
    tail<<"/>"<<endl;

    _shp=ShapeName::Mask;
    
    //clear();
    //head<<"<rect x='"<<x<<"' y='"<<y<<"' width='"<<width<<"' height='"<<height<<"'";
    //tail<<"/>"<<endl;
    //_shp=ShapeName::Rectangle;
    
    return *this;
  }
  
  /*
  static Shape Point(const float& x, const float& y, const float& radio, const string& name){
    clear();
    head<<"<circle cx='"<<x<<"' cy='"<<y<<"' r='"<<radio<<"'";
    tail<<"/>"<<endl;
    return Shape(ShapeName::Point);
  }
  */
  Shape& x(const float& __x);
  Shape& y(const float& __y);
  Shape& x1(const float& __x);
  Shape& y1(const float& __y);
  Shape& x2(const float& __x);
  Shape& y2(const float& __y);
  Shape& width(const float& w);
  Shape& height(const float& h);
  
  Shape& Tx(const float& __x);
  Shape& Ty(const float& __y);
  Shape& Tx1(const float& __x);
  Shape& Ty1(const float& __y);
  Shape& Tx2(const float& __x);
  Shape& Ty2(const float& __y);
  Shape& Swidth(const float& w);
  Shape& Sheight(const float& h);
  
  Shape& txt(const string& txt);
  Shape& id(const string& id);
  
  Shape& fontSize(const int& __size);
  Shape& fontFamily(const string& __font);
  
  //Shape& fill(const string& _fill);
  Shape& fill(const int& __r, const int& __g, const int& __b);
  Shape& fillUrl(const string& str);
  
  Shape& fillOpacity(const float& __alpha);
  
  //Shape& stroke(const string& _stroke);
  Shape& stroke(const int& __r, const int& __g, const int& __b);
  Shape& strokeWidth(const float& __w);
  
  Shape& opacity(const float& __alpha);
  
  Shape& align(const string& __align);
  Shape& stop(const int& _r, const int& _g, const int& _b);

  Shape& strokeLinecap(const string& _linecap);//butt round square
  
  private:
  void clear(){
    head.str( std::string() );head.clear();
    body.str( std::string() );body.clear();
    tail.str( std::string() );tail.clear();
  };
  
  struct color{
    int r,g,b;
    color(int _r,int _g,int _b):r(_r),g(_g),b(_b){};
  };
  
  friend ostream& operator << (ostream &o,const Shape& tex);
  stringstream out;
  string _txt;
  ShapeName _shp;
  vector<color> C;
  
  
  stringstream head;
  stringstream tail;
  stringstream body;
  string nameShapeFile; 
};



inline Shape& Shape::x(const float& _x){ body<<" x='"<<_x<<"'";return *this;}
inline Shape& Shape::y(const float& _y){ body<<" y='"<<_y<<"'"; return *this;}
inline Shape& Shape::x1(const float& _x){ body<<" x1='"<<_x<<"'";return *this;}
inline Shape& Shape::y1(const float& _y){ body<<" y1='"<<_y<<"'"; return *this;}
inline Shape& Shape::x2(const float& _x){ body<<" x2='"<<_x<<"'";return *this;}
inline Shape& Shape::y2(const float& _y){ body<<" y2='"<<_y<<"'"; return *this;}
inline Shape& Shape::width(const float& w){ body<<" width='"<<w<<"'";return *this;}
inline Shape& Shape::height(const float& h){ body<<" height='"<<h<<"'"; return *this;}

inline Shape& Shape::Tx(const float& _x){ body<<" x='Tx:"<<_x<<"'";return *this;}
inline Shape& Shape::Ty(const float& _y){ body<<" y='Ty:"<<_y<<"'"; return *this;}
inline Shape& Shape::Tx1(const float& _x){ body<<" x1='Tx:"<<_x<<"'";return *this;}
inline Shape& Shape::Ty1(const float& _y){ body<<" y1='Ty:"<<_y<<"'"; return *this;}
inline Shape& Shape::Tx2(const float& _x){ body<<" x2='Tx:"<<_x<<"'";return *this;}
inline Shape& Shape::Ty2(const float& _y){ body<<" y2='Ty:"<<_y<<"'"; return *this;}
inline Shape& Shape::Swidth(const float& w){ body<<" width='Sx"<<w<<"'";return *this;}
inline Shape& Shape::Sheight(const float& h){ body<<" height='Sy"<<h<<"'"; return *this;}

inline Shape& Shape::txt(const string& _txt){ this->_txt=_txt; return *this;}
inline Shape& Shape::id(const string& _id){ body<<" id='"<<_id<<"'"; return *this;}

inline Shape& Shape::fontSize(const int& size){ body<<" font-size='"<<size<<"'";return *this;}
inline Shape& Shape::fontFamily(const string& font){ body<<" font-family='"<<font<<"'";return *this;}
inline Shape& Shape::align(const string& _align){ body<<" text-anchor='"<<_align<<"'";return *this;}
  
inline Shape& Shape::fill(const int& r, const int& g, const int& b)
{ body<<" fill='rgb("<<r<<","<<g<<","<<b<<")'"; return *this;}

inline Shape& Shape::fillOpacity(const float& alpha)
{ body<<" fill-opacity='"<<alpha<<"'"; return *this;}
  
inline Shape& Shape::fillUrl(const string& str)
{ body<<" fill='url(#"<<str<<")'"; return *this;}

inline Shape& Shape::stroke(const int& r, const int& g, const int& b)
{ body<<" stroke='rgb("<<r<<","<<g<<","<<b<<")'"; return *this;}

inline Shape& Shape::strokeWidth(const float& w)
{ body<<" stroke-width='"<<w<<"'"; return *this;}

inline Shape& Shape::opacity(const float& alpha)
{ body<<" opacity='"<<alpha<<"'"; return *this;}
  
inline Shape& Shape::stop(const int& _r, const int& _g, const int& _b)
{ C.emplace_back(_r,_g,_b); return *this;}

inline Shape& Shape::strokeLinecap(const string& _linecap)//butt round square
{ body<<" stroke-linecap='"<<_linecap<<"'"; return *this;}

#endif // SHAPE_HPP

