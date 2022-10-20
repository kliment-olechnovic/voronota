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
#include "PairEnergy.h"

void Transpose(float **&tab,int i,int j)
{
  float** tmp=new float* [j];
  for(int k=0;k<j;k++){
    tmp[k]=new float [i];
    for(int l=0;l<i;l++){
      tmp[k][l]=tab[l][k];
    }
  }
  tab=tmp;
}



PairEnergy::~PairEnergy()
{
  int i,j,k;
  for(i=0;i<nres;i++){
    for(j=0;j<nres;j++){
      if(eTablePair[i][j]!=NULL){
        for(k=0;k<nrots[i];k++){
          delete [] eTablePair[i][j][k];
        }
      }
      delete [] eTablePair[i][j];
    }
    delete [] eTablePair[i];
  }
  delete [] eTablePair;
}



bool PairEnergy::EnergyRotamerSidechainAndRotamerSidechain(int site1,int site2,float **&tab)
{
  tab= new float* [nrots[site1]];
  for(int i=0;i<nrots[site1];i++){
    tab[i]=new float [nrots[site2]];
    for(int j=0;j<nrots[site2];j++){
      tab[i][j]=0;
    }
  }
  
  for(int i=5;i<static_cast<int>(stru[site1].atTypes.size());i++){
    for(int j=5;j<static_cast<int>(stru[site2].atTypes.size());j++){
      float eij=sqrt(depth[site1][i]*depth[site2][j]);
      for(int k=0;k<nrots[site1];k++){
        for(int l=0;l<nrots[site2];l++){
          float dist=Distance(sc[site1][k][i-5],sc[site2][l][j-5]);
          float rij=VDWType(atomIdx[site1][i],atomIdx[site2][j],radius[site1][i]+radius[site2][j],dist);
          tab[k][l]+=VDWEnergyAtomAndAtom(dist/rij,eij);
        }
      }
    }
  }

  //side-chain vs. main-chain polar energy
  for(int k=0;k<nrots[site1];k++){
    for(int l=0;l<nrots[site2];l++){
      tab[k][l]+=EnergyPolarSidechainAndSidechain(site1,k,site2,l);
    }
  }
  
  for(int i=0;i<nrots[site1];i++){
    for(int j=0;j<nrots[site2];j++){
      if(tab[i][j]!=0){
        return true;
      }
    }
  }
  
  return false;
}

void PairEnergy::CalcPairEnergy()
{
  int i,j;
  eTablePair= new float***[nres];
  for(i=0;i<nres;i++){
    eTablePair[i]= new float** [nres];
    for(j=0;j<nres;j++)
    eTablePair[i][j]=NULL;
  }
  float **tab;
  int ip;
  for(i=0;i<nres-1;i++){
    if(nrots[i]<2) continue;
    for(j=0;j<static_cast<int>(conMap[i].size());j++){
      ip=conMap[i][j];
      if(nrots[ip]<2) continue;
      if(ip<i) continue;
      if(EnergyRotamerSidechainAndRotamerSidechain(i,ip,tab)){
        eTablePair[i][ip]=tab;
        Transpose(tab,nrots[i],nrots[ip]);
        eTablePair[ip][i]=tab;
      }
    }
  }
}

void PairEnergy::ShowPairEnergy(voronota::scripting::StandardOutputMockup& som)
{
  for(int i=0;i<nres-1;i++){
    for(int j=i+1;j<nres;j++){
      if(eTablePair[i][j] != NULL){
        som.cout() <<"pairwise energies between site "<<i<<" and "<<j<<":";
        for(int k=0;k<nrots[i];k++){
          for(int s=0;s<nrots[j];s++){
        	som.cout() <<" "<<eTablePair[i][j][k][s];
          }
        }
        som.cout() <<endl;
      }
    }
  }
}

