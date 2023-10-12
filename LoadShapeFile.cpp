#include "LoadShapeFile.hpp"

void ReadLineShapeFile::polyCentroid(){
	int i, j;
	double ai, atmp = 0.0, xtmp = 0.0, ytmp = 0.0;

	int n=vpol[imax].p.size();
	
	for (i = n-1, j = 0; j < n; i = j, j++)
	{
		ai = vpol[imax].p[i].x * vpol[imax].p[j].y - vpol[imax].p[j].x * vpol[imax].p[i].y;
		atmp += ai;
		xtmp += (vpol[imax].p[j].x + vpol[imax].p[i].x) * ai;
		ytmp += (vpol[imax].p[j].y + vpol[imax].p[i].y) * ai;
	}
	//*area = atmp / 2;
	if (atmp != 0.0)
	{
		xCentroid = TXY.x( xtmp / (3.0 * atmp) );
		yCentroid = TXY.y( ytmp / (3.0 * atmp) );
	}

}


ReadLineShapeFile::ReadLineShapeFile(Transform2 &TXY, string& line){
	this->TXY=TXY;
  size_t ini=line.find_first_of("\"");
  size_t end=line.find_first_of("\"",ini+1);
  stringstream data(line.substr(end+1));
  line=line.substr(ini+1,end-ini-1);
  
  std::string scol;
  while(getline(data,scol,',')){
    col.push_back(scol);
  }
  
  vector<size_t> s;
  bool balanced=true;
  size_t index=0;
  
  
  while(index<line.size() && balanced){
    index=line.find_first_of("()",index);
    if(line[index]=='('){
      s.push_back(index);
    }
    else{
      if(s.empty()){
        balanced=false;
      }
      else{
        size_t ini=s.back();
        size_t end=index;
        int nivel=s.size();
        s.pop_back();
        size_t p=line.substr(ini+1,end-ini-1).find_first_of("()");
        if(p==std::string::npos){
          stringstream ssPolygon(line.substr(ini+1,end-ini-1));
          vector<p2D> polygonX;
          polygon pol;
          //cout<<"("<<ini<<" - "<<nivel<<" - "<<end<<")"<<endl;
          p2D point;
          string sPoint;
          int N=0;
          
          while( getline(ssPolygon,sPoint,',') ){
            stringstream ssPoint(sPoint);
            ssPoint>>point.x>>point.y;
            //cout<<point.x<<" "<<point.y<<endl;
            polygonX.push_back(point);
            pol.p.push_back(point);
            pol.cx+=point.x;
            pol.cy+=point.y;
            pol.xmin=(point.x<pol.xmin)?point.x:pol.xmin;
            pol.xmax=(point.x>pol.xmax)?point.x:pol.xmax;
            pol.ymin=(point.y<pol.ymin)?point.y:pol.ymin;
            pol.ymax=(point.y>pol.ymax)?point.y:pol.ymax;
			PartiallyInsideTheBox |= TXY.withinGeoCoor(point.x,point.y);
            N++;
          }
          
          pol.cx/=(double)N;
          pol.cy/=(double)N;
          //shapes.push_back(polygonX);
          vpol.push_back(pol);
        }
      }
    }
    index++;
  }
  polygonArea();
  
  polyCentroid();
  
  //_cx/=(double)N;
  //_cy/=(double)N;

  if(balanced && s.empty()){
    //cout<<"true"<<endl;
  }
  else{
    //cout<<"false"<<endl;
  }
}


string ReadLineShapeFile::getPath(){
  stringstream ssPolygon;
  /*
  for(auto &&pols:shapes){
    //ssPolygon<<"M "<<T().x(pols[0].x)<<" "<<T().y(pols[0].y)<<" L ";
    ssPolygon<<"M"<<T().x(pols[0].x)<<" "<<T().y(pols[0].y)<<" L";
    for(int i=1;i<pols.size()-1;i++){
      ssPolygon<<T().x(pols[i].x)<<" "<<T().y(pols[i].y)<<" ";
    }
  }
  */
  
  
	  for(auto &&pol:vpol){
  		  //ssPolygon<<"M "<<T().x(pols[0].x)<<" "<<T().y(pols[0].y)<<" L ";
		  ssPolygon<<"M"<<TXY.x(pol.p[0].x)<<" "<<TXY.y(pol.p[0].y)<<" L";
  		  for(int i=1;i<pol.p.size()-1;i++){
			ssPolygon<<TXY.x(pol.p[i].x)<<" "<<TXY.y(pol.p[i].y)<<" ";
  		  }
  	}
  	ssPolygon<<"z";

  
  
  return ssPolygon.str();
}

//https://blog.mapbox.com/a-new-algorithm-for-finding-a-visual-center-of-a-polygon-7c77e6492fbc

//https://www.mathopenref.com/coordpolygonarea2.html
/*
void ReadLineShapeFile::polygonArea(){ 

  cout<<"Área:";
  double Amax=-numeric_limits<double>::max();
  int index=0;
  for(auto &&pol:shapes){
    double area = 0;   // Accumulates area 
    int j = pol.size()-1;
    for (int i=0; i<pol.size(); i++){
      area +=  (pol[j].x+pol[i].x) * (pol[j].y-pol[i].y); 
      j = i;  //j is previous vertex to i
    }
    area/=2.0;
    if(area>Amax){
      Amax=area;
      imax=index;
    }
    index++;
    cout<<area<<" ";
  }
  cout<<" Amax="<<Amax<<endl;
}

int ReadLineShapeFile::pnpoly(double x, double y){
  int i,j;
  int ct=0;
  for(auto &&pol:shapes){
    int c = 0;
    int n=pol.size();
    for (i = 0, j = n-1; i < n; j = i++) {
      if ( ((pol[i].y>y) != (pol[j].y>y)) &&
          (x < (pol[j].x-pol[i].x) * (y-pol[i].y) / (pol[j].y-pol[i].y) + pol[i].x) )
        c = !c;
    }
    ct+=c;
  }
  return ct;
}
*/

void ReadLineShapeFile::polygonArea(){ 

  //cout<<"Área:";
  double Amax=-numeric_limits<double>::max();
  int index=0;
  for(auto &&pol:vpol){
    double area = 0;   // Accumulates area 
    int j = pol.p.size()-1;
    for (int i=0; i<pol.p.size(); i++){
      area +=  (pol.p[j].x+pol.p[i].x) * (pol.p[j].y-pol.p[i].y); 
      j = i;  //j is previous vertex to i
    }
    area/=2.0;
    if(area>Amax){
      Amax=area;
      imax=index;
    }
    index++;
    //cout<<area<<" ";
  }
  //cout<<" Amax="<<Amax<<endl;
}

int ReadLineShapeFile::pnpoly(double x, double y){
  int i,j;
  int ct=0;
  for(auto &&pol:vpol){
    int c = 0;
    int n=pol.p.size();
    for (i = 0, j = n-1; i < n; j = i++) {
      if ( ((pol.p[i].y>y) != (pol.p[j].y>y)) &&
          (x < (pol.p[j].x-pol.p[i].x) * (y-pol.p[i].y) / (pol.p[j].y-pol.p[i].y) + pol.p[i].x) )
        c = !c;
    }
    ct+=c;
  }
  return ct;
}



