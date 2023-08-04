#ifndef LUT_HPP
#define LUT_HPP

#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

class lut{
    std::vector<uint8_t> vec;
  public:
    lut();
    void push(uint8_t r,uint8_t g,uint8_t b);
    int N()const{return vec.size()/3;};
    int size(){return vec.size();};
    const uint8_t *data() const{return vec.data();};
    void getColor(float lambda, uint8_t &r, uint8_t &g, uint8_t &b);
    bool colorMapDefault;
};

#endif
