#ifndef CONFIGDATA_HPP
#define CONFIGDATA_HPP

#include <iostream>
#include<sstream>
#include<vector>
#include<limits>

#include"Utils.hpp"

using namespace std;
enum component{X=0,Y,Z};

struct configComponent{
	component comp;
	float min, max;
	vector<int> px;
	vector<int> py;
	vector<int> pz;
	configComponent(component _comp, float _min, float _max):comp(_comp),min(_min),max(_max){}
};

class configData
{
public:
	configData(int argc, char *argv[]);
	string path;
	int step=-1;
	int initialstep=-1;
	int xhip=std::numeric_limits<int>::max();
	int yhip=std::numeric_limits<int>::max();
	double Ms=-1;
	double Dp=-1;
	unsigned int Eq=0;
	unsigned int Group=0;

	vector<configComponent> data;
};

#endif // CONFIGDATA_HPP
