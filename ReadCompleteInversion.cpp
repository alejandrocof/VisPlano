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
	    if(key=="Invs" && colon==":"){
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
