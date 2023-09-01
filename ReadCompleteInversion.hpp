#ifndef READ_COMPLETE_INVERSION_HPP
#define SHAPE_HPP

#include<iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <iomanip>
#include<limits>


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



#endif // READ_COMPLETE_INVERSION_HPP


