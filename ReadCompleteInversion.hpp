#ifndef READ_COMPLETE_INVERSION_HPP
#define READ_COMPLETE_INVERSION_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <iomanip>
#include <limits>

#include "Coord.hpp"
#include "NiceScale.hpp"

using namespace std;


struct variable{
	string nombre;
	string valor;
	string unidades="";
};

struct str_id{
	string str;
	int id; // 0=nombre variable 1=igual 2=valor 3=unidad
};


struct pointSlip{
public:
	Coord coord;
	double slip;
	pointSlip(Coord coord, double slip){
		this->coord=coord;
		this->slip=slip;
	}

	pointSlip(){
		this->coord=Coord(0,0);
		this->slip=0.0;
	}

	pointSlip(const pointSlip& ps){
		this->coord=ps.coord;
		this->slip=ps.slip;
	}
};

struct dataSlip{
	int Nx;
	int Nz;
	double LAT;
	double LON;
	double STRK;
	double min;
	double max;
	//NiceScale ns;
	vector< vector< pointSlip > > data;
};


class ReadCompleteInversion{
  public:
    int myNum;
    string myString;
    ReadCompleteInversion(ifstream &File);
    
    template<typename T>
    bool get(string nombre, T &val);
    vector<variable> variables;
};

template<typename T>
bool ReadCompleteInversion::get(string nombre, T &val){
	bool encontrado=false;
	for(int i=0;i<variables.size();i++){
		if(variables[i].nombre==nombre){
			stringstream ssvalor(variables[i].valor);
			;
			if( !(ssvalor>>val) ){
				cout<<"Los tipos no coinciden para la variable:"<<nombre<<endl;
			}
			encontrado=true;
			break;
		}
	}
	
	if(!encontrado){
		cout<<"No existe la variable:"<<nombre<<endl;
	}
	return encontrado;
}


void ReadSlip(dataSlip &slip, string filename="complete_inversion.fsp");

#endif // READ_COMPLETE_INVERSION_HPP


