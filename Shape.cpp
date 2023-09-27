#include "Shape.hpp"

ostream& operator << (ostream &o,const Shape& tex){
  switch(tex._shp){
	case ShapeName::IniGroup:
	case ShapeName::EndGroup:
    case ShapeName::Rectangle:
    case ShapeName::Circle:
    case ShapeName::Ellipse:
    case ShapeName::Line:
    case ShapeName::Polyline:
    case ShapeName::Polygon:
    case ShapeName::Path:
    case ShapeName::Text:
    case ShapeName::Image:
    case ShapeName::ColorMap:
    case ShapeName::ColorBar:
    case ShapeName::Point:
    case ShapeName::Mesh:
    case ShapeName::Use:
    case ShapeName::Mask:
	case ShapeName::MaskRaw:
    case ShapeName::ShapeFile:
      o<<tex.head.str()<<tex.body.str()<<tex.tail.str();
    break;
    case ShapeName::LinearGradient:
      o << tex.head.str();
      for(int i=0;i<tex.C.size();i++){
        o << "\t<stop offset='" << 100.0*i/(tex.C.size()-1)  <<"%' stop-color='rgb(";
        o << tex.C[i].r<<","<<tex.C[i].g<<","<<tex.C[i].b<<")'"<<"/>"<<endl;
      }
      o <<tex.tail.str();
    break;
  };
  return o;
}
