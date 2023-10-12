#include "Transform.hpp"

double Transform::width  = 0.0;
double Transform::height = 0.0;
double Transform::west  = 0.0;
double Transform::east  = 0.0;
double Transform::south = 0.0;
double Transform::north = 0.0;
double Transform::textHeight = 0.0;
double Transform::pointHeight = 0.0;
double Transform::m_x = 0.0;
double Transform::b_x = 0.0;
double Transform::m_y = 0.0;
double Transform::b_y = 0.0;

double latlon2m=111320.0;

void rot(const double angle,const double cx, const double cy, double& px, double& py){
    double theta=angle*M_PI/180.0;
    double x=px-cx;
    double y=py-cy;
    double xrot=x*cos(theta)-y*sin(theta);
    double yrot=x*sin(theta)+y*cos(theta);
    px=xrot+cx;
    py=yrot+cy;
}

