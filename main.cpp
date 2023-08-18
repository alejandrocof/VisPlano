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

#include "TinyPngOut.hpp"
#include "configdata.hpp"

#include <cmath>

#include "SVG2D.hpp"
//#include "Shape.hpp"
#include "Transform.hpp"
#include "ImgTransform.hpp"
#include "sigfigs.hpp"
#include "Coord.hpp"

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

lut colorMap2(
		vector<Color>{
			Color(0,0,255),
			Color(0,255,255),
			Color(0,255,0),
			Color(255,255,0),
			Color(255,0,0),
		}
		);

lut colorMap3(
		vector<Color>{
			Color(255,255,255),
			Color(0,255,255),
			Color(0,255,0),
			Color(255,213,0),
			Color(255,0,0),
		}
		);

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

vector< vector< pair<Coord,float> > > coordInvSlip={
	vector< pair<Coord,float> >{
	pair<Coord, float>( Coord( -93.8361, 14.7849 ), 1.2085 ),
	pair<Coord, float>( Coord( -93.9031, 14.8474 ), 0.3241 ),
	pair<Coord, float>( Coord( -93.9701, 14.9099 ), 2.1644 ),
	pair<Coord, float>( Coord( -94.0371, 14.9724 ), 3.7018 ),
	pair<Coord, float>( Coord( -94.1041, 15.035 ), 5.2758 ),
	pair<Coord, float>( Coord( -94.1711, 15.0975 ), 8.4523 ),
	pair<Coord, float>( Coord( -94.2381, 15.16 ), 11.0243 ),
	pair<Coord, float>( Coord( -94.3051, 15.2225 ), 12.4847 ),
	pair<Coord, float>( Coord( -94.3721, 15.285 ), 12.0623 ),
	pair<Coord, float>( Coord( -94.4391, 15.3475 ), 5.9833 ),
	pair<Coord, float>( Coord( -94.5061, 15.41 ), 2.7855 ),
	pair<Coord, float>( Coord( -94.5731, 15.4726 ), 2.9093 ),
	pair<Coord, float>( Coord( -94.6401, 15.5351 ), 3.86 ),
	pair<Coord, float>( Coord( -94.7071, 15.5976 ), 4.278 ),
	pair<Coord, float>( Coord( -94.7741, 15.6601 ), 4.3245 ),
	pair<Coord, float>( Coord( -94.8411, 15.7226 ), 3.199 ),
	pair<Coord, float>( Coord( -94.9081, 15.7851 ), 1.9694 ),
	pair<Coord, float>( Coord( -94.9751, 15.8476 ), 0.8588 ),
	pair<Coord, float>( Coord( -95.0421, 15.9102 ), 1.2472 ),
	pair<Coord, float>( Coord( -95.1091, 15.9727 ), 2.4485 ),
	pair<Coord, float>( Coord( -95.1761, 16.0352 ), 2.2737 ),
	pair<Coord, float>( Coord( -95.2431, 16.0977 ), 1.8826 ),
	pair<Coord, float>( Coord( -95.3101, 16.1602 ), 0.2158 ),
	pair<Coord, float>( Coord( -95.3771, 16.2227 ), 0.0081 )
},
	vector< pair<Coord,float> >{
	pair<Coord, float>( Coord( -93.826, 14.7949 ), 1.3704 ),
	pair<Coord, float>( Coord( -93.893, 14.8574 ), 0.1086 ),
	pair<Coord, float>( Coord( -93.96, 14.9199 ), 0.3831 ),
	pair<Coord, float>( Coord( -94.027, 14.9825 ), 2.3605 ),
	pair<Coord, float>( Coord( -94.094, 15.045 ), 4.2536 ),
	pair<Coord, float>( Coord( -94.161, 15.1075 ), 8.9265 ),
	pair<Coord, float>( Coord( -94.228, 15.17 ), 13.8957 ),
	pair<Coord, float>( Coord( -94.295, 15.2325 ), 17.2856 ),
	pair<Coord, float>( Coord( -94.362, 15.295 ), 14.6803 ),
	pair<Coord, float>( Coord( -94.429, 15.3575 ), 6.9058 ),
	pair<Coord, float>( Coord( -94.496, 15.4201 ), 1.7789 ),
	pair<Coord, float>( Coord( -94.563, 15.4826 ), 1.544 ),
	pair<Coord, float>( Coord( -94.63, 15.5451 ), 3.8333 ),
	pair<Coord, float>( Coord( -94.697, 15.6076 ), 4.6964 ),
	pair<Coord, float>( Coord( -94.764, 15.6701 ), 4.0011 ),
	pair<Coord, float>( Coord( -94.831, 15.7326 ), 3.7765 ),
	pair<Coord, float>( Coord( -94.898, 15.7951 ), 2.5071 ),
	pair<Coord, float>( Coord( -94.965, 15.8577 ), 0.6191 ),
	pair<Coord, float>( Coord( -95.032, 15.9202 ), 0.7485 ),
	pair<Coord, float>( Coord( -95.099, 15.9827 ), 2.4753 ),
	pair<Coord, float>( Coord( -95.166, 16.0452 ), 3.132 ),
	pair<Coord, float>( Coord( -95.233, 16.1077 ), 2.596 ),
	pair<Coord, float>( Coord( -95.3, 16.1702 ), 0.6318 ),
	pair<Coord, float>( Coord( -95.3671, 16.2327 ), 0.1851 )
},
	vector< pair<Coord,float> >{
	pair<Coord, float>( Coord( -93.816, 14.8049 ), 0.3362 ),
	pair<Coord, float>( Coord( -93.883, 14.8675 ), 0.1048 ),
	pair<Coord, float>( Coord( -93.95, 14.93 ), 0.0185 ),
	pair<Coord, float>( Coord( -94.017, 14.9925 ), 0.0186 ),
	pair<Coord, float>( Coord( -94.084, 15.055 ), 1.7433 ),
	pair<Coord, float>( Coord( -94.151, 15.1175 ), 5.6352 ),
	pair<Coord, float>( Coord( -94.218, 15.18 ), 10.9505 ),
	pair<Coord, float>( Coord( -94.285, 15.2425 ), 15.618 ),
	pair<Coord, float>( Coord( -94.352, 15.3051 ), 13.8754 ),
	pair<Coord, float>( Coord( -94.419, 15.3676 ), 7.2563 ),
	pair<Coord, float>( Coord( -94.486, 15.4301 ), 1.9013 ),
	pair<Coord, float>( Coord( -94.553, 15.4926 ), 0.0568 ),
	pair<Coord, float>( Coord( -94.62, 15.5551 ), 2.4806 ),
	pair<Coord, float>( Coord( -94.687, 15.6176 ), 4.1033 ),
	pair<Coord, float>( Coord( -94.754, 15.6801 ), 3.6514 ),
	pair<Coord, float>( Coord( -94.821, 15.7427 ), 3.2483 ),
	pair<Coord, float>( Coord( -94.888, 15.8052 ), 2.8411 ),
	pair<Coord, float>( Coord( -94.955, 15.8677 ), 1.1402 ),
	pair<Coord, float>( Coord( -95.022, 15.9302 ), 1.6413 ),
	pair<Coord, float>( Coord( -95.089, 15.9927 ), 3.3868 ),
	pair<Coord, float>( Coord( -95.156, 16.0552 ), 4.3071 ),
	pair<Coord, float>( Coord( -95.223, 16.1177 ), 3.3376 ),
	pair<Coord, float>( Coord( -95.29, 16.1803 ), 0.6541 ),
	pair<Coord, float>( Coord( -95.357, 16.2428 ), 0.5982 )
},
	vector< pair<Coord,float> >{
	pair<Coord, float>( Coord( -93.806, 14.815 ), 0.2497 ),
	pair<Coord, float>( Coord( -93.873, 14.8775 ), 0.0969 ),
	pair<Coord, float>( Coord( -93.94, 14.94 ), 0.0053 ),
	pair<Coord, float>( Coord( -94.007, 15.0025 ), 0.0124 ),
	pair<Coord, float>( Coord( -94.074, 15.065 ), 0.2348 ),
	pair<Coord, float>( Coord( -94.141, 15.1275 ), 4.5377 ),
	pair<Coord, float>( Coord( -94.208, 15.1901 ), 8.9421 ),
	pair<Coord, float>( Coord( -94.275, 15.2526 ), 12.2809 ),
	pair<Coord, float>( Coord( -94.342, 15.3151 ), 11.2973 ),
	pair<Coord, float>( Coord( -94.409, 15.3776 ), 7.4108 ),
	pair<Coord, float>( Coord( -94.476, 15.4401 ), 2.6302 ),
	pair<Coord, float>( Coord( -94.543, 15.5026 ), 0.0471 ),
	pair<Coord, float>( Coord( -94.61, 15.5651 ), 0.5304 ),
	pair<Coord, float>( Coord( -94.677, 15.6277 ), 2.5736 ),
	pair<Coord, float>( Coord( -94.744, 15.6902 ), 3.1575 ),
	pair<Coord, float>( Coord( -94.811, 15.7527 ), 2.5954 ),
	pair<Coord, float>( Coord( -94.878, 15.8152 ), 2.4186 ),
	pair<Coord, float>( Coord( -94.945, 15.8777 ), 2.2999 ),
	pair<Coord, float>( Coord( -95.012, 15.9402 ), 3.3153 ),
	pair<Coord, float>( Coord( -95.079, 16.0027 ), 4.7565 ),
	pair<Coord, float>( Coord( -95.146, 16.0653 ), 3.9718 ),
	pair<Coord, float>( Coord( -95.213, 16.1278 ), 2.9274 ),
	pair<Coord, float>( Coord( -95.28, 16.1903 ), 0.6426 ),
	pair<Coord, float>( Coord( -95.347, 16.2528 ), 0.0523 )
},
	vector< pair<Coord,float> >{
	pair<Coord, float>( Coord( -93.796, 14.825 ), 0.0328 ),
	pair<Coord, float>( Coord( -93.863, 14.8875 ), 0.9085 ),
	pair<Coord, float>( Coord( -93.93, 14.95 ), 2.0728 ),
	pair<Coord, float>( Coord( -93.997, 15.0125 ), 1.4529 ),
	pair<Coord, float>( Coord( -94.064, 15.0751 ), 0.1774 ),
	pair<Coord, float>( Coord( -94.131, 15.1376 ), 2.3134 ),
	pair<Coord, float>( Coord( -94.198, 15.2001 ), 5.9243 ),
	pair<Coord, float>( Coord( -94.265, 15.2626 ), 9.3017 ),
	pair<Coord, float>( Coord( -94.332, 15.3251 ), 7.8389 ),
	pair<Coord, float>( Coord( -94.399, 15.3876 ), 5.6346 ),
	pair<Coord, float>( Coord( -94.466, 15.4501 ), 2.7051 ),
	pair<Coord, float>( Coord( -94.533, 15.5127 ), 0.0328 ),
	pair<Coord, float>( Coord( -94.6, 15.5752 ), 0.0304 ),
	pair<Coord, float>( Coord( -94.667, 15.6377 ), 1.5418 ),
	pair<Coord, float>( Coord( -94.734, 15.7002 ), 2.2125 ),
	pair<Coord, float>( Coord( -94.801, 15.7627 ), 1.8635 ),
	pair<Coord, float>( Coord( -94.868, 15.8252 ), 1.1782 ),
	pair<Coord, float>( Coord( -94.935, 15.8877 ), 2.2034 ),
	pair<Coord, float>( Coord( -95.002, 15.9503 ), 3.2518 ),
	pair<Coord, float>( Coord( -95.069, 16.0128 ), 3.718 ),
	pair<Coord, float>( Coord( -95.136, 16.0753 ), 3.068 ),
	pair<Coord, float>( Coord( -95.203, 16.1378 ), 1.5472 ),
	pair<Coord, float>( Coord( -95.27, 16.2003 ), 0.0491 ),
	pair<Coord, float>( Coord( -95.337, 16.2628 ), 0.0347 )
},
	vector< pair<Coord,float> >{
	pair<Coord, float>( Coord( -93.786, 14.835 ), 0.0667 ),
	pair<Coord, float>( Coord( -93.853, 14.8975 ), 1.7382 ),
	pair<Coord, float>( Coord( -93.92, 14.9601 ), 3.7073 ),
	pair<Coord, float>( Coord( -93.987, 15.0226 ), 3.1796 ),
	pair<Coord, float>( Coord( -94.054, 15.0851 ), 0.4228 ),
	pair<Coord, float>( Coord( -94.121, 15.1476 ), 0.6852 ),
	pair<Coord, float>( Coord( -94.188, 15.2101 ), 2.3665 ),
	pair<Coord, float>( Coord( -94.255, 15.2726 ), 7.0848 ),
	pair<Coord, float>( Coord( -94.322, 15.3351 ), 8.404 ),
	pair<Coord, float>( Coord( -94.389, 15.3977 ), 8.2425 ),
	pair<Coord, float>( Coord( -94.456, 15.4602 ), 4.9315 ),
	pair<Coord, float>( Coord( -94.523, 15.5227 ), 2.8058 ),
	pair<Coord, float>( Coord( -94.59, 15.5852 ), 1.7892 ),
	pair<Coord, float>( Coord( -94.657, 15.6477 ), 2.1855 ),
	pair<Coord, float>( Coord( -94.724, 15.7102 ), 1.2722 ),
	pair<Coord, float>( Coord( -94.791, 15.7727 ), 1.1808 ),
	pair<Coord, float>( Coord( -94.858, 15.8352 ), 0.6305 ),
	pair<Coord, float>( Coord( -94.925, 15.8978 ), 1.0094 ),
	pair<Coord, float>( Coord( -94.992, 15.9603 ), 0.772 ),
	pair<Coord, float>( Coord( -95.059, 16.0228 ), 1.4909 ),
	pair<Coord, float>( Coord( -95.126, 16.0853 ), 1.245 ),
	pair<Coord, float>( Coord( -95.193, 16.1478 ), 0.0057 ),
	pair<Coord, float>( Coord( -95.26, 16.2103 ), 0.2409 ),
	pair<Coord, float>( Coord( -95.327, 16.2729 ), 0.0366 )
},
	vector< pair<Coord,float> >{
	pair<Coord, float>( Coord( -93.7759, 14.845 ), 0.9981 ),
	pair<Coord, float>( Coord( -93.8429, 14.9076 ), 1.3444 ),
	pair<Coord, float>( Coord( -93.9099, 14.9701 ), 1.1165 ),
	pair<Coord, float>( Coord( -93.9769, 15.0326 ), 1.1299 ),
	pair<Coord, float>( Coord( -94.0439, 15.0951 ), 0.0202 ),
	pair<Coord, float>( Coord( -94.1109, 15.1576 ), 0.0428 ),
	pair<Coord, float>( Coord( -94.1779, 15.2201 ), 1.4192 ),
	pair<Coord, float>( Coord( -94.2449, 15.2826 ), 6.3503 ),
	pair<Coord, float>( Coord( -94.3119, 15.3452 ), 8.9372 ),
	pair<Coord, float>( Coord( -94.3789, 15.4077 ), 9.1017 ),
	pair<Coord, float>( Coord( -94.4459, 15.4702 ), 6.9985 ),
	pair<Coord, float>( Coord( -94.5129, 15.5327 ), 4.4127 ),
	pair<Coord, float>( Coord( -94.5799, 15.5952 ), 2.6181 ),
	pair<Coord, float>( Coord( -94.6469, 15.6577 ), 3.3315 ),
	pair<Coord, float>( Coord( -94.7139, 15.7202 ), 1.8858 ),
	pair<Coord, float>( Coord( -94.7809, 15.7828 ), 1.0494 ),
	pair<Coord, float>( Coord( -94.8479, 15.8453 ), 0.6243 ),
	pair<Coord, float>( Coord( -94.9149, 15.9078 ), 0.6064 ),
	pair<Coord, float>( Coord( -94.9819, 15.9703 ), 0.6085 ),
	pair<Coord, float>( Coord( -95.0489, 16.0328 ), 0.0484 ),
	pair<Coord, float>( Coord( -95.1159, 16.0953 ), 0.7107 ),
	pair<Coord, float>( Coord( -95.1829, 16.1578 ), 0.0417 ),
	pair<Coord, float>( Coord( -95.2499, 16.2204 ), 0.1087 ),
	pair<Coord, float>( Coord( -95.3169, 16.2829 ), 0.0847 )
},
	vector< pair<Coord,float> >{
	pair<Coord, float>( Coord( -93.7659, 14.8551 ), 0.0326 ),
	pair<Coord, float>( Coord( -93.8329, 14.9176 ), 0.0535 ),
	pair<Coord, float>( Coord( -93.8999, 14.9801 ), 0.0859 ),
	pair<Coord, float>( Coord( -93.9669, 15.0426 ), 0.0139 ),
	pair<Coord, float>( Coord( -94.0339, 15.1051 ), 0.0717 ),
	pair<Coord, float>( Coord( -94.1009, 15.1676 ), 0.0461 ),
	pair<Coord, float>( Coord( -94.1679, 15.2302 ), 1.8136 ),
	pair<Coord, float>( Coord( -94.2349, 15.2927 ), 4.328 ),
	pair<Coord, float>( Coord( -94.3019, 15.3552 ), 6.8623 ),
	pair<Coord, float>( Coord( -94.3689, 15.4177 ), 7.5617 ),
	pair<Coord, float>( Coord( -94.4359, 15.4802 ), 6.0261 ),
	pair<Coord, float>( Coord( -94.5029, 15.5427 ), 2.8364 ),
	pair<Coord, float>( Coord( -94.5699, 15.6052 ), 2.0865 ),
	pair<Coord, float>( Coord( -94.6369, 15.6678 ), 2.2582 ),
	pair<Coord, float>( Coord( -94.7039, 15.7303 ), 1.0811 ),
	pair<Coord, float>( Coord( -94.7709, 15.7928 ), 1.1201 ),
	pair<Coord, float>( Coord( -94.8379, 15.8553 ), 1.2561 ),
	pair<Coord, float>( Coord( -94.9049, 15.9178 ), 0.962 ),
	pair<Coord, float>( Coord( -94.9719, 15.9803 ), 0.8502 ),
	pair<Coord, float>( Coord( -95.0389, 16.0428 ), 0.0002 ),
	pair<Coord, float>( Coord( -95.1059, 16.1054 ), 0.3045 ),
	pair<Coord, float>( Coord( -95.1729, 16.1679 ), 0.0255 ),
	pair<Coord, float>( Coord( -95.2399, 16.2304 ), 0.0036 ),
	pair<Coord, float>( Coord( -95.3069, 16.2929 ), 0.347 )
	}
};


inputData infoData;
ImgT img;
//vector< vector<Coord> > coords;
vector< vector< vector<Coord> > > isovel; //isovel coords coord
vector< Color > color;

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
		getdata(file,theta);
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
	svgmap.add( Shape().ShapeFile(nameFileCSV,colLabelFileCSV,0.7) );
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


	//SVG2D svg(nameSVG_Z.str(),image_width,image_height);
	SVG2D svg(nameSVG,image_width,image_height);
	svg.add(Shape().ColorMap(colorMap1,"ColorMap"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );


	//NBGX primer nodo x que contiene receptores de superficie
	//NEDX último nodo x que contiene receptores de superficie
	//NXSC x posición nodal para fuente puntual

	double x=infoData.NBGX()-1;
	double y=infoData.NBGY()-1;
	double dx=infoData.NEDX()-infoData.NBGX()+1;
	double dy=infoData.NEDY()-infoData.NBGY()+1;

	double Tx=T().x(epi_lon+(x-infoData.NXSC()-xhip)*dh/latlon2m);
	double Ty=T().y(epi_lat+(y-infoData.NYSC()-yhip+dy)*dh/latlon2m);

	double Tdx=T().sx(dx*dh/latlon2m);
	double Tdy=T().sy(dy*dh/latlon2m);

	double epi_x=T().x(epi_lon);
	double epi_y=T().y(epi_lat);


	//infoData.NXSC()-infoData.NBGX()

	/*
	double dx=infoData.NEDX()-infoData.NBGX()+1;
	double dy=infoData.NEDY()-infoData.NBGY()+1;

	double Tx=T().x(epi_lon-dx*dh/latlon2m);
	double Ty=T().y(epi_lat+dy*dh/latlon2m);
	//double Tdx=T().sx(1);
	//double Tdy=T().sy(1);
	double Tdx=T().sx(dx*dh/latlon2m);
	double Tdy=T().sy(dy*dh/latlon2m);
	*/


	//stringstream nameFile;
	//nameFile<<"prueba"<<setw(ndigID)<<setfill('0')<<id<<setw(ndigTI)<<setfill('0')<<NTST<<"_"<<setw(5)<<setfill('0')<<LAYERGLOBAL<<".ppm";
	//svg.add(Shape().Image(Tx+0.5*Tdx,Ty+0.5*Tdy,Tdx,Tdy,"./Z/Vx3D-Z00-TI0379.png",0,0,0));
	double rotx=epi_x-Tx;
	//double rotx=0.0;
	double roty=epi_y-Ty;

	//svg.add(Shape().Image(Tx,Ty,Tdx,Tdy,namePNG,0,0,0));
	svg.add(Shape().Image(Tx+rotx,Ty+roty,Tdx,Tdy,namePNG,rotx,roty,theta));
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

	for(int k=0; k<isovel.size(); k++ ){
		for(const vector<Coord>& vs: isovel[k] ){
			for(int i=0; i<vs.size()-1; i+=2){
				svg.add( Shape().Line( T().x(vs[i].lon), T().y(vs[i].lat), T().x(vs[i+1].lon), T().y(vs[i+1].lat) ).stroke( 0.7*color[k].r, 0.7*color[k].g, 0.7*color[k].b).strokeWidth(1.0).strokeLinecap("round") );
			}
		}
	}

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
		double y = epi_lat+T().inv_sy( T().pointHeight );
		rot(alpha,epi_lon, epi_lat, x, y);
		polygon.push_back( T().x(x) );
		polygon.push_back( T().y(y) );

		//		cout<<"alpha="<<alpha<<" ("<<x<<", "<<y<<") "<< T().pointHeight << endl;
	}



	svg.add( Shape().Polygon(polygon).fill(255,0,0).stroke(0,0,0).strokeWidth(0.1*T().textHeight) );
	//	svg.add( Shape().Text(T().x(epi_lon),T().y(epi_lat)-2*T().pointHeight,"Epic")
	//			 .align("middle").fontSize(T().textHeight).fontFamily("Times").opacity(0.8) );

	for(int i=0; i<infoData.HOLE_N(); i++){
		//double x=T().x(epi_lon+(x-infoData.HOLE_X(i))*dh/latlon2m);
		//double y=T().y(epi_lat+(y-infoData.HOLE_Y(i)+dy)*dh/latlon2m);
		double x=Tx+T().sx(infoData.HOLE_X(i)*dh/latlon2m);
		double y=Ty+Tdy-T().sy(infoData.HOLE_Y(i)*dh/latlon2m);
		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
				 .fill(0,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
		stringstream coordEst;
		coordEst<<"("<<infoData.HOLE_X(i)<<", "<<infoData.HOLE_Y(i)<<")";
		svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() )
				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
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

Coord interpola(double alpha, double beta, Coord a, Coord b, Coord c, Coord d){
	Coord ab=a+alpha*(b-a);
	Coord dc=d+alpha*(c-d);
	return ab+beta*(dc-ab);
}


void make_svg_max(string nameSVG, string namePNG, double vmin, double vmax){

	int image_width=T().width;
	int image_height=T().height;

	SVG2D svg(nameSVG,image_width,image_height);
	svg.add(Shape().ColorMap(colorMap2,"ColorMap2"));
	svg.add(Shape().Rectangle(0, 0,image_width,image_height).fill(192, 192, 210) );

	double x=infoData.NBGX()-1;
	double y=infoData.NBGY()-1;
	double dx=infoData.NEDX()-infoData.NBGX()+1;
	double dy=infoData.NEDY()-infoData.NBGY()+1;

	double Tx=T().x(epi_lon+(x-infoData.NXSC()-xhip)*dh/latlon2m);
	double Ty=T().y(epi_lat+(y-infoData.NYSC()-yhip+dy)*dh/latlon2m);

	double Tdx=T().sx(dx*dh/latlon2m);
	double Tdy=T().sy(dy*dh/latlon2m);

	double epi_x=T().x(epi_lon);
	double epi_y=T().y(epi_lat);


	double rotx=epi_x-Tx;
	double roty=epi_y-Ty;

	svg.add(Shape().Image(Tx+rotx,Ty+roty,Tdx,Tdy,namePNG,rotx,roty,theta));


	{
		int j=0;
		for(int i=1; i<coordInvSlip[j].size()-1; i++ ){
			vector<float> p;
			Coord p1=interpola(0.5,-0.5,coordInvSlip[j][i-1].first,coordInvSlip[j][i].first,coordInvSlip[j+1][i].first,coordInvSlip[j+1][i-1].first);
			Coord p2=interpola(0.5,-0.5,coordInvSlip[j][i].first,coordInvSlip[j][i+1].first,coordInvSlip[j+1][i+1].first,coordInvSlip[j+1][i].first);
			Coord p3=interpola(0.5,0.5,coordInvSlip[j][i].first,coordInvSlip[j][i+1].first,coordInvSlip[j+1][i+1].first,coordInvSlip[j+1][i].first);
			Coord p4=interpola(0.5,0.5,coordInvSlip[j][i-1].first,coordInvSlip[j][i].first,coordInvSlip[j+1][i].first,coordInvSlip[j+1][i-1].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);

			svg.add( Shape().Polygon(p)
					 .fill( (int)r, (int)g, (int)b ).stroke(0,0,0).strokeWidth(0.005*T().textHeight) );
		}
		j=coordInvSlip.size()-1;
		for(int i=1; i<coordInvSlip[j].size()-1; i++ ){
			vector<float> p;
			Coord p1=interpola(0.5,0.5,coordInvSlip[j-1][i-1].first,coordInvSlip[j-1][i].first,coordInvSlip[j][i].first,coordInvSlip[j][i-1].first);
			Coord p2=interpola(0.5,0.5,coordInvSlip[j-1][i].first,coordInvSlip[j-1][i+1].first,coordInvSlip[j][i+1].first,coordInvSlip[j][i].first);
			Coord p3=interpola(0.5,1.5,coordInvSlip[j-1][i].first,coordInvSlip[j-1][i+1].first,coordInvSlip[j][i+1].first,coordInvSlip[j][i].first);
			Coord p4=interpola(0.5,1.5,coordInvSlip[j-1][i-1].first,coordInvSlip[j-1][i].first,coordInvSlip[j][i].first,coordInvSlip[j][i-1].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);

			svg.add( Shape().Polygon(p)
					 .fill( (int)r, (int)g, (int)b ).stroke(0,0,0).strokeWidth(0.005*T().textHeight) );
		}
	}

	{
		int i=0;
		for(int j=1; j<coordInvSlip.size()-1; j++ ){
			vector<float> p;
			Coord p1=interpola(-0.5,0.5,coordInvSlip[j-1][i].first,coordInvSlip[j-1][i+1].first,coordInvSlip[j][i+1].first,coordInvSlip[j][i].first);
			Coord p2=interpola(0.5,0.5,coordInvSlip[j-1][i].first,coordInvSlip[j-1][i+1].first,coordInvSlip[j][i+1].first,coordInvSlip[j][i].first);
			Coord p3=interpola(0.5,0.5,coordInvSlip[j][i].first,coordInvSlip[j][i+1].first,coordInvSlip[j+1][i+1].first,coordInvSlip[j+1][i].first);
			Coord p4=interpola(-0.5,0.5,coordInvSlip[j][i].first,coordInvSlip[j][i+1].first,coordInvSlip[j+1][i+1].first,coordInvSlip[j+1][i].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);

			svg.add( Shape().Polygon(p)
					 .fill( (int)r, (int)g, (int)b ).stroke(0,0,0).strokeWidth(0.005*T().textHeight) );
		}
		i=coordInvSlip[0].size()-1;
		for(int j=1; j<coordInvSlip.size()-1; j++ ){
			vector<float> p;
			Coord p1=interpola(0.5,0.5,coordInvSlip[j-1][i-1].first,coordInvSlip[j-1][i].first,coordInvSlip[j][i].first,coordInvSlip[j][i-1].first);
			Coord p2=interpola(1.5,0.5,coordInvSlip[j-1][i-1].first,coordInvSlip[j-1][i].first,coordInvSlip[j][i].first,coordInvSlip[j][i-1].first);
			Coord p3=interpola(1.5,0.5,coordInvSlip[j][i-1].first,coordInvSlip[j][i].first,coordInvSlip[j+1][i].first,coordInvSlip[j+1][i-1].first);
			Coord p4=interpola(0.5,0.5,coordInvSlip[j][i-1].first,coordInvSlip[j][i].first,coordInvSlip[j+1][i].first,coordInvSlip[j+1][i-1].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);

			svg.add( Shape().Polygon(p)
					 .fill( (int)r, (int)g, (int)b ).stroke(0,0,0).strokeWidth(0.005*T().textHeight) );
		}
	}

	{

		{
			int i=0;
			int j=0;//
			vector<float> p;
			Coord p1=interpola(-0.5,-0.5,coordInvSlip[j][i].first,coordInvSlip[j][i+1].first,coordInvSlip[j+1][i+1].first,coordInvSlip[j+1][i].first);
			Coord p2=interpola(0.5,-0.5,coordInvSlip[j][i].first,coordInvSlip[j][i+1].first,coordInvSlip[j+1][i+1].first,coordInvSlip[j+1][i].first);
			Coord p3=interpola(0.5,0.5,coordInvSlip[j][i].first,coordInvSlip[j][i+1].first,coordInvSlip[j+1][i+1].first,coordInvSlip[j+1][i].first);
			Coord p4=interpola(-0.5,0.5,coordInvSlip[j][i].first,coordInvSlip[j][i+1].first,coordInvSlip[j+1][i+1].first,coordInvSlip[j+1][i].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);

			svg.add( Shape().Polygon(p)
					 .fill( (int)r, (int)g, (int)b ).stroke(0,0,0).strokeWidth(0.005*T().textHeight) );
		}

		{
			int i=coordInvSlip[0].size()-1;
			int j=0;//
			vector<float> p;
			Coord p1=interpola(0.5,-0.5,coordInvSlip[j][i-1].first,coordInvSlip[j][i].first,coordInvSlip[j+1][i].first,coordInvSlip[j+1][i-1].first);
			Coord p2=interpola(1.5,-0.5,coordInvSlip[j][i-1].first,coordInvSlip[j][i].first,coordInvSlip[j+1][i].first,coordInvSlip[j+1][i-1].first);
			Coord p3=interpola(1.5,0.5,coordInvSlip[j][i-1].first,coordInvSlip[j][i].first,coordInvSlip[j+1][i].first,coordInvSlip[j+1][i-1].first);
			Coord p4=interpola(0.5,0.5,coordInvSlip[j][i-1].first,coordInvSlip[j][i].first,coordInvSlip[j+1][i].first,coordInvSlip[j+1][i-1].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);

			svg.add( Shape().Polygon(p)
					 .fill( (int)r, (int)g, (int)b ).stroke(0,0,0).strokeWidth(0.005*T().textHeight) );
		}


		{
			int i=0;
			int j=coordInvSlip.size()-1;
			vector<float> p;
			Coord p1=interpola(-0.5,0.5,coordInvSlip[j-1][i].first,coordInvSlip[j-1][i+1].first,coordInvSlip[j][i+1].first,coordInvSlip[j][i].first);
			Coord p2=interpola(0.5,0.5,coordInvSlip[j-1][i].first,coordInvSlip[j-1][i+1].first,coordInvSlip[j][i+1].first,coordInvSlip[j][i].first);
			Coord p3=interpola(0.5,1.5,coordInvSlip[j-1][i].first,coordInvSlip[j-1][i+1].first,coordInvSlip[j][i+1].first,coordInvSlip[j][i].first);
			Coord p4=interpola(-0.5,1.5,coordInvSlip[j-1][i].first,coordInvSlip[j-1][i+1].first,coordInvSlip[j][i+1].first,coordInvSlip[j][i].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);

			svg.add( Shape().Polygon(p)
					 .fill( (int)r, (int)g, (int)b ).stroke(0,0,0).strokeWidth(0.005*T().textHeight) );
		}

		{
			int i=coordInvSlip[0].size()-1;
			int j=coordInvSlip.size()-1;
			vector<float> p;
			Coord p1=interpola(0.5,0.5,coordInvSlip[j-1][i-1].first,coordInvSlip[j-1][i].first,coordInvSlip[j][i].first,coordInvSlip[j][i-1].first);
			Coord p2=interpola(1.5,0.5,coordInvSlip[j-1][i-1].first,coordInvSlip[j-1][i].first,coordInvSlip[j][i].first,coordInvSlip[j][i-1].first);
			Coord p3=interpola(1.5,1.5,coordInvSlip[j-1][i-1].first,coordInvSlip[j-1][i].first,coordInvSlip[j][i].first,coordInvSlip[j][i-1].first);
			Coord p4=interpola(0.5,1.5,coordInvSlip[j-1][i-1].first,coordInvSlip[j-1][i].first,coordInvSlip[j][i].first,coordInvSlip[j][i-1].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);

			svg.add( Shape().Polygon(p)
					 .fill( (int)r, (int)g, (int)b ).stroke(0,0,0).strokeWidth(0.005*T().textHeight) );
		}
	}

	for(int j=1; j<coordInvSlip.size()-1; j++ ){
		for(int i=1; i<coordInvSlip[j].size()-1; i++ ){
			vector<float> p;
			Coord p1=interpola(0.5,0.5,coordInvSlip[j-1][i-1].first,coordInvSlip[j-1][i].first,coordInvSlip[j][i].first,coordInvSlip[j][i-1].first);
			Coord p2=interpola(0.5,0.5,coordInvSlip[j-1][i].first,coordInvSlip[j-1][i+1].first,coordInvSlip[j][i+1].first,coordInvSlip[j][i].first);
			Coord p3=interpola(0.5,0.5,coordInvSlip[j][i].first,coordInvSlip[j][i+1].first,coordInvSlip[j+1][i+1].first,coordInvSlip[j+1][i].first);
			Coord p4=interpola(0.5,0.5,coordInvSlip[j][i-1].first,coordInvSlip[j][i].first,coordInvSlip[j+1][i].first,coordInvSlip[j+1][i-1].first);
			p.push_back( T().x(p1.lon) );p.push_back( T().y(p1.lat) );
			p.push_back( T().x(p2.lon) );p.push_back( T().y(p2.lat) );
			p.push_back( T().x(p3.lon) );p.push_back( T().y(p3.lat) );
			p.push_back( T().x(p4.lon) );p.push_back( T().y(p4.lat) );
			uint8_t r, g, b;
			double f = (coordInvSlip[j][i].second -nsSlip->niceMin)/(nsSlip->niceMax-nsSlip->niceMin);
			colorMap3.getColor(f, r, g, b);

			svg.add( Shape().Polygon(p)
					 .fill( (int)r, (int)g, (int)b ).stroke(0,0,0).strokeWidth(0.005*T().textHeight) );
		}
	}


	svg.add( Shape().Use("mapa.svg","mapa") );
	svg.add( Shape().Mask(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()).fill(192, 192, 210) );

	double colorBarX=21.0*image_width/24.0;
	double colorBarY=1.0*image_height/8.0;
	double colorBarWidth=1.0*image_width/40.0;
	double colorBarHeight=3.0*image_height/4.0;


	for(int k=0; k<isovel.size(); k++ ){
		for(const vector<Coord>& vs: isovel[k] ){
			for(int i=0; i<vs.size()-1; i+=2){
				svg.add( Shape().Line( T().x(vs[i].lon), T().y(vs[i].lat), T().x(vs[i+1].lon), T().y(vs[i+1].lat) ).stroke( 0.7*color[k].r, 0.7*color[k].g, 0.7*color[k].b).strokeWidth(1.0).strokeLinecap("round") );
			}
		}
	}

	svg.add(Shape().ColorBar(colorBarX, colorBarY,
							 colorBarWidth, colorBarHeight,"ColorMap2",
							 vmin, vmax, "Vx (m/s)") );


	vector<float> polygon;
	int Npol=3;
	for(int i=0; i<Npol; i++){
		double alpha=360*i/Npol;
		double x = epi_lon;
		double y = epi_lat+T().inv_sy( T().pointHeight );
		rot(alpha,epi_lon, epi_lat, x, y);
		polygon.push_back( T().x(x) );
		polygon.push_back( T().y(y) );

	}



	svg.add( Shape().Polygon(polygon).fill(255,0,0).stroke(0,0,0).strokeWidth(0.1*T().textHeight) );

	for(int i=0; i<infoData.HOLE_N(); i++){
		double x=Tx+T().sx(infoData.HOLE_X(i)*dh/latlon2m);
		double y=Ty+Tdy-T().sy(infoData.HOLE_Y(i)*dh/latlon2m);
		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
				 .fill(0,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
		stringstream coordEst;
		coordEst<<"("<<infoData.HOLE_X(i)<<", "<<infoData.HOLE_Y(i)<<")";
		svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() )
				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
//		svg.add( Shape().Text(x,y-1.5*T().pointHeight,to_string(i+1))
//				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
	}


	vector< pair<int,int> > vp={
		pair<int,int>(1,1),
		pair<int,int>(infoData.NX(),1),
		pair<int,int>(infoData.NX(),infoData.NY()),
		pair<int,int>(1,infoData.NY())
	};
	for(const auto& p: vp ){
		double x=Tx+T().sx(p.first*dh/latlon2m);
		double y=Ty+Tdy-T().sy(p.second*dh/latlon2m);
		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
				 .fill(64,64,64).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
		stringstream coordEst;
		coordEst<<"("<<p.first<<", "<<p.second<<")";
		svg.add( Shape().Text(x,y-1.5*T().pointHeight,coordEst.str() )
				 .align("middle").fontSize(0.5*T().textHeight).fontFamily("Times").opacity(0.8) );
	}




	for(const Coord& est: estaciones ){
		double x=T().x(est.lon);
		double y=T().y(est.lat);
		svg.add( Shape().Circle(x,y,0.6*T().pointHeight)
				 .fill(255,0,255).stroke(0,0,0).strokeWidth(0.1*T().textHeight).opacity(0.8) );
	}



	svg.add( Shape().Mesh(BB().xmin(),BB().xmax(),BB().ymin(),BB().ymax()) );

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
	val.resize( SX*SY*SZ );

	vector<float> valmax( SX*SY, -std::numeric_limits<float>::max() );


	int NParticionesSX=infoData.IndexXend()-infoData.IndexXini()+1;
	int NParticionesSY=infoData.IndexYend()-infoData.IndexYini()+1;
	int NParticionesSZ=infoData.IndexZend()-infoData.IndexZini()+1;
	int NParticiones=NParticionesSX*NParticionesSY*NParticionesSZ;

	vector<int> particionesSXo(NParticiones),particionesSXf(NParticiones);
	vector<int> particionesSYo(NParticiones),particionesSYf(NParticiones);
	vector<int> particionesSZo(NParticiones),particionesSZf(NParticiones);
	vector<int> particionesSizeArray(NParticiones);
	vector<int> particionesID(NParticiones);

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
						valmax[index_xy] = std::max( valmax[index_xy], fabs(v) );
						colorMap1.getColor(f, r, g, b);
						//cout<<x+SX*y+SX*SY*cdat.data[i].pz[j]<<" ";
						//cout<<val[x+SX*y+SX*SY*cdat.data[i].pz[j]]-min<<" ";
						//cout<<"["<<y<<"]["<<x<<"]:"<<cdat.data[i].pz[j]<<" ";
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

	float velMaxZ = -std::numeric_limits<float>::max();
	float velMinZ =  std::numeric_limits<float>::max();
	for (int i = 0; i < valmax.size(); i++){
		float v = fabs( valmax[i] );
		if( velMaxZ < v ) velMaxZ = v;
		if( velMinZ > v ) velMinZ = v;
	}
	//	velMaxZ=0.002;
	NiceScale nsMax(0,velMaxZ);


	cout<<"velMinZ="<<velMinZ<<endl;
	cout<<"velMaxZ="<<velMaxZ<<endl;

	int CX=SX-1;
	int CY=SY-1;
	//vector<unsigned char> caseCorners( CX*CY,0 );
	//cout<<"**** xy(0,0)->"<<img.xy(0,0).lon<<" "<<img.xy(0,0).lat<<endl;
	//cout<<"**** xy(100,100)->"<<img.xy(100,100).lon<<" "<<img.xy(100,100).lat<<endl;

	NiceScale nsThresholdMax(0,velMaxZ,12);
	int N=nsThresholdMax.N();
	for ( int k=1; k<N-1; k++ ){
		float valThreshold=nsThresholdMax.niceMin + (float)k*nsThresholdMax.tickSpacing;
		//vector<float> valsThreshold{ 0.005, 0.010, 0.015, 0.020, 0.025, 0.030, 0.035, 0.040, 0.045, 0.050};
		//for ( auto valThreshold: valsThreshold ){
		vector< vector<Coord> > coords;
		for (int j = 0; j < CY; j++){
			for (int i = 0; i < CX; i++){
				int i0 = i + SX*j;
				int i1 = i+1 + SX*j;
				int i2 = i+1 + SX*(j+1);
				int i3 = i + SX*(j+1);
				unsigned char valKey=0;
				if( valmax[i0]>valThreshold )valKey|=0x01;
				if( valmax[i1]>valThreshold )valKey|=0x02;
				if( valmax[i2]>valThreshold )valKey|=0x04;
				if( valmax[i3]>valThreshold )valKey|=0x08;
				//			float v = valmax[index];
				//			if( velMaxZ < v ) velMaxZ = v;

				//cout<<hex<<(int)valKey<<" ";
				vector<Coord> line;
				float x,y,p;
				switch(valKey){
				case 1:
					//				cout<<"case1"<<endl;
					//				cout<<"i1="<<valmax[i1]<<" thr="<<valThreshold<<" i0="<<valmax[i0]<<endl;
					//				cout<<"i3="<<valmax[i3]<<" thr="<<valThreshold<<" i0="<<valmax[i0]<<endl;
				case 14:
					x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
					y=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
					line.push_back(img.xy(i+x,j));
					line.push_back(img.xy(i,j+y));
					break;
				case 2:
				case 13:
					x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
					y=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
					line.push_back(img.xy(i+x, j));
					line.push_back(img.xy(i+1, j+y));
					break;
				case 3:
				case 12:
					x=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
					y=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
					line.push_back(img.xy(i,j+x));
					line.push_back(img.xy(i+1,j+y));
					break;
				case 4:
				case 11:
					x=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
					y=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
					line.push_back(img.xy(i+1, j+x));
					line.push_back(img.xy(i+y, j+1));
					break;
				case 5:
				case 10:
					p=(valmax[i0]+valmax[i1]+valmax[i2]+valmax[i3])/4.0;
					if(fabs(valmax[i0]-p)+fabs(valmax[i2]-p)<fabs(valmax[i1]-p)+fabs(valmax[i3]-p)){
						//7 y 8
						x=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
						y=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
						line.push_back(img.xy(i+x, j+1));
						line.push_back(img.xy(i, j+y));
						//2 y 13
						x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
						y=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
						line.push_back(img.xy(i+x, j));
						line.push_back(img.xy(i+1, j+y));
					}
					else{
						//4 y 11
						x=(valThreshold-valmax[i1])/(valmax[i2]-valmax[i1]);
						y=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
						line.push_back(img.xy(i+1, j+x));
						line.push_back(img.xy(i+y, j+1));
						//1 y 14
						x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
						y=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
						line.push_back(img.xy(i+x,j));
						line.push_back(img.xy(i,j+y));
					}
					break;
				case 6:
				case 9:
					x=(valThreshold-valmax[i0])/(valmax[i1]-valmax[i0]);
					y=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
					line.push_back(img.xy(i+x, j));
					line.push_back(img.xy(i+y, j+1));
					break;
				case 7:
				case 8:
					x=(valThreshold-valmax[i3])/(valmax[i2]-valmax[i3]);
					y=(valThreshold-valmax[i0])/(valmax[i3]-valmax[i0]);
					line.push_back(img.xy(i+x, j+1));
					line.push_back(img.xy(i, j+y));
					break;
				}

				if( line.size()>0 ){
					//cout<<"size line"<<line.size()<<endl;
					coords.push_back(line);
				}
			}
			//cout<<endl;
		}
		Color _color;
		double f = (valThreshold-nsThresholdMax.niceMin)/(nsThresholdMax.niceMax-nsThresholdMax.niceMin);
		colorMap2.getColor(f, _color.r, _color.g, _color.b);
		cout << "valThreshold=" << valThreshold << " Color (" << (int)_color.r << ", " << (int)_color.g << ", " << (int)_color.b << ")" << endl;
		color.push_back(_color);
		isovel.push_back(coords);
	}


	//	(0,SY-1)...(SX-1,SY-1)
	//	  .             .
	//	  .             .
	//	  .             .
	//	(0,0)......(SX-1,0)

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
			double f = (valmax[index_xy]-nsMax.niceMin)/(nsMax.niceMax-nsMax.niceMin);
			colorMap2.getColor(f, r, g, b);
			lineMaxZ[x * 3 + 0] = static_cast<uint8_t>(r);
			lineMaxZ[x * 3 + 1] = static_cast<uint8_t>(g);
			lineMaxZ[x * 3 + 2] = static_cast<uint8_t>(b);
		}
		//cout<<endl;
		pngoutMaxZ.write(lineMaxZ.data(), static_cast<size_t>(SX));
	}
	make_svg_max("./imagenes/"+nameSVG_MaxZ.str(),"./"+namePNG_MaxZ.str(), 0,velMaxZ);



	auto t1T = clk::now();
	auto diff = duration_cast<seconds>(t1T-t0T);
	std::cout <<" tiempo de lectura "<<fixed<< diff.count() << "s"<<endl;

	return 0;
}
