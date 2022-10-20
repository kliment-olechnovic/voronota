/*******************************************************************************************************************************
This file is a part of the protein side-chain packing software FASPR

Copyright (c) 2020 Xiaoqiang Huang (tommyhuangthu@foxmail.com, xiaoqiah@umich.edu)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, 
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************************************/
#include "Utility.h"


float Distance(FV1 &p1,FV1 &p2){
  return sqrt((p1[0]-p2[0])*(p1[0]-p2[0])+(p1[1]-p2[1])*(p1[1]-p2[1])+(p1[2]-p2[2])*(p1[2]-p2[2]));
}

float VectorAngle(FV1 &c1,FV1 &c2){
  VectorNormalization(c1);
  VectorNormalization(c2);
  float angle=VectorDotProduct(c1,c2);
  if(angle>1.)angle=1.;
  if(angle<-1.)angle=-1.;
  return acos(angle)*RAD2DEG;
}

float Angle(FV1 &p1,FV1 &p2,FV1 &p3){
  FV1 c1,c2;
  VectorSubtract(p1,p2,c1);
  VectorSubtract(p3,p2,c2);  
  return VectorAngle(c1,c2);
}

void VectorCrossProduct(FV1 &c1,FV1 &c2,FV1 &cc){
  cc.clear();
  cc.push_back(c1[1]*c2[2]-c1[2]*c2[1]);
  cc.push_back(c1[2]*c2[0]-c1[0]*c2[2]);
  cc.push_back(c1[0]*c2[1]-c2[0]*c1[1]);
}

float VectorDotProduct(FV1 &c1,FV1 &c2){
  return c1[0]*c2[0]+c1[1]*c2[1]+c1[2]*c2[2];
}

void VectorAdd(FV1 &c1,FV1 &c2,FV1 &cc){
  cc.clear();
  cc.push_back(c1[0]+c2[0]);
  cc.push_back(c1[1]+c2[1]);
  cc.push_back(c1[2]+c2[2]);
}

void VectorSubtract(FV1 &c1,FV1 &c2,FV1 &cc){
  cc.clear();
  cc.push_back(c1[0]-c2[0]);
  cc.push_back(c1[1]-c2[1]);
  cc.push_back(c1[2]-c2[2]);
}

void MatrixByVector(FV2 &mtr,FV1 &vec,FV1 &cc){
  cc.clear();
  cc.push_back(VectorDotProduct(mtr[0],vec));
  cc.push_back(VectorDotProduct(mtr[1],vec));
  cc.push_back(VectorDotProduct(mtr[2],vec));
}

bool VectorNormalization(FV1 &c){
  float len=sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
  if(len==1.00){
    return true;
  }
  if(len<=DELTA){
    //cout<<"warning! the vector is zero-vector"<<endl;
    return false;
  }
  c[0]/=len;
  c[1]/=len;
  c[2]/=len;
  return true;
}

void VectorMultiply(float par,FV1 &c)
{
  c[0]*=par;
  c[1]*=par;
  c[2]*=par;
}

float Dihedral(FV1 &p1,FV1 &p2,FV1 &p3,FV1 &p4){
  FV1 c1,c2,c3;
  VectorSubtract(p1,p2,c1);
  VectorSubtract(p2,p3,c2);
  VectorSubtract(p3,p4,c3);
  FV1 v1,v2,v3;
  VectorCrossProduct(c2,c1,v1);
  VectorCrossProduct(c3,c2,v2);
  VectorCrossProduct(v2,v1,v3);
  return  Sign(VectorDotProduct(v3,c2))*VectorAngle(v1,v2);
}

bool Internal2Cartesian(FV1 &c1,FV1 &c2,FV1 &c3,FV1 &p,FV1 &cc){
  FV1 d2(3,0);
  FV2 mtr(3,d2);
  d2[0]=p[0]*cos(DEG2RAD*p[1]);
  d2[1]=p[0]*cos(DEG2RAD*p[2])*sin(DEG2RAD*p[1]);
  d2[2]=p[0]*sin(DEG2RAD*p[2])*sin(DEG2RAD*p[1]);
  FV1 ab,bc,n;
  VectorSubtract(c2, c1, ab);
  VectorSubtract(c3, c2, bc);
  VectorNormalization(bc);
  VectorCrossProduct(ab,bc,n);
  VectorNormalization(n);
  VectorCrossProduct(n,bc,ab);
  for(int i=0;i<3;i++){
    mtr[i][0]=-bc[i];
    mtr[i][1]=ab[i];
    mtr[i][2]=n[i];
  }
  MatrixByVector(mtr,d2,bc);
  VectorAdd(c3,bc,cc);

  return true;
}

float VectorModulo(FV1 &c)
{
  return sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
}

float Sign(float c)
{
  if(c>0.)
    return 1.;
  else if(c<0.)
    return -1.;
  else
    return 0.;
}

bool VectorL2Norm(FV1 &data)
{
  int i,n=data.size();
  double tot=0;
  for(i=0;i<n;i++){
    if(data[i]!=0){
      tot+=data[i]*data[i];
    }
  }
  if(tot<=0)return false;
  tot=sqrt(tot);
  for(i=0;i<n;i++)
  data[i]/=tot;
  return true;
}

void VectorMinus(FV1 &cc)
{
  cc[0]=-cc[0];
  cc[1]=-cc[1];
  cc[2]=-cc[2];
}


