#ifndef INPUTDATA_HPP
#define INPUTDATA_HPP

#include<iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <math.h>

using namespace std;
enum COMPONENT{_X=0,_Y=1,_Z=2};


class inputData
{
public:
    inputData();
    ~inputData();
    bool read(string path);
    void show();
    string Path(){return path;};
    bool ready(){return READY;};
    int NX(){return n[_X];};
    int NY(){return n[_Y];};
    int NZ(){return n[_Z];};
    int NGX(){return ng[_X];};
    int NGY(){return ng[_Y];};
    int NGZ(){return ng[_Z];};
    int SX(){return st(_X);};
    int SY(){return st(_Y);};
    int SZ(){return st(_Z);};
    int SXi(int i){return s(i,_X);};
    int SYj(int j){return s(j,_Y);};
    int SZk(int k){return s(k,_Z);};
    int ID(int i, int j, int k){return id(i,j,k);};
    int SizeFile(int i, int j, int k){return 4*s(k,_Z)*(s(i,_X)*s(j,_Y)+2);};
    int IndexXini(){return indexini[_X];};
    int IndexXend(){return indexend[_X];};
    int IndexYini(){return indexini[_Y];};
    int IndexYend(){return indexend[_Y];};
    int IndexZini(){return indexini[_Z];};
    int IndexZend(){return indexend[_Z];};

    float DT(){return _DT;};
    int DH(){return _DH;};
    int NT(){return _NT;};
    int NTISKP(){return _NTISKP;};
    
    int NSKPX(){return _NSKP[_X];};
    int NSKPY(){return _NSKP[_Y];};
    int NSKPZ(){return _NSKP[_Z];};
    
    int NBGX(){return NBG[_X];};
    int NBGY(){return NBG[_Y];};
    int NBGZ(){return NBG[_Z];};
    
    int NEDX(){return NED[_X];};
    int NEDY(){return NED[_Y];};
    int NEDZ(){return NED[_Z];};
    //cout<<"DT="<<DT<<" NT="<<NT<<" NTISKP"<<NTISKP<<" Tiempo total="<< NT*DT <<" Paso de tiempo entre archivos="<< DT*NTISKP<<endl;

    //datos relativos al dominio reducido
    int lx(){return _DH*_NSKP[_X]*st(_X);};
    int ly(){return _DH*_NSKP[_Y]*st(_Y);};
    int lz(){return _DH*_NSKP[_Z]*st(_Z);};
    float dt(){return _DT*_NTISKP;};

		int NXSC(){return _NXSC;};
		int NYSC(){return _NYSC;};
		int NZSC(){return _NZSC;};
		int NDEPTH(){return _NDEPTH;};
		
		int xHip(){return x_hypocenterSlip;};
		int yHip(){return y_hypocenterSlip;};
		
		int HOLE_X(int i){return hole[i].first;};
		int HOLE_Y(int i){return hole[i].second;};
		int HOLE_N(){return hole.size();};

private:

    bool READY=false;
    string path;

    //int s(int i, COMPONENT w);
    int s(int i, int w);
    int st(COMPONENT w);
    int id(int i, int j, int k);
    void readLine(int &n,string name="");
    template<typename T> void getValuePerLine(T &n);
    template<typename T, typename... Args> void getValuePerLine(T &n,Args & ... args);
    void getValuesInLine(int &a, int &b);

    ifstream file;
    int nline;

    int _NT,n[3],ng[3];
    int _DH;
    float _DT;
    int STR,DIP,RAKE,SLIP;
    int _NXSC,_NYSC,_NZSC,_NDEPTH;
    int IXPOS,IYPOS;
    int NBG[3],NED[3],_NSKP[3];
    int _NTISKP,NDAMP,IFRE,NSV,V3D,fo,nstr,nholes;
    int N_SD[3],indexini[3],indexend[3];
    vector<std::pair <int,int>> hole;
    
    int ruptureSlip;
    int dxSlip;
    float dtSlip;
    int nTimeStepsSlip;
    float reducSlip;
    int lengthSlip,widthSlip;
    int x_hypocenterSlip,y_hypocenterSlip;
    float durationSlip;
    float riseTimeSlip;
//    vector<int> uo,vo,wo;
//    vector<int> uf,vf,wf;
};





template<typename T>
void inputData::getValuePerLine(T &n){
    nline++;

    string line;
    if( !getline(file,line) ){
        cout<<"Error en la línea "<<nline<<endl;
        exit (EXIT_FAILURE);
    }

    stringstream sline=stringstream(line);
    if( !(sline>>n) ){
        cout<<"Error en la línea "<<nline<<" se esperaba un valor de otro tipo:" <<endl;
        cout<<line<<endl;
        exit (EXIT_FAILURE);
    }
}

template<typename T,typename... Args>
void inputData::getValuePerLine(T &n, Args & ... args){
    getValuePerLine(n);
    getValuePerLine(args...);
}

#endif // INPUTDATA_HPP
