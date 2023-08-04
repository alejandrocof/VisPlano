#ifndef BOUNDINGBOX_HPP
#define BOUNDINGBOX_HPP

#include <limits> 

using namespace std;

//https://en.wikibooks.org/wiki/More_C++_Idioms/Named_Constructor
//https://isocpp.org/wiki/faq/ctors#named-parameter-idiom



class BoundingBox{

  static double _xmin;
  static double _xmax;
  static double _ymin;
  static double _ymax;
  
public:
  BoundingBox(const double& x, const double& y){
    _xmin=(x<_xmin)?x:_xmin;
    _xmax=(x>_xmax)?x:_xmax;
    _ymin=(y<_ymin)?y:_ymin;
    _ymax=(y>_ymax)?y:_ymax;
  }

  BoundingBox(){}
  
  double xmin(){return _xmin;}
  double xmax(){return _xmax;}
  double ymin(){return _ymin;}
  double ymax(){return _ymax;}
  double cx(){return (_xmin+_xmax)/2.0;}
  double cy(){return (_ymin+_ymax)/2.0;}
};

using BB = BoundingBox;

#endif // BOUNDINGBOX_HPP

