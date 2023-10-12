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

extern double latlon2m;
//double latlon2m=104270.0;
//double VELXMIN;
//double VELXMAX;
//double epi_lon;
//double epi_lat;

//int image_width;
//int image_height;
//string nameFileCSV;
//int colLabelFileCSV;

//double nx;//celdas en x del dominio físico de simulación
//double ny;//celdas en y del dominio físico de simulación

//double lx;//ancho dominio físico de simulación en metros
//double ly;//alto dominio físico de simulación en metros

double dt;
int digTime;

int ndigID=4;//número de dígitos para el identificador del sub-dominio
int ndigTI;//5 número de dígitos para el índice del tiempo, los datos mas actuales tiene valor de 5

//double theta=0.0;
vector< vector < float > > vSlip;
//NiceScale *nsSlip;
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
extern lut colorMap4;

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
//ImgT img;
//vector< vector<Coord> > coords;
//vector< vector< vector<Coord> > > isovel; //isovel coords coord
//vector< Color > color;

struct curve{
	vector< vector<Coord> > iso;
	Color color;
};



template<typename T>
struct data2D{
	vector<T> data;
	int Nx,Ny;
	T max = -std::numeric_limits<T>::max();
	T min = std::numeric_limits<T>::max();
	T theta;

	ImgT imgT;
	string fileName;
	string labelColorMap;

	data2D(int Nx, int Ny):Nx(Nx),Ny(Ny){
		data.resize( Nx*Ny );
	}

	data2D(int Nx, int Ny, T ini):Nx(Nx),Ny(Ny){
		data.assign( Nx*Ny, ini );
	}

	data2D(ImgT imgT):imgT(imgT){
		this->Nx=this->imgT.SX;
		this->Ny=this->imgT.SY;
		data.assign( this->Nx*this->Ny, -std::numeric_limits<float>::max() );
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


Coord interpola(double alpha, double beta, Coord a, Coord b, Coord c, Coord d){
	Coord ab=a+alpha*(b-a);
	Coord dc=d+alpha*(c-d);
	return ab+beta*(dc-ab);
}
//agregando función readSlip







//void slip2(SVG2D &svg, vector< vector< pair<Coord,float> > > coordInvSlip, bool addColor=true){
void addSlip2SVG(SVG2D &svg, dataSlip slip, bool addColor=true){
	float strokeWidth=0.005*svg.TXY->textHeight;
	int J1, J2, J3, J4;
	int I1, I2, I3, I4;
	double beta1, beta2, beta3, beta4;
	double alpha1, alpha2, alpha3, alpha4;
	//cout<<"coordInvSlip.size()"<<slip.data.size()<<endl;
	svg.add( Shape().IniGroup("Slip"));
	for(int j=0; j<slip.data.size(); j++ ){
		J1=J2=J3=J4=j;
		beta1=beta2=beta3=beta4=0.5;
		if(j==0){
			J1=J2=j+1;
			beta1=beta2=-0.5;
		}
		if(j==slip.data.size()-1){
			J3=J4=j-1;
			beta3=beta4=1.5;
		}
		for(int i=0; i<slip.data[j].size(); i++ ){
			vector<float> p;
			I1=I2=I3=I4=i;
			alpha1=alpha2=alpha3=alpha4=0.5;
			if(i==0){
				I1=I4=i+1;
				alpha1=alpha4=-0.5;
			}
			if(i==slip.data[j].size()-1){
				I2=I3=i-1;
				alpha2=alpha3=1.5;
			}

			Coord p1=interpola(alpha1,beta1,slip.data[J1-1][I1-1].coord,slip.data[J1-1][I1].coord,slip.data[J1][I1].coord,slip.data[J1][I1-1].coord);
			Coord p2=interpola(alpha2,beta2,slip.data[J2-1][I2].coord,slip.data[J2-1][I2+1].coord,slip.data[J2][I2+1].coord,slip.data[J2][I2].coord);
			Coord p3=interpola(alpha3,beta3,slip.data[J3][I3].coord,slip.data[J3][I3+1].coord,slip.data[J3+1][I3+1].coord,slip.data[J3+1][I3].coord);
			Coord p4=interpola(alpha4,beta4,slip.data[J4][I4-1].coord,slip.data[J4][I4].coord,slip.data[J4+1][I4].coord,slip.data[J4+1][I4-1].coord);
			p.push_back( svg.TXY->x(p1.lon) );p.push_back( svg.TXY->y(p1.lat) );
			p.push_back( svg.TXY->x(p2.lon) );p.push_back( svg.TXY->y(p2.lat) );
			p.push_back( svg.TXY->x(p3.lon) );p.push_back( svg.TXY->y(p3.lat) );
			p.push_back( svg.TXY->x(p4.lon) );p.push_back( svg.TXY->y(p4.lat) );
//			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
//			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
//			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
//			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (slip.data[j][i].slip -slip.min)/(slip.max-slip.min);
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

void addHyp2SVG(SVG2D &svg, dataSlip slip){
	svg.add( Shape().IniGroup("Hyp"));
	vector<float> polygon;
	int Npol=3;
	for(int i=0; i<Npol; i++){
		double alpha=360*i/Npol;
		double lng = slip.LON;
		double lat = slip.LAT+0.2*svg.TXY->inv_sy( svg.TXY->pointHeight );
		rot(alpha,slip.LON, slip.LAT, lng, lat);
		polygon.push_back( svg.TXY->x(lng) );
		polygon.push_back( svg.TXY->y(lat) );
	}
	svg.add( Shape().Polygon(polygon).fill(255,0,0).stroke(0,0,0).strokeWidth(0.01*svg.TXY->textHeight).opacity(0.8) );
	svg.add( Shape().EndGroup());
}

void addHoles2SVG(SVG2D &svg, inputData &infoData){
	svg.add( Shape().IniGroup("Holes"));
	for(int i=0; i<infoData.HOLE_N(); i++){
		//double x=T().x(epi_lon+(x-infoData.HOLE_X(i))*dh/latlon2m);
		//double y=T().y(epi_lat+(y-infoData.HOLE_Y(i)+dy)*dh/latlon2m);
		//double x=Tx+T().sx(infoData.HOLE_X(i)*dh/latlon2m);
		//double y=Ty+Tdy-T().sy(infoData.HOLE_Y(i)*dh/latlon2m);
		//svg.add( Shape().Circle(x,y,0.6*T().pointHeight).fill(0,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
		stringstream coordEst;
		//coordEst<<"("<<infoData.HOLE_X(i)<<", "<<infoData.HOLE_Y(i)<<")";
		//svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() ).align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );


//		svg.add( Shape().Text(x,y-1.5*T().pointHeight,to_string(i+1))
//				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
		//cout<<"hole["<<i<<"]=("<<infoData.HOLE_X(i)<<","<<infoData.HOLE_Y(i)<<")=("<<x<<","<<y<<")"<<endl;

	}
	svg.add( Shape().EndGroup());
}

void addPoints2SVG(SVG2D &svg, vector<Coord> puntos){
	svg.add( Shape().IniGroup("Puntos"));
	for(const Coord& punto: puntos ){
		double x=svg.TXY->x(punto.lon);
		double y=svg.TXY->y(punto.lat);
		svg.add( Shape().Circle(x,y,0.6*svg.TXY->pointHeight)
				 .fill(255,0,255).stroke(0,0,0).strokeWidth(0.05*svg.TXY->textHeight).opacity(0.8) );
	}
	svg.add( Shape().EndGroup());
}





vector< curve > isoValue(data2D<float> &value){
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
	NiceScale nsThresholdMax(0,velMaxZ,20);
	int N=nsThresholdMax.N();
	for ( int k=1; k<N-1; k++ ){
		float valThreshold=nsThresholdMax.niceMin + (float)k*nsThresholdMax.tickSpacing;
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
				vector<Coord> lines;
				float x,y,p;
				switch(valKey){
				case 1:
				case 14:
					x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
					y=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
					lines.push_back(value.imgT.ij2latlng(i+x,j));
					lines.push_back(value.imgT.ij2latlng(i,j+y));
					break;
				case 2:
				case 13:
					x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
					y=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
					lines.push_back(value.imgT.ij2latlng(i+x, j));
					lines.push_back(value.imgT.ij2latlng(i+1, j+y));
					break;
				case 3:
				case 12:
					x=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
					y=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
					lines.push_back(value.imgT.ij2latlng(i,j+x));
					lines.push_back(value.imgT.ij2latlng(i+1,j+y));
					break;
				case 4:
				case 11:
					x=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
					y=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
					lines.push_back(value.imgT.ij2latlng(i+1, j+x));
					lines.push_back(value.imgT.ij2latlng(i+y, j+1));
					break;
				case 5:
				case 10:
					p=(value.data[i0]+value.data[i1]+value.data[i2]+value.data[i3])/4.0;
					if(fabs(value.data[i0]-p)+fabs(value.data[i2]-p)<fabs(value.data[i1]-p)+fabs(value.data[i3]-p)){
						//7 y 8
						x=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
						y=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
						lines.push_back(value.imgT.ij2latlng(i+x, j+1));
						lines.push_back(value.imgT.ij2latlng(i, j+y));
						//2 y 13
						x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
						y=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
						lines.push_back(value.imgT.ij2latlng(i+x, j));
						lines.push_back(value.imgT.ij2latlng(i+1, j+y));
					}
					else{
						//4 y 11
						x=(valThreshold-value.data[i1])/(value.data[i2]-value.data[i1]);
						y=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
						lines.push_back(value.imgT.ij2latlng(i+1, j+x));
						lines.push_back(value.imgT.ij2latlng(i+y, j+1));
						//1 y 14
						x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
						y=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
						lines.push_back(value.imgT.ij2latlng(i+x,j));
						lines.push_back(value.imgT.ij2latlng(i,j+y));
					}
					break;
				case 6:
				case 9:
					x=(valThreshold-value.data[i0])/(value.data[i1]-value.data[i0]);
					y=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
					lines.push_back(value.imgT.ij2latlng(i+x, j));
					lines.push_back(value.imgT.ij2latlng(i+y, j+1));
					break;
				case 7:
				case 8:
					x=(valThreshold-value.data[i3])/(value.data[i2]-value.data[i3]);
					y=(valThreshold-value.data[i0])/(value.data[i3]-value.data[i0]);
					lines.push_back(value.imgT.ij2latlng(i+x, j+1));
					lines.push_back(value.imgT.ij2latlng(i, j+y));
					break;
				}
				if( lines.size()>0 ){
					isovalue.iso.push_back(lines);
				}
			}
		}
		Color _color;
		double f = (valThreshold-nsThresholdMax.niceMin)/(nsThresholdMax.niceMax-nsThresholdMax.niceMin);
		colorMap2.getColor(f, _color.r, _color.g, _color.b);
		cout << "valThreshold=" << valThreshold << " Color (" << (int)_color.r << ", " << (int)_color.g << ", " << (int)_color.b << ")" << endl;
		isovalue.color.set(_color);
		cs.push_back(isovalue);
	}
	return cs;
}

template <typename T>
void getdata(ifstream &file, T &x){
	string str;
	getline (file, str);
	stringstream sstr(str);
	sstr>>x;
}



DisplaySettings loadDisplaySettings(string path){
	DisplaySettings dsettings;
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
		getdata(file,dsettings.imageWidth);
		getdata(file,dsettings.imageHeight);
		getdata(file,dsettings.nameFileCSV);
		getdata(file,dsettings.colLabelFileCSV);
		getdata(file,dsettings.epi_lon);
		getdata(file,dsettings.epi_lat);
		getdata(file,dsettings.VELXMIN);
		getdata(file,dsettings.VELXMAX);
		getdata(file,dsettings.theta);
//		=image_width;
//		dsettings.imageHeight=image_height;
		//dsettings.nameFileCSV=nameFileCSV;
		//dsettings.colLabelFileCSV=colLabelFileCSV;
		//dsettings.epi_lon=epi_lon;
		//dsettings.epi_lat=epi_lat;
		//dsettings.VELXMIN=VELXMIN;
		//dsettings.VELXMAX=VELXMAX;
		//dsettings.theta=theta;


#if defined DEBUG        
		cout<<"tamaña de imagenes: "<<dsettings.imageWidth<<"x"<<dsettings.imageHeight<<endl;
		cout<<"nameFileCSV: "<<dsettings.nameFileCSV<<endl;
		cout<<"colLabelFileCSV: "<<dsettings.colLabelFileCSV<<endl;
		cout<<"epi: ("<<dsettings.epi_lon<<","<<dsettings.epi_lat<<")"<<endl;
		cout<<"VelX: "<<dsettings.VELXMIN<<" "<<dsettings.VELXMAX<<endl;
#endif
	}
	return dsettings;

}



Transform2 transformation_settings2_( ImgT &img, DisplaySettings &dsettings){

	Coord p00=img.ij2latlng(0,0);
	Coord p10=img.ij2latlng(img.SX-1,0);
	Coord p01=img.ij2latlng(0,img.SY-1);
	Coord p11=img.ij2latlng(img.SX-1,img.SY-1);
	cout<<"****************"<<endl;
	cout<<p00.lon<<", "<<p00.lat<<endl;
	cout<<p10.lon<<", "<<p10.lat<<endl;
	cout<<p01.lon<<", "<<p01.lat<<endl;
	cout<<p11.lon<<", "<<p11.lat<<endl;
	cout<<"****************"<<endl;
	BB(p00.lon,p00.lat);
	BB(p10.lon,p10.lat);
	BB(p01.lon,p01.lat);
	BB(p11.lon,p11.lat);

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
	if(dsettings.imageHeight>dsettings.imageWidth){
		double r=(double)dsettings.imageHeight/(double)dsettings.imageWidth;
		west=xc-lambda*dmax;
		east=xc+lambda*dmax;
		south=yc-lambda*dmax*r;
		north=yc+lambda*dmax*r;
	}
	else{
		double r=(double)dsettings.imageWidth/(double)dsettings.imageHeight;
		west=xc-lambda*dmax*r;
		east=xc+lambda*dmax*r;
		south=yc-lambda*dmax;
		north=yc+lambda*dmax;
	}
	cout<<"**************************"<<endl;
	cout<<"image_width="<<dsettings.imageWidth<<" image_height="<<dsettings.imageHeight<<endl;
	cout<<"west="<<west<<" east="<<east<<endl;
	cout<<"south="<<south<<" north="<<north<<endl;

	return Transform2( dsettings.imageWidth, dsettings.imageHeight,
	   west,east,south,north, ns_lat, we_long);

}

void make_map_(Transform2 &TXY, DisplaySettings &dsettings){
	cout<<"make_map_ "<<TXY.width<<" x "<<TXY.height<<endl;
	SVG2D svgmap(TXY,"./imagenes/mapa.svg","mapa");
	svgmap.add( Shape().ShapeFile(TXY, dsettings.nameFileCSV, dsettings.colLabelFileCSV,0.003*TXY.textHeight) );//pone nombre de estados
	//svgmap.add( Shape().ShapeFile(nameFileCSV) );//no pone nombre de estados
#if defined DEBUG
	cout<<"*****make_map*****"<<endl;
	cout<<"Se agrego Mapa"<<endl;
	cout<<"nameFileCSV: "<<dsettings.nameFileCSV<<endl;
	cout<<"colLabelFileCSV: "<<dsettings.colLabelFileCSV<<endl;
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
void make_svg_(Transform2 &TXY, int NTST, string nameSVG, data2D<float> &dataV, dataSlip slip, lut &colormap, NiceScale ns){

	double vmin = dataV.Min();
	double vmax = dataV.Max();
	int image_width=(int)TXY.width;
	int image_height=(int)TXY.height;

	SVG2D svg(TXY, nameSVG);

	svg.add(Shape().ColorMap(colormap,"ColorMap"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

	svg.add(Shape().Image( TXY.x(dataV.imgT.lng_nxsc), TXY.y(dataV.imgT.lat_nysc),
						  TXY.sx(dataV.imgT.LX/latlon2m), TXY.sy(dataV.imgT.LY/latlon2m),
						  dataV.fileName,
						  TXY.sx(dataV.imgT.lxsc/latlon2m), TXY.sy((dataV.imgT.LY-dataV.imgT.lysc)/latlon2m),
						   dataV.imgT.theta));

	addSlip2SVG(svg, slip);


	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Mask(TXY).fill(192, 192, 210) );

	if(NTST>=0.0){
		stringstream txt_time;
		txt_time << "Time:";
		//*dt
		//cout<<"sigfigs(dt)="<<sigfigs(dt)<<" dt="<<dt<<endl;
		txt_time << fixed << setprecision(digTime) << NTST*dataV.imgT.DT << "s";
		svg.add(Shape().Text( image_width/2.0,image_height/20.0, txt_time.str())
				.align( "middle" ).fontSize(2.0*TXY.textHeight)
				.fontFamily( "Times" ) );
	}

	double colorBarX=21.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=1.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;
	svg.add(Shape().ColorBar(colorBarX, colorBarY,
							 colorBarWidth, colorBarHeight,"ColorMap",
							 ns, dataV.labelColorMap) );

	addHyp2SVG(svg,slip);
	addHoles2SVG(svg,infoData);
	addPoints2SVG(svg, estaciones);

	svg.add( Shape().Mesh(TXY) );
	//svg.add(Shape().Rectangle(T().x(-99), T().y(12),T().sx(1),T().sy(1)).fill( 0, 255, 0) );


}




void make_svg_max(Transform2 &TXY,string nameSVG, data2D<float> &dataVmax, dataSlip slip, lut &colormap, NiceScale ns, vector< curve > &isovel){

	double vmin = 0.0;
	double vmax = dataVmax.Max();
	int image_width=TXY.width;
	int image_height=TXY.height;


	//SVG2D svg(nameSVG,image_width,image_height);
	SVG2D svg(TXY, nameSVG);
	//svg.setTransform();

	svg.add(Shape().ColorMap(colormap,"ColorMap"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

	svg.add(Shape().Image( TXY.x(dataVmax.imgT.lng_nxsc), TXY.y(dataVmax.imgT.lat_nysc),
						  TXY.sx(dataVmax.imgT.LX/latlon2m), TXY.sy(dataVmax.imgT.LY/latlon2m),
						  dataVmax.fileName,
						  TXY.sx(dataVmax.imgT.lxsc/latlon2m), TXY.sy((dataVmax.imgT.LY-dataVmax.imgT.lysc)/latlon2m),
						   dataVmax.imgT.theta ));
	addSlip2SVG(svg, slip);
	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Mask(TXY).fill(192, 192, 210) );


	svg.add( Shape().IniGroup("IsocurvasRotadas"));
	for(int k=0; k<isovel.size(); k++ ){
		for(const vector<Coord>& vs: isovel[k].iso ){
			for(int i=0; i<vs.size()-1; i+=2){
				svg.add( Shape().Line( TXY.x(vs[i].lon), TXY.y(vs[i].lat),
									   TXY.x(vs[i+1].lon), TXY.y(vs[i+1].lat) )
						.stroke( 0.7*isovel[k].color.r, 0.7*isovel[k].color.g, 0.7*isovel[k].color.b).strokeWidth(1.0).strokeLinecap("round") );
			}
		}
	}
	svg.add( Shape().EndGroup());

	Coord C00 = dataVmax.imgT.ij2latlng( 0, 0 );
	Coord C10 = dataVmax.imgT.ij2latlng( dataVmax.imgT.SX, 0 );
	Coord C01 = dataVmax.imgT.ij2latlng( 0, dataVmax.imgT.SY );
	Coord C11 = dataVmax.imgT.ij2latlng( dataVmax.imgT.SX, dataVmax.imgT.SY );
	Coord Cnsc = dataVmax.imgT.ij2latlng( dataVmax.imgT.NXSC, dataVmax.imgT.NYSC );
	svg.add( Shape().Circle(TXY.x(C00.lon), TXY.y(C00.lat),0.3*TXY.pointHeight)
			 .fill(0,255,0).stroke(0,0,0)
			 .strokeWidth(0.05*TXY.textHeight)
			 .opacity(0.8) );
	svg.add( Shape().Circle(TXY.x(C10.lon), TXY.y(C10.lat),0.3*TXY.pointHeight)
			 .fill(255,0,0).stroke(0,0,0)
			 .strokeWidth(0.05*TXY.textHeight)
			 .opacity(0.8) );
	svg.add( Shape().Circle(TXY.x(C01.lon), TXY.y(C01.lat),0.3*TXY.pointHeight)
			 .fill(255,0,0).stroke(0,0,0)
			 .strokeWidth(0.05*TXY.textHeight)
			 .opacity(0.8) );
	svg.add( Shape().Circle(TXY.x(C11.lon), TXY.y(C11.lat),0.3*TXY.pointHeight)
			 .fill(255,0,0).stroke(0,0,0)
			 .strokeWidth(0.05*TXY.textHeight)
			 .opacity(0.8) );
	svg.add( Shape().Circle(TXY.x(Cnsc.lon), TXY.y(Cnsc.lat),0.3*TXY.pointHeight)
			 .fill(0,0,0).stroke(255,0,0)
			 .strokeWidth(0.05*TXY.textHeight)
			 .opacity(0.8) );


	double colorBarX=21.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=1.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;
	svg.add(Shape().ColorBar(colorBarX, colorBarY,
							 colorBarWidth, colorBarHeight,"ColorMap",
							 ns, dataVmax.labelColorMap) );
	addHyp2SVG(svg,slip);
	svg.add( Shape().Mesh(TXY) );
}


void make_svg_mmi(Transform2 &TXY, string nameSVG, data2D<float> &dataVmax, vector<double> &vRadio, dataSlip slip){
	double vmin = 0.0;
	double vmax = dataVmax.Max();
	int image_width=TXY.width;
	int image_height=TXY.height;

//	vector< vector< pair<Coord,float> > > coordInvSlip;
//	dataSlip dSlip=ReadSlip(coordInvSlip);
//	dataSlip slip;
//	ReadSlip(slip);

	//SVG2D svg(nameSVG,image_width,image_height);
	SVG2D svg(TXY, nameSVG);

	svg.add(Shape().ColorMap(colorMap2,"ColorMap2"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

	svg.add(Shape().Image( TXY.x(dataVmax.imgT.lng_nxsc), TXY.y(dataVmax.imgT.lat_nysc),
						  TXY.sx(dataVmax.imgT.LX/latlon2m), TXY.sy(dataVmax.imgT.LY/latlon2m),
						  dataVmax.fileName,
						  TXY.sx(dataVmax.imgT.lxsc/latlon2m), TXY.sy((dataVmax.imgT.LY-dataVmax.imgT.lysc)/latlon2m),
						   dataVmax.imgT.theta ));
	addSlip2SVG(svg, slip);
	//slip2(svg, coordInvSlip);


	for(int i=0; i<vRadio.size() && vRadio[i]>0; i++ ){
		double lon = slip.LON;
		double lat = slip.LAT;
		double x=TXY.x(lon);
		double y=TXY.y(lat);
		double r=TXY.sx(vRadio[i]/latlon2m);
		double dlabel=TXY.sx(0.5*(vRadio[i]+vRadio[i-1])/latlon2m);
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
				.fillNone().stroke(255,0,0).strokeWidth(0.01*TXY.textHeight) );
		svg.add( Shape().Text( x+dlabel*cos(alpha), y-dlabel*sin(alpha), label )
				 .align("middle").fontSize(0.4*TXY.textHeight).fontFamily("Times") );
	}
	//svg.add( Shape().MaskRaw(lon1,lon2,lat2,lat1).fill(192, 192, 210) );
	//cout<<"xmin:"<<BB().xmin()<<" xmax:"<<BB().xmax()<<" lon1:"<<lon1<<" lon2:"<<lon2<<endl;
	//cout<<"ymin:"<<BB().ymin()<<" ymax:"<<BB().ymax()<<" lat1:"<<lat1<<" lat2:"<<lat2<<endl;
	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Mask(TXY).fill(192, 192, 210) );


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

		svg.add(Shape().Circle(x,y,2*TXY.pointHeight)
				.fill(mmi.c.r,mmi.c.g,mmi.c.b).stroke(0,0,0).strokeWidth(0.1*TXY.textHeight) );
		svg.add(Shape().Rectangle(x-0.5*colorBarWidth, y-0.5*h,colorBarWidth,h).fill(mmi.c.r,mmi.c.g,mmi.c.b) );
		svg.add( Shape().Text(x,y+TXY.pointHeight,mmi.label )
				 .align("middle").fontSize(0.7*TXY.textHeight).fontFamily("Times").stroke(255,255,255).strokeWidth(0.2*TXY.textHeight).opacity(0.3) );
		svg.add( Shape().Text(x,y+TXY.pointHeight,mmi.label )
				 .align("middle").fontSize(0.7*TXY.textHeight).fontFamily("Times") );
		i++;
	}

	addHyp2SVG(svg,slip);



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
	svg.add( Shape().Mesh(TXY) );

}



void make_svg_mmi_maxvel(Transform2 &TXY, string nameSVG, data2D<float> &dataVmax, vector<double> &vRadio, dataSlip slip, lut &colormap, NiceScale ns, vector< curve > &isovel){
	double vmin = 0.0;
	double vmax = dataVmax.Max();
	int image_width=TXY.width;
	int image_height=TXY.height;

//	vector< vector< pair<Coord,float> > > coordInvSlip;
//	dataSlip dSlip=ReadSlip(coordInvSlip);
//	dataSlip slip;
//	ReadSlip(slip);

//	SVG2D svg(nameSVG,image_width,image_height);
	SVG2D svg(TXY, nameSVG);
	svg.add(Shape().ColorMap(colormap,"ColorMap"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

	svg.add(Shape().Image( TXY.x(dataVmax.imgT.lng_nxsc), TXY.y(dataVmax.imgT.lat_nysc),
						  TXY.sx(dataVmax.imgT.LX/latlon2m), TXY.sy(dataVmax.imgT.LY/latlon2m),
						  dataVmax.fileName,
						  TXY.sx(dataVmax.imgT.lxsc/latlon2m), TXY.sy((dataVmax.imgT.LY-dataVmax.imgT.lysc)/latlon2m),
						   dataVmax.imgT.theta ));
	//slip(svg);
//	vector< vector< pair<Coord,float> > > coordInvSlip;
//	ReadSlip(coordInvSlip);
	//slip2(svg, coordInvSlip);
	addSlip2SVG(svg, slip);
	//svg.add(Shape().Image(Tx+rotx,Ty+roty,Tdx,Tdy,namePNG,rotx,roty,theta));
//	svg.add( Shape().MaskRaw(lon1,lon2,lat2,lat1).fill(192, 192, 210) );
//	cout<<"xmin:"<<BB().xmin()<<" xmax:"<<BB().xmax()<<" lon1:"<<lon1<<" lon2:"<<lon2<<endl;
//	cout<<"ymin:"<<BB().ymin()<<" ymax:"<<BB().ymax()<<" lat1:"<<lat1<<" lat2:"<<lat2<<endl;


	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Mask(TXY).fill(192, 192, 210) );

	double colorBarX=21.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=1.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;



	svg.add( Shape().IniGroup("IsocurvasRotadas"));
	for(int k=0; k<isovel.size(); k++ ){
		for(const vector<Coord>& vs: isovel[k].iso ){
			for(int i=0; i<vs.size()-1; i+=2){
				svg.add( Shape().Line( TXY.x(vs[i].lon), TXY.y(vs[i].lat),
									   TXY.x(vs[i+1].lon), TXY.y(vs[i+1].lat) )
						.stroke( 0.7*isovel[k].color.r, 0.7*isovel[k].color.g, 0.7*isovel[k].color.b).strokeWidth(1.0).strokeLinecap("round") );
			}
		}
	}
	svg.add( Shape().EndGroup());

	svg.add( Shape().IniGroup("IsocurvasMMI"));
	for(int i=1; i<vRadio.size() && vRadio[i]>0; i++ ){
		double lon = slip.LON;
		double lat = slip.LAT;
		double x=TXY.x(lon);
		double y=TXY.y(lat);
		double r=TXY.sx(vRadio[i]/latlon2m);
		double dlabel=svg.TXY->sx(0.5*(vRadio[i]+vRadio[i-1])/latlon2m);
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
				.fillNone().stroke(255,0,0).strokeWidth(0.01*TXY.textHeight) );
		svg.add( Shape().Text( x+dlabel*cos(alpha), y-dlabel*sin(alpha), label )
				 .align("middle").fontSize(0.2*TXY.textHeight).fontFamily("Times") );
	}
	svg.add( Shape().EndGroup());

	svg.add(Shape().ColorBar(colorBarX, colorBarY,
							 colorBarWidth, colorBarHeight,"ColorMap",
							 ns, dataVmax.labelColorMap) );


	addHyp2SVG(svg,slip);


	svg.add( Shape().Mesh( TXY ) );


}

void make_svg_slip(Transform2 &TXY, string nameSVG, string namePNG, lut &colormap, NiceScale ns){

	int image_width=TXY.width;
	int image_height=TXY.height;

	SVG2D svg(nameSVG,image_width,image_height);
	svg.add(Shape().ColorMap(colormap,"ColorMap"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

	svg.add(Shape().Image(0.05*image_width,0.125*image_height, 0.75*image_width, 0.75*image_height,namePNG,0,0,0));

	double colorBarX=21.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=1.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;

	svg.add(Shape().ColorBar(colorBarX, colorBarY,
							 colorBarWidth, colorBarHeight,"ColorMap",
							 ns, "Slip (m)") );
	//svg.add( Shape().Mesh(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()) );

}


int main(int argc, char *argv[])
{

	configData configdat(argc, argv);





	cout<<"ruta "<<configdat.path<<" paso="<<configdat.step<<endl;
	for(unsigned int i=0;i<configdat.data.size();i++){
		cout<<"Componente "<<configdat.data[i].comp<<" mínimo="<<configdat.data[i].min<<" máximo="<<configdat.data[i].max<<endl;

		if(configdat.data[i].px.size()>0){
			cout<<" Plano X"<<endl;
			for(unsigned int j=0;j<configdat.data[i].px.size();j++){
				cout<<" "<<configdat.data[i].px[j];
			}
			cout<<endl;
		}

		if(configdat.data[i].py.size()>0){
			cout<<" Plano Y"<<endl;
			for(unsigned int j=0;j<configdat.data[i].py.size();j++){
				cout<<" "<<configdat.data[i].py[j];
			}
			cout<<endl;
		}

		if(configdat.data[i].pz.size()>0){
			cout<<" Plano Z"<<endl;
			for(unsigned int j=0;j<configdat.data[i].pz.size();j++){
				cout<<" "<<configdat.data[i].pz[j];
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
	cout<<"Archivo:"<<configdat.path<<endl<<endl;
	infoData.read(configdat.path);

	infoData.show();

	DisplaySettings dsettings =	loadDisplaySettings(configdat.path);
	//vector< vector< pair<Coord,float> > > coordInvSlip;
	//dataSlip dSlip=ReadSlip(coordInvSlip);
//	theta=270.0-dSlip.STRK;
	dataSlip slip;
	ReadSlip(slip);
	slip.max=-std::numeric_limits<double>::max();
	slip.min=std::numeric_limits<double>::max();
	for (const auto& row : slip.data){
		for (const auto& p : row){
			if(slip.max<p.slip){
				slip.max=p.slip;
			}
			if(slip.min>p.slip){
				slip.min=p.slip;
			}
		}
	}
	//theta=270.0-slip.STRK;

	ImgT img=ImgT(infoData, dsettings, slip);


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

	//	xhip = infoData.xHip();
	//	yhip = infoData.yHip();


	double Ms=configdat.Ms;
	double Dp=configdat.Dp;
	unsigned int Eq=configdat.Eq;
	unsigned int Group=configdat.Group;


	//la transformación se debe revisar que parámetros recibe
	//transformation_settings_( infoData.DH(), infoData.NXSC()-infoData.NBGX(), infoData.NYSC()-infoData.NBGY(), infoData.SX()*infoData.NSKPX(), infoData.SY()*infoData.NSKPY());
	//transformation_settings_( infoData );
	Transform2 TXY=transformation_settings2_( img, dsettings );
//	Coord p00=img.ij2latlng(0,0);
//	Coord p10=img.ij2latlng(img.SX-1,0);
//	Coord p01=img.ij2latlng(0,img.SY-1);
//	Coord p11=img.ij2latlng(img.SX-1,img.SY-1);
//	cout<<"****************"<<endl;
//	cout<<p00.lon<<", "<<p00.lat<<endl;
//	cout<<p10.lon<<", "<<p10.lat<<endl;
//	cout<<p01.lon<<", "<<p01.lat<<endl;
//	cout<<p11.lon<<", "<<p11.lat<<endl;
//	cout<<"****************"<<endl;
//	BB(p00.lon,p00.lat);
//	BB(p10.lon,p10.lat);
//	BB(p01.lon,p01.lat);
//	BB(p11.lon,p11.lat);

//	NiceScale ns_lat(BB().ymin(),BB().ymax(),12);
//	NiceScale we_long(BB().xmin(),BB().xmax(),12);

//	Transform2 TXY( dsettings.imageWidth, dsettings.imageHeight,-115.0, -70.0, -3, 33.0);
//	Transform2 TXY( dsettings.imageWidth, dsettings.imageHeight,
//					BB().xmin(), BB().xmax(),
//					BB().ymin(), BB().ymax());
//	Transform2 TXY( dsettings.imageWidth, dsettings.imageHeight,
//					we_long.niceMin, we_long.niceMax,
//					ns_lat.niceMin, ns_lat.niceMax);

	make_map_(TXY, dsettings);
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
//	val.resize( SX*SY*SZ );
//	valID.resize( SX*SY*SZ );

	vector<vector<float>> valxyz(3);
	vector<vector<int>> valxyzID(3);
	for(int i=0; i<3; i++){
		valxyz[i].resize( SX*SY*SZ );
		valxyzID[i].resize( SX*SY*SZ );
	}

//	vector<float> valVx, valVy, valVz;
//	vector<int> valVxID, valVyID, valVzID;
//	valVx.resize( SX*SY*SZ );
//	valVy.resize( SX*SY*SZ );
//	valVz.resize( SX*SY*SZ );
//	valVxID.resize( SX*SY*SZ );
//	valVyID.resize( SX*SY*SZ );
//	valVzID.resize( SX*SY*SZ );

	//data2D<float> valmax2( SX, SY, -std::numeric_limits<float>::max() );

	data2D<float> valmax2( img );
	data2D<float> valxyzmax2( img );
	data2D<float> valmmi( img );

	bool calculateVmax = false;
	bool calculateVxyzmax = false;

	if( configdat.step == -1 || !ifstream("./velMax_bin.dat").good() ){
		cout<<"Se calculará vmax"<<endl;
		calculateVmax = true;
	}
	else{
		cout<<"No se calculará vmax"<<endl;
	}

	if( configdat.step == -1 || !ifstream("./velxyzMax_bin.dat").good() ){
		cout<<"Se calculará vxyzmax"<<endl;
		calculateVxyzmax = true;
	}
	else{
		cout<<"No se calculará vxyzmax"<<endl;
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

	if(configdat.step==-1){
#ifdef MI_PC
		stepIndexIni=1;
#else
		stepIndexIni=infoData.NTISKP();
#endif
		stepIndexEnd=infoData.NT();
	}
	else{
		stepIndexIni=configdat.step;
		stepIndexEnd=configdat.step+1;
	}

	if(configdat.initialstep!=-1){
		stepIndexIni=configdat.initialstep;
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

	//inicializa el arreglo
//	for(int id=0; id<NParticiones; id++){
//		for(int kk=particionesSZo[id]; kk<particionesSZf[id]; kk++){
//			int Pz=kk*SX*SY;
//			for(int jj=particionesSYo[id]; jj<particionesSYf[id]; jj++){
//				int Ly=jj*SX;
//				for(int ii=particionesSXo[id]; ii<particionesSXf[id]; ii++){
//					val[ii+Ly+Pz]=(float)id/NParticiones;
//				}
//			}
//		}
//	}

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
	NiceScale nsSlip(slipMin, slipMax,ticks);
	std::ofstream outSlip("./slip.png", std::ios::binary);
	TinyPngOut pngoutSlip(static_cast<uint32_t>(slipN), static_cast<uint32_t>(slipM), outSlip);
	std::vector<uint8_t> lineSlip(static_cast<size_t>(slipN) * 3);
	for(auto &line: vSlip){
		int i=0;
		for(auto &slip: line){
			uint8_t r, g, b;
			double f = (slip-nsSlip.niceMin)/(nsSlip.niceMax-nsSlip.niceMin);
			colorMap3.getColor(f, r, g, b);
			lineSlip[i * 3 + 0] = static_cast<uint8_t>(r);
			lineSlip[i * 3 + 1] = static_cast<uint8_t>(g);
			lineSlip[i * 3 + 2] = static_cast<uint8_t>(b);
			i++;
		}
		//cout<<endl;
		pngoutSlip.write(lineSlip.data(), static_cast<size_t>(slipN));
	}
	cout<<slipMin<<" "<<slipMax<<" -> "<<nsSlip.niceMin<<" "<<nsSlip.niceMax<<endl;
	make_svg_slip(TXY, "./slip.svg", "./slip.png", colorMap3,nsSlip );
	/***********************/

	digTime=sigfigs(infoData.DT()*infoData.NTISKP());

	string nFilePrefix;
	for(unsigned int i=0;i<configdat.data.size();i++){
		if(  configdat.data[i].px.size()==0
			 && configdat.data[i].py.size()==0
			 && configdat.data[i].pz.size()==0 ){
			continue;
		}

		if( configdat.data[i].comp==X )
			nFilePrefix="Vx3D";
		if( configdat.data[i].comp==Y )
			nFilePrefix="Vy3D";
		if( configdat.data[i].comp==Z )
			nFilePrefix="Vz3D";
		//#pragma omp parallel for
		//#pragma omp parallel for num_threads(2)
		//#pragma omp parallel for num_threads(2) collapse(2)
		//#pragma omp parallel for collapse(2)
		vector< vector< data2D<float> > > vel(3);
		vector< data2D<float> > velxyz;
		for(int timeIndex=stepIndexIni;timeIndex<stepIndexEnd;timeIndex+=dstepIndex)
		{


			//double min=cdat.data[i].min;
			//double max=cdat.data[i].max;

			//        double max=-std::numeric_limits<double>::max();
			//        double min=std::numeric_limits<double>::max();
			auto t0 = clk::now();
			cout<<nFilePrefix<<" "<<timeIndex;

			NiceScale ns(dsettings.VELXMIN,dsettings.VELXMAX);
//#pragma omp parallel for
//			for(int id=0; id<NParticiones; id++){
//				stringstream nameFile;
//				stringstream nameFileR;
//				nameFile<<nFilePrefix;
//				nameFile<<setw(ndigID)<<setfill('0')<<particionesID[id];
//				nameFile<<setw(ndigTI)<<setfill('0')<<timeIndex;
//				nameFileR<<configdat.path<<"/"<<SubLocation<<"/"<<nameFile.str();
//				std::ifstream file(nameFileR.str().c_str(),std::ios::binary|std::ios::in);
//				if( file.fail() ){
//					cerr<<" ERROR: no se puede abrir el archivo "<<nameFileR.str().c_str()<<std::endl;
//					exit (EXIT_FAILURE);
//					//return 1;
//				}

//				std::filebuf* pbuf = file.rdbuf();
//				std::size_t size = pbuf->pubseekoff (0,file.end,file.in);
//				if(size!= (unsigned long)( 4*particionesSizeArray[id] ) ){
//					cerr<<" size: "<<size<<" particionesSizeArray["<<id<<"]: "<<particionesSizeArray[id]<<std::endl;
//					cerr<<" ERROR: El tamaño del archivo no es correcto "<<nameFileR.str().c_str()<<std::endl;
//					exit (EXIT_FAILURE);
//					//return 1;
//				}

//				vector<float> buffer( particionesSizeArray[id] );
//				pbuf->pubseekpos (0,file.in);
//				pbuf->sgetn ((char*)&buffer[0],size);
//				pbuf->close();
//				file.close();

//				auto it = buffer.begin();
//				for(int kk=particionesSZo[id]; kk<particionesSZf[id]; kk++){
//					it++;
//					int Pz=(particionesSZf[id]+particionesSZo[id]-1-kk)*SX*SY;
//					for(int jj=particionesSYo[id]; jj<particionesSYf[id]; jj++){
//						int Ly=jj*SX;
//						for(int ii=particionesSXo[id]; ii<particionesSXf[id]; ii++){
//							val[ii+Ly+Pz]=*it;
//							valID[ii+Ly+Pz]=id;

//							//if(particionesID[id]==55)val[ii+Ly+Pz]=1.0;
//							it++;
//						}
//					}
//					it++;
//				}
//			}//for NParticiones #pragma omp parallel for

			string	iFilePrefix[]={"Vx3D","Vy3D","Vz3D"};
			for(int iprefix=0; iprefix<3;iprefix++){
#pragma omp parallel for
				for(int id=0; id<NParticiones; id++){
					stringstream nameFile;
					stringstream nameFileR;
					nameFile<<iFilePrefix[iprefix];
					nameFile<<setw(ndigID)<<setfill('0')<<particionesID[id];
					nameFile<<setw(ndigTI)<<setfill('0')<<timeIndex;
					nameFileR<<configdat.path<<"/"<<SubLocation<<"/"<<nameFile.str();
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
								valxyz[iprefix][ii+Ly+Pz]=*it;
								valxyzID[iprefix][ii+Ly+Pz]=id;

								//if(particionesID[id]==55)val[ii+Ly+Pz]=1.0;
								it++;
							}
						}
						it++;
					}
				}//for NParticiones #pragma omp parallel for
			}

//////////////////////////////////////
//#pragma omp parallel for
//			for(int id=0; id<NParticiones; id++){
//				stringstream nameFileVx;
//				stringstream nameFileVxR;
//				nameFileVx<<"Vx3D"<<setw(ndigID)<<setfill('0')<<particionesID[id]<<setw(ndigTI)<<setfill('0')<<timeIndex;
//				nameFileVy<<"Vy3D"<<setw(ndigID)<<setfill('0')<<particionesID[id]<<setw(ndigTI)<<setfill('0')<<timeIndex;
//				nameFileVz<<"Vz3D"<<setw(ndigID)<<setfill('0')<<particionesID[id]<<setw(ndigTI)<<setfill('0')<<timeIndex;
//				nameFileVxR<<configdat.path<<"/"<<SubLocation<<"/"<<nameFileVx.str();
//				nameFileVyR<<configdat.path<<"/"<<SubLocation<<"/"<<nameFileVy.str();
//				nameFileVzR<<configdat.path<<"/"<<SubLocation<<"/"<<nameFileVz.str();

//				std::ifstream fileVx(nameFileVxR.str().c_str(),std::ios::binary|std::ios::in);
//				std::ifstream fileVy(nameFileVyR.str().c_str(),std::ios::binary|std::ios::in);
//				std::ifstream fileVz(nameFileVzR.str().c_str(),std::ios::binary|std::ios::in);
//				if( fileVx.fail() ){
//					cerr<<" ERROR: no se puede abrir el archivo "<<nameFileVxR.str().c_str()<<std::endl;
//					exit (EXIT_FAILURE);
//				}
//				if( fileVy.fail() ){
//					cerr<<" ERROR: no se puede abrir el archivo "<<nameFileVyR.str().c_str()<<std::endl;
//					exit (EXIT_FAILURE);
//				}
//				if( fileVz.fail() ){
//					cerr<<" ERROR: no se puede abrir el archivo "<<nameFileVzR.str().c_str()<<std::endl;
//					exit (EXIT_FAILURE);
//				}

//				std::filebuf* pbufVx = fileVx.rdbuf();
//				std::filebuf* pbufVy = fileVy.rdbuf();
//				std::filebuf* pbufVz = fileVz.rdbuf();
//				std::size_t sizeVx = pbufVx->pubseekoff (0,fileVx.end,fileVx.in);
//				std::size_t sizeVy = pbufVy->pubseekoff (0,fileVy.end,fileVy.in);
//				std::size_t sizeVz = pbufVz->pubseekoff (0,fileVz.end,fileVz.in);
//				if(sizeVx!= (unsigned long)( 4*particionesSizeArray[id] ) ){
//					cerr<<" size: "<<sizeVx<<" particionesSizeArray["<<id<<"]: "<<particionesSizeArray[id]<<std::endl;
//					cerr<<" ERROR: El tamaño del archivo no es correcto "<<nameFileVxR.str().c_str()<<std::endl;
//					exit (EXIT_FAILURE);
//				}
//				if(sizeVy!= (unsigned long)( 4*particionesSizeArray[id] ) ){
//					cerr<<" size: "<<sizeVy<<" particionesSizeArray["<<id<<"]: "<<particionesSizeArray[id]<<std::endl;
//					cerr<<" ERROR: El tamaño del archivo no es correcto "<<nameFileVyR.str().c_str()<<std::endl;
//					exit (EXIT_FAILURE);
//				}
//				if(sizeVz!= (unsigned long)( 4*particionesSizeArray[id] ) ){
//					cerr<<" size: "<<sizeVz<<" particionesSizeArray["<<id<<"]: "<<particionesSizeArray[id]<<std::endl;
//					cerr<<" ERROR: El tamaño del archivo no es correcto "<<nameFileVzR.str().c_str()<<std::endl;
//					exit (EXIT_FAILURE);
//				}

//				vector<float> bufferVx( particionesSizeArray[id] );
//				pbufVx->pubseekpos (0,fileVx.in);
//				pbufVx->sgetn ((char*)&bufferVx[0],sizeVx);
//				pbufVx->close();
//				fileVx.close();

//				vector<float> bufferVx( particionesSizeArray[id] );
//				pbufVx->pubseekpos (0,fileVx.in);
//				pbufVx->sgetn ((char*)&bufferVx[0],sizeVx);
//				pbufVx->close();
//				fileVx.close();

//				auto it = bufferVx.begin();
//				for(int kk=particionesSZo[id]; kk<particionesSZf[id]; kk++){
//					it++;
//					int Pz=(particionesSZf[id]+particionesSZo[id]-1-kk)*SX*SY;
//					for(int jj=particionesSYo[id]; jj<particionesSYf[id]; jj++){
//						int Ly=jj*SX;
//						for(int ii=particionesSXo[id]; ii<particionesSXf[id]; ii++){
//							val[ii+Ly+Pz]=*it;
//							valID[ii+Ly+Pz]=id;

//							//if(particionesID[id]==55)val[ii+Ly+Pz]=1.0;
//							it++;
//						}
//					}
//					it++;
//				}
//			}//for NParticiones #pragma omp parallel for
//////////////////////////////////////

//			for(unsigned int j=0;j<configdat.data[i].px.size();j++){
//				stringstream namePNG_X;
//				namePNG_X<<"./imagenes/X/"<<nFilePrefix<<"-X"<<setw(calculaCifras(SX))<<setfill('0')<<configdat.data[i].px[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex<<".png";
//				cout<<namePNG_X.str()<<endl;
//				std::ofstream outX(namePNG_X.str(), std::ios::binary);
//				TinyPngOut pngoutX(static_cast<uint32_t>(SY), static_cast<uint32_t>(SZ), outX);
//				std::vector<uint8_t> lineX(static_cast<size_t>(SY) * 3);
//				for (int z = SZ-1; z >= 0; z--) {
//					for (int y = 0; y < SY; y++) {
//						uint8_t r, g, b;
//						double f = (val[configdat.data[i].px[j]+SX*y+SX*SY*z]-ns.niceMin)/(ns.niceMax-ns.niceMin);
//						colorMap1.getColor(f, r, g, b);
//						lineX[y * 3 + 0] = static_cast<uint8_t>(r);
//						lineX[y * 3 + 1] = static_cast<uint8_t>(g);
//						lineX[y * 3 + 2] = static_cast<uint8_t>(b);
//					}
//					pngoutX.write(lineX.data(), static_cast<size_t>(SY));
//				}
//			}

//			for(unsigned int j=0;j<configdat.data[i].py.size();j++){
//				stringstream namePNG_Y;
//				namePNG_Y<<"./imagenes/Y/"<<nFilePrefix<<"-Y"<<setw(calculaCifras(SY))<<setfill('0')<<configdat.data[i].py[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex<<".png";
//				cout<<namePNG_Y.str()<<endl;
//				std::ofstream outY(namePNG_Y.str(), std::ios::binary);
//				TinyPngOut pngoutY(static_cast<uint32_t>(SX), static_cast<uint32_t>(SZ), outY);
//				std::vector<uint8_t> lineY(static_cast<size_t>(SX) * 3);
//				for (int z = SZ-1; z >= 0; z--) {
//					for (int x = 0; x < SX; x++) {
//						uint8_t r, g, b;
//						double f = (val[x+SX*configdat.data[i].py[j]+SX*SY*z]-ns.niceMin)/(ns.niceMax-ns.niceMin);
//						colorMap1.getColor(f, r, g, b);
//						lineY[x * 3 + 0] = static_cast<uint8_t>(r);
//						lineY[x * 3 + 1] = static_cast<uint8_t>(g);
//						lineY[x * 3 + 2] = static_cast<uint8_t>(b);
//					}
//					pngoutY.write(lineY.data(), static_cast<size_t>(SX));
//				}

//			}

//			for(unsigned int j=0;j<configdat.data[i].pz.size();j++){
//				stringstream namePNG_Z,nameSVG_Z,nameRoot_Z;
//				nameRoot_Z<<nFilePrefix<<"-Z"<<setw(calculaCifras(SZ))<<setfill('0')<<configdat.data[i].pz[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex;
//				namePNG_Z<<nameRoot_Z.str()<<".png";
//				nameSVG_Z<<nameRoot_Z.str()<<".svg";
//				cout<<namePNG_Z.str()<<endl;
//				std::ofstream outZ("./imagenes/Z/"+namePNG_Z.str(), std::ios::binary);
//				vel.back().fileName="./Z/"+namePNG_Z.str();
//				vel.back().labelColorMap="Vx (m/s)";
//				TinyPngOut pngoutZ(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outZ);
//				std::vector<uint8_t> lineZ(static_cast<size_t>(SX) * 3);
//				//cout<<"min:"<<min<<" max:"<<max<<endl;
//				int index_z=SX*SY*configdat.data[i].pz[j];
//				for (int y = SY-1; y >= 0; y--) {
//					int index_yz=SX*y+index_z;
//					for (int x = 0; x < SX; x++) {
//						uint8_t r, g, b;
//						//double f = (val[x+SX*y+SX*SY*cdat.data[i].pz[j]]-ns.niceMin)/(ns.niceMax-ns.niceMin);
//						int index_xyz = x + index_yz;
//						int index_xy = x + SX * y;
//						float v=val[index_xyz];
//						vel.back().set(x,y,v);
//						double f = (v-ns.niceMin)/(ns.niceMax-ns.niceMin);
//						//valmax[index_xy] = std::max( valmax[index_xy], fabs(v) );

//						if( calculateVmax ){
//							valmax2.set(x,y,std::max( valmax2.get(x,y), fabs(v) ));
//						}

//						colorMap1.getColor(f, r, g, b);
//						//if( particionesID[ valID[index_xy] ]==43 ){
//						//	r=r;
//						//	g=3*g/4;
//						//	b=3*b/4;
//						//}

//						if( id2ijk.at(valID[index_xy]).i % 2 != id2ijk.at(valID[index_xy]).j % 2 ){
//							r=3*r/4;
//							g=3*g/4;
//							b=3*b/4;
//						}
//						if( (x-infoData.NXSC())*(x-infoData.NXSC())+(y-infoData.NYSC())*(y-infoData.NYSC())<100 ){
//							r=r/2;
//							g=g/2;
//							b=b/2;
//						}
//						if( x==infoData.NXSC() && y==infoData.NYSC() ){
//							r=0;
//							g=0;
//							b=0;
//						}
//						lineZ[x * 3 + 0] = static_cast<uint8_t>(r);
//						lineZ[x * 3 + 1] = static_cast<uint8_t>(g);
//						lineZ[x * 3 + 2] = static_cast<uint8_t>(b);
//					}
//					//cout<<endl;
//					pngoutZ.write(lineZ.data(), static_cast<size_t>(SX));
//				}
//				make_svg_(TXY,timeIndex,"./imagenes/"+nameSVG_Z.str(),vel.back(), slip, colorMap1,ns);

//				for(int iprefix=0; iprefix<3;iprefix++){
//					vel[iprefix].push_back( data2D<float>(img) );
//				}

//			}

			//////////////////////////////
			NiceScale nsxyz(0,dsettings.VELXMAX);
			for(unsigned int j=0;j<configdat.data[i].pz.size();j++){

				velxyz.push_back( data2D<float>(img) );
				vel[X].push_back( data2D<float>(img) );
				vel[Y].push_back( data2D<float>(img) );
				vel[Z].push_back( data2D<float>(img) );

				stringstream namePNG_XYZ,nameSVG_XYZ,nameRoot_XYZ;
				stringstream namePNG_X,nameSVG_X,nameRoot_X;
				stringstream namePNG_Y,nameSVG_Y,nameRoot_Y;
				stringstream namePNG_Z,nameSVG_Z,nameRoot_Z;

				nameRoot_XYZ<<"Vxyz3D-Z"<<setw(calculaCifras(SZ))<<setfill('0')<<configdat.data[i].pz[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex;
				namePNG_XYZ<<nameRoot_XYZ.str()<<".png";
				nameSVG_XYZ<<nameRoot_XYZ.str()<<".svg";
				cout<<namePNG_XYZ.str()<<endl;

				nameRoot_X<<iFilePrefix[X]<<"-Z"<<setw(calculaCifras(SZ))<<setfill('0')<<configdat.data[i].pz[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex;
				namePNG_X<<nameRoot_X.str()<<".png";
				nameSVG_X<<nameRoot_X.str()<<".svg";
				cout<<namePNG_X.str()<<endl;

				nameRoot_Y<<iFilePrefix[Y]<<"-Z"<<setw(calculaCifras(SZ))<<setfill('0')<<configdat.data[i].pz[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex;
				namePNG_Y<<nameRoot_Y.str()<<".png";
				nameSVG_Y<<nameRoot_Y.str()<<".svg";
				cout<<namePNG_Y.str()<<endl;

				nameRoot_Z<<iFilePrefix[Z]<<"-Z"<<setw(calculaCifras(SZ))<<setfill('0')<<configdat.data[i].pz[j]<<"-TI"<<setw(ndigTI)<<setfill('0')<<timeIndex;
				namePNG_Z<<nameRoot_Z.str()<<".png";
				nameSVG_Z<<nameRoot_Z.str()<<".svg";
				cout<<namePNG_Z.str()<<endl;

				std::ofstream outXYZ("./imagenes/Z/"+namePNG_XYZ.str(), std::ios::binary);
				std::ofstream outX("./imagenes/Z/"+namePNG_X.str(), std::ios::binary);
				std::ofstream outY("./imagenes/Z/"+namePNG_Y.str(), std::ios::binary);
				std::ofstream outZ("./imagenes/Z/"+namePNG_Z.str(), std::ios::binary);

				velxyz.back().fileName="./Z/"+namePNG_XYZ.str();
				velxyz.back().labelColorMap="V (m/s)";

				vel[X].back().fileName="./Z/"+namePNG_X.str();
				vel[X].back().labelColorMap="Vx (m/s)";

				vel[Y].back().fileName="./Z/"+namePNG_Y.str();
				vel[Y].back().labelColorMap="Vy (m/s)";

				vel[Z].back().fileName="./Z/"+namePNG_Z.str();
				vel[Z].back().labelColorMap="Vz (m/s)";

				TinyPngOut pngoutXYZ(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outXYZ);
				TinyPngOut pngoutX(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outX);
				TinyPngOut pngoutY(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outY);
				TinyPngOut pngoutZ(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outZ);

				std::vector<uint8_t> lineXYZ(static_cast<size_t>(SX) * 3);
				std::vector<uint8_t> lineX(static_cast<size_t>(SX) * 3);
				std::vector<uint8_t> lineY(static_cast<size_t>(SX) * 3);
				std::vector<uint8_t> lineZ(static_cast<size_t>(SX) * 3);

				//cout<<"min:"<<min<<" max:"<<max<<endl;
				int index_z=SX*SY*configdat.data[i].pz[j];
				for (int y = SY-1; y >= 0; y--) {
					int index_yz=SX*y+index_z;
					for (int x = 0; x < SX; x++) {
						uint8_t rxyz, gxyz, bxyz;
						uint8_t rx, gx, bx;
						uint8_t ry, gy, by;
						uint8_t rz, gz, bz;
						//double f = (val[x+SX*y+SX*SY*cdat.data[i].pz[j]]-ns.niceMin)/(ns.niceMax-ns.niceMin);
						int index_xyz = x + index_yz;
						int index_xy = x + SX * y;
//						float v=val[index_xyz];

						float vx=valxyz[X][index_xyz];
						float vy=valxyz[Y][index_xyz];
						float vz=valxyz[Z][index_xyz];
						float vxyz=sqrt(vx*vx+vy*vy+vz*vz);

						velxyz.back().set(x,y,vxyz);
						vel[X].back().set(x,y,vx);
						vel[Y].back().set(x,y,vy);
						vel[Z].back().set(x,y,vz);

						if( calculateVmax ){
							valmax2.set(x,y,std::max( valmax2.get(x,y), fabs(vx) ));
						}
						if( calculateVxyzmax ){
							valxyzmax2.set(x,y,std::max( valxyzmax2.get(x,y), fabs(vxyz) ));
						}

						double fxyz = (vxyz-nsxyz.niceMin)/(nsxyz.niceMax-nsxyz.niceMin);
						double fx = (vx-ns.niceMin)/(ns.niceMax-ns.niceMin);
						double fy = (vy-ns.niceMin)/(ns.niceMax-ns.niceMin);
						double fz = (vz-ns.niceMin)/(ns.niceMax-ns.niceMin);

						colorMap4.getColor(fxyz, rxyz, gxyz, bxyz);
						colorMap1.getColor(fx, rx, gx, bx);
						colorMap1.getColor(fy, ry, gy, by);
						colorMap1.getColor(fz, rz, gz, bz);
						//if( particionesID[ valID[index_xy] ]==43 ){
						//	r=r;
						//	g=3*g/4;
						//	b=3*b/4;
						//}

						if( id2ijk.at(valxyzID[X][index_xy]).i % 2 != id2ijk.at(valxyzID[X][index_xy]).j % 2 ){
							rxyz=3*rxyz/4; gxyz=3*gxyz/4; bxyz=3*bxyz/4;
							rx=3*rx/4; gx=3*gx/4; bx=3*bx/4;
							ry=3*ry/4; gy=3*gy/4; by=3*by/4;
							rz=3*rz/4; gz=3*gz/4; bz=3*bz/4;
						}
						if( (x-infoData.NXSC())*(x-infoData.NXSC())+(y-infoData.NYSC())*(y-infoData.NYSC())<100 ){
							rxyz=rxyz/2; gxyz=gxyz/2; bxyz=bxyz/2;
							rx=rx/2; gx=gx/2; bx=bx/2;
							ry=ry/2; gy=gy/2; by=by/2;
							rz=rz/2; gz=gz/2; bz=bz/2;
						}
						if( x==infoData.NXSC() && y==infoData.NYSC() ){
							rxyz=0; gxyz=0; bxyz=0;
							rx=0; gx=0; bx=0;
							ry=0; gy=0; by=0;
							rz=0; gz=0; bz=0;
						}
						lineXYZ[x * 3 + 0] = static_cast<uint8_t>(rxyz);
						lineXYZ[x * 3 + 1] = static_cast<uint8_t>(gxyz);
						lineXYZ[x * 3 + 2] = static_cast<uint8_t>(bxyz);

						lineX[x * 3 + 0] = static_cast<uint8_t>(rx);
						lineX[x * 3 + 1] = static_cast<uint8_t>(gx);
						lineX[x * 3 + 2] = static_cast<uint8_t>(bx);

						lineY[x * 3 + 0] = static_cast<uint8_t>(ry);
						lineY[x * 3 + 1] = static_cast<uint8_t>(gy);
						lineY[x * 3 + 2] = static_cast<uint8_t>(by);

						lineZ[x * 3 + 0] = static_cast<uint8_t>(rz);
						lineZ[x * 3 + 1] = static_cast<uint8_t>(gz);
						lineZ[x * 3 + 2] = static_cast<uint8_t>(bz);
					}
					//cout<<endl;
					pngoutXYZ.write(lineXYZ.data(), static_cast<size_t>(SX));
					pngoutX.write(lineX.data(), static_cast<size_t>(SX));
					pngoutY.write(lineY.data(), static_cast<size_t>(SX));
					pngoutZ.write(lineZ.data(), static_cast<size_t>(SX));
				}

				make_svg_(TXY,timeIndex,"./imagenes/"+nameSVG_XYZ.str(),velxyz.back(), slip, colorMap4,nsxyz);
				make_svg_(TXY,timeIndex,"./imagenes/"+nameSVG_X.str(),vel[X].back(), slip, colorMap1,ns);
				make_svg_(TXY,timeIndex,"./imagenes/"+nameSVG_Y.str(),vel[Y].back(), slip, colorMap1,ns);
				make_svg_(TXY,timeIndex,"./imagenes/"+nameSVG_Z.str(),vel[Z].back(), slip, colorMap1,ns);


			}
			////////////////////////////////

			auto t1 = clk::now();
			auto diff = duration_cast<microseconds>(t1-t0);
			std::cout <<" tiempo de dibujado "<<fixed<< diff.count() << "us"<<endl;
		}//fin for tiempo


	}//fin for configdat


	if( calculateVmax ){
//		ofstream output_file("./velMax.dat");
//		ostream_iterator<float> output_iterator(output_file, "\n");
//		copy(valmax2.data.begin(), valmax2.data.end(), output_iterator);

		ofstream output_file_bin("./velMax_bin.dat", ios::out | ios::binary);
		output_file_bin.write( (char*)&valmax2.data.at(0) , valmax2.data.size() * sizeof(float));
		output_file_bin.close();
	}
	else{
		valmax2.data=readFile("./velMax_bin.dat");
	}

	if( calculateVxyzmax ){
		ofstream output_file_bin("./velxyzMax_bin.dat", ios::out | ios::binary);
		output_file_bin.write( (char*)&valxyzmax2.data.at(0) , valxyzmax2.data.size() * sizeof(float));
		output_file_bin.close();
	}
	else{
		valxyzmax2.data=readFile("./velxyzMax_bin.dat");
	}


	NiceScale nsMax(0,valmax2.Max() );


	vector< curve > isovel;
	vector< curve > isovelxyz;
	isovel=isoValue(valmax2);
	isovelxyz=isoValue(valxyzmax2);

	stringstream namePNG_MaxZ,nameSVG_MaxZ,nameRoot_MaxZ;
	nameRoot_MaxZ<<nFilePrefix<<"-MaxZ";
	namePNG_MaxZ<<nameRoot_MaxZ.str()<<".png";
	nameSVG_MaxZ<<nameRoot_MaxZ.str()<<".svg";
	cout<<namePNG_MaxZ.str()<<endl;
	std::ofstream outZ("./imagenes/"+namePNG_MaxZ.str(), std::ios::binary);
	valmax2.fileName="./"+namePNG_MaxZ.str();
	valmax2.labelColorMap="Vxmax (m/s)";
	TinyPngOut pngoutMaxZ(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outZ);
	std::vector<uint8_t> lineMaxZ(static_cast<size_t>(SX) * 3);
	cout<<"antes del for "<<namePNG_MaxZ.str()<<endl;
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

			if( id2ijk.at(valxyzID[X][index_xy]).i % 2 != id2ijk.at(valxyzID[X][index_xy]).j % 2 ){
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
			lineMaxZ[x * 3 + 0] = static_cast<uint8_t>(r);
			lineMaxZ[x * 3 + 1] = static_cast<uint8_t>(g);
			lineMaxZ[x * 3 + 2] = static_cast<uint8_t>(b);
		}
		pngoutMaxZ.write(lineMaxZ.data(), static_cast<size_t>(SX));
	}
	make_svg_max(TXY,"./imagenes/"+nameSVG_MaxZ.str(), valmax2, slip, colorMap2,nsMax, isovel );

	NiceScale nsxyzMax(0,valxyzmax2.Max() );
	stringstream namePNG_MaxXYZ,nameSVG_MaxXYZ,nameRoot_MaxXYZ;
	nameRoot_MaxXYZ<<nFilePrefix<<"-MaxXYZ";
	namePNG_MaxXYZ<<nameRoot_MaxXYZ.str()<<".png";
	nameSVG_MaxXYZ<<nameRoot_MaxXYZ.str()<<".svg";
	cout<<namePNG_MaxXYZ.str()<<endl;
	std::ofstream outXYZ("./imagenes/"+namePNG_MaxXYZ.str(), std::ios::binary);
	valxyzmax2.fileName="./"+namePNG_MaxXYZ.str();
	valxyzmax2.labelColorMap="Vmax (m/s)";
	TinyPngOut pngoutMaxXYZ(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outXYZ);
	std::vector<uint8_t> lineMaxXYZ(static_cast<size_t>(SX) * 3);
	for (int y = SY-1; y >= 0; y--) {
		for (int x = 0; x < SX; x++) {
			uint8_t r, g, b;
			int index_xy = x + SX * y;
			double f = (valxyzmax2.get(x,y)-nsxyzMax.niceMin)/(nsxyzMax.niceMax-nsxyzMax.niceMin);

			colorMap2.getColor(f, r, g, b);

			if( id2ijk.at(valxyzID[X][index_xy]).i % 2 != id2ijk.at(valxyzID[X][index_xy]).j % 2 ){
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
			lineMaxXYZ[x * 3 + 0] = static_cast<uint8_t>(r);
			lineMaxXYZ[x * 3 + 1] = static_cast<uint8_t>(g);
			lineMaxXYZ[x * 3 + 2] = static_cast<uint8_t>(b);
		}
		pngoutMaxXYZ.write(lineMaxXYZ.data(), static_cast<size_t>(SX));
	}
	make_svg_max(TXY,"./imagenes/"+nameSVG_MaxXYZ.str(), valxyzmax2, slip, colorMap2,nsxyzMax, isovelxyz);

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

		stringstream namePNG_MMI,nameSVG_MMI,nameRoot_MMI;
		nameRoot_MMI<<nFilePrefix<<"-MMI";
		namePNG_MMI<<nameRoot_MMI.str()<<".png";
		nameSVG_MMI<<nameRoot_MMI.str()<<".svg";
		cout<<namePNG_MMI.str()<<endl;
		std::ofstream outMMI("./imagenes/"+namePNG_MMI.str(), std::ios::binary);
		valmmi.fileName="./"+namePNG_MMI.str();
		valmmi.labelColorMap="";
		TinyPngOut pngoutMMI(static_cast<uint32_t>(SX), static_cast<uint32_t>(SY), outMMI);
		std::vector<uint8_t> lineMMI(static_cast<size_t>(SX) * 3);

		for (int y = SY-1; y >= 0; y--) {
			for (int x = 0; x < SX; x++) {
				uint8_t r, g, b;
				//double dx=x*infoData.DH()-epicx;
				//double dy=y*infoData.DH()-epicy;
				Coord p=valmmi.imgT.ij2latlng(x,y);
				double dx=(p.lon-slip.LON)*latlon2m;
				double dy=(p.lat-slip.LAT)*latlon2m;
				double D=sqrt(dx*dx+dy*dy);
				if(D>Dp){
					double mmi = EqMMI(Eq, Group, D, Dp, Ms);
					valmmi.set(x,y,mmi);
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

		make_svg_mmi(TXY,"./imagenes/"+nameSVG_MMI.str(),valmmi,vRadio, slip);
		//void make_svg_mmi(Transform2 &TXY, string nameSVG, data2D<float> &dataVmax, vector<double> &vRadio, dataSlip slip){

		stringstream namePNG_MMI_MaxVel,nameSVG_MMI_MaxVel,nameRoot_MMI_MaxVel;
		nameRoot_MMI_MaxVel<<nFilePrefix<<"-MMI_MaxVel";
		namePNG_MMI_MaxVel<<nameRoot_MMI_MaxVel.str()<<".png";
		nameSVG_MMI_MaxVel<<nameRoot_MMI_MaxVel.str()<<".svg";
		cout<<namePNG_MMI_MaxVel.str()<<endl;
		make_svg_mmi_maxvel(TXY,"./imagenes/"+nameSVG_MMI_MaxVel.str(),valmax2,vRadio, slip, colorMap2,nsMax,isovel );
	}




	auto t1T = clk::now();
	auto diff = duration_cast<seconds>(t1T-t0T);
	std::cout <<" tiempo de lectura "<<fixed<< diff.count() << "s"<<endl;

	return 0;
}
