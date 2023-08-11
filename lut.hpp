#ifndef LUT_HPP
#define LUT_HPP

#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

struct Color{
	uint8_t r;
	uint8_t g;
	uint8_t b;

	Color(uint8_t r, uint8_t g, uint8_t b){
		this->r=r;
		this->g=g;
		this->b=b;
	}

	Color(){}
};

class lut{
    std::vector<uint8_t> vec;
  public:
    lut();
	lut(std::vector<Color>);
    void push(uint8_t r,uint8_t g,uint8_t b);
	void push(Color color);
    int N()const{return vec.size()/3;};
    int size(){return vec.size();};
    const uint8_t *data() const{return vec.data();};
    void getColor(float lambda, uint8_t &r, uint8_t &g, uint8_t &b);
	void getColor(float lambda, Color &color);
    bool colorMapDefault;
};

#endif
