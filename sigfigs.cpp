#include"sigfigs.hpp"

int sigfigs(double dato) {
	double mag=log10(fabs(dato));
	double div=pow(10.0,floor(mag));
	double param, fractpart, intpart;
	int ndig=fabs(floor(mag));
	
	if(mag>=0) return 0;
	
	param = dato/div;
	//cout<<"param="<<param;
	fractpart = modf (param , &intpart);
	//cout<<" "<<fractpart;
	int maxsig=10;
	while(fractpart!=0.0 && fractpart<0.999 && ndig<maxsig){
		param*=10.0;
		fractpart = modf (param , &intpart);
		//cout<<" "<<fractpart;
		ndig++;
	}
	cout<<endl;
	
	return ndig;
}
