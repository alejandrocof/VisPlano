#include "inputdata.hpp"

inputData::inputData()
{

}

inputData::~inputData(){
    file.close();
}

int nint(float rval)
{
   if(rval < 0.0)
      return (rval - 0.5);
   else
      return (rval + 0.5);
}

//int inputData::s(int i, COMPONENT w){
int inputData::s(int i, int w){
    int ini,fin;
    int r=0;

    if(i==indexini[w])ini=NBG[w];
    else ini=N_SD[w]*i+1;

    //if(i==indexend[w])fin=NED[w];
    //else fin=N_SD[w]*(i+1);
    fin=N_SD[w]*(i+1);
    if(fin>NED[w])fin=NED[w];



//    if( (ini-NBG[w]+1)%_NSKP[w]==1 && (fin-NBG[w]+1)%_NSKP[w]==1 )r=1;

//    return (fin-ini+1)/_NSKP[w]+r;

    /*
    lngthx = nint(real((nedx-nbgx+1))/_NSKPx)
    lngthy = nint(real((nedy-nbgy+1))/_NSKPy)
    */

    return nint( float(fin-ini+1)/_NSKP[w]);

}

int inputData::st(COMPONENT w){
//    int r=((NED[w]-NBG[w]+1)%_NSKP[w]==1)?1:0;
//    return (NED[w]-NBG[w]+1)/_NSKP[w]+r;
    return ceil( float((NED[w]-NBG[w]+1))/_NSKP[w]);
}

int inputData::id(int i, int j, int k){
    return k+j*ng[_Z]+i*ng[_Z]*ng[_Y];
}

void inputData::show(){

    cout<<"Parámetros de tiempo de la simulación"<<endl;
    cout<<"DT="<<_DT<<" NT="<<_NT<<" NTISKP="<<_NTISKP<<endl<<endl;
    cout<<"Parámetros de tiempo en archivos"<<endl;
    cout<<"Tiempo total="<<fixed<< _DT*_NT <<"s"<<endl;
    cout<<"dt="<< _DT*_NTISKP<<"s"<<endl<<endl;
    cout<<"Dominio completo"<<endl;
    cout<<n[_X]<<"steps x "<<n[_Y]<<"steps x "<<n[_Z]<<"steps"<<endl;
    cout<<_DH*n[_X]/1000.0<<"km x "<<_DH*n[_Y]/1000.0<<"km x "<<_DH*n[_Z]/1000.0<<"km"<<endl;
    cout<<_DH*n[_X]/111320.0<<"º x "<<_DH*n[_Y]/111320.0<<"º x "<<_DH*n[_Z]/111320.0<<"º"<<endl<<endl;

    int Sx=st(_X);
    int Sy=st(_Y);
    int Sz=st(_Z);
    cout<<"Dominio reducido"<<endl;
    cout<<Sx<<"steps x "<<Sy<<"steps x "<<Sz<<"steps"<<endl;
    cout<<_DH*_NSKP[_X]*Sx/1000.0<<"km x "<<_DH*_NSKP[_Y]*Sy/1000.0<<"km x "<<_DH*_NSKP[_Z]*Sz/1000.0<<"km"<<endl;
    cout<<_DH*_NSKP[_X]*Sx/111320.0<<"º x "<<_DH*_NSKP[_Y]*Sy/111320.0<<"º x "<<_DH*_NSKP[_Z]*Sz/111320.0<<"º"<<endl<<endl;

    cout<<"Profundida del punto fuente:"<<endl;
    cout<<n[_Z]-_NZSC<<"steps "<<_DH*(n[_Z]-_NZSC)/1000.0<<"km "<<_DH*(n[_Z]-_NZSC)/111320.0<<"º"<<endl<<endl;

    int total=0;
    for(int i=indexini[_X];i<=indexend[_X];i++){
        int Sx=s(i,_X);
        for(int j=indexini[_Y];j<=indexend[_Y];j++){
            int Sy=s(j,_Y);
            for(int k=indexini[_Z];k<=indexend[_Z];k++){
                int Sz=s(k,_Z);
                cout<<"("<<i<<","<<j<<","<<k<<")";
                cout<<"Id="<<id(i,j,k);
                cout<<" Sx="<< Sx;
                cout<<" Sy="<< Sy;
                cout<<" Sz="<< Sz;
                cout<<" tamaño="<<4*Sx*Sy*Sz+8*Sz;
                //cout<<" x("<<Sx
                cout<< endl;
                total+=4*Sx*Sy*Sz+8*Sz;
            }
        }
    }
    cout<<" Total="<<total<<endl;
}


bool inputData::read(string path){
    this->path=path;

    stringstream FileInputData;
    FileInputData<<path<<"/input.dat";

    if (file.is_open()){
        file.close();
        file.clear();
    }
    file.open(FileInputData.str(), std::ifstream::in);
    if (!file.is_open()){
        cerr<<"No se pudo leer el archivo: "<<FileInputData.str()<<endl;
        READY=false;
        return false;
        //exit (EXIT_FAILURE);
    }
    else{
        READY=true;
    }

    nline=0;
    getValuePerLine(_NT,n[_X],n[_Y],n[_Z],ng[_X],ng[_Y],ng[_Z]);
    getValuePerLine(_DH,_DT,STR,DIP,RAKE,SLIP);
    getValuePerLine(_NXSC,_NYSC,_NZSC,_NDEPTH);
    getValuePerLine(IXPOS,IYPOS);
    getValuePerLine(NBG[_X],NED[_X],_NSKP[_X],NBG[_Y],NED[_Y],_NSKP[_Y],NBG[_Z],NED[_Z],_NSKP[_Z]);
    getValuePerLine(_NTISKP,NDAMP,IFRE,NSV,V3D,fo,nstr,nholes);
    hole.resize(nholes);
    for(int i=0;i<nholes;i++){
	    getValuesInLine(hole[i].first,hole[i].second);
	    //cout<<hole[i].first<<" "<<hole[i].second<<endl;
    }
    
    getValuePerLine(ruptureSlip,dxSlip,dtSlip);
    getValuePerLine(nTimeStepsSlip,reducSlip);
    getValuePerLine(lengthSlip,widthSlip);
    getValuePerLine(x_hypocenterSlip,y_hypocenterSlip);
    getValuePerLine(durationSlip);
    getValuePerLine(riseTimeSlip);
    
    //cout<<x_hypocenterSlip<<" "<<y_hypocenterSlip<<endl;
    

    for(int w=_X; w<=_Z;w++){
        if( !(NBG[w]>=0 && NBG[w]<=NED[w] && NED[w]<=n[w]) ){
            cerr<<"Error en los indices del subdominio principal"<<endl;
            READY=false;
            return false;
            //exit (EXIT_FAILURE);
        }else{
            READY=true;
        }
        N_SD[w]=n[w]/ng[w];
        indexini[w]=NBG[w]/N_SD[w];
        indexend[w]=NED[w]/N_SD[w];
        if( s(indexini[w],w)==0 )indexini[w]+=1;
        if( s(indexend[w],w)==0 )indexend[w]-=1;
        if( indexend[w]>=ng[w] ) indexend[w]=ng[w]-1;
        cout<<w<<" n="<<n[w]<<" ng="<<ng[w]<<" N_SD="<<N_SD[w]<<endl;
        cout<<"NBG="<<NBG[w]<<" NED="<<NED[w]<<endl;
        cout<<"indexini["<<w<<"]"<<indexini[w]<<" ";
        cout<<"indexend["<<w<<"]"<<indexend[w]<<endl<<endl;
    }
    return true;

}

void inputData::readLine(int &n,string name){
    nline++;

    string line;
    if( !getline(file,line) ){
        cout<<"Error en la línea "<<nline<<endl;
        exit (EXIT_FAILURE);
    }

    stringstream sline=stringstream(line);

    if(name==""){
        if( !(sline>>n) ){
            cerr<<"Error en la línea "<<nline<<" se esperaba un valor tipo entero:" <<endl;
            cerr<<line<<endl;
            exit (EXIT_FAILURE);
        }
    }
    else{
        string rname;
        if( !(sline>>n>>rname) ){
            cerr<<"Error en la línea "<<nline<<" se esperaba un valor tipo entero y una cadena:" <<endl;
            cerr<<line<<endl;
            exit (EXIT_FAILURE);
        }
        if(name!=rname){
            cerr<<"Error en la línea "<<nline<<" se esperaba la etiqueta "<<name;
            cerr<<" y se encontró "<<rname<<":" <<endl;
            cerr<<line<<endl;
            exit (EXIT_FAILURE);
        }
    }
}

void inputData::getValuesInLine(int &a, int &b){
		nline++;

    string line;
    if( !getline(file,line) ){
        cout<<"Error en la línea "<<nline<<endl;
        exit (EXIT_FAILURE);
    }

    stringstream sline=stringstream(line);
    if( !(sline>>a>>b) ){
        cout<<"Error en la línea "<<nline<<" se esperaba un valor de otro tipo:" <<endl;
        cout<<line<<endl;
        exit (EXIT_FAILURE);
    }
}



