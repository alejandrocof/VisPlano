#include<iostream>
#include<string>
#include<sstream>


#include"inputdata.hpp"
#include<iomanip>

#include<limits>
#include<chrono>
#include<omp.h>

#include <algorithm>
#include <vector>
#include <map>
#include <tuple>
#include <iterator>


#include "TinyPngOut.hpp"
#include "configdata.hpp"

#include <cmath>

#include "SVG2D.hpp"
//#include "Shape.hpp"
#include "Transform.hpp"
#include "ImgTransform.hpp"
#include "sigfigs.hpp"
#include "Coord.hpp"
#include "ReadCompleteInversion.hpp"
#include "lut.hpp"

//for i in Vx3D*.svg;do svg="${i}";png="${i}.png";echo "$png";rsvg-convert -w 3000 -h 2000 $svg -o $png; done
//
//for f in ./Vx3D-Z0-TI0*.svg; do echo $(basename "${f}" .svg); done  | xargs -I{} --max-procs 4 bash -c " echo 'Processing ID' {}.svg '..'; rsvg-convert -w 2200 -h 2000 {}.svg -o {}.png; echo 'Done {}.png'; exit 0;" 2>&1 | tee sal-bis


//mencoder 'mf://Vx3D*.png' -mf fps=25 -ovc lavc -lavcopts vcodec=mpeg4 -o video01.avi
//mencoder "mf://./imagenes/Vx3D*.png" -mf fps=33.33333 -o video04.avi -ovc x264

//#define MI_PC
#define DEBUG
//https://www.nayuki.io/page/tiny-png-output para guardar imagenes png
using namespace std;
using namespace std::chrono;
using clk = high_resolution_clock;
using sec = duration<double>;

using std::uint8_t;
using std::uint32_t;
using std::size_t;

//std::vector<uint8_t> lutColor;

double latlon2m=111320.0;
//double latlon2m=104270.0;
double VELXMIN;
double VELXMAX;
double epi_lon;
double epi_lat;
int xhip;
int yhip;
int image_width;
int image_height;
string nameFileCSV;
int colLabelFileCSV;

double nx;//celdas en x del dominio físico de simulación
double ny;//celdas en y del dominio físico de simulación

double lx;//ancho dominio físico de simulación en metros
double ly;//alto dominio físico de simulación en metros
int 	nxsc;
int   nysc;
double dh;
double textHeight;
double dt;
int digTime;

int ndigID=4;//número de dígitos para el identificador del sub-dominio
int ndigTI;//5 número de dígitos para el índice del tiempo, los datos mas actuales tiene valor de 5

double theta=0.0;
vector< vector < float > > vSlip;
NiceScale *nsSlip;
//push(153,102,255);//min
//push(0,0,255);
//push(0,255,0);
//push(255,255,255);
//push(255,255,0);
//push(255,102,0);
//push(255,0,0);//max

extern lut colorMap1;
extern lut colorMap2;
extern lut colorMap3;

vector<Coord> estaciones={
	Coord(-93.94, 14.94),
	Coord(-96.49, 15.67),
	Coord(-92.27, 14.92),
	Coord(-96.7, 17.07),
	Coord(-96.72, 17.07),
	Coord(-94.54, 17.99),
	Coord(-98.17, 19.04),
	Coord(-98.56, 17.8),
	Coord(-96.9, 19.53),
	Coord(-100.09, 17),
	Coord(-99.89, 16.87),
	Coord(-100.16, 18.9),
	Coord(-100.43, 17.21),
	Coord(-99.82, 16.91),
	Coord(-100.72, 20.04),
	Coord(-101.26, 17.54),
	Coord(-94.88, 17.09),
	Coord(-93.12, 16.78),
	Coord(-94.42, 18.03),
	Coord(-97.36, 18.42),
	Coord(-98.63, 19.07),
	Coord(-98.13, 16.39),
	Coord(-97.8, 17.83),
	Coord(-98.57, 17.56),
	Coord(-99.62, 17.92),
	Coord(-101.46, 17.61),
	Coord(-99.04, 20.3)
};

struct mmi_val{
	Color c;
	string label;
	double min, max, val;

	mmi_val(Color c, string label, double min, double max, double val):
		c(c), label(label), min(min),	max(max), val(val){};
};

vector< mmi_val > MMI_Scale={
	mmi_val( Color(255,255,255),"I. Not felt",-std::numeric_limits<double>::max(),1.5,1.0),
	mmi_val( Color(190,204,252),"II. Weak", 1.5, 2.5, 2.0),
	mmi_val( Color(155,154,250),"III. Weak", 2.5, 3.5, 3.0),
	mmi_val( Color(115,255,254),"IV. Light", 3.5, 4.5, 4.0),
	mmi_val( Color( 99,248,155),"V. Moderate", 4.5, 5.5, 5.0),
	mmi_val( Color(254,254, 69),"VI. Strong", 5.5, 6.5, 6.0),
	mmi_val( Color(255,220, 60),"VII. Very strong", 6.5, 7.5, 7.0),
	mmi_val( Color(255,144, 42),"VIII. Severe", 7.5, 8.5, 8.0),
	mmi_val( Color(255,  0, 22),"IX. Violent", 8.5, 9.5, 9.0),
	mmi_val( Color(230,  0, 18),"X. Extreme", 9.5, 10.5, 10.0),
	mmi_val( Color(142,  0,  7),"XI. Extreme", 10.5, 11.5, 11.0),
	mmi_val( Color( 71,  0,  2),"XII. Extreme", 11.5, std::numeric_limits<double>::max(), 12.0),
};

//B's para la ecuación 2
//el índice indica el grupo,
//los grupos son 1, 2, 3,
//el indice cero es basura
//Grupo 1 Costa
//Grupo 2 Interior
//Grupo 3 Interior
//Ejemplos G1: Oaxaca, Acapulco, Tecoman
//Ejemplos G2: Puebla, Chiapas(dentro de placa)
double B0[2][3] = { { 1.1090, 1.5188, 3.0021 }, { 1.3891, 0.6013, 2.0922 } };
double B1[2][3] = { {-0.1399,-0.0672,-0.3057 }, {-0.0475,-0.0337,-0.0881 } };
double B2[2][3] = { {-0.0011,-0.0021, 0.0019 }, {-0.0220,-0.0224,-0.0233 } };
double B3[2][3] = { { 0.5209, 0.3314,-0.0325 }, { 0.3627, 0.7745, 0.0351 } };

double EqMMI(unsigned int Eq, unsigned int group, double D, double Dp, double Ms){
//	double B0= 1.1090;
//	double B1=-0.1399;
//	double B2=-0.0011;
//	double B3=0.5209;
	if( Eq == 2 )
		return exp( B0[0][group-1] + B1[0][group-1]*log( D/Dp ) + B2[0][group-1]*( (D-Dp)/1000.0 ) + B3[0][group-1]*log(Ms) );
	if( Eq == 3 )
		return exp( B0[1][group-1] + B1[1][group-1]*( D/Dp ) + B2[1][group-1]*log( (D-Dp)/1000.0 ) + B3[1][group-1]*log(Ms) );
	return 0;
}

double dEqMMI_dD(unsigned int Eq, unsigned int group, double D, double Dp, double Ms){
//	double B0= 1.1090;
//	double B1=-0.1399;
//	double B2=-0.0011;
//	double B3=0.5209;
	if( Eq == 2 )
		return exp( B0[0][group-1] + B1[0][group-1]*log( D/Dp ) + B2[0][group-1]*( (D-Dp)/1000.0 ) + B3[0][group-1]*log(Ms) )*( B1[0][group-1]/D + B2[0][group-1]/1000.0 );
	if( Eq == 3 )
		return exp( B0[1][group-1] + B1[1][group-1]*( D/Dp ) + B2[1][group-1]*log( (D-Dp)/1000.0 ) + B3[1][group-1]*log(Ms) )*( B1[1][group-1]/Dp + B2[1][group-1]/(D-Dp));
	return 0;
}

inputData infoData;
ImgT img;
//vector< vector<Coord> > coords;
//vector< vector< vector<Coord> > > isovel; //isovel coords coord
//vector< Color > color;

struct curve{
	vector< vector<Coord> > iso;
	Color color;
};

vector< curve > isovel;

struct points{
	double lat;
	double lon;
	string name;
	points(string name, double latitude, double longitude){
		lat=latitude;
		lon=longitude;
		this->name=name;
	}
	points(){
		lat=0.0;
		lon=0.0;
		name="";
	}
};

points origen;

template<typename T>
struct data2D{
	vector<T> data;
	int Nx,Ny;
	T max = -std::numeric_limits<T>::max();
	T min = std::numeric_limits<T>::max();

	data2D(int Nx, int Ny):Nx(Nx),Ny(Ny){
		data.resize( Nx*Ny );
	}

	data2D(int Nx, int Ny, T ini):Nx(Nx),Ny(Ny){
		data.assign( Nx*Ny, ini );
	}

	void set(int i, int j, T val){
		data[i + Nx * j]=val;
	}

	T get(int i, int j){
		return data[i + Nx * j];
	}

	T Max(){
		if(min<max)return max;
		find_minmax();
		return max;
	}

	T Min(){
		if(min<max)return min;
		find_minmax();
		return min;
	}

	void find_minmax(){
		auto it_min = min_element(begin(data), end(data));
		auto it_max = max_element(begin(data), end(data));
		min=*it_min;
		max=*it_max;
	}
};


bool compara0(points A,points B) { return ( (A.lat-origen.lat)*(A.lat-origen.lat)+(A.lon-origen.lon)*(A.lon-origen.lon) < (B.lat-origen.lat)*(B.lat-origen.lat)+(B.lon-origen.lon)*(B.lon-origen.lon) ); }

int calculaCifras(int num){
	int contador=1;
	while(num/10>0)
	{
		num=num/10;
		contador++;
	}
	return contador;
}


std::vector<float> readFile(const char* filename)
{
	// open the file:
	std::streampos fileSize;
	std::ifstream file(filename, std::ios::binary);

	// get its size:
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	cout<<"FILE SIZE: "<<fileSize<<endl;

	// read the data:
	std::vector<float> fileData(fileSize);
	file.read((char*) &fileData[0], fileSize);
	return fileData;
}
//void pushColor(uint8_t r,uint8_t g,uint8_t b){
//	lutColor.push_back(r);
//	lutColor.push_back(g);
//	lutColor.push_back(b);
//}

//void colorMap(double lambda, uint8_t &r, uint8_t &g, uint8_t &b){
//	//double lambda=(val-minimumValue)/(maximumValue-minimumValue);

//	if(lambda>1.0)lambda=1.0;
//	if(lambda<0.0)lambda=0.0;
//	int size=lutColor.size()/3;
//	double f=lambda*(size-1);
//	double indice=floor(f);
//	double delta=f-indice;
//	r = lutColor[3*indice] + delta*( lutColor[3*(indice+1)]-lutColor[3*indice] );
//	g = lutColor[3*indice+1] + delta*( lutColor[3*(indice+1)+1]-lutColor[3*indice+1] );
//	b = lutColor[3*indice+2] + delta*( lutColor[3*(indice+1)+2]-lutColor[3*indice+2] );
//	//cout<< lambda <<" "<<r<<" "<<g<<" "<<b<<endl;
//}

Coord interpola(double alpha, double beta, Coord a, Coord b, Coord c, Coord d){
	Coord ab=a+alpha*(b-a);
	Coord dc=d+alpha*(c-d);
	return ab+beta*(dc-ab);
}
//agregando función readSlip
struct dataSlip{
	int Nx;
	int Nz;
	double LAT;
	double LON;
	double STRK;
};

dataSlip ReadSlip(vector< vector< pair<Coord,float> > > &coordInvSlip, string filename="complete_inversion.fsp"){
	ifstream inFile;
	dataSlip data;
	inFile.open(filename, std::ifstream::in);
	if (inFile.is_open()){
		cout<<"Abriendo complete_inversion.fsp"<<endl;
		ReadCompleteInversion Ext(inFile);
		int Nx,Nz;
		double LAT_centro, LON_centro, STRK;
		if( Ext.get("Nx",Nx) && Ext.get("Nz",Nz) && Ext.get("LAT",LAT_centro) && Ext.get("LON",LON_centro)  && Ext.get("STRK",STRK)){
			cout<<"Nx:"<<Nx<<" Nz:"<<Nz<<" LAT:"<<LAT_centro<<" LON:"<<LON_centro<<endl;
			data.Nx=Nx;
			data.Nz=Nz;
			data.LAT=LAT_centro;
			data.LON=LON_centro;
			data.STRK=STRK;
			float LAT,LON,X,Y,Z,SLIP,RAKE,TRUP,RISE,SF_MOMENT;
			float max=-std::numeric_limits<float>::max();
			float min=std::numeric_limits<float>::max();
			vector<float> u(Nx*Nz);
			int i=0,j=0;
			int k=0;
			//vector< vector< pair<Coord,float> > > coordInvSlip(Nz);
			coordInvSlip.resize(Nz);
			for( int j=0; j<Nz; j++){
				vector< pair<Coord,float> > row(Nx);
				for( int i=0; i<Nx; i++){
					string line;
					getline(inFile,line);
					stringstream ssline(line);
					if(! (ssline>>LAT>>LON>>X>>Y>>Z>>SLIP>>RAKE>>TRUP>>RISE>>SF_MOMENT) ){
						cout<<"Se esperaba un parámetro más en la lectura"<<endl;
					}
					row[i]=pair<Coord, float>( Coord( LON, LAT ), SLIP );
				}
				coordInvSlip[j]=row;
			}
		}
	}else{
		cout<<"No se encontro el archivo: complete_inversion.fsp"<<endl;
	}

	return data;
}

void slip2(SVG2D &svg, vector< vector< pair<Coord,float> > > coordInvSlip, bool addColor=true){
	float strokeWidth=0.005*T().textHeight;
	int J1, J2, J3, J4;
	int I1, I2, I3, I4;
	double beta1, beta2, beta3, beta4;
	double alpha1, alpha2, alpha3, alpha4;
	cout<<"coordInvSlip.size()"<<coordInvSlip.size()<<endl;
	svg.add( Shape().IniGroup("Slip"));
	for(int j=0; j<coordInvSlip.size(); j++ ){
		J1=J2=J3=J4=j;
		beta1=beta2=beta3=beta4=0.5;
		if(j==0){
			J1=J2=j+1;
			beta1=beta2=-0.5;
		}
		if(j==coordInvSlip.size()-1){
			J3=J4=j-1;
			beta3=beta4=1.5;
		}
		for(int i=0; i<coordInvSlip[j].size(); i++ ){
			vector<float> p;
			I1=I2=I3=I4=i;
			alpha1=alpha2=alpha3=alpha4=0.5;
			if(i==0){
				I1=I4=i+1;
				alpha1=alpha4=-0.5;
			}
			if(i==coordInvSlip[j].size()-1){
				I2=I3=i-1;
				alpha2=alpha3=1.5;
			}

			Coord p1=interpola(alpha1,beta1,coordInvSlip[J1-1][I1-1].first,coordInvSlip[J1-1][I1].first,coordInvSlip[J1][I1].first,coordInvSlip[J1][I1-1].first);
			Coord p2=interpola(alpha2,beta2,coordInvSlip[J2-1][I2].first,coordInvSlip[J2-1][I2+1].first,coordInvSlip[J2][I2+1].first,coordInvSlip[J2][I2].first);
			Coord p3=interpola(alpha3,beta3,coordInvSlip[J3][I3].first,coordInvSlip[J3][I3+1].first,coordInvSlip[J3+1][I3+1].first,coordInvSlip[J3+1][I3].first);
			Coord p4=interpola(alpha4,beta4,coordInvSlip[J4][I4-1].first,coordInvSlip[J4][I4].first,coordInvSlip[J4+1][I4].first,coordInvSlip[J4+1][I4-1].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);
			if( addColor ){
				svg.add( Shape().Polygon(p)
				.fill( (int)r, (int)g, (int)b ).fillOpacity(0.5)
				.stroke(0,0,0).strokeWidth(strokeWidth) );
			}else{
				svg.add( Shape().Polygon(p)
				.fillNone()
				.stroke(0,0,0).strokeWidth(strokeWidth) );
			}
		}
	}

	svg.add( Shape().EndGroup());
}

void slip(SVG2D &svg,bool addColor=true, string filename="complete_inversion.fsp"){
	ifstream inFile;
	inFile.open(filename, std::ifstream::in);
	if (inFile.is_open()){
		ReadCompleteInversion Ext(inFile);
		int Nx,Nz;
		if( Ext.get("Nx",Nx) && Ext.get("Nz",Nz) ){
			cout<<"Nx:"<<Nx<<" Nz:"<<Nz<<endl;
			float LAT,LON,X,Y,Z,SLIP,RAKE,TRUP,RISE,SF_MOMENT;
			float max=-std::numeric_limits<float>::max();
			float min=std::numeric_limits<float>::max();
			vector<float> u(Nx*Nz);
			int i=0,j=0;
			int k=0;
			vector< vector< pair<Coord,float> > > coordInvSlip(Nz);
			for( int j=0; j<Nz; j++){
				vector< pair<Coord,float> > row(Nx);
				for( int i=0; i<Nx; i++){
					string line;
					getline(inFile,line);
					stringstream ssline(line);
					if(! (ssline>>LAT>>LON>>X>>Y>>Z>>SLIP>>RAKE>>TRUP>>RISE>>SF_MOMENT) ){
						cout<<"Se esperaba un parámetro más en la lectura"<<endl;
					}
					row[i]=pair<Coord, float>( Coord( LON, LAT ), SLIP );
				}
				coordInvSlip[j]=row;
			}
//			for(const auto& row: coordInvSlip){
//				for(const auto& col: row){
//					cout<<"Coord:"<<col.first.lon<<", "<<col.first.lat<<" ,"<<col.second<<endl;
//				}
//			}
			float strokeWidth=0.005*T().textHeight;
			int J1, J2, J3, J4;
			int I1, I2, I3, I4;
			double beta1, beta2, beta3, beta4;
			double alpha1, alpha2, alpha3, alpha4;
			for(int j=0; j<coordInvSlip.size(); j++ ){
				J1=J2=J3=J4=j;
				beta1=beta2=beta3=beta4=0.5;
				if(j==0){
					J1=J2=j+1;
					beta1=beta2=-0.5;
				}
				if(j==coordInvSlip.size()-1){
					J3=J4=j-1;
					beta3=beta4=1.5;
				}
				for(int i=0; i<coordInvSlip[j].size(); i++ ){
					vector<float> p;
					I1=I2=I3=I4=i;
					alpha1=alpha2=alpha3=alpha4=0.5;
					if(i==0){
						I1=I4=i+1;
						alpha1=alpha4=-0.5;
					}
					if(i==coordInvSlip[j].size()-1){
						I2=I3=i-1;
						alpha2=alpha3=1.5;
					}

					Coord p1=interpola(alpha1,beta1,coordInvSlip[J1-1][I1-1].first,coordInvSlip[J1-1][I1].first,coordInvSlip[J1][I1].first,coordInvSlip[J1][I1-1].first);
					Coord p2=interpola(alpha2,beta2,coordInvSlip[J2-1][I2].first,coordInvSlip[J2-1][I2+1].first,coordInvSlip[J2][I2+1].first,coordInvSlip[J2][I2].first);
					Coord p3=interpola(alpha3,beta3,coordInvSlip[J3][I3].first,coordInvSlip[J3][I3+1].first,coordInvSlip[J3+1][I3+1].first,coordInvSlip[J3+1][I3].first);
					Coord p4=interpola(alpha4,beta4,coordInvSlip[J4][I4-1].first,coordInvSlip[J4][I4].first,coordInvSlip[J4+1][I4].first,coordInvSlip[J4+1][I4-1].first);
					p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
					p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
					p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
					p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
					uint8_t r, g, b;
					double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
					colorMap3.getColor(f, r, g, b);
					if( addColor ){
						svg.add( Shape().Polygon(p)
						.fill( (int)r, (int)g, (int)b ).fillOpacity(0.5)
						.stroke(0,0,0).strokeWidth(strokeWidth) );
					}else{
						svg.add( Shape().Polygon(p)
						.fillNone()
						.stroke(0,0,0).strokeWidth(strokeWidth) );
					}
				}
			}
		}
	}else{
		cout<<"No se encontro el archivo: complete_inversion.fsp"<<endl;
	}
}

//struct scells{
//	Color color;
//	vector< vector<Coord> > isocurve;
//}

//vector< vector< vector<Coord> > >
//void isoValue(vector<float> &value, int NX, int NY){
vector< curve > isoValue(data2D<float> &value){
//vector< vector< vector<Coord> > > isoValue(data2D<float> &value){
	//vector< vector< vector<Coord> > > isovel; //isovel coords coord
	//vector< Color > color;
	vector<curve> cs;

	float velMaxZ = -std::numeric_limits<float>::max();
	float velMinZ =  std::numeric_limits<float>::max();
	for (int i = 0; i < value.data.size(); i++){
		float v = fabs( value.data[i] );
		if( velMaxZ < v ) velMaxZ = v;
		if( velMinZ > v ) velMinZ = v;
	}



	NiceScale nsMax(0,velMaxZ);


	cout<<"velMinZ="<<velMinZ<<endl;
	cout<<"velMaxZ="<<velMaxZ<<endl;

	int CX=value.Nx-1;
	int CY=value.Ny-1;
	//vector<unsigned char> caseCorners( CX*CY,0 );
	//cout<<"**** xy(0,0)->"<<img.xy(0,0).lon<<" "<<img.xy(0,0).lat<<endl;
	//cout<<"**** xy(100,100)->"<<img.xy(100,100).lon<<" "<<img.xy(100,100).lat<<endl;

	NiceScale nsThresholdMax(0,velMaxZ,20);
	int N=nsThresholdMax.N();
	for ( int k=1; k<N-1; k++ ){
		float valThreshold=nsThresholdMax.niceMin + (float)k*nsThresholdMax.tickSpacing;
		//vector<float> valsThreshold{ 0.005, 0.010, 0.015, 0.020, 0.025, 0.030, 0.035, 0.040, 0.045, 0.050};
		//for ( auto valThreshold: valsThreshold ){
		//vector< vector<Coord> > isovalue;
		curve isovalue;
		for (int j = 0; j < CY; j++){
			for (int i = 0; i < CX; i++){
				int i0 = i + value.Nx*j;
				int i1 = i+1 + value.Nx*j;
				int i2 = i+1 + value.Nx*(j+1);
				int i3 = i + value.Nx*(j+1);
				unsigned char valKey=0;
				if( value.data[i0]>valThreshold )valKey|=0x01;
				if( value.data[i1]>valThreshold )valKey|=0x02;
				if( value.data[i2]>valThreshold )valKey|=0x04;
				if( value.data[i3]>valThreshold )valKey|=0x08;
				//			float v = valmax[index];
				//			if( velMaxZ < v ) velMaxZ = v;

				//cout<<hex<<(int)valKey<<" ";
				vector<Coord> lines;
				float x,y,p;
				switch(valKey){
				case 1:
					//				cout<<"case1"<<endl;
					//				cout<<"i1="<<valmax[i1]<<" thr="<<valThreshold<<" i0="<<valmax[i0]<<endl;
					//				cout<<"i3="<<valmax[i3]<<" thr="<<valThreshold<<" i0="<<valmax[i0]<<endl;
				case 14:
					x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
					y=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
					lines.push_back(img.xy(i+x,j));
					lines.push_back(img.xy(i,j+y));
					break;
				case 2:
				case 13:
					x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
					y=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
					lines.push_back(img.xy(i+x, j));
					lines.push_back(img.xy(i+1, j+y));
					break;
				case 3:
				case 12:
					x=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
					y=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
					lines.push_back(img.xy(i,j+x));
					lines.push_back(img.xy(i+1,j+y));
					break;
				case 4:
				case 11:
					x=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
					y=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
					lines.push_back(img.xy(i+1, j+x));
					lines.push_back(img.xy(i+y, j+1));
					break;
				case 5:
				case 10:
					p=(value.data[i0]+value.data[i1]+value.data[i2]+value.data[i3])/4.0;
					if(fabs(value.data[i0]-p)+fabs(value.data[i2]-p)<fabs(value.data[i1]-p)+fabs(value.data[i3]-p)){
						//7 y 8
						x=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
						y=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
						lines.push_back(img.xy(i+x, j+1));
						lines.push_back(img.xy(i, j+y));
						//2 y 13
						x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
						y=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
						lines.push_back(img.xy(i+x, j));
						lines.push_back(img.xy(i+1, j+y));
					}
					else{
						//4 y 11
						x=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
						y=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
						lines.push_back(img.xy(i+1, j+x));
						lines.push_back(img.xy(i+y, j+1));
						//1 y 14
						x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
						y=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
						lines.push_back(img.xy(i+x,j));
						lines.push_back(img.xy(i,j+y));
					}
					break;
				case 6:
				case 9:
					x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
					y=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
					lines.push_back(img.xy(i+x, j));
					lines.push_back(img.xy(i+y, j+1));
					break;
				case 7:
				case 8:
					x=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
					y=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
					lines.push_back(img.xy(i+x, j+1));
					lines.push_back(img.xy(i, j+y));
					break;
				}

				if( lines.size()>0 ){
					//cout<<"size line"<<line.size()<<endl;
					isovalue.iso.push_back(lines);
				}
			}
			//cout<<endl;
		}
		Color _color;
		double f = (valThreshold-nsThresholdMax.niceMin)/(nsThresholdMax.niceMax-nsThresholdMax.niceMin);
		colorMap2.getColor(f, _color.r, _color.g, _color.b);
		cout << "valThreshold=" << valThreshold << " Color (" << (int)_color.r << ", " << (int)_color.g << ", " << (int)_color.b << ")" << endl;
		isovalue.color.set(_color);


//		c.color.push_back(_color);
//		c.iso.push_back(isovalue);
		cs.push_back(isovalue);
	}
	//return isovel;
	return cs;
}

template <typename T>
void getdata(ifstream &file, T &x){
	string str;
	getline (file, str);
	stringstream sstr(str);
	sstr>>x;
}

void initialize_visualization_(string path){

#if defined DEBUG
	cout<<"***********initialize_visualization_"<<endl;
#endif
	ifstream file;
	//file.open(path+"/input_vis.dat", std::ifstream::in);
	file.open("input_vis.dat", std::ifstream::in);
	if (!file.is_open()){
		cout<<"No se pudo leer el archivo: "+path+"/input_vis.dat"<<endl;
	}
	else{
		getdata(file,image_width);
		getdata(file,image_height);
		getdata(file,nameFileCSV);
		getdata(file,colLabelFileCSV);
		getdata(file,epi_lon);
		getdata(file,epi_lat);
		getdata(file,VELXMIN);
		getdata(file,VELXMAX);
		//getdata(file,theta);
#if defined DEBUG        
		cout<<"tamaña de imagenes: "<<image_width<<"x"<<image_height<<endl;
		cout<<"nameFileCSV: "<<nameFileCSV<<endl;
		cout<<"colLabelFileCSV: "<<colLabelFileCSV<<endl;
		cout<<"epi: ("<<epi_lon<<","<<epi_lat<<")"<<endl;
		cout<<"VelX: "<<VELXMIN<<" "<<VELXMAX<<endl;
#endif
	}

}


void transformation_settings_( double DH, int NXSC, int NYSC, int LX, int LY){
	double epicx=(double)(NXSC+xhip)*(DH);
	double epicy=(double)(NYSC+yhip)*(DH);
	nx=(LX);
	ny=(LY);
	lx=(double)(LX)*(DH);
	ly=(double)(LY)*(DH);

	nxsc=NXSC;
	nysc=NYSC;
	dh=DH;

	double xmin=epi_lon-epicx/latlon2m;
	double xmax=epi_lon+(lx-epicx)/latlon2m;
	double ymin=epi_lat-epicy/latlon2m;
	double ymax=epi_lat+(ly-epicy)/latlon2m;

	cout<<"*******************"<<endl;
	cout<<"****** ImgT *******"<<endl;
	img=ImgT(xmin, xmax, ymin, ymax, nx, ny);
	cout<<"*******************"<<endl;

	cout<<"xmin="<<xmin<<" xmax="<<xmax<<endl;
	cout<<"ymin="<<ymin<<" ymax="<<ymax<<endl;

	double x=xmin;
	double y=ymin;

	//#if defined DEBUG
	cout<<"*****transformation_settings*****"<<endl;
	cout<<"DH="<<DH<<" LX="<<LX<<" LY="<<LY<<endl;
	cout<<"NXSC="<<NXSC<<" NYSC="<<NYSC<<endl;
	cout<<"x_hip="<<xhip<<" y_hip="<<yhip<<endl;
	cout<<"epicx="<<epicx<<" epicy="<<epicy<<endl;
	cout<<"Epicentro: ("<<epi_lon<<" ,"<<epi_lat<<")"<<endl;
	cout<<"Imagen tamaño: ("<<image_width<<"x"<<image_height<<") "<<lx<<" "<<ly<<endl;
	//cout<<"xmin"<<xmin<<"xmax"<<xmax<<endl;
	//cout<<"ymin"<<ymin<<"ymax"<<ymax<<endl;

	cout<<"**********"<<endl;


	//#endif


	rot(theta,epi_lon,epi_lat,x,y);
	BB(x,y);
	x=xmax;
	y=ymin;
	rot(theta,epi_lon,epi_lat,x,y);
	BB(x,y);
	x=xmin;
	y=ymax;
	rot(theta,epi_lon,epi_lat,x,y);
	BB(x,y);
	x=xmax;
	y=ymax;
	rot(theta,epi_lon,epi_lat,x,y);
	BB(x,y);

	NiceScale ns_lat(BB().ymin(),BB().ymax(),12);
	NiceScale we_long(BB().xmin(),BB().xmax(),12);
	double dmax=0,dwest,deast,dsouth,dnorth;

	double xc=(we_long.niceMin+we_long.niceMax)/2.0;
	double yc=(ns_lat.niceMin+ns_lat.niceMax)/2.0;
	dwest=xc-we_long.niceMin;
	deast=we_long.niceMax-xc;
	dsouth=yc-ns_lat.niceMin;
	dnorth=ns_lat.niceMax-yc;

	dmax=(dwest>dmax)?dwest:dmax;
	dmax=(deast>dmax)?deast:dmax;
	dmax=(dsouth>dmax)?dsouth:dmax;
	dmax=(dnorth>dmax)?dnorth:dmax;

	double lambda=1.2;
	double west, east, south, north;
	if(image_height>image_width){
		double r=(double)image_height/(double)image_width;
		textHeight=lambda*dmax*r;
		west=xc-lambda*dmax;
		east=xc+lambda*dmax;
		south=yc-lambda*dmax*r;
		north=yc+lambda*dmax*r;
	}
	else{
		double r=(double)image_width/(double)image_height;
		textHeight=lambda*dmax*r;
		west=xc-lambda*dmax*r;
		east=xc+lambda*dmax*r;
		south=yc-lambda*dmax;
		north=yc+lambda*dmax;
	}

	cout<<"west="<<west<<" east="<<east<<endl;

	T( image_width, image_height,
	   west,east,south,north);
}

void make_map_(){
	SVG2D svgmap("./imagenes/mapa.svg",T().width,T().height,"mapa");
	svgmap.add( Shape().ShapeFile(nameFileCSV,colLabelFileCSV,0.003*T().textHeight) );//pone nombre de estados
	//svgmap.add( Shape().ShapeFile(nameFileCSV) );//no pone nombre de estados
#if defined DEBUG
	cout<<"*****make_map*****"<<endl;
	cout<<"Se agrego Mapa"<<endl;
	cout<<"nameFileCSV: "<<nameFileCSV<<endl;
	cout<<"colLabelFileCSV: "<<colLabelFileCSV<<endl;
	cout<<"**********"<<endl;
#endif
}

/*
void holes_(int *nbhx, int *nbhy, int *nholes){
	NBHX=nbhx;
	NBHY=nbhy;
	NHOLES=*nholes;

	int image_width=T().width;
	int image_height=T().height;

#if defined DEBUG
	cout<<"Holes"<<endl;
	for(int i=0;i<NHOLES;i++){
		cout<<i<<" "<<NBHX[i]<<" "<<NBHY[i]<<endl;
	}
	cout<<"Creando archivo Holes"<<endl;
#endif

	SVG2D svg("./imagenes/holes.svg",T().width,T().height);
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );
	svg.add(Shape().Rectangle(T().x(cell2geo_lon(0)),T().y(cell2geo_lat(ny)),T().sx(nx*dh/latlon2m),T().sy(ny*dh/latlon2m)).fill(255,255,255) );
	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Circle(T().x(epi_lon),T().y(epi_lat),T().pointHeight)
			 .fill(255,0,0).stroke(0,0,0).strokeWidth(T().sx(0.02)) );
	svg.add( Shape().Text(T().x(epi_lon),T().y(epi_lat)-T().pointHeight,"Epic")
			 .align("middle").fontSize(T().textHeight).fontFamily("Times").opacity(0.8) );

	for(int i=0;i<NHOLES;i++){
		double Tx=T().x(cell2geo_lon(NBHX[i]));
		double Ty=T().y(cell2geo_lat(NBHY[i]));
		svg.add( Shape().Circle(Tx,Ty,0.5*T().pointHeight)
				 .fill(128,128,255).stroke(0,0,0).strokeWidth(T().sx(0.02)).opacity(0.8) );
		stringstream ssindex;
		ssindex << i+1 ;
		svg.add(Shape().Text( Tx, Ty-0.5*T().pointHeight, ssindex.str())
				.align( "middle" ).fontSize(0.25*T().textHeight)
				.fontFamily( "Times" ).opacity(0.8) );
	}
	svg.add( Shape().Mesh(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()) );
#if defined DEBUG
	cout<<"Holes ok"<<endl;
#endif

}
*/



//void make_svg_(int *ntst, double *dt, int *nbgxs, int *nedxs, int *nbgys, int *nedys, int *nbgzs, int *nedzs, int *size,int *layerGlobal){
void make_svg_(int NTST, string nameSVG, string namePNG, double vmin, double vmax){

	//int NTST=*ntst;
	//double DT=*dt;
	//int SIZE=*size;
	//int LAYERGLOBAL=*layerGlobal;
	/*
	cout<<"C++:"<<endl;
	for(int i=0;i<SIZE;i++){
		cout<<i<<" x("<<nbgxs[i]<<","<<nedxs[i]<<") y("<<nbgys[i]<<","<<nedys[i]<<") z("<<nbgzs[i]<<","<<nedzs[i]<<") ";
	}
	cout<<endl;
	*/
	//stringstream nameSVG_Z;

	//nameSVG_Z<<"./imagenes/";
	//nameSVG_Z<<"imagen"<<setw(ndigTI)<<setfill('0')<<NTST;
	//nameSVG_Z<<".svg";



	int image_width=T().width;
	int image_height=T().height;

	vector< vector< pair<Coord,float> > > coordInvSlip;
	dataSlip dSlip=ReadSlip(coordInvSlip);


	//SVG2D svg(nameSVG_Z.str(),image_width,image_height);
	SVG2D svg(nameSVG,image_width,image_height);
	svg.add(Shape().ColorMap(colorMap1,"ColorMap"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );


	double nbgx=infoData.NBGX()-1;
	double nbgy=infoData.NBGY()-1;
	double nLx=infoData.NEDX()-infoData.NBGX()+1;
	double nLy=infoData.NEDY()-infoData.NBGY()+1;

//	double Tx=T().x(epi_lon+(x-infoData.NXSC()-xhip)*dh/latlon2m);
//	double Ty=T().y(epi_lat+(y-infoData.NYSC()-yhip+dy)*dh/latlon2m);
	//double iniSlip_lat=coordInvSlip[0][dSlip.Nx-1].first.lat;
	//double iniSlip_lon=coordInvSlip[0][dSlip.Nx-1].first.lon;
//	double iniSlip_lat=coordInvSlip[0][0].first.lat;
//	double iniSlip_lon=coordInvSlip[0][0].first.lon;

	double maxSlip=-std::numeric_limits<double>::max();
	double maxSlipLAT;
	double maxSlipLON;
	for (const auto& row : coordInvSlip){
		for (const auto& p : row){
			if(maxSlip<=p.second){
				maxSlip=p.second;
				maxSlipLAT=p.first.lat;
				maxSlipLON=p.first.lon;
			}
		}
	}
	double iniSlip_lat=maxSlipLAT;
	double iniSlip_lon=maxSlipLON;
//	double iniSlip_lat=coordInvSlip[0][(dSlip.Nx-1)/2].first.lat;
//	double iniSlip_lon=coordInvSlip[0][(dSlip.Nx-1)/2].first.lon;

	double Tx=T().x(iniSlip_lon+(nbgx-infoData.NXSC())*dh/latlon2m);
	double Ty=T().y(iniSlip_lat+(nbgy-infoData.NYSC()+nLy)*dh/latlon2m);

	double Tdx=T().sx(nLx*dh/latlon2m);
	double Tdy=T().sy(nLy*dh/latlon2m);

	double epi_x=T().x(epi_lon);
	double epi_y=T().y(epi_lat);


	double rotx=T().x(iniSlip_lon)-Tx;
	double roty=T().y(iniSlip_lat)-Ty;




	//svg.add(Shape().Image(Tx,Ty,Tdx,Tdy,namePNG,0,0,0));
	svg.add(Shape().Image(Tx+rotx,Ty+roty,Tdx,Tdy,namePNG,rotx,roty,theta));

	//svg.add( Shape().Circle(Tx,Ty,1.2*T().pointHeight).fill(255,0,255).stroke(128,0,128).strokeWidth(0.2*T().textHeight).opacity(0.9) );
	//svg.add( Shape().Circle(Tx+infoData.NXSC(),Ty+infoData.NYSC(),0.6*T().pointHeight).fill(255,0,255).opacity(0.7) );

	//svg.add( Shape().Circle(Tx+infoData.NXSC(),Ty+infoData.NYSC(),0.6*T().pointHeight).fill(255,0,255).opacity(0.7) );
	//svg.add( Shape().Circle(T().x(coordInvSlip[0][0].first.lon),T().y(coordInvSlip[0][0].first.lat),0.6*T().pointHeight).fill(0,0,0).opacity(0.7) );
	//svg.add( Shape().Circle(T().x(coordInvSlip[0][dSlip.Nx-1].first.lon),T().y(coordInvSlip[0][dSlip.Nx-1].first.lat),0.6*T().pointHeight).fill(0,0,0).opacity(0.7) );
	//svg.add( Shape().Circle(T().x(epi_lon),T().y(epi_lat),0.6*T().pointHeight).fill(255,255,0).stroke(128,128,0).strokeWidth(0.1*T().textHeight).opacity(1) );



	//slip(svg);

	slip2(svg, coordInvSlip);

	//svg.add(Shape().Image(Tx,Ty,Tdx,Tdy,namePNG,epi_x,epi_y,-22));

	//svg.add(Shape().Image(Tx+(Tx-epi_x),Ty+(Ty-Tdy+epi_y),Tdx,Tdy,namePNG,(Tx-epi_x),(Ty-Tdy+epi_y),45));
	//svg.add(Shape().Image(Tx+epi_x,Ty-Tdy+epi_y,Tdx,Tdy,namePNG,epi_x,epi_y,45));

	//svg.add( Shape().Circle(Tx+rotx,Ty+roty,T().pointHeight)
	//         .fill(0,0,255).stroke(0,0,0).strokeWidth(T().sx(0.002)) );


	//cout<<BB().xmax()-BB().xmin()<<endl;
	//cout<<BB().ymax()-BB().ymin()<<endl;
	//svg.add(Shape().Image(T().x(BB().xmin()),T().y(BB().ymax()),T().sx(BB().xmax()-BB().xmin()),T().sy(BB().ymax()-BB().ymin()),namePNG,0,0,0));
	//svg.add(Shape().Image(T().x(BB().xmin()),T().y(BB().ymax()),T().sx(4),T().sy(4),namePNG,0,0,0));//-22
	/*
	for(int id=0; id<SIZE; id++){
		//cout<<"rank:"<<id<<" "<<nbgzs[id]<<" "<<LAYERGLOBAL<<" "<<nedzs[id]<<endl;
		if( nbgzs[id]<=LAYERGLOBAL  && LAYERGLOBAL<=nedzs[id] ){
			stringstream nameFile;
			//nameFile<<"prueba"<<setw(ndigID)<<setfill('0')<<id<<setw(ndigTI)<<setfill('0')<<NTST<<"_"<<setw(5)<<setfill('0')<<LAYERGLOBAL<<".ppm";
			nameFile << "prueba";
			nameFile << setw(ndigID) << setfill('0') << id;
			nameFile << setw(ndigTI) << setfill('0') << NTST;
			nameFile << ".ppm";
			//cout<<nameFile.str()<<endl;
			double x=nbgxs[id]-1;
			double y=nbgys[id]-1;
			double dx=nedxs[id]-nbgxs[id]+1;
			double dy=nedys[id]-nbgys[id]+1;

			double Tx=T().x(epi_lon+(x-nxsc)*dh/latlon2m);
			double Ty=T().y(epi_lat+(y-nysc+dy)*dh/latlon2m);
			double Tdx=T().sx(dx*dh/latlon2m);
			double Tdy=T().sy(dy*dh/latlon2m);

			svg.add(Shape().Image(Tx,Ty,Tdx,Tdy,
								  nameFile.str(),0,0,0));
			stringstream ssrank;
			ssrank << id ;
			svg.add(Shape().Text( Tx+0.5*Tdx, Ty+0.5*Tdy, ssrank.str())
					.align( "middle" ).fontSize(2.0*T().textHeight)
					.fontFamily( "Times" ).opacity(0.1) );
		}
	}
*/    
	//svg.add( Shape().Use("mapa.svg","mapa") );   // commented out by CAFE on 05042021
	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Mask(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()).fill(192, 192, 210) );

	if(NTST>=0.0){
		stringstream txt_time;
		txt_time << "Time:";
		//*dt
		//cout<<"sigfigs(dt)="<<sigfigs(dt)<<" dt="<<dt<<endl;
		txt_time << fixed << setprecision(digTime) << NTST*dt << "s";
		svg.add(Shape().Text( image_width/2.0,image_height/20.0, txt_time.str())
				.align( "middle" ).fontSize(2.0*T().textHeight)
				.fontFamily( "Times" ) );
	}

	double colorBarX=21.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=1.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;

#if defined DEBUG
	//cout<<"VelX: "<<VELXMIN<<" "<<VELXMAX<<endl;
	cout<<"VelX: "<<vmin<<" "<<vmax<<endl;
#endif


	//	for(const vector<vector<Coord>>& iso: isovel ){
	//		for(const vector<Coord>& vs: iso ){
	//			for(int i=0; i<vs.size()-1; i+=2){
	//				svg.add( Shape().Line( T().x(vs[i].lon), T().y(vs[i].lat), T().x(vs[i+1].lon), T().y(vs[i+1].lat) ).stroke( 128, 128, 128).strokeWidth(3.0).strokeLinecap("round") );
	//			}
	//		}
	//	}

//	for(int k=0; k<isovel.size(); k++ ){
//		for(const vector<Coord>& vs: isovel[k].iso ){
//			for(int i=0; i<vs.size()-1; i+=2){
//				svg.add( Shape().Line( T().x(vs[i].lon), T().y(vs[i].lat), T().x(vs[i+1].lon), T().y(vs[i+1].lat) ).stroke( 0.7*isovel[k].color.r, 0.7*isovel[k].color.g, 0.7*isovel[k].color.b).strokeWidth(1.0).strokeLinecap("round") );
//			}
//		}
//	}

	svg.add(Shape().ColorBar(colorBarX, colorBarY,
							 colorBarWidth, colorBarHeight,"ColorMap",
							 vmin, vmax, "Vx (m/s)") );

	//	svg.add( Shape().Circle(T().x(epi_lon),T().y(epi_lat),T().pointHeight)
	//			 .fill(255,0,0).stroke(0,0,0).strokeWidth(0.1*T().textHeight) );
	//	svg.add( Shape().Text(T().x(epi_lon),T().y(epi_lat)-2*T().pointHeight,"Epic")
	//			 .align("middle").fontSize(T().textHeight).fontFamily("Times").opacity(0.8) );

	vector<float> polygon;
	int Npol=3;
	for(int i=0; i<Npol; i++){
		double alpha=360*i/Npol;
		double x = epi_lon;
		double y = epi_lat+0.2*T().inv_sy( T().pointHeight );
		rot(alpha,epi_lon, epi_lat, x, y);
		polygon.push_back( T().x(x) );
		polygon.push_back( T().y(y) );

		//		cout<<"alpha="<<alpha<<" ("<<x<<", "<<y<<") "<< T().pointHeight << endl;
	}
	svg.add( Shape().Polygon(polygon).fill(255,0,0).stroke(0,0,0).strokeWidth(0.01*T().textHeight).opacity(0.8) );


	//	svg.add( Shape().Text(T().x(epi_lon),T().y(epi_lat)-2*T().pointHeight,"Epic")
	//			 .align("middle").fontSize(T().textHeight).fontFamily("Times").opacity(0.8) );

	for(int i=0; i<infoData.HOLE_N(); i++){
		//double x=T().x(epi_lon+(x-infoData.HOLE_X(i))*dh/latlon2m);
		//double y=T().y(epi_lat+(y-infoData.HOLE_Y(i)+dy)*dh/latlon2m);
		double x=Tx+T().sx(infoData.HOLE_X(i)*dh/latlon2m);
		double y=Ty+Tdy-T().sy(infoData.HOLE_Y(i)*dh/latlon2m);
		//svg.add( Shape().Circle(x,y,0.6*T().pointHeight).fill(0,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
		stringstream coordEst;
		coordEst<<"("<<infoData.HOLE_X(i)<<", "<<infoData.HOLE_Y(i)<<")";
		//svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() ).align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );


//		svg.add( Shape().Text(x,y-1.5*T().pointHeight,to_string(i+1))
//				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
		//cout<<"hole["<<i<<"]=("<<infoData.HOLE_X(i)<<","<<infoData.HOLE_Y(i)<<")=("<<x<<","<<y<<")"<<endl;

	}

	//	vector<Coord> estaciones={
	//		Coord(-93.94, 14.94),
	//		Coord(-96.49, 15.67),
	//		Coord(-92.27, 14.92),
	//		Coord(-96.7, 17.07),
	//		Coord(-96.72, 17.07),
	//		Coord(-94.54, 17.99),
	//		Coord(-98.17, 19.04),
	//		Coord(-98.56, 17.8),
	//		Coord(-96.9, 19.53),
	//		Coord(-100.09, 17),
	//		Coord(-99.89, 16.87),
	//		Coord(-100.16, 18.9),
	//		Coord(-100.43, 17.21),
	//		Coord(-99.82, 16.91),
	//		Coord(-100.72, 20.04),
	//		Coord(-101.26, 17.54),
	//		Coord(-94.88, 17.09),
	//		Coord(-93.12, 16.78),
	//		Coord(-94.42, 18.03),
	//		Coord(-97.36, 18.42),
	//		Coord(-98.63, 19.07),
	//		Coord(-98.13, 16.39),
	//		Coord(-97.8, 17.83),
	//		Coord(-98.57, 17.56),
	//		Coord(-99.62, 17.92),
	//		Coord(-101.46, 17.61),
	//		Coord(-99.04, 20.3)
	//	};

	//	for(const Coord& est: estaciones ){
	//		double x=T().x(est.lon);
	//		double y=T().y(est.lat);
	//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
	//				 .fill(255,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
	//	}


	/*
	//ubicacion plano
	double div=10;
	double _dx=Tdx/div;
	double _dy=Tdy/div;
	for(int j=0;Ty+j*_dy<Ty+Tdy;j++){
		for(int i=0;Tx+i*_dx<Tx+Tdx;i++){
			if((i+j)%2){
				int r=128;
				int g=128;
				int b=255;
				//if( (i%5+ j%5)%2)r=255;
				//else g=255;
				svg.add( Shape().Rectangle(Tx+i*_dx,Ty+j*_dy,_dx,_dy)
				 .fill(r,g,b).opacity(0.8) );
		  }
		}
	  }
	  
	vector<points> pp;
	origen=points("EPI",	16.626, -95.078);//IXTEPEC_OAXACA Sismo23092017
	pp.push_back(origen);

		//pp.emplace_back("EPI",	16.82, -99.78);//Acapulco
	pp.emplace_back("CU",		19.33, -99.18);
		pp.emplace_back("PANG",	15.67,	-96.49);
		pp.emplace_back("OXBJ",	17.07,	-96.72);
		pp.emplace_back("HMTT",	17.80,	-98.56);
		pp.emplace_back("XALA",	19.53,	-96.90);
		//pp.emplace_back("COYC",	17.00,	-100.09);
		//pp.emplace_back("ACP2",	16.87,	-99.89);
		//pp.emplace_back("TEJU",	18.90,	-100.16);
		//pp.emplace_back("ATYC",	17.21,	-100.43);
		//pp.emplace_back("ACAM",	20.04,	-100.72);
		//pp.emplace_back("UNIO",	17.99,	-101.81);
		//pp.emplace_back("URUA",	19.42,	-102.07);
		pp.emplace_back("NILT",	16.57,	-94.62);
		//pp.emplace_back("COMA",	19.33,	-103.76);
		pp.emplace_back("CHPA",	16.25,	-93.91);
		pp.emplace_back("OXLC",	17.07,	-96.70);
		pp.emplace_back("VNTA",	16.91,	-99.82);
		pp.emplace_back("PHPU",	19.04,	-98.17);
		//pp.emplace_back("PET2",	17.54,	-101.26);
		pp.emplace_back("CMIG",	17.09,	-94.88);
		pp.emplace_back("TPIG",	18.42,	-97.36);
		pp.emplace_back("PPIG",	19.07,	-98.63);
		pp.emplace_back("TLIG",	17.56,	-98.57);
		pp.emplace_back("MEIG",	17.92,	-99.62);
		//pp.emplace_back("ZIIG",	17.61,	-101.46);
		pp.emplace_back("DHIG",	20.30,	-99.04);
		//pp.emplace_back("CAIG",	17.05,	-100.27);
		//pp.emplace_back("MMIG",	18.29,	-103.35);
		pp.emplace_back("TUIG",	18.03,	-94.42);
		
		pp.emplace_back("MIHL",	17.99,	-94.54);
		pp.emplace_back("TAJN",	14.92,	-92.27);
		pp.emplace_back("PNIG",	16.39,	-98.13);
		pp.emplace_back("HLIG",	17.83,	-97.8);
		pp.emplace_back("TGIG",	16.78,	-93.12);
		
		sort(pp.begin(),pp.end(),compara0);

	double o_lon=epi_lon+(x-infoData.NXSC()-infoData.xHip())*dh/latlon2m;
	double o_lat=epi_lat+(y-infoData.NYSC()-infoData.yHip())*dh/latlon2m;

	svg.add( Shape().Circle(T().x(o_lon),T().y(o_lat),0.6*T().pointHeight)
			 .fill(0,255,0).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );

	for(int i=0;i<pp.size();i++){
		stringstream label;
		stringstream id;
		int xcel=round((pp[i].lon-o_lon)*latlon2m/1000.0);
		int ycel=round((pp[i].lat-o_lat)*latlon2m/1000.0);
		label<<pp[i].name<<" ("<<xcel<<","<<ycel<<")";
		id<<i+1;
		cout<<id.str()<<" "<<label.str()<<" ("<<setprecision(2)<<std::setfill( '0' )<<pp[i].lon<<","<<pp[i].lat<<")"<<endl;
		svg.add( Shape().Circle(T().x(pp[i].lon),T().y(pp[i].lat),3.5*T().pointHeight)
			 .fill(255,0,0).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.5) );
	  svg.add( Shape().Text(T().x(pp[i].lon),T().y(pp[i].lat)+2.0*T().pointHeight,id.str())
			 .align("middle").fontSize(1.5*T().textHeight).fontFamily("Times").stroke(255,200,200).strokeWidth(0.1*T().textHeight).opacity(0.5) );
	  svg.add( Shape().Text(T().x(pp[i].lon),T().y(pp[i].lat)+2.0*T().pointHeight,id.str())
			 .align("middle").fontSize(1.5*T().textHeight).fontFamily("Times").opacity(0.8) );
		svg.add( Shape().Text(T().x(pp[i].lon),T().y(pp[i].lat)-1.5*T().pointHeight,label.str())
			 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );

	}
	*/
	//ubicacion plano




	//svg.add( Shape().Circle(T().x(-101.0),T().y(10.0),T().pointHeight)
	//         .fill(0,255,0).stroke(0,0,0).strokeWidth(0.1*T().textHeight) );

	/*
	for(int i=0;i<NHOLES;i++){
		double Tx=T().x(cell2geo_lon(NBHX[i]));
		double Ty=T().y(cell2geo_lat(NBHY[i]));
		svg.add( Shape().Circle(Tx,Ty,0.5*T().pointHeight)
				 .fill(128,128,255).stroke(0,0,0).strokeWidth(T().sx(0.02)).opacity(0.8) );
		stringstream ssindex;
		ssindex << i+1 ;
		svg.add(Shape().Text( Tx, Ty-0.5*T().pointHeight, ssindex.str())
				.align( "middle" ).fontSize(0.25*T().textHeight)
				.fontFamily( "Times" ).opacity(0.8) );
	}
	*/

//	for(const auto& est: coordInvSlip ){
//		double x=T().x(est.first.lon);
//		double y=T().y(est.first.lat);
//		svg.add( Shape().Circle(x,y,0.3*T().pointHeight)
//				 .fill(255,213,0).stroke(0,0,0).strokeWidth(0.02*T().textHeight).opacity(0.5) );
//	}

	//svg.add(Shape().Rectangle(T().x(BB().xmax()), 0,image_width,image_height).fill(255, 0, 0) );
	//image_width
	svg.add( Shape().Mesh(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()) );
	//svg.add(Shape().Rectangle(T().x(-99), T().y(12),T().sx(1),T().sy(1)).fill( 0, 255, 0) );


}




void make_svg_max(string nameSVG, string namePNG, double vmin, double vmax){

	int image_width=T().width;
	int image_height=T().height;

	vector< vector< pair<Coord,float> > > coordInvSlip;
	dataSlip dSlip=ReadSlip(coordInvSlip);

	SVG2D svg(nameSVG,image_width,image_height);
	svg.add(Shape().ColorMap(colorMap2,"ColorMap2"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

	double nbgx=infoData.NBGX()-1;
	double nbgy=infoData.NBGY()-1;
	double nLx=infoData.NEDX()-infoData.NBGX()+1;
	double nLy=infoData.NEDY()-infoData.NBGY()+1;

//	double Tx=T().x(epi_lon+(x-infoData.NXSC()-xhip)*dh/latlon2m);
//	double Ty=T().y(epi_lat+(y-infoData.NYSC()-yhip+dy)*dh/latlon2m);
	//double iniSlip_lat=coordInvSlip[0][dSlip.Nx-1].first.lat;
	//double iniSlip_lon=coordInvSlip[0][dSlip.Nx-1].first.lon;
//	double iniSlip_lat=coordInvSlip[0][0].first.lat;
//	double iniSlip_lon=coordInvSlip[0][0].first.lon;

	double maxSlip=-std::numeric_limits<double>::max();
	double maxSlipLAT;
	double maxSlipLON;
	for (const auto& row : coordInvSlip){
		for (const auto& p : row){
			if(maxSlip<=p.second){
				maxSlip=p.second;
				maxSlipLAT=p.first.lat;
				maxSlipLON=p.first.lon;
			}
		}
	}
	double iniSlip_lat=maxSlipLAT;
	double iniSlip_lon=maxSlipLON;
//	double iniSlip_lat=coordInvSlip[0][(dSlip.Nx-1)/2].first.lat;
//	double iniSlip_lon=coordInvSlip[0][(dSlip.Nx-1)/2].first.lon;

	double Tx=T().x(iniSlip_lon+(nbgx-infoData.NXSC())*dh/latlon2m);
	double Ty=T().y(iniSlip_lat+(nbgy-infoData.NYSC()+nLy)*dh/latlon2m);

	double Tdx=T().sx(nLx*dh/latlon2m);
	double Tdy=T().sy(nLy*dh/latlon2m);

	double epi_x=T().x(epi_lon);
	double epi_y=T().y(epi_lat);


	double rotx=T().x(iniSlip_lon)-Tx;
	double roty=T().y(iniSlip_lat)-Ty;




	//svg.add(Shape().Image(Tx,Ty,Tdx,Tdy,namePNG,0,0,0));
	svg.add(Shape().Image(Tx+rotx,Ty+roty,Tdx,Tdy,namePNG,rotx,roty,theta));

//	svg.add( Shape().Circle(Tx,Ty,0.6*T().pointHeight)
//					 .fill(255,0,0).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//	svg.add( Shape().Circle(Tx+Tdx,Ty+Tdy,0.6*T().pointHeight)
//					 .fill(255,0,0).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );

	//slip(svg);
//	vector< vector< pair<Coord,float> > > coordInvSlip;
//	ReadSlip(coordInvSlip);
	slip2(svg, coordInvSlip);





	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Mask(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()).fill(192, 192, 210) );

	double colorBarX=21.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=1.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;


	double lonRot=iniSlip_lon;
	double latRot=iniSlip_lat;
//	svg.add( Shape().Circle(T().x(lonRot),T().y(latRot),0.6*T().pointHeight)
//					 .fill(255,0,0).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );

//	svg.add( Shape().IniGroup("IsocurvaSinRotar"));
//	for(int k=0; k<isovel.size(); k++ ){
//		for(const vector<Coord>& vs: isovel[k].iso ){
//			for(int i=0; i<vs.size()-1; i+=2){
//				double lon1=vs[i].lon;
//				double lat1=vs[i].lat;
//				double lon2=vs[i+1].lon;
//				double lat2=vs[i+1].lat;
//				svg.add( Shape().Line( T().x(lon1), T().y(lat1), T().x(lon2), T().y(lat2) ).stroke(0,255,0).strokeWidth(1.0).strokeLinecap("round") );
//			}
//		}
//	}
//	svg.add( Shape().EndGroup());

	svg.add( Shape().IniGroup("IsocurvasRotadas"));
	for(int k=0; k<isovel.size(); k++ ){
		for(const vector<Coord>& vs: isovel[k].iso ){
			for(int i=0; i<vs.size()-1; i+=2){
				double lon1=vs[i].lon;
				double lat1=vs[i].lat;
				double lon2=vs[i+1].lon;
				double lat2=vs[i+1].lat;

				rot(theta,lonRot, latRot, lon1, lat1);
				rot(theta,lonRot, latRot, lon2, lat2);
				svg.add( Shape().Line( T().x(lon1), T().y(lat1), T().x(lon2), T().y(lat2) ).stroke( 0.7*isovel[k].color.r, 0.7*isovel[k].color.g, 0.7*isovel[k].color.b).strokeWidth(1.0).strokeLinecap("round") );
				//svg.add( Shape().Line( T().x(lon1), T().y(lat1), T().x(lon2), T().y(lat2) ).stroke(255,0,0).strokeWidth(1.0).strokeLinecap("round") );
			}
		}
	}
	svg.add( Shape().EndGroup());

	svg.add(Shape().ColorBar(colorBarX, colorBarY,
							 colorBarWidth, colorBarHeight,"ColorMap2",
							 vmin, vmax, "Vx (m/s)") );


	vector<float> polygon;
	int Npol=3;
	for(int i=0; i<Npol; i++){
		double alpha=360*i/Npol;
		double x = epi_lon;
		double y = epi_lat+0.2*T().inv_sy( T().pointHeight );
		rot(alpha,epi_lon, epi_lat, x, y);
		polygon.push_back( T().x(x) );
		polygon.push_back( T().y(y) );

	}
	svg.add( Shape().Polygon(polygon).fill(255,0,0).stroke(0,0,0).strokeWidth(0.01*T().textHeight).opacity(0.8) );

//	for(int i=0; i<infoData.HOLE_N(); i++){
//		double x=Tx+T().sx(infoData.HOLE_X(i)*dh/latlon2m);
//		double y=Ty+Tdy-T().sy(infoData.HOLE_Y(i)*dh/latlon2m);
//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
//				 .fill(0,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//		stringstream coordEst;
//		coordEst<<i<<" ("<<infoData.HOLE_X(i)<<", "<<infoData.HOLE_Y(i)<<")";
//		svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() )
//				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
////		svg.add( Shape().Text(x,y-1.5*T().pointHeight,to_string(i+1))
////				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
//	}


	vector< pair<int,int> > vp={
		pair<int,int>(1,1),
		pair<int,int>(infoData.NX(),1),
		pair<int,int>(infoData.NX(),infoData.NY()),
		pair<int,int>(1,infoData.NY())
	};
//	for(const auto& p: vp ){
//		double x=Tx+T().sx(p.first*dh/latlon2m);
//		double y=Ty+Tdy-T().sy(p.second*dh/latlon2m);
//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
//				 .fill(64,64,64).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//		stringstream coordEst;
//		coordEst<<"("<<p.first<<", "<<p.second<<")";
//		svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() )
//				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
//	}




//	for(const Coord& est: estaciones ){
//		double x=T().x(est.lon);
//		double y=T().y(est.lat);
//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
//				 .fill(255,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//	}



	svg.add( Shape().Mesh(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()) );

//	{
//		double x1=Tx+T().sx(dh/latlon2m);
//		double y1=Ty+Tdy-T().sy(dh/latlon2m);
//		double x2=Tx+T().sx(infoData.NXSC()*dh/latlon2m);
//		double y2=Ty+Tdy-T().sy(infoData.NYSC()*dh/latlon2m);
//		svg.add( Shape().Line( x1, y1, x2, y2 ).stroke( 255, 0, 0).strokeWidth(5.0).strokeLinecap("round") );
//		x1=Tx+T().sx(infoData.NXSC()*dh/latlon2m);
//		y1=Ty+Tdy-T().sy(infoData.NYSC()*dh/latlon2m);
//		x2=Tx+T().sx((infoData.NXSC()+xhip)*dh/latlon2m);
//		y2=Ty+Tdy-T().sy((infoData.NYSC()+yhip)*dh/latlon2m);
//		svg.add( Shape().Line( x1, y1, x2, y2 ).stroke( 255, 0, 0).strokeWidth(5.0).strokeLinecap("round") );
//		x1=Tx+T().sx(infoData.NXSC()*dh/latlon2m);
//		y1=Ty+Tdy-T().sy(infoData.NYSC()*dh/latlon2m);
//		x2=Tx+T().sx((infoData.NXSC()+216)*dh/latlon2m);
//		y2=Ty+Tdy-T().sy((infoData.NYSC()+40)*dh/latlon2m);
//		svg.add( Shape().Line( x1, y1, x2, y2 ).stroke( 255, 255, 0).strokeWidth(5.0).strokeLinecap("round") );
//	}


}


void make_svg_mmi(string nameSVG, string namePNG, vector<double> &vRadio){

	int image_width=T().width;
	int image_height=T().height;

	vector< vector< pair<Coord,float> > > coordInvSlip;
	dataSlip dSlip=ReadSlip(coordInvSlip);

	SVG2D svg(nameSVG,image_width,image_height);
	svg.add(Shape().ColorMap(colorMap2,"ColorMap2"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

	double nbgx=infoData.NBGX()-1;
	double nbgy=infoData.NBGY()-1;
	double nLx=infoData.NEDX()-infoData.NBGX()+1;
	double nLy=infoData.NEDY()-infoData.NBGY()+1;

//	double Tx=T().x(epi_lon+(x-infoData.NXSC()-xhip)*dh/latlon2m);
//	double Ty=T().y(epi_lat+(y-infoData.NYSC()-yhip+dy)*dh/latlon2m);
	//double iniSlip_lat=coordInvSlip[0][dSlip.Nx-1].first.lat;
	//double iniSlip_lon=coordInvSlip[0][dSlip.Nx-1].first.lon;
//	double iniSlip_lat=coordInvSlip[0][0].first.lat;
//	double iniSlip_lon=coordInvSlip[0][0].first.lon;

	double maxSlip=-std::numeric_limits<double>::max();
	double maxSlipLAT;
	double maxSlipLON;
	for (const auto& row : coordInvSlip){
		for (const auto& p : row){
			if(maxSlip<=p.second){
				maxSlip=p.second;
				maxSlipLAT=p.first.lat;
				maxSlipLON=p.first.lon;
			}
		}
	}
	double iniSlip_lat=maxSlipLAT;
	double iniSlip_lon=maxSlipLON;
//	double iniSlip_lat=coordInvSlip[0][(dSlip.Nx-1)/2].first.lat;
//	double iniSlip_lon=coordInvSlip[0][(dSlip.Nx-1)/2].first.lon;

	double Tx=T().x(iniSlip_lon+(nbgx-infoData.NXSC())*dh/latlon2m);
	double Ty=T().y(iniSlip_lat+(nbgy-infoData.NYSC()+nLy)*dh/latlon2m);

	double Tdx=T().sx(nLx*dh/latlon2m);
	double Tdy=T().sy(nLy*dh/latlon2m);

	double epi_x=T().x(epi_lon);
	double epi_y=T().y(epi_lat);


	double rotx=T().x(iniSlip_lon)-Tx;
	double roty=T().y(iniSlip_lat)-Ty;

	svg.add(Shape().Image(Tx+rotx,Ty+roty,Tdx,Tdy,namePNG,rotx,roty,theta));
	slip2(svg, coordInvSlip);


	for(int i=1; i<vRadio.size() && vRadio[i]>0; i++ ){
		double lon = epi_lon;
		double lat = epi_lat;
		double x=T().x(lon);
		double y=T().y(lat);
		double r=T().sx(vRadio[i]/latlon2m);
		double dlabel=T().sx(0.5*(vRadio[i]+vRadio[i-1])/latlon2m);
		double alpha=225.0*M_PI/180.0;
		string label;
		if(i==0)label.assign("I");
		if(i==1)label.assign("II");
		if(i==2)label.assign("III");
		if(i==3)label.assign("IV");
		if(i==4)label.assign("V");
		if(i==5)label.assign("VI");
		if(i==6)label.assign("VII");
		if(i==7)label.assign("VIII");
		if(i==8)label.assign("IX");
		if(i==9)label.assign("IX");
		if(i==10)label.assign("X");
		if(i==11)label.assign("XI");
		if(i==12)label.assign("XII");
		cout<<"i:"<<i<<" r:"<<vRadio[i]<<endl;

		svg.add(Shape().Circle(x,y,r)
				.fillNone().stroke(255,0,0).strokeWidth(0.1*T().textHeight) );
		svg.add( Shape().Text( x+dlabel*cos(alpha), y-dlabel*sin(alpha), label )
				 .align("middle").fontSize(0.4*T().textHeight).fontFamily("Times") );
	}
	//svg.add( Shape().MaskRaw(lon1,lon2,lat2,lat1).fill(192, 192, 210) );
	//cout<<"xmin:"<<BB().xmin()<<" xmax:"<<BB().xmax()<<" lon1:"<<lon1<<" lon2:"<<lon2<<endl;
	//cout<<"ymin:"<<BB().ymin()<<" ymax:"<<BB().ymax()<<" lat1:"<<lat1<<" lat2:"<<lat2<<endl;
	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Mask(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()).fill(192, 192, 210) );

	double colorBarX=22.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=5.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;

	int N=MMI_Scale.size();
	int i=0;
	double h=colorBarHeight/(N+1);
	for(const auto& mmi: MMI_Scale ){
		double x=colorBarX;
		double y=colorBarY+colorBarHeight*i/(N-1);

		svg.add(Shape().Circle(x,y,2*T().pointHeight)
				.fill(mmi.c.r,mmi.c.g,mmi.c.b).stroke(0,0,0).strokeWidth(0.1*T().textHeight) );
		svg.add(Shape().Rectangle(x-0.5*colorBarWidth, y-0.5*h,colorBarWidth,h).fill(mmi.c.r,mmi.c.g,mmi.c.b) );
		svg.add( Shape().Text(x,y+T().pointHeight,mmi.label )
				 .align("middle").fontSize(0.7*T().textHeight).fontFamily("Times").stroke(255,255,255).strokeWidth(0.2*T().textHeight).opacity(0.3) );
		svg.add( Shape().Text(x,y+T().pointHeight,mmi.label )
				 .align("middle").fontSize(0.7*T().textHeight).fontFamily("Times") );
		i++;
	}

	vector<float> polygon;
	int Npol=3;
	for(int i=0; i<Npol; i++){
		double alpha=360*i/Npol;
		double x = epi_lon;
		double y = epi_lat+0.2*T().inv_sy( T().pointHeight );
		rot(alpha,epi_lon, epi_lat, x, y);
		polygon.push_back( T().x(x) );
		polygon.push_back( T().y(y) );

	}
	svg.add( Shape().Polygon(polygon).fill(255,0,0).stroke(0,0,0).strokeWidth(0.01*T().textHeight).opacity(0.8) );



//	for(int i=0; i<infoData.HOLE_N(); i++){
//		double x=Tx+T().sx(infoData.HOLE_X(i)*dh/latlon2m);
//		double y=Ty+Tdy-T().sy(infoData.HOLE_Y(i)*dh/latlon2m);
//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
//				 .fill(0,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//		stringstream coordEst;
//		coordEst<<i<<" ("<<infoData.HOLE_X(i)<<", "<<infoData.HOLE_Y(i)<<")";
//		svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() )
//				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
////		svg.add( Shape().Text(x,y-1.5*T().pointHeight,to_string(i+1))
////				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
//	}


//	vector< pair<int,int> > vp={
//		pair<int,int>(1,1),
//		pair<int,int>(infoData.NX(),1),
//		pair<int,int>(infoData.NX(),infoData.NY()),
//		pair<int,int>(1,infoData.NY())
//	};
//	for(const auto& p: vp ){
//		double x=Tx+T().sx(p.first*dh/latlon2m);
//		double y=Ty+Tdy-T().sy(p.second*dh/latlon2m);
//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
//				 .fill(64,64,64).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//		stringstream coordEst;
//		coordEst<<"("<<p.first<<", "<<p.second<<")";
//		svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() )
//				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
//	}

//	for(const Coord& est: estaciones ){
//		double x=T().x(est.lon);
//		double y=T().y(est.lat);
//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
//				 .fill(255,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//	}
	svg.add( Shape().Mesh(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()) );

}



void make_svg_mmi_maxvel(string nameSVG, string namePNG, double vmin, double vmax,  vector<double> &vRadio){

	int image_width=T().width;
	int image_height=T().height;

	vector< vector< pair<Coord,float> > > coordInvSlip;
	dataSlip dSlip=ReadSlip(coordInvSlip);

	SVG2D svg(nameSVG,image_width,image_height);
	svg.add(Shape().ColorMap(colorMap2,"ColorMap2"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

//	double x=infoData.NBGX()-1;
//	double y=infoData.NBGY()-1;
//	double dx=infoData.NEDX()-infoData.NBGX()+1;
//	double dy=infoData.NEDY()-infoData.NBGY()+1;

//	double lon1=epi_lon+(x-infoData.NXSC()-xhip)*dh/latlon2m;
//	double lat1=epi_lat+(y-infoData.NYSC()-yhip+dy)*dh/latlon2m;
//	double lon2=lon1+dx*dh/latlon2m;
//	double lat2=lat1-dy*dh/latlon2m;

	double nbgx=infoData.NBGX()-1;
	double nbgy=infoData.NBGY()-1;
	double nLx=infoData.NEDX()-infoData.NBGX()+1;
	double nLy=infoData.NEDY()-infoData.NBGY()+1;

//	double Tx=T().x(epi_lon+(x-infoData.NXSC()-xhip)*dh/latlon2m);
//	double Ty=T().y(epi_lat+(y-infoData.NYSC()-yhip+dy)*dh/latlon2m);
	//double iniSlip_lat=coordInvSlip[0][dSlip.Nx-1].first.lat;
	//double iniSlip_lon=coordInvSlip[0][dSlip.Nx-1].first.lon;
//	double iniSlip_lat=coordInvSlip[0][0].first.lat;
//	double iniSlip_lon=coordInvSlip[0][0].first.lon;

	double maxSlip=-std::numeric_limits<double>::max();
	double maxSlipLAT;
	double maxSlipLON;
	for (const auto& row : coordInvSlip){
		for (const auto& p : row){
			if(maxSlip<=p.second){
				maxSlip=p.second;
				maxSlipLAT=p.first.lat;
				maxSlipLON=p.first.lon;
			}
		}
	}
	double iniSlip_lat=maxSlipLAT;
	double iniSlip_lon=maxSlipLON;
//	double iniSlip_lat=coordInvSlip[0][(dSlip.Nx-1)/2].first.lat;
//	double iniSlip_lon=coordInvSlip[0][(dSlip.Nx-1)/2].first.lon;

	double Tx=T().x(iniSlip_lon+(nbgx-infoData.NXSC())*dh/latlon2m);
	double Ty=T().y(iniSlip_lat+(nbgy-infoData.NYSC()+nLy)*dh/latlon2m);

	double Tdx=T().sx(nLx*dh/latlon2m);
	double Tdy=T().sy(nLy*dh/latlon2m);

	double epi_x=T().x(epi_lon);
	double epi_y=T().y(epi_lat);


	double rotx=T().x(iniSlip_lon)-Tx;
	double roty=T().y(iniSlip_lat)-Ty;




	//svg.add(Shape().Image(Tx,Ty,Tdx,Tdy,namePNG,0,0,0));
	svg.add(Shape().Image(Tx+rotx,Ty+roty,Tdx,Tdy,namePNG,rotx,roty,theta));
	//slip(svg);
//	vector< vector< pair<Coord,float> > > coordInvSlip;
//	ReadSlip(coordInvSlip);
	slip2(svg, coordInvSlip);







	//svg.add(Shape().Image(Tx+rotx,Ty+roty,Tdx,Tdy,namePNG,rotx,roty,theta));



//	svg.add( Shape().MaskRaw(lon1,lon2,lat2,lat1).fill(192, 192, 210) );
//	cout<<"xmin:"<<BB().xmin()<<" xmax:"<<BB().xmax()<<" lon1:"<<lon1<<" lon2:"<<lon2<<endl;
//	cout<<"ymin:"<<BB().ymin()<<" ymax:"<<BB().ymax()<<" lat1:"<<lat1<<" lat2:"<<lat2<<endl;


	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Mask(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()).fill(192, 192, 210) );

	double colorBarX=21.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=1.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;

	double lonRot=iniSlip_lon;
	double latRot=iniSlip_lat;

	svg.add( Shape().IniGroup("IsocurvasRotadas"));
	for(int k=0; k<isovel.size(); k++ ){
		for(const vector<Coord>& vs: isovel[k].iso ){
			for(int i=0; i<vs.size()-1; i+=2){
				double lon1=vs[i].lon;
				double lat1=vs[i].lat;
				double lon2=vs[i+1].lon;
				double lat2=vs[i+1].lat;

				rot(theta,lonRot, latRot, lon1, lat1);
				rot(theta,lonRot, latRot, lon2, lat2);
				svg.add( Shape().Line( T().x(lon1), T().y(lat1), T().x(lon2), T().y(lat2) ).stroke( 0.7*isovel[k].color.r, 0.7*isovel[k].color.g, 0.7*isovel[k].color.b).strokeWidth(1.0).strokeLinecap("round") );
//				svg.add( Shape().Line( T().x(vs[i].lon), T().y(vs[i].lat), T().x(vs[i+1].lon), T().y(vs[i+1].lat) ).stroke( 0.7*isovel[k].color.r, 0.7*isovel[k].color.g, 0.7*isovel[k].color.b).strokeWidth(1.0).strokeLinecap("round") );
			}
		}
	}
	svg.add( Shape().EndGroup());

	svg.add( Shape().IniGroup("IsocurvasMMI"));
	for(int i=1; i<vRadio.size() && vRadio[i]>0; i++ ){
		double lon = epi_lon;
		double lat = epi_lat;
		double x=T().x(lon);
		double y=T().y(lat);
		double r=T().sx(vRadio[i]/latlon2m);
		double dlabel=T().sx(0.5*(vRadio[i]+vRadio[i-1])/latlon2m);
		double alpha=225.0*M_PI/180.0;
		string label;
		if(i==0)label.assign("I");
		if(i==1)label.assign("II");
		if(i==2)label.assign("III");
		if(i==3)label.assign("IV");
		if(i==4)label.assign("V");
		if(i==5)label.assign("VI");
		if(i==6)label.assign("VII");
		if(i==7)label.assign("VIII");
		if(i==8)label.assign("IX");
		if(i==9)label.assign("IX");
		if(i==10)label.assign("X");
		if(i==11)label.assign("XI");
		if(i==12)label.assign("XII");
		cout<<"i:"<<i<<" r:"<<vRadio[i]<<endl;

		svg.add(Shape().Circle(x,y,r)
				.fillNone().stroke(255,0,0).strokeWidth(0.01*T().textHeight) );
		svg.add( Shape().Text( x+dlabel*cos(alpha), y-dlabel*sin(alpha), label )
				 .align("middle").fontSize(0.2*T().textHeight).fontFamily("Times") );
	}
	svg.add( Shape().EndGroup());

	svg.add(Shape().ColorBar(colorBarX, colorBarY,
							 colorBarWidth, colorBarHeight,"ColorMap2",
							 vmin, vmax, "Vx (m/s)") );


	vector<float> polygon;
	int Npol=3;
	for(int i=0; i<Npol; i++){
		double alpha=360*i/Npol;
		double x = epi_lon;
		double y = epi_lat+0.2*T().inv_sy( T().pointHeight );
		rot(alpha,epi_lon, epi_lat, x, y);
		polygon.push_back( T().x(x) );
		polygon.push_back( T().y(y) );

	}
	svg.add( Shape().Polygon(polygon).fill(255,0,0).stroke(0,0,0).strokeWidth(0.01*T().textHeight).opacity(0.8) );

//	for(int i=0; i<infoData.HOLE_N(); i++){
//		double x=Tx+T().sx(infoData.HOLE_X(i)*dh/latlon2m);
//		double y=Ty+Tdy-T().sy(infoData.HOLE_Y(i)*dh/latlon2m);
//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
//				 .fill(0,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//		stringstream coordEst;
//		coordEst<<i<<" ("<<infoData.HOLE_X(i)<<", "<<infoData.HOLE_Y(i)<<")";
//		svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() )
//				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
////		svg.add( Shape().Text(x,y-1.5*T().pointHeight,to_string(i+1))
////				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
//	}


//	vector< pair<int,int> > vp={
//		pair<int,int>(1,1),
//		pair<int,int>(infoData.NX(),1),
//		pair<int,int>(infoData.NX(),infoData.NY()),
//		pair<int,int>(1,infoData.NY())
//	};
//	for(const auto& p: vp ){
//		double x=Tx+T().sx(p.first*dh/latlon2m);
//		double y=Ty+Tdy-T().sy(p.second*dh/latlon2m);
//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
//				 .fill(64,64,64).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//		stringstream coordEst;
//		coordEst<<"("<<p.first<<", "<<p.second<<")";
//		svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() )
//				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
//	}




//	for(const Coord& est: estaciones ){
//		double x=T().x(est.lon);
//		double y=T().y(est.lat);
//		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
//				 .fill(255,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
//	}



	svg.add( Shape().Mesh(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()) );

//	{
//		double x1=Tx+T().sx(dh/latlon2m);
//		double y1=Ty+Tdy-T().sy(dh/latlon2m);
//		double x2=Tx+T().sx(infoData.NXSC()*dh/latlon2m);
//		double y2=Ty+Tdy-T().sy(infoData.NYSC()*dh/latlon2m);
//		svg.add( Shape().Line( x1, y1, x2, y2 ).stroke( 255, 0, 0).strokeWidth(5.0).strokeLinecap("round") );
//		x1=Tx+T().sx(infoData.NXSC()*dh/latlon2m);
//		y1=Ty+Tdy-T().sy(infoData.NYSC()*dh/latlon2m);
//		x2=Tx+T().sx((infoData.NXSC()+xhip)*dh/latlon2m);
//		y2=Ty+Tdy-T().sy((infoData.NYSC()+yhip)*dh/latlon2m);
//		svg.add( Shape().Line( x1, y1, x2, y2 ).stroke( 255, 0, 0).strokeWidth(5.0).strokeLinecap("round") );
//		x1=Tx+T().sx(infoData.NXSC()*dh/latlon2m);
//		y1=Ty+Tdy-T().sy(infoData.NYSC()*dh/latlon2m);
//		x2=Tx+T().sx((infoData.NXSC()+216)*dh/latlon2m);
//		y2=Ty+Tdy-T().sy((infoData.NYSC()+40)*dh/latlon2m);
//		svg.add( Shape().Line( x1, y1, x2, y2 ).stroke( 255, 255, 0).strokeWidth(5.0).strokeLinecap("round") );
//	}


}

void make_svg_slip(string nameSVG, string namePNG, double vmin, double vmax, int ticks=6){

	int image_width=T().width;
	int image_height=T().height;

	SVG2D svg(nameSVG,image_width,image_height);
	svg.add(Shape().ColorMap(colorMap3,"ColorMap3"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

	svg.add(Shape().Image(0.05*image_width,0.125*image_height, 0.75*image_width, 0.75*image_height,namePNG,0,0,0));

	double colorBarX=21.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=1.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;

	svg.add(Shape().ColorBar(colorBarX, colorBarY,
							 colorBarWidth, colorBarHeight,"ColorMap3",
							 vmin, vmax, "Slip (m)",ticks) );
	//svg.add( Shape().Mesh(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()) );

}


int main(int argc, char *argv[])
{

	configData cdat(argc, argv);





	cout<<"ruta "<<cdat.path<<" paso="<<cdat.step<<endl;
	for(unsigned int i=0;i<cdat.data.size();i++){
		cout<<"Componente "<<cdat.data[i].comp<<" mínimo="<<cdat.data[i].min<<" máximo="<<cdat.data[i].max<<endl;

		if(cdat.data[i].px.size()>0){
			cout<<" Plano X"<<endl;
			for(unsigned int j=0;j<cdat.data[i].px.size();j++){
				cout<<" "<<cdat.data[i].px[j];
			}
			cout<<endl;
		}

		if(cdat.data[i].py.size()>0){
			cout<<" Plano Y"<<endl;
			for(unsigned int j=0;j<cdat.data[i].py.size();j++){
				cout<<" "<<cdat.data[i].py[j];
			}
			cout<<endl;
		}

		if(cdat.data[i].pz.size()>0){
			cout<<" Plano Z"<<endl;
			for(unsigned int j=0;j<cdat.data[i].pz.size();j++){
				cout<<" "<<cdat.data[i].pz[j];
			}
			cout<<endl;
		}
	}


	string SubLocation="result";


#ifdef MI_PC
	ndigTI=4;
#else
	ndigTI=5;
#endif

	//inputData infoData;

	//stringstream FileInputData;
	//FileInputData<<cdat.path<<"/input.dat";
	//FileInputData<<cdat.path;
	//cout<<"Archivo:"<<FileInputData.str()<<endl<<endl;
	//infoData.read(FileInputData.str());
	cout<<"Archivo:"<<cdat.path<<endl<<endl;
	infoData.read(cdat.path);

	infoData.show();

	initialize_visualization_(cdat.path);
	vector< vector< pair<Coord,float> > > coordInvSlip;
	dataSlip dSlip=ReadSlip(coordInvSlip);
	theta=270.0-dSlip.STRK;

#if defined DEBUG
	cout<<"DH="<<infoData.DH()<<endl;
	cout<<"NXSC="<<infoData.NXSC()<<endl;
	cout<<"NYSC="<<infoData.NYSC()<<endl;
	cout<<"nx="<<infoData.NX()<<endl;
	cout<<"ny="<<infoData.NY()<<endl;
	cout<<"nz="<<infoData.NZ()<<endl;
	cout<<"DH="<<infoData.DH()<<endl;

	cout<<"SX="<<infoData.SX()<<endl;
	cout<<"SY="<<infoData.SY()<<endl;
	cout<<"SZ="<<infoData.SZ()<<endl;

	cout<<"NSKPX="<<infoData.NSKPX()<<endl;
	cout<<"NSKPY="<<infoData.NSKPY()<<endl;
	cout<<"NSKPZ="<<infoData.NSKPZ()<<endl;

	cout<<"lx="<<infoData.lx()<<endl;
	cout<<"ly="<<infoData.ly()<<endl;

#endif
	cout<<"cdat.xhip="<<cdat.xhip<<endl;
	cout<<"cdat.yhip="<<cdat.yhip<<endl;
	if( cdat.xhip == std::numeric_limits<int>::max() ){
		xhip = infoData.xHip();
	}
	else{
		xhip = cdat.xhip;
	}

	if( cdat.yhip == std::numeric_limits<int>::max() ){
		yhip = infoData.yHip();
	}
	else{
		yhip = cdat.yhip;
	}

	cout<<"xhip="<<xhip<<endl;
	cout<<"yhip="<<yhip<<endl;
	//	xhip = infoData.xHip();
	//	yhip = infoData.yHip();


	double Ms=cdat.Ms;
	double Dp=cdat.Dp;
	unsigned int Eq=cdat.Eq;
	unsigned int Group=cdat.Group;


	//la transformación se debe revisar que parámetros recibe
	transformation_settings_( infoData.DH(), infoData.NXSC()-infoData.NBGX(), infoData.NYSC()-infoData.NBGY(), infoData.SX()*infoData.NSKPX(), infoData.SY()*infoData.NSKPY());
	make_map_();
	dt=infoData.DT();

	//make_svg_(200);
	//make_svg_(timeIndex,"./imagenes/"+nameSVG_Z.str(),"./Z/"+namePNG_Z.str());
	//return 0;

	int SX=infoData.SX();
	int SY=infoData.SY();
	int SZ=infoData.SZ();

	auto t0T = clk::now();

	vector<float> val;
	vector<int> valID;
	val.resize( SX*SY*SZ );
	valID.resize( SX*SY*SZ );

//	vector<float> valmax( SX*SY, -std::numeric_limits<float>::max() );
	data2D<float> valmax2( SX, SY, -std::numeric_limits<float>::max() );
	bool calculateVmax = false;

	if( cdat.step == -1 || !ifstream("./velMax_bin.dat").good() ){
		cout<<"Se calculará vmax"<<endl;
		calculateVmax = true;
	}
	else{
		cout<<"No se calculará vmax"<<endl;
	}




	int NParticionesSX=infoData.IndexXend()-infoData.IndexXini()+1;
	int NParticionesSY=infoData.IndexYend()-infoData.IndexYini()+1;
	int NParticionesSZ=infoData.IndexZend()-infoData.IndexZini()+1;
	int NParticiones=NParticionesSX*NParticionesSY*NParticionesSZ;

	vector<int> particionesSXo(NParticiones),particionesSXf(NParticiones);
	vector<int> particionesSYo(NParticiones),particionesSYf(NParticiones);
	vector<int> particionesSZo(NParticiones),particionesSZf(NParticiones);
	vector<int> particionesSizeArray(NParticiones);
	vector<int> particionesID(NParticiones);

	struct ijk{
		int i, j, k;
		ijk(int i, int j, int k):i(i),j(j),k(k){}
	};

	map<int, ijk> id2ijk;



	int id=0;
	int SXo=0;
	int SXf=0;
	for(int i=infoData.IndexXini(); i<=infoData.IndexXend(); i++){
		int SYo=0;
		int SYf=0;
		int SXi=infoData.SXi(i);
		SXf+=SXi;
		for(int j=infoData.IndexYini(); j<=infoData.IndexYend(); j++){
			int SZo=0;
			int SZf=0;
			int SYj=infoData.SYj(j);
			SYf+=SYj;
			for(int k=infoData.IndexZini(); k<=infoData.IndexZend(); k++){
				int SZk=infoData.SZk(k);
				SZf+=SZk;
				particionesID[id]=infoData.ID(i,j,k);
				particionesSXo[id]=SXo;particionesSXf[id]=SXf;
				particionesSYo[id]=SYo;particionesSYf[id]=SYf;
				particionesSZo[id]=SZo;particionesSZf[id]=SZf;
				particionesSizeArray[id]=SXi*SYj*SZk+2*SZk;
				//ijk ccc=ijk(i,j,k);
				id2ijk.emplace(id,ijk(i,j,k));
				//cout<<id2ijk.at(id).i<<endl;
				//id2ijk[id]=ccc;
				//particionesSizeArray[id]=SXi*SYj*SZk;
				//if(SZk!=1)
				//	particionesSizeArray[id]+=2*SZk;

				id++;
				SZo+=SZk;
			}
			SYo+=SYj;
		}
		SXo+=SXi;
	}



	int stepIndexIni;
	int stepIndexEnd;
	int dstepIndex;

	if(cdat.step==-1){
#ifdef MI_PC
		stepIndexIni=1;
#else
		stepIndexIni=infoData.NTISKP();
#endif
		stepIndexEnd=infoData.NT();
	}
	else{
		stepIndexIni=cdat.step;
		stepIndexEnd=cdat.step+1;
	}

	if(cdat.initialstep!=-1){
		stepIndexIni=cdat.initialstep;
		stepIndexEnd=infoData.NT();
	}

	dstepIndex=infoData.NTISKP();

	//	pushColor(153,102,255);//min
	//	pushColor(0,0,255);
	//	pushColor(0,255,0);

	////	pushColor(255,255,255);
	//	pushColor(0,0,0);

	//	pushColor(255,255,0);
	//	pushColor(255,102,0);
	//	pushColor(255,0,0);//max

	for(int id=0; id<NParticiones; id++){
		for(int kk=particionesSZo[id]; kk<particionesSZf[id]; kk++){
			int Pz=kk*SX*SY;
			for(int jj=particionesSYo[id]; jj<particionesSYf[id]; jj++){
				int Ly=jj*SX;
				for(int ii=particionesSXo[id]; ii<particionesSXf[id]; ii++){
					val[ii+Ly+Pz]=(float)id/NParticiones;
				}
			}
		}
	}

	//    stringstream namePNG_X;
	//    namePNG_X<<"./imagenes/Particiones-X"<<setw(calculaCifras(SX))<<setfill('0')<<planoX<<".png";
	//    std::ofstream outX(namePNG_X.str(), std::ios::binary);
	//    TinyPngOut pngoutX(static_cast<uint32_t>(SY), static_cast<uint32_t>(SZ), outX);
	//    std::vector<uint8_t> lineX(static_cast<size_t>(SY) * 3);
	//    for (int z = SZ-1; z >= 0; z--) {
	//        for (int y = 0; y < SY; y++) {
	//            uint8_t r, g, b;
	//            colorMap(val[planoX+SX*y+SX*SY*z], r, g, b);
	//            lineX[y * 3 + 0] = static_cast<uint8_t>(r);
	//            lineX[y * 3 + 1] = static_cast<uint8_t>(g);
	//            lineX[y * 3 + 2] = static_cast<uint8_t>(b);
	//        }
	//        pngoutX.write(lineX.data(), static_cast<size_t>(SY));
	//    }

	//    stringstream namePNG_Y;
	//    namePNG_Y<<"./imagenes/Particiones-Y"<<setw(calculaCifras(SY))<<setfill('0')<<planoY<<".png";
	//    std::ofstream outY(namePNG_Y.str(), std::ios::binary);
	//    TinyPngOut pngoutY(static_cast<uint32_t>(SX), static_cast<uint32_t>(SZ), outY);
	//    std::vector<uint8_t> lineY(static_cast<size_t>(SX) * 3);
	//    for (int z = SZ-1; z >= 0; z--) {
	//        for (int x = 0; x < SX; x++) {
	//            uint8_t r, g, b;
	//            colorMap(val[x+SX*planoY+SX*SY*z], r, g, b);
	//            lineY[x * 3 + 0] = static_cast<uint8_t>(r);
	//            lineY[x * 3 + 1] = static_cast<uint8_t>(g);
	//            lineY[x * 3 + 2] = static_cast<uint8_t>(b);
	//        }
	//        pngoutY.write(lineY.data(), static_cast<size_t>(SX));
	//    }

	//    stringstream namePNG_Z;
	//    namePNG_Z<<"./imagenes/Particiones-Z"<<setw(calculaCifras(SZ))<<setfill('0')<<planoZ<<".png";
	//    std::ofstream outZ(namePNG_Z.str(), std::ios::binary);
	//    TinyPngOut pngoutZ(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outZ);
	//    std::vector<uint8_t> lineZ(static_cast<size_t>(SX) * 3);
	//    for (int y = SY-1; y >= 0; y--) {
	//        for (int x = 0; x < SX; x++) {
	//            uint8_t r, g, b;
	//            colorMap(val[x+SX*y+SX*SY*planoZ], r, g, b);
	//            lineZ[x * 3 + 0] = static_cast<uint8_t>(r);
	//            lineZ[x * 3 + 1] = static_cast<uint8_t>(g);
	//            lineZ[x * 3 + 2] = static_cast<uint8_t>(b);
	//        }
	//        pngoutZ.write(lineZ.data(), static_cast<size_t>(SX));
	//    }

	/***********************/
	ifstream fileSlip;
	fileSlip.open("Slip_dist.dat", std::ifstream::in);
	cout<<"Slip_dist.dat"<<endl;

	float slipMax=-std::numeric_limits<float>::max();
	float slipMin=std::numeric_limits<float>::max();

	string line;
	int slipN=0;
	int slipM=0;
	while( getline (fileSlip, line) ){
		stringstream sline(line);
		float slip;
		int Np=0;
		vector < float > vSlipLine;
		while (sline>>slip){
			//cout<<slip<<endl;
			vSlipLine.push_back(slip);
			if(slipMax<slip)slipMax=slip;
			if(slipMin>slip)slipMin=slip;
			Np++;
		}
		if(slipM==0)slipN=Np;
		if(slipN!=Np){
			cout<<"Error en la lectura del slip, renglon "<<slipM+1<<" de tamaño diferente"<<endl;
		}
		vSlip.push_back(vSlipLine);
		slipM++;
	}

	int ticks=8;
	//NiceScale nsSlip(slipMin, slipMax,ticks);
	nsSlip= new NiceScale(slipMin, slipMax,ticks);
	std::ofstream outSlip("./slip.png", std::ios::binary);
	TinyPngOut pngoutSlip(static_cast<uint32_t>(slipN), static_cast<uint32_t>(slipM), outSlip);
	std::vector<uint8_t> lineSlip(static_cast<size_t>(slipN) * 3);
	for(auto &line: vSlip){
		int i=0;
		for(auto &slip: line){
			uint8_t r, g, b;
			double f = (slip-nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);
			lineSlip[i * 3 + 0] = static_cast<uint8_t>(r);
			lineSlip[i * 3 + 1] = static_cast<uint8_t>(g);
			lineSlip[i * 3 + 2] = static_cast<uint8_t>(b);
			i++;
		}
		//cout<<endl;
		pngoutSlip.write(lineSlip.data(), static_cast<size_t>(slipN));
	}
	cout<<slipMin<<" "<<slipMax<<" -> "<<nsSlip->niceMin<<" "<<nsSlip->niceMax<<endl;
	make_svg_slip("./slip.svg", "./slip.png", slipMin, slipMax, ticks);
	/***********************/

	digTime=sigfigs(dt*infoData.NTISKP());

	string nFilePrefix;
	for(unsigned int i=0;i<cdat.data.size();i++){
		if(  cdat.data[i].px.size()==0
			 && cdat.data[i].py.size()==0
			 && cdat.data[i].pz.size()==0 ){
			continue;
		}

		if( cdat.data[i].comp==X )
			nFilePrefix="Vx3D";
		if( cdat.data[i].comp==Y )
			nFilePrefix="Vy3D";
		if( cdat.data[i].comp==Z )
			nFilePrefix="Vz3D";
		//#pragma omp parallel for
		//#pragma omp parallel for num_threads(2)
		//#pragma omp parallel for num_threads(2) collapse(2)
		//#pragma omp parallel for collapse(2)
		for(int timeIndex=stepIndexIni;timeIndex<stepIndexEnd;timeIndex+=dstepIndex)
		{


			//double min=cdat.data[i].min;
			//double max=cdat.data[i].max;

			//        double max=-std::numeric_limits<double>::max();
			//        double min=std::numeric_limits<double>::max();
			auto t0 = clk::now();
			cout<<nFilePrefix<<" "<<timeIndex;

			NiceScale ns(VELXMIN,VELXMAX);
#pragma omp parallel for
			for(int id=0; id<NParticiones; id++){
				stringstream nameFile;
				stringstream nameFileR;
				nameFile<<nFilePrefix;
				nameFile<<setw(ndigID)<<setfill('0')<<particionesID[id];
				nameFile<<setw(ndigTI)<<setfill('0')<<timeIndex;
				nameFileR<<cdat.path<<"/"<<SubLocation<<"/"<<nameFile.str();
				std::ifstream file(nameFileR.str().c_str(),std::ios::binary|std::ios::in);
				if( file.fail() ){
					cerr<<" ERROR: no se puede abrir el archivo "<<nameFileR.str().c_str()<<std::endl;
					exit (EXIT_FAILURE);
					//return 1;
				}

				std::filebuf* pbuf = file.rdbuf();
				std::size_t size = pbuf->pubseekoff (0,file.end,file.in);
				if(size!= (unsigned long)( 4*particionesSizeArray[id] ) ){
					cerr<<" size: "<<size<<" particionesSizeArray["<<id<<"]: "<<particionesSizeArray[id]<<std::endl;
					cerr<<" ERROR: El tamaño del archivo no es correcto "<<nameFileR.str().c_str()<<std::endl;
					exit (EXIT_FAILURE);
					//return 1;
				}

				vector<float> buffer( particionesSizeArray[id] );
				pbuf->pubseekpos (0,file.in);
				pbuf->sgetn ((char*)&buffer[0],size);
				pbuf->close();
				file.close();

				auto it = buffer.begin();
				for(int kk=particionesSZo[id]; kk<particionesSZf[id]; kk++){
					it++;
					int Pz=(particionesSZf[id]+particionesSZo[id]-1-kk)*SX*SY;
					for(int jj=particionesSYo[id]; jj<particionesSYf[id]; jj++){
						int Ly=jj*SX;
						for(int ii=particionesSXo[id]; ii<particionesSXf[id]; ii++){
							val[ii+Ly+Pz]=*it;
							valID[ii+Ly+Pz]=id;

							//if(particionesID[id]==55)val[ii+Ly+Pz]=1.0;
							it++;
						}
					}
					it++;
				}
			}//for NParticiones #pragma omp parallel for

			for(unsigned int j=0;j<cdat.data[i].px.size();j++){
				stringstream namePNG_X;
				namePNG_X<<"./imagenes/X/"<<nFilePrefix<<"-X"<<setw(calculaCifras(SX))<<setfill('0')<<cdat.data[i].px[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex<<".png";
				cout<<namePNG_X.str()<<endl;
				std::ofstream outX(namePNG_X.str(), std::ios::binary);
				TinyPngOut pngoutX(static_cast<uint32_t>(SY), static_cast<uint32_t>(SZ), outX);
				std::vector<uint8_t> lineX(static_cast<size_t>(SY) * 3);
				for (int z = SZ-1; z >= 0; z--) {
					for (int y = 0; y < SY; y++) {
						uint8_t r, g, b;
						double f = (val[cdat.data[i].px[j]+SX*y+SX*SY*z]-ns.niceMin)/(ns.niceMax-ns.niceMin);
						colorMap1.getColor(f, r, g, b);
						lineX[y * 3 + 0] = static_cast<uint8_t>(r);
						lineX[y * 3 + 1] = static_cast<uint8_t>(g);
						lineX[y * 3 + 2] = static_cast<uint8_t>(b);
					}
					pngoutX.write(lineX.data(), static_cast<size_t>(SY));
				}
			}

			for(unsigned int j=0;j<cdat.data[i].py.size();j++){
				stringstream namePNG_Y;
				namePNG_Y<<"./imagenes/Y/"<<nFilePrefix<<"-Y"<<setw(calculaCifras(SY))<<setfill('0')<<cdat.data[i].py[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex<<".png";
				cout<<namePNG_Y.str()<<endl;
				std::ofstream outY(namePNG_Y.str(), std::ios::binary);
				TinyPngOut pngoutY(static_cast<uint32_t>(SX), static_cast<uint32_t>(SZ), outY);
				std::vector<uint8_t> lineY(static_cast<size_t>(SX) * 3);
				for (int z = SZ-1; z >= 0; z--) {
					for (int x = 0; x < SX; x++) {
						uint8_t r, g, b;
						double f = (val[x+SX*cdat.data[i].py[j]+SX*SY*z]-ns.niceMin)/(ns.niceMax-ns.niceMin);
						colorMap1.getColor(f, r, g, b);
						lineY[x * 3 + 0] = static_cast<uint8_t>(r);
						lineY[x * 3 + 1] = static_cast<uint8_t>(g);
						lineY[x * 3 + 2] = static_cast<uint8_t>(b);
					}
					pngoutY.write(lineY.data(), static_cast<size_t>(SX));
				}

			}

			for(unsigned int j=0;j<cdat.data[i].pz.size();j++){
				stringstream namePNG_Z,nameSVG_Z,nameRoot_Z;
				nameRoot_Z<<nFilePrefix<<"-Z"<<setw(calculaCifras(SZ))<<setfill('0')<<cdat.data[i].pz[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex;
				namePNG_Z<<nameRoot_Z.str()<<".png";
				nameSVG_Z<<nameRoot_Z.str()<<".svg";
				cout<<namePNG_Z.str()<<endl;
				std::ofstream outZ("./imagenes/Z/"+namePNG_Z.str(), std::ios::binary);
				TinyPngOut pngoutZ(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outZ);
				std::vector<uint8_t> lineZ(static_cast<size_t>(SX) * 3);
				//cout<<"min:"<<min<<" max:"<<max<<endl;
				int index_z=SX*SY*cdat.data[i].pz[j];
				for (int y = SY-1; y >= 0; y--) {
					int index_yz=SX*y+index_z;
					for (int x = 0; x < SX; x++) {
						uint8_t r, g, b;
						//double f = (val[x+SX*y+SX*SY*cdat.data[i].pz[j]]-ns.niceMin)/(ns.niceMax-ns.niceMin);
						int index_xyz = x + index_yz;
						int index_xy = x + SX * y;
						float v=val[index_xyz];
						double f = (v-ns.niceMin)/(ns.niceMax-ns.niceMin);
						//valmax[index_xy] = std::max( valmax[index_xy], fabs(v) );

						if( calculateVmax ){
							valmax2.set(x,y,std::max( valmax2.get(x,y), fabs(v) ));
						}

						colorMap1.getColor(f, r, g, b);
						//cout<<x+SX*y+SX*SY*cdat.data[i].pz[j]<<" ";
						//cout<<val[x+SX*y+SX*SY*cdat.data[i].pz[j]]-min<<" ";
						//cout<<"["<<y<<"]["<<x<<"]:"<<cdat.data[i].pz[j]<<" ";
//						lineZ[x * 3 + 0] = static_cast<uint8_t>(r);
//						lineZ[x * 3 + 1] = static_cast<uint8_t>(g);
//						lineZ[x * 3 + 2] = static_cast<uint8_t>(b);

//						cout<<index_xy<<" -> "<<particionesID[ valID[index_xy] ]<<endl;
//						if( particionesID[ valID[index_xy] ]==43 )
//						{
//							r=r;
//							g=3*g/4;
//							b=3*b/4;
//						}
//						if( particionesID[ valID[index_xy] ]==47 )
//						{
//							r=r;
//							g=g;
//							b=3*b/4;
//						}
//						if( particionesID[ valID[index_xy] ]==51 )
//						{
//							r=3*r/4;
//							g=g;
//							b=3*b/4;
//						}
//						if( particionesID[ valID[index_xy] ]==55 )
//						{
//							r=3*r/4;
//							g=g;
//							b=b;
//						}
//						if( particionesID[ valID[index_xy] ]==59 )
//						{
//							r=3*r/4;
//							g=3*g/4;
//							b=b;
//						}
						if( id2ijk.at(valID[index_xy]).i % 2 != id2ijk.at(valID[index_xy]).j % 2 ){
							r=3*r/4;
							g=3*g/4;
							b=3*b/4;
						}
						if( (x-infoData.NXSC())*(x-infoData.NXSC())+(y-infoData.NYSC())*(y-infoData.NYSC())<100 ){
							r=r/2;
							g=g/2;
							b=b/2;
						}
						if( x==infoData.NXSC() && y==infoData.NYSC() ){
							r=0;
							g=0;
							b=0;
						}
						lineZ[x * 3 + 0] = static_cast<uint8_t>(r);
						lineZ[x * 3 + 1] = static_cast<uint8_t>(g);
						lineZ[x * 3 + 2] = static_cast<uint8_t>(b);
					}
					//cout<<endl;
					pngoutZ.write(lineZ.data(), static_cast<size_t>(SX));
				}

				make_svg_(timeIndex,"./imagenes/"+nameSVG_Z.str(),"./Z/"+namePNG_Z.str(),VELXMIN,VELXMAX);

			}

			auto t1 = clk::now();
			auto diff = duration_cast<microseconds>(t1-t0);
			std::cout <<" tiempo de dibujado "<<fixed<< diff.count() << "us"<<endl;
		}


	}//fin for tiempo


	if( calculateVmax ){
//		ofstream output_file("./velMax.dat");
//		ostream_iterator<float> output_iterator(output_file, "\n");
//		copy(valmax2.data.begin(), valmax2.data.end(), output_iterator);

		ofstream output_file_bin("./velMax_bin.dat", ios::out | ios::binary);
		output_file_bin.write( (char*)&valmax2.data.at(0) , valmax2.data.size() * sizeof(float));
		output_file_bin.close();
		//	velMaxZ=0.002;
	}
	else{

//		cout<<"leyendo vmax"<<endl;
//		ifstream input_file_bin("./velMax_bin.dat", ios::binary);
//		cout<<"copiando vmax"<<endl;
//		std::copy(std::istream_iterator<char>(input_file_bin),
//				  std::istream_iterator<char>(),
//				  std::back_inserter(valmax2.data));

//		valmax2.data.insert(valmax2.data.begin(),
//							std::istream_iterator<unsigned char>(input_file_bin),
//							std::istream_iterator<unsigned char>());
		valmax2.data=readFile("./velMax_bin.dat");

	}


//	float velMaxZ = -std::numeric_limits<float>::max();
//	float velMinZ =  std::numeric_limits<float>::max();
//	for (int i = 0; i < valmax.size(); i++){
//		float v = fabs( valmax[i] );
//		if( velMaxZ < v ) velMaxZ = v;
//		if( velMinZ > v ) velMinZ = v;
//	}

	//valmax2.find_minmax();

	//NiceScale nsMax(0,velMaxZ);
	NiceScale nsMax(0,valmax2.Max() );


//	cout<<"velMinZ="<<velMinZ<<endl;
//	cout<<"velMaxZ="<<velMaxZ<<endl;

//	int CX=SX-1;
//	int CY=SY-1;
//	//vector<unsigned char> caseCorners( CX*CY,0 );
//	//cout<<"**** xy(0,0)->"<<img.xy(0,0).lon<<" "<<img.xy(0,0).lat<<endl;
//	//cout<<"**** xy(100,100)->"<<img.xy(100,100).lon<<" "<<img.xy(100,100).lat<<endl;

//	NiceScale nsThresholdMax(0,velMaxZ,12);
//	int N=nsThresholdMax.N();
//	for ( int k=1; k<N-1; k++ ){
//		float valThreshold=nsThresholdMax.niceMin + (float)k*nsThresholdMax.tickSpacing;
//		//vector<float> valsThreshold{ 0.005, 0.010, 0.015, 0.020, 0.025, 0.030, 0.035, 0.040, 0.045, 0.050};
//		//for ( auto valThreshold: valsThreshold ){
//		vector< vector<Coord> > coords;
//		for (int j = 0; j < CY; j++){
//			for (int i = 0; i < CX; i++){
//				int i0 = i + SX*j;
//				int i1 = i+1 + SX*j;
//				int i2 = i+1 + SX*(j+1);
//				int i3 = i + SX*(j+1);
//				unsigned char valKey=0;
//				if( valmax[i0]>valThreshold )valKey|=0x01;
//				if( valmax[i1]>valThreshold )valKey|=0x02;
//				if( valmax[i2]>valThreshold )valKey|=0x04;
//				if( valmax[i3]>valThreshold )valKey|=0x08;
//				//			float v = valmax[index];
//				//			if( velMaxZ < v ) velMaxZ = v;

//				//cout<<hex<<(int)valKey<<" ";
//				vector<Coord> line;
//				float x,y,p;
//				switch(valKey){
//				case 1:
//					//				cout<<"case1"<<endl;
//					//				cout<<"i1="<<valmax[i1]<<" thr="<<valThreshold<<" i0="<<valmax[i0]<<endl;
//					//				cout<<"i3="<<valmax[i3]<<" thr="<<valThreshold<<" i0="<<valmax[i0]<<endl;
//				case 14:
//					x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
//					y=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
//					line.push_back(img.xy(i+x,j));
//					line.push_back(img.xy(i,j+y));
//					break;
//				case 2:
//				case 13:
//					x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
//					y=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
//					line.push_back(img.xy(i+x, j));
//					line.push_back(img.xy(i+1, j+y));
//					break;
//				case 3:
//				case 12:
//					x=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
//					y=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
//					line.push_back(img.xy(i,j+x));
//					line.push_back(img.xy(i+1,j+y));
//					break;
//				case 4:
//				case 11:
//					x=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
//					y=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
//					line.push_back(img.xy(i+1, j+x));
//					line.push_back(img.xy(i+y, j+1));
//					break;
//				case 5:
//				case 10:
//					p=(valmax[i0]+valmax[i1]+valmax[i2]+valmax[i3])/4.0;
//					if(fabs(valmax[i0]-p)+fabs(valmax[i2]-p)<fabs(valmax[i1]-p)+fabs(valmax[i3]-p)){
//						//7 y 8
//						x=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
//						y=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
//						line.push_back(img.xy(i+x, j+1));
//						line.push_back(img.xy(i, j+y));
//						//2 y 13
//						x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
//						y=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
//						line.push_back(img.xy(i+x, j));
//						line.push_back(img.xy(i+1, j+y));
//					}
//					else{
//						//4 y 11
//						x=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
//						y=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
//						line.push_back(img.xy(i+1, j+x));
//						line.push_back(img.xy(i+y, j+1));
//						//1 y 14
//						x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
//						y=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
//						line.push_back(img.xy(i+x,j));
//						line.push_back(img.xy(i,j+y));
//					}
//					break;
//				case 6:
//				case 9:
//					x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
//					y=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
//					line.push_back(img.xy(i+x, j));
//					line.push_back(img.xy(i+y, j+1));
//					break;
//				case 7:
//				case 8:
//					x=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
//					y=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
//					line.push_back(img.xy(i+x, j+1));
//					line.push_back(img.xy(i, j+y));
//					break;
//				}

//				if( line.size()>0 ){
//					//cout<<"size line"<<line.size()<<endl;
//					coords.push_back(line);
//				}
//			}
//			//cout<<endl;
//		}
//		Color _color;
//		double f = (valThreshold-nsThresholdMax.niceMin)/(nsThresholdMax.niceMax-nsThresholdMax.niceMin);
//		colorMap2.getColor(f, _color.r, _color.g, _color.b);
//		cout << "valThreshold=" << valThreshold << " Color (" << (int)_color.r << ", " << (int)_color.g << ", " << (int)_color.b << ")" << endl;
//		color.push_back(_color);
//		isovel.push_back(coords);
//	}


	//	(0,SY-1)...(SX-1,SY-1)
	//	  .             .
	//	  .             .
	//	  .             .
	//	(0,0)......(SX-1,0)

	//isovel=new (isoValue(valmax, SX, SY));
	//isoValue(valmax, SX, SY);
	//isoValue(valmax2.data, SX, SY);
	double nbgx=infoData.NBGX()-1;
	double nbgy=infoData.NBGY()-1;
	double nLx=infoData.NEDX()-infoData.NBGX()+1;
	double nLy=infoData.NEDY()-infoData.NBGY()+1;
	double maxSlip=-std::numeric_limits<double>::max();
	double maxSlipLAT;
	double maxSlipLON;
	for (const auto& row : coordInvSlip){
		for (const auto& p : row){
			if(maxSlip<=p.second){
				maxSlip=p.second;
				maxSlipLAT=p.first.lat;
				maxSlipLON=p.first.lon;
			}
		}
	}
	double iniSlip_lat=maxSlipLAT;
	double iniSlip_lon=maxSlipLON;

	double imglon1=iniSlip_lon+(nbgx-infoData.NXSC())*dh/latlon2m;
	double imglat1=iniSlip_lat+(nbgy-infoData.NYSC())*dh/latlon2m;

	double imglon2=imglon1+nLx*dh/latlon2m;
	double imglat2=imglat1+nLy*dh/latlon2m;
	img=ImgT(imglon1,imglon2,imglat1,imglat2, nx, ny);
	isovel=isoValue(valmax2);

	stringstream namePNG_MaxZ,nameSVG_MaxZ,nameRoot_MaxZ;
	nameRoot_MaxZ<<nFilePrefix<<"-MaxZ";
	namePNG_MaxZ<<nameRoot_MaxZ.str()<<".png";
	nameSVG_MaxZ<<nameRoot_MaxZ.str()<<".svg";
	cout<<namePNG_MaxZ.str()<<endl;
	std::ofstream outZ("./imagenes/"+namePNG_MaxZ.str(), std::ios::binary);
	TinyPngOut pngoutMaxZ(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outZ);
	std::vector<uint8_t> lineMaxZ(static_cast<size_t>(SX) * 3);
	for (int y = SY-1; y >= 0; y--) {
		for (int x = 0; x < SX; x++) {
			uint8_t r, g, b;
			int index_xy = x + SX * y;
			//double f = (valmax[index_xy]-nsMax.niceMin)/(nsMax.niceMax-nsMax.niceMin);
			double f = (valmax2.get(x,y)-nsMax.niceMin)/(nsMax.niceMax-nsMax.niceMin);

			colorMap2.getColor(f, r, g, b);

			//cout<<index_xy<<" "<<particionesID[valID[index_xy]]<<endl;
//			if( particionesID[valID[index_xy]]==63){
//				r=255;
//				g=255;
//				b=255;
//			}

//			if( id2ijk.at(valID[index_xy]).i % 2 != id2ijk.at(valID[index_xy]).j % 2 ){
//				r=3*r/4;
//				g=3*g/4;
//				b=3*b/4;
//			}

//			if( (x-1)*(x-1)+(y-1)*(y-1)<100 ){
//				r=r/2;
//				g=g/2;
//				b=b/2;
//			}

//			if( (x-infoData.NXSC())*(x-infoData.NXSC())+(y-infoData.NYSC())*(y-infoData.NYSC())<100 ){
//				r=r/2;
//				g=g/2;
//				b=b/2;
//			}
//			if( x==infoData.NXSC() && y==infoData.NYSC() ){
//				r=0;
//				g=0;
//				b=0;
//			}

//			if( id2ijk.at(valID[index_xy]).i % 2 == id2ijk.at(valID[index_xy]).j % 2 ){
//				r=3*r/4;
//				g=3*g/4;
//				b=3*b/4;
//			}
//			if( (x-infoData.NXSC())*(x-infoData.NXSC())+(y-infoData.NYSC())*(y-infoData.NYSC())<100 ){
//				r=r/2;
//				g=g/2;
//				b=b/2;
//			}
//			if( x==infoData.NXSC() && y==infoData.NYSC() ){
//				r=0;
//				g=0;
//				b=0;
//			}
			lineMaxZ[x * 3 + 0] = static_cast<uint8_t>(r);
			lineMaxZ[x * 3 + 1] = static_cast<uint8_t>(g);
			lineMaxZ[x * 3 + 2] = static_cast<uint8_t>(b);
		}
		//cout<<endl;
		pngoutMaxZ.write(lineMaxZ.data(), static_cast<size_t>(SX));
	}
	make_svg_max("./imagenes/"+nameSVG_MaxZ.str(),"./"+namePNG_MaxZ.str(), 0,valmax2.Max());

	//Oaxaca eq 2 group 1
	//double Ms=7.3;
	//double Dp=20000.0;

	//Acapulco eq 2 group 1
	//double Ms=7.1;
	//double Dp=15000.0;

	//Chiapas 2017 eq 2 group 2
	//double Ms=8.3;
	//double Dp=40000.0;

	if( Ms>0.0	&& Dp>0.0
			&& (Eq==1 || Eq==2)
			&& (Group==1 || Group==2 || Group==3 ) ){
		ofstream outDataMMI("ValoresMMI.dat",ios::out);
		if (!outDataMMI)
			cout << "\n No se pudo crear el archivo ValoresMMI.dat"<<endl;
		else {
			outDataMMI<<"Ms="<<Ms<<endl;
			outDataMMI<<"Dp="<<Dp<<endl;
			outDataMMI<<"Eq="<<Eq<<endl;
			outDataMMI<<"Group="<<Group<<endl;
			outDataMMI<<"B0:"<<B0[Eq-2][Group-1]<<endl;
			outDataMMI<<"B1:"<<B1[0][Group-1]<<endl;
			outDataMMI<<"B2:"<<B2[0][Group-1]<<endl;
			outDataMMI<<"B3:"<<B3[0][Group-1]<<endl;
			outDataMMI.close();
		}

		double epicx=(double)(infoData.NXSC()-infoData.NBGX()+xhip)*(infoData.DH());
		double epicy=(double)(infoData.NYSC()-infoData.NBGY()+yhip)*(infoData.DH());
		double Lx=infoData.SX()*infoData.NSKPX()*infoData.DH();
		double Ly=infoData.SY()*infoData.NSKPY()*infoData.DH();
		stringstream namePNG_MMI,nameSVG_MMI,nameRoot_MMI;
		nameRoot_MMI<<nFilePrefix<<"-MMI";
		namePNG_MMI<<nameRoot_MMI.str()<<".png";
		nameSVG_MMI<<nameRoot_MMI.str()<<".svg";
		cout<<namePNG_MMI.str()<<endl;
		std::ofstream outMMI("./imagenes/"+namePNG_MMI.str(), std::ios::binary);
		TinyPngOut pngoutMMI(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outMMI);
		std::vector<uint8_t> lineMMI(static_cast<size_t>(SX) * 3);

		for (int y = SY-1; y >= 0; y--) {
			for (int x = 0; x < SX; x++) {
				uint8_t r, g, b;
				double dx=x*infoData.DH()-epicx;
				double dy=y*infoData.DH()-epicy;
				double D=sqrt(dx*dx+dy*dy);
				if(D>Dp){
					double mmi = EqMMI(Eq, Group, D, Dp, Ms);
					for(const auto& m: MMI_Scale ){
						if( mmi >=  m.min && mmi <  m.max ){ r=m.c.r; g=m.c.g; b=m.c.b;}
					}
				}
				else{
					r=0;
					g=0;
					b=0;
				}

				lineMMI[x * 3 + 0] = static_cast<uint8_t>(r);
				lineMMI[x * 3 + 1] = static_cast<uint8_t>(g);
				lineMMI[x * 3 + 2] = static_cast<uint8_t>(b);
			}
			pngoutMMI.write(lineMMI.data(), static_cast<size_t>(SX));
		}

		vector<double> vRadio;

		for(int i=1; i<=12; i++){
			double Di=Dp;
			double I=i+0.5;
			double Df=Di + ( I - EqMMI( Eq, Group, Di, Dp, Ms) )/dEqMMI_dD( Eq, Group, Di, Dp, Ms );
			while(fabs(I - EqMMI( Eq, Group, Di, Dp, Ms ))>0.001){
				Di=Df;
				Df = Di + ( I - EqMMI( Eq, Group, Di, Dp, Ms ) )/dEqMMI_dD( Eq, Group, Di, Dp, Ms );
			}
			cout<<"Df:"<<Df<<" mmi:"<<EqMMI( Eq, Group, Df, Dp, Ms)<<endl;
			vRadio.push_back(Df);
		}

		make_svg_mmi("./imagenes/"+nameSVG_MMI.str(),"./"+namePNG_MMI.str(),vRadio);

		stringstream namePNG_MMI_MaxVel,nameSVG_MMI_MaxVel,nameRoot_MMI_MaxVel;
		nameRoot_MMI_MaxVel<<nFilePrefix<<"-MMI_MaxVel";
		namePNG_MMI_MaxVel<<nameRoot_MMI_MaxVel.str()<<".png";
		nameSVG_MMI_MaxVel<<nameRoot_MMI_MaxVel.str()<<".svg";
		cout<<namePNG_MMI_MaxVel.str()<<endl;
		make_svg_mmi_maxvel("./imagenes/"+nameSVG_MMI_MaxVel.str(),"./"+namePNG_MaxZ.str(), 0,valmax2.Max(),vRadio);
	}




	auto t1T = clk::now();
	auto diff = duration_cast<seconds>(t1T-t0T);
	std::cout <<" tiempo de lectura "<<fixed<< diff.count() << "s"<<endl;

	return 0;
}
