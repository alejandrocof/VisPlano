#include "lut.hpp"

void lut::push(uint8_t r,uint8_t g,uint8_t b){
  if(colorMapDefault){
    vec.clear();
    colorMapDefault=false;
  }
  vec.push_back(r);
  vec.push_back(g);
  vec.push_back(b);
}

lut::lut(){
  push(153,102,255);//min
  push(0,0,255);
  push(0,255,0);
  push(255,255,255);
  push(255,255,0);
  push(255,102,0);
  push(255,0,0);//max
  colorMapDefault = true;
}


void lut::getColor(float lambda, uint8_t &r, uint8_t &g, uint8_t &b){
    if(lambda>1.0)lambda=1.0;
    if(lambda<0.0)lambda=0.0;
    int size=N();
    float f=lambda*(size-1);
    float indice=floor(f);
    float delta=f-indice;
    r = vec[3*indice] + delta*( vec[3*(indice+1)]-vec[3*indice] );
    g = vec[3*indice+1] + delta*( vec[3*(indice+1)+1]-vec[3*indice+1] );
    b = vec[3*indice+2] + delta*( vec[3*(indice+1)+2]-vec[3*indice+2] );
}
