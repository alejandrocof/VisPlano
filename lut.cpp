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

void lut::push(Color color){
  if(colorMapDefault){
	vec.clear();
	colorMapDefault=false;
  }
  vec.push_back(color.r);
  vec.push_back(color.g);
  vec.push_back(color.b);
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

lut::lut(std::vector<Color> vColor){
	for( auto &color : vColor){
		push(color);
	}
  colorMapDefault = true;
}


void lut::getColor(float lambda, uint8_t &r, uint8_t &g, uint8_t &b){
	if(lambda>1.0)lambda=1.0;
	if(lambda<0.0)lambda=0.0;
	int size=N();
	float f=lambda*(size-1);
	float indice=floor(f);
	float indice2=indice+1;
	float delta=f-indice;
	if( indice2 == size ){
		indice2=indice;
	}
	r = vec[3*indice] + delta*( vec[3*indice2]-vec[3*indice] );
	g = vec[3*indice+1] + delta*( vec[3*indice2+1]-vec[3*indice+1] );
	b = vec[3*indice+2] + delta*( vec[3*indice2+2]-vec[3*indice+2] );
}

void lut::getColor(float lambda, Color &color){
	if(lambda>1.0)lambda=1.0;
	if(lambda<0.0)lambda=0.0;
	int size=N();
	float f=lambda*(size-1);
	float indice=floor(f);
	float delta=f-indice;
	color.r = vec[3*indice] + delta*( vec[3*(indice+1)]-vec[3*indice] );
	color.g = vec[3*indice+1] + delta*( vec[3*(indice+1)+1]-vec[3*indice+1] );
	color.b = vec[3*indice+2] + delta*( vec[3*(indice+1)+2]-vec[3*indice+2] );
}

//velocidad
lut colorMap1(
		vector<Color>{
			Color(153,102,255),
			Color(0,0,255),
			Color(0,255,0),
			Color(255,255,255),
			Color(255,255,0),
			Color(255,102,0),
			Color(255,0,0),
		}
		);

//velmaxima
lut colorMap2(
		vector<Color>{
			Color(255,255,255),
			Color(153,102,255),
			Color(0,0,255),
			Color(0,255,255),
			Color(0,255,0),
			Color(255,255,0),
			Color(255,0,0),
		}
		);

//slip
lut colorMap3(
		vector<Color>{
			Color(255,255,255),
			Color(255,255,255),
			Color(254,255,255),
			Color(198,255,255),
			Color(144,255,255),
			Color(76,255,255),
			Color(9,255,253),
			Color(3,255,170),
			Color(0,255,88),
			Color(43,255,43),
			Color(88,255,2),
			Color(170,253,0),
			Color(250,252,0),
			Color(255,226,0),
			Color(255,201,0),
			Color(255,161,0),
			Color(255,121,0),
			Color(255,65,0),
			Color(255,8,0),
		}
		);

//normavelocidad
lut colorMap4(
		vector<Color>{
			Color(255,255,255),
			Color(153,102,255),
			Color(0,0,255),
			Color(0,255,255),
			Color(0,255,0),
			Color(255,255,0),
			Color(255,0,0),
		}
		);
