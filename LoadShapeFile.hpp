#ifndef LOAD_SHAPE_FILE_HPP
#define LOAD_SHAPE_FILE_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>

#include "Transform.hpp"

using namespace std;

class ReadLineShapeFile{
  struct p2D{
    double x;
    double y;
  };
  struct polygon{
    vector<p2D> p;
    double cx=0,cy=0;
    double xmin=numeric_limits<double>::max();
    double xmax=-numeric_limits<double>::max();
    double ymin=numeric_limits<double>::max();
    double ymax=-numeric_limits<double>::max();

    
    
  };
  //vector<vector<p2D>> shapes;
  vector<polygon> vpol;
  /*
  double _cx=0.0;
  double _cy=0.0;
  double _xmin=numeric_limits<double>::max();
  double _xmax=-numeric_limits<double>::max();
  double _ymin=numeric_limits<double>::max();
  double _ymax=-numeric_limits<double>::max();
  */
  int imax;
  
public:

  ReadLineShapeFile(Transform2 &TXY, string& line);
  //double cx(){return T().x(_cx);};
  //double cy(){return T().y(_cy);};
  /*
  double cx(){return T().x( (_xmin+_xmax)/2.0 );};
  double cy(){return T().y( (_ymin+_ymax)/2.0 );};
  double xmin(){return T().x(_xmin);};
  double xmax(){return T().x(_xmax);};
  double ymin(){return T().y(_ymin);};
  double ymax(){return T().y(_ymax);};
  */
  double cx(){return TXY.x( (vpol[imax].xmin+vpol[imax].xmax)/2.0 );};
  double cy(){return TXY.y( (vpol[imax].ymin+vpol[imax].ymax)/2.0 );};
  double xmin(){return TXY.x(vpol[imax].xmin);};
  double xmax(){return TXY.x(vpol[imax].xmax);};
  double ymin(){return TXY.y(vpol[imax].ymin);};
  double ymax(){return TXY.y(vpol[imax].ymax);};
  
  void polyCentroid();
  int pnpoly(double x, double y);
  void polygonArea();
  string getPath();
  vector<string> col;
  bool TotallyOutOfTheBox=true;
  bool PartiallyInsideTheBox=false;
  double xCentroid;
  double yCentroid;
  Transform2 TXY;
};

#endif // LOAD_SHAPE_FILE_HPP
