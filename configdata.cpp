#include "configdata.hpp"

configData::configData(int argc, char *argv[])
{

	string arg1(argv[1]);
	vector<string> allArgs(argv+1, argv + argc);
	float min,max;

	for(unsigned int i=0; i<allArgs.size(); i++){
		if( allArgs[i].front()=='-' ){
			//cout<<i<<" opción "<<allArgs[i]<<endl;

			switch( FnvHash(allArgs[i].c_str()) ){

			case FnvHash("-r"):
				if( ! (stringstream(allArgs[++i]) >> path) ){
					cout<<"Error en la ruta: "<<allArgs[i]<<endl;
				}
				break;

			case FnvHash("-step"):
			{
				if( !(stringstream(allArgs[++i]) >> step) ){
					cout<<"Error en el valor de step: "<<allArgs[i]<<endl;
				}
			}
				break;
			case FnvHash("-initialstep"):
			{
				if( !(stringstream(allArgs[++i]) >> initialstep) ){
					cout<<"Error en el valor de initialstep: "<<allArgs[i]<<endl;
				}
			}
				break;
			case FnvHash("-xhip"):
			{
				if( !(stringstream(allArgs[++i]) >> xhip) ){
					cout<<"Error en el valor de xhip: "<<allArgs[i]<<endl;
				}
			}
				break;
			case FnvHash("-yhip"):
			{
				if( !(stringstream(allArgs[++i]) >> yhip) ){
					cout<<"Error en el valor de yhip: "<<allArgs[i]<<endl;
				}
			}
				break;
			case FnvHash("-Ms"):
			{
				if( !(stringstream(allArgs[++i]) >> Ms) ){
					cout<<"Error en el valor de Ms: "<<allArgs[i]<<endl;
				}
			}
				break;
			case FnvHash("-Dp")://Dp se debe de dar en metros
			{
				if( !(stringstream(allArgs[++i]) >> Dp) ){
					cout<<"Error en el valor de Dp: "<<allArgs[i]<<endl;
				}
			}
				break;
			case FnvHash("-Eq"):
			{
				if( !(stringstream(allArgs[++i]) >> Eq) ){
					cout<<"Error en el valor de Eq: "<<allArgs[i]<<endl;
				}
			}
				break;
			case FnvHash("-Group"):
			{
				if( !(stringstream(allArgs[++i]) >> Group) ){
					cout<<"Error en el valor de Group: "<<allArgs[i]<<endl;
				}
			}
				break;
			case FnvHash("-vx"):
				data.emplace_back(X,min,max);
				break;
			case FnvHash("-vy"):
				data.emplace_back(Y,min,max);
				break;
			case FnvHash("-vz"):
				data.emplace_back(Z,min,max);
				break;

			case FnvHash("-min"):
			{
				//if(data.size()==0)data.emplace_back(X,min,max);

				if( ++i<allArgs.size() ){
					if( !(stringstream(allArgs[i]) >> min) ){
						cout<<"Error en el valor del mínimo: "<<allArgs[i]<<endl;
					}
				}
				else{
					cout<<"Final de argumentos inesperado"<<endl;
				}

				data.back().min=min;
			}
				break;

			case FnvHash("-max"):
			{
				//if(data.size()==0)data.emplace_back(X,min,max);

				if( ++i<allArgs.size() ){
					if( !(stringstream(allArgs[i]) >> max) ){
						cout<<"Error en el valor del máximo: "<<allArgs[i]<<endl;
					}
				}
				else{
					cout<<"Final de argumentos inesperado"<<endl;
				}

				data.back().max=max;
			}
				break;

			case FnvHash("-px"):
			{

				int px;
				//if(data.size()==0)data.emplace_back(X,min,max);
				while(  ++i<allArgs.size() && stringstream(allArgs[i]) >> px ){
					data.back().px.push_back(px);
				}
				i--;
			}
				break;

			case FnvHash("-py"):
			{
				int py;
				//if(data.size()==0)data.emplace_back(X,min,max);
				while( ++i<allArgs.size() && stringstream(allArgs[i]) >> py ){
					data.back().py.push_back(py);
				}
				i--;
			}
				break;

			case FnvHash("-pz"):
			{
				int pz;
				//if(data.size()==0)data.emplace_back(X,min,max);
				while(  ++i<allArgs.size() && stringstream(allArgs[i]) >> pz ){
					data.back().pz.push_back(pz);
				}
				i--;
			}
				break;
			default:
				cerr<<"ERROR: opción "<<allArgs[i]<<" no definida"<<endl;
			}
		}
		else{
			cerr<<"ERROR: opción "<<allArgs[i]<<" no definida"<<endl;
		}

	}

}
