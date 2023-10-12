#include "ReadCompleteInversion.hpp"

ReadCompleteInversion::ReadCompleteInversion(ifstream &File){
		string line;
    streampos oldpos;
    int id=-1;
    vector<str_id> lista;
    while(getline(File,line) && line.front()=='%'){
	    oldpos=File.tellg();
	    string commentary,key,colon;
	    stringstream ssline(line);
	    ssline>>commentary>>key>>colon;
		if( ( key=="Invs" || key=="Loc" || key=="Mech" ) && colon==":"){
		    getline(ssline,line);
		    std::size_t found;
		    found = line.find("(");
		    line=line.substr (0,found); 
		    //cout<<"lineaCompleta:"<<line<<endl;
		    stringstream ssline2(line);
		    
		    str_id v;
		    while(ssline2>>v.str){
		    	if(v.str=="="){
			    	id++;
		    		lista.back().id=id;
		    	}
		    	v.id=id;
		    	lista.push_back(v);
		    }

	   	}
	    
    }
    File.seekg (oldpos);
    
    //for(int i=0;i<lista.size();i++){
    //	cout<<lista[i].id<<" "<<lista[i].str<<endl;
    //}
    
    int index=0;
		variables.resize(id+1);
    for(int i=0;i<lista.size();){
    	//cout<<"asigna nombre:"<<i<<" "<<lista[i].id<<" "<<lista[i].str<<endl;
    	variables[ lista[i].id ].nombre=lista[i].str;
    	i+=2;
    	variables[ lista[i].id ].valor=lista[i].str;
    	if( lista[i].id == lista[i+1].id ){
    		i++;
    		variables[ lista[i].id ].unidades=lista[i].str;
    	}
    	i++;
    }
    
    //for(int i=0;i<variables.size();i++){
    //	cout<<"nombre:"<<variables[i].nombre;
    //	cout<<" valor:"<<variables[i].valor;
    //	cout<<" unidades:"<<variables[i].unidades<<endl;
    //}
}


void ReadSlip(dataSlip &slip, string filename){
	ifstream inFile;
	//dataSlip data;
	inFile.open(filename, std::ifstream::in);
	if (inFile.is_open()){
		cout<<"Abriendo complete_inversion.fsp"<<endl;
		ReadCompleteInversion Ext(inFile);
		int Nx,Nz;
		double LAT_centro, LON_centro, STRK;
		if( Ext.get("Nx",Nx) && Ext.get("Nz",Nz) && Ext.get("LAT",LAT_centro) && Ext.get("LON",LON_centro)  && Ext.get("STRK",STRK)){
			cout<<"Nx:"<<Nx<<" Nz:"<<Nz<<" LAT:"<<LAT_centro<<" LON:"<<LON_centro<<endl;
			slip.Nx=Nx;
			slip.Nz=Nz;
			slip.LAT=LAT_centro;
			slip.LON=LON_centro;
			slip.STRK=STRK;
			float LAT,LON,X,Y,Z,SLIP,RAKE,TRUP,RISE,SF_MOMENT;
			float max=-std::numeric_limits<float>::max();
			float min=std::numeric_limits<float>::max();
			vector<float> u(Nx*Nz);
			int i=0,j=0;
			int k=0;
			//vector< vector< pair<Coord,float> > > coordInvSlip(Nz);
			slip.data.resize(Nz);
			for( int j=0; j<Nz; j++){
				vector< pointSlip > row(Nx);
				for( int i=0; i<Nx; i++){
					string line;
					getline(inFile,line);
					stringstream ssline(line);
					if(! (ssline>>LAT>>LON>>X>>Y>>Z>>SLIP>>RAKE>>TRUP>>RISE>>SF_MOMENT) ){
						cout<<"Se esperaba un parámetro más en la lectura"<<endl;
					}
					//row[i]=pair<Coord, float>( Coord( LON, LAT ), SLIP );
					row[i]=pointSlip(Coord( LON, LAT ), SLIP);
				}
				slip.data[j]=row;
			}
		}
	}else{
		cout<<"No se encontro el archivo: complete_inversion.fsp"<<endl;
	}

	//return data;
}
