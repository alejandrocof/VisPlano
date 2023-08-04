#include "boundingBox.hpp"

double BoundingBox::_xmin  = std::numeric_limits<double>::max();
double BoundingBox::_xmax  = -std::numeric_limits<double>::max();
double BoundingBox::_ymin  = std::numeric_limits<double>::max();
double BoundingBox::_ymax  = -std::numeric_limits<double>::max();

