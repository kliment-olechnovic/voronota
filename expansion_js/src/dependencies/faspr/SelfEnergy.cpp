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
#include "SelfEnergy.h"

SelfEnergy::~SelfEnergy(){
  bestrot.clear();
  eTableSelf.clear();
  conMap.clear();
  radius.clear();
  depth.clear();
  atomIdx.clear();
}

void SelfEnergy::AssignConMap()
{
  bestrot.assign(nres,-1);
  map<char,float> res_rad;
  res_rad['G']=2.4;
  res_rad['A']=res_rad['C']=res_rad['D']=res_rad['I']=res_rad['L']=res_rad['N']=res_rad['P']=res_rad['S']=res_rad['T']=res_rad['V']=3.2;
  res_rad['Q']=res_rad['E']=res_rad['H']=3.7;
  res_rad['M']=res_rad['F']=4.3;
  res_rad['K']=5.0;
  res_rad['W']=5.3;
  res_rad['Y']=5.7;
  res_rad['R']=6.2;

  IV1 ivtmp;
  conMap.assign(nres,ivtmp);
  for(int i=0;i<nres;i++){
    if(nrots[i]<2){
      if(nrots[i]==1){
        bestrot[i]=0;
      }
      continue;
    }
    for(int j=0;j<nres;j++){
      if(j==i){
        continue;
      }
      int cb=(seq[j]=='G'||seq[j]=='A')?1:4;
      if(Distance(stru[i].xyz[4],stru[j].xyz[cb])<res_rad[seq[i]]+res_rad[seq[j]]+RESI_DIST_CUT &&
        Distance(stru[i].xyz[4],stru[j].xyz[cb])<Distance(stru[i].xyz[1],stru[j].xyz[1])+CACB_DIST_CUT){
        ivtmp.push_back(j);
      }
    }
    conMap[i]=ivtmp;
    ivtmp.clear();
  }
  res_rad.clear();
}


void SelfEnergy::SetVdwPar()
{
  //set vdw parameters: [X][0]->radius; [X][1]->well-depth
  FV1 vtmp(2,2.0);
  FV2 vdwpar(18,vtmp);//18 atom types in CHARMM19
  vdwpar[ 0][0]=RADIUS_C1; vdwpar[ 0][1]=DEPTH_C1;//1  mainchain CA
  vdwpar[ 1][0]=RADIUS_C2; vdwpar[ 1][1]=DEPTH_C2;//2  mainchain C
  vdwpar[ 2][0]=RADIUS_C3; vdwpar[ 2][1]=DEPTH_C3;//3  CH1
  vdwpar[ 3][0]=RADIUS_C4; vdwpar[ 3][1]=DEPTH_C4;//4  CH2
  vdwpar[ 4][0]=RADIUS_C5; vdwpar[ 4][1]=DEPTH_C5;//5  CH3
  vdwpar[ 5][0]=RADIUS_C6; vdwpar[ 5][1]=DEPTH_C6;//6  Aromatic CH1 and C (Phe/Tyr/Trp)
  vdwpar[ 6][0]=RADIUS_C7; vdwpar[ 6][1]=DEPTH_C7;//7  CO,COO,NCNN
  vdwpar[ 7][0]=RADIUS_C8; vdwpar[ 7][1]=DEPTH_C8;//8  Cys CB
  vdwpar[ 8][0]=RADIUS_N1; vdwpar[ 8][1]=DEPTH_N1;//9  mainchain NH
  vdwpar[ 9][0]=RADIUS_N2; vdwpar[ 9][1]=DEPTH_N2;//10 His/Arg/Trp NH, Asn/Gln/Arg NH2, Lys NH3
  vdwpar[10][0]=RADIUS_N3; vdwpar[10][1]=DEPTH_N3;//11 His C=N-C
  vdwpar[11][0]=RADIUS_N4; vdwpar[11][1]=DEPTH_N4;//12 Pro N
  vdwpar[12][0]=RADIUS_O1; vdwpar[12][1]=DEPTH_O1;//13 mainchain O
  vdwpar[13][0]=RADIUS_O2; vdwpar[13][1]=DEPTH_O2;//14 sidechain C=O
  vdwpar[14][0]=RADIUS_O3; vdwpar[14][1]=DEPTH_O3;//15 sidechain COO
  vdwpar[15][0]=RADIUS_O4; vdwpar[15][1]=DEPTH_O4;//16 Ser/Thr/Tyr OH
  vdwpar[16][0]=RADIUS_S1; vdwpar[16][1]=DEPTH_S1;//17 Cys S
  vdwpar[17][0]=RADIUS_S2; vdwpar[17][1]=DEPTH_S2;//18 Met S

  map<char,IV1> vAtomIdx;//vector of atom index
  IV1 inx(4,9);
  inx[1]=1;inx[2]=2;inx[3]=13;vAtomIdx['G']=inx;//4 atoms
  inx.push_back(5);vAtomIdx['A']=inx;//5 atoms
  inx[4]=8;inx.push_back(17); vAtomIdx['C']=inx;//6 atoms
  inx[4]=4;inx[5]=16; vAtomIdx['S']=inx;//6 atoms
  inx[5]=4;inx.push_back(4);inx[0]=12; vAtomIdx['P']=inx;//7 atoms
  inx[0]=9;inx[4]=3;inx[5]=16;inx[6]=5; vAtomIdx['T']=inx;//7 atoms
  inx[5]=5; vAtomIdx['V']=inx;//7 atoms
  inx[4]=4;inx[5]=4;inx[6]=18;inx.push_back(5);vAtomIdx['M']=inx;//8 atoms
  inx[5]=7;inx[6]=14;inx[7]=10;  vAtomIdx['N']=inx;//8 atoms
  inx[6]=15;inx[7]=15; vAtomIdx['D']=inx;//8 atooms
  inx[5]=3;inx[6]=5;inx[7]=5;  vAtomIdx['L']=inx;//8 atoms
  inx[4]=3;inx[5]=4; vAtomIdx['I']=inx;//8 atoms
  inx[4]=4;inx[6]=7;inx[7]=14;inx.push_back(10);vAtomIdx['Q']=inx;//9 atoms
  inx[7]=15;inx[8]=15; vAtomIdx['E']=inx;//9 atoms
  inx[6]=4;inx[7]=4;inx[8]=10; vAtomIdx['K']=inx;//9 atoms
  //for His, index 6 stands for ND1, index 9 stands for NE2
  inx[5]=6;inx[6]=10;inx[7]=6;inx[8]=6;inx.push_back(11);vAtomIdx['H']=inx; //10 atoms
  inx[5]=4;inx[6]=4;inx[7]=10;inx[8]=7;inx[9]=10;inx.push_back(10);vAtomIdx['R']=inx;//11 atoms
  inx[5]=6;inx[6]=6;inx[7]=6;inx[8]=6;inx[9]=6;inx[10]=6;vAtomIdx['F']=inx;//11 atoms
  inx.push_back(16);vAtomIdx['Y']=inx;//12 atoms
  inx[8]=10;inx[11]=6;inx.push_back(6);inx.push_back(6);vAtomIdx['W']=inx;//14 atoms
  for(int i=0;i<nres;i++){
    FV1 radtmp,depthtmp;
    IV1 ati_tmp=vAtomIdx[seq[i]];
    for(int j=0;j<static_cast<int>(ati_tmp.size());j++){
      //get the atom type index of atom j on residue i
      radtmp.push_back(vdwpar[ati_tmp[j]-1][0]);
      depthtmp.push_back(vdwpar[ati_tmp[j]-1][1]);      
    }
    radius.push_back(radtmp);
    depth.push_back(depthtmp);
    atomIdx.push_back(ati_tmp);
    radtmp.clear();
    depthtmp.clear();
    ati_tmp.clear();
  }
}

float SelfEnergy::VDWType(int a,int b,float rij,float dist)
{
  int i1=a,i2=b;
  if(i1>i2) swap(i1,i2);
  if(i1==9 && (i2==11 || i2==13 || i2==14 || i2==15 || i2==16)){
    if(dist>MIN_HBOND_DIST && dist<MAX_HBOND_DIST){
      return OPT_HBOND_DIST;
    }
  }
  else if(i1==10 && (i2==11 || i2==13 || i2==14 || i2==15 || i2==16)){
    if(dist>MIN_HBOND_DIST && dist<MAX_HBOND_DIST){
      return OPT_HBOND_DIST;
    }
  }
  else if(i1==11 && i2==16){
    if(dist>MIN_HBOND_DIST && dist<MAX_HBOND_DIST){
      return OPT_HBOND_DIST;
    }
  }
  else if((i1==13 || i1==14 || i1==15) && i2==16){
    if(dist>MIN_HBOND_DIST && dist<MAX_HBOND_DIST){
      return OPT_HBOND_DIST;
    }
  }
  else if(i1==16 && i2==16){
    if(dist>MIN_HBOND_DIST && dist<MAX_HBOND_DIST){
      return OPT_HBOND_DIST;
    }
  }
  else if(i1==17 && i2==17){//disulfide bond
    if(dist>MIN_SSBOND_DIST && dist<MAX_SSBOND_DIST){
      return OPT_SSBOND_DIST;
    }
  }
  return rij;
}



//dstar=dij/rij
float SelfEnergy::VDWEnergyAtomAndAtom(float dstar,float eij)
{
  if(dstar>DSTAR_MAX_CUT) return 0.;
  else if(dstar>1){
    double energy = 4*eij*(pow(1/dstar,12)-pow(1/dstar,6));
    return energy;
  }
  else if(dstar>DSTAR_MIN_CUT){
    return VDW_REP_CUT*(dstar-1)/(DSTAR_MIN_CUT-1);
  }
  else{
    return VDW_REP_CUT;
  }
}

float SelfEnergy::HbondEnergyAtomAndAtom(FV1 &DBxyz,FV1 &Dxyz,FV1 &Axyz,FV1 &ABxyz,float Dangle,float Aangle){
  float dist=Distance(Dxyz,Axyz);
  if(dist>MAX_HBOND_DIST || dist<MIN_HBOND_DIST) return 0.;
  float theta=Angle(DBxyz,Dxyz,Axyz);
  if(theta<MIN_HBOND_THETA) return 0.;
  float phi=Angle(Dxyz,Axyz,ABxyz);
  if(phi<MIN_HBOND_PHI) return 0.;
  float fdist=5.*pow(OPT_HBOND_DIST/dist,12)-6.*pow(OPT_HBOND_DIST/dist,10);
  float ftheta=cos((theta-Dangle)*DEG2RAD)*cos((theta-Dangle)*DEG2RAD);
  float fphi=cos((phi-Aangle)*DEG2RAD)*cos((phi-Aangle)*DEG2RAD);
  float energy = fdist*ftheta*fphi;
  energy *= WGT_HBOND;
  if(energy>0.) energy=0.;
  return energy;
}

float SelfEnergy::SSbondEnergyAtomAndAtom(FV1 &,FV1 &CB1xyz,FV1 &SG1xyz,FV1 &SG2xyz,FV1 &CB2xyz,FV1 &){
  float dist=Distance(SG1xyz,SG2xyz);
  if(dist>MAX_SSBOND_DIST || dist<MIN_SSBOND_DIST) return 0.;
  float ang1=Angle(CB1xyz,SG1xyz,SG2xyz);
  if(ang1>MAX_SSBOND_ANGL || ang1<MIN_SSBOND_ANGL) return 0;
  float ang2=Angle(CB2xyz,SG2xyz,SG1xyz);
  if(ang2>MAX_SSBOND_ANGL || ang2<MIN_SSBOND_ANGL) return 0.;
  float torsion1=Dihedral(CB1xyz,SG1xyz,SG2xyz,CB2xyz);
  float energy=100.*(dist-OPT_SSBOND_DIST)*(dist-OPT_SSBOND_DIST)-4.
    +0.01*(ang1-OPT_SSBOND_ANGL)*(ang1-OPT_SSBOND_ANGL)-2.
    +0.01*(ang2-OPT_SSBOND_ANGL)*(ang2-OPT_SSBOND_ANGL)-2.
    +2.*cos(2.*torsion1*DEG2RAD);
  energy *= WGT_SSBOND;
  if(energy>0.) energy=0.;
  return energy;
}


float SelfEnergy::RotamerPreferenceEnergy(int site,int rot)
{
  float elib=-1.*log(probRot[site][rot]/maxProb[site]);
  if(elib>5.){
    elib=5.;
  }
  return wRotlib[seq[site]]*elib;
}

float SelfEnergy::EnergyPolarSidechainAndBackbone(int site1,int rot1,int site2){
  float energy=0.;
  if(site1-site2<=1 && site1-site2>=-1) return energy;
  if(seq[site1]=='D'){
    energy = HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
      HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.);
  }
  else if(seq[site1]=='E'){
    energy = HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
      HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][3],sc[site1][rot1][1],120.,120.);
  }
  else if(seq[site1]=='K'){
    energy = HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],stru[site2].xyz[3],stru[site2].xyz[2],109.5,120.);
  }
  else if(seq[site1]=='R'){
    energy = HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],stru[site2].xyz[3],stru[site2].xyz[2],120.,120.)+
      HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],stru[site2].xyz[3],stru[site2].xyz[2],120.,120.)+
      HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],stru[site2].xyz[3],stru[site2].xyz[2],120.,120.);
  }
  else if(seq[site1]=='W'){
    energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],stru[site2].xyz[3],stru[site2].xyz[2],120.,120.);
  }
  else if(seq[site1]=='H'){
    energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],stru[site2].xyz[3],stru[site2].xyz[2],120.,120.)+
      HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][4],sc[site1][rot1][2],120.,120.);
  }
  else if(seq[site1]=='N'){
    energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],stru[site2].xyz[3],stru[site2].xyz[2],120.,120.)+
      HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.);
  }
  else if(seq[site1]=='Q'){
    energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],stru[site2].xyz[3],stru[site2].xyz[2],120.,120.)+
      HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.);
  }
  else if(seq[site1]=='S'){
    energy = HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],stru[site2].xyz[3],stru[site2].xyz[2],109.5,120.)+
      HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5);
  }
  else if(seq[site1]=='T'){
    energy = HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],stru[site2].xyz[3],stru[site2].xyz[2],109.5,120.)+
      HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5);
  }
  else if(seq[site1]=='Y'){
    energy = HbondEnergyAtomAndAtom(sc[site1][rot1][5],sc[site1][rot1][6],stru[site2].xyz[3],stru[site2].xyz[2],120.,120.)+
      HbondEnergyAtomAndAtom(stru[site2].xyz[1],stru[site2].xyz[0],sc[site1][rot1][6],sc[site1][rot1][5],120.,120.);
  }
  return energy;
}


float SelfEnergy::EnergyPolarSidechainAndSidechain(int site1,int rot1,int site2,int rot2){
  float energy=0.;
  if(seq[site1]=='C'){
    if(seq[site2]=='C'){
      energy = SSbondEnergyAtomAndAtom(stru[site1].xyz[1],stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][0],stru[site2].xyz[4],stru[site2].xyz[1]);
    }
  }
  else if(seq[site1]=='D'){
    if(seq[site2]=='K'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.);
    }
    else if(seq[site2]=='R'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.);
    }
    else if(seq[site2]=='W'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][1],sc[site1][rot1][0],109.5,120.)+
        HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][2],sc[site1][rot1][0],109.5,120.);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][1],sc[site1][rot1][0],109.5,120.)+
        HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][2],sc[site1][rot1][0],109.5,120.);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.);
    }
  }
  else if(seq[site1]=='E'){
    if(seq[site2]=='K'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][1],109.5,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][3],sc[site1][rot1][1],109.5,120.);
    }
    else if(seq[site2]=='R'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][3],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][3],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][3],sc[site1][rot1][1],120.,120.);
    }
    else if(seq[site2]=='W'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][3],sc[site1][rot1][1],120.,120.);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][3],sc[site1][rot1][1],120.,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][3],sc[site1][rot1][1],120.,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][3],sc[site1][rot1][1],120.,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][2],sc[site1][rot1][1],109.5,120.)+
        HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][3],sc[site1][rot1][1],109.5,120.);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][2],sc[site1][rot1][1],109.5,120.)+
        HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][3],sc[site1][rot1][1],109.5,120.);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][3],sc[site1][rot1][1],120.,120.);
    }
  }
  else if(seq[site1]=='K'){
    if(seq[site2]=='D'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][1],sc[site2][rot2][0],109.5,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][2],sc[site2][rot2][0],109.5,120.);
    }
    else if(seq[site2]=='E'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][2],sc[site2][rot2][1],109.5,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][3],sc[site2][rot2][1],109.5,120.);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][4],sc[site2][rot2][2],109.5,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][1],sc[site2][rot2][0],109.5,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][2],sc[site2][rot2][1],109.5,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][0],stru[site2].xyz[4],109.5,109.5);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][0],stru[site2].xyz[4],109.5,109.5);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][2],sc[site1][rot1][3],sc[site2][rot2][6],sc[site2][rot2][5],109.5,120.);
    }
  }
  else if(seq[site1]=='R'){
    if(seq[site2]=='D'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='E'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][4],sc[site2][rot2][2],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][4],sc[site2][rot2][2],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][4],sc[site2][rot2][2],120.,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][4],sc[site2][rot2][6],sc[site2][rot2][5],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][3],sc[site1][rot1][5],sc[site2][rot2][6],sc[site2][rot2][5],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][2],sc[site2][rot2][6],sc[site2][rot2][5],120.,120.);
    }
  }
  else if(seq[site1]=='W'){
    if(seq[site2]=='D'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='E'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][4],sc[site2][rot2][2],120.,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][6],sc[site2][rot2][5],120.,120.);
    }
  }
  else if(seq[site1]=='H'){
    if(seq[site2]=='D'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='E'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='K'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][4],sc[site1][rot1][2],109.5,120.);
    }
    else if(seq[site2]=='R'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][4],sc[site1][rot1][2],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][4],sc[site1][rot1][2],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][4],sc[site1][rot1][2],120.,120.);
    }
    else if(seq[site2]=='W'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][0],120.,120.);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][4],sc[site2][rot2][2],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][4],sc[site1][rot1][2],120.,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][4],sc[site1][rot1][2],120.,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][4],sc[site1][rot1][2],120.,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][4],sc[site1][rot1][2],120.,109.5);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][4],sc[site1][rot1][2],120.,109.5);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][1],sc[site2][rot2][6],sc[site2][rot2][5],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][4],sc[site1][rot1][2],120.,120.);
    }
  }
  else if(seq[site1]=='N'){
    if(seq[site2]=='D'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='E'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='K'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][1],sc[site1][rot1][0],109.5,120.);
    }
    else if(seq[site2]=='R'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.);
    }
    else if(seq[site2]=='W'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][4],sc[site2][rot2][2],120.,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][1],sc[site1][rot1][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][1],sc[site1][rot1][0],109.5,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][1],sc[site1][rot1][0],109.5,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][1],sc[site1][rot1][0],109.5,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][0],sc[site1][rot1][2],sc[site2][rot2][6],sc[site2][rot2][5],120.,120.);
    }
  }
  else if(seq[site1]=='Q'){
    if(seq[site2]=='D'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='E'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='K'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][1],109.5,120.);
    }
    else if(seq[site2]=='R'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.);
    }
    else if(seq[site2]=='W'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][4],sc[site2][rot2][2],120.,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][2],sc[site1][rot1][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][2],sc[site1][rot1][1],109.5,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][2],sc[site1][rot1][1],109.5,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][0],stru[site2].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][2],sc[site1][rot1][1],109.5,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][1],sc[site1][rot1][3],sc[site2][rot2][6],sc[site2][rot2][5],120.,120.);
    }
  }
  else if(seq[site1]=='S'){
    if(seq[site2]=='D'){
      energy = HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='E'){
      energy = HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='K'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][0],stru[site1].xyz[4],109.5,109.5);
    }
    else if(seq[site2]=='R'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='W'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][4],sc[site2][rot2][2],109.5,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][1],sc[site2][rot2][0],109.5,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][2],sc[site2][rot2][1],109.5,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][0],stru[site1].xyz[4],109.5,109.5);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][0],stru[site1].xyz[4],109.5,109.5);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][0],stru[site1].xyz[4],120.0,109.5);
    }
  }
  else if(seq[site1]=='T'){
    if(seq[site2]=='D'){
      energy = HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='E'){
      energy = HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='K'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][0],stru[site1].xyz[4],109.5,109.5);
    }
    else if(seq[site2]=='R'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='W'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][4],sc[site2][rot2][2],109.5,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][1],sc[site2][rot2][0],109.5,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][0],stru[site1].xyz[4],120.,109.5)+
        HbondEnergyAtomAndAtom(stru[site1].xyz[4],sc[site1][rot1][0],sc[site2][rot2][2],sc[site2][rot2][1],109.5,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][0],stru[site1].xyz[4],109.5,109.5);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][0],stru[site1].xyz[4],109.5,109.5);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][0],stru[site1].xyz[4],120.0,109.5);
    }
  }
  else if(seq[site1]=='Y'){
    if(seq[site2]=='D'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][5],sc[site1][rot1][6],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][5],sc[site1][rot1][6],sc[site2][rot2][2],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='E'){
      energy = HbondEnergyAtomAndAtom(sc[site1][rot1][5],sc[site1][rot1][6],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][5],sc[site1][rot1][6],sc[site2][rot2][3],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='K'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][2],sc[site2][rot2][3],sc[site1][rot1][6],sc[site1][rot1][5],109.5,120.);
    }
    else if(seq[site2]=='R'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][4],sc[site1][rot1][6],sc[site1][rot1][5],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][3],sc[site2][rot2][5],sc[site1][rot1][6],sc[site1][rot1][5],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][2],sc[site1][rot1][6],sc[site1][rot1][5],120.,120.);
    }
    else if(seq[site2]=='W'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][6],sc[site1][rot1][5],120.,120.);
    }
    else if(seq[site2]=='H'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][1],sc[site1][rot1][6],sc[site1][rot1][5],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][5],sc[site1][rot1][6],sc[site2][rot2][4],sc[site2][rot2][2],120.,120.);
    }
    else if(seq[site2]=='N'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][0],sc[site2][rot2][2],sc[site1][rot1][6],sc[site1][rot1][5],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][5],sc[site1][rot1][6],sc[site2][rot2][1],sc[site2][rot2][0],120.,120.);
    }
    else if(seq[site2]=='Q'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][1],sc[site2][rot2][3],sc[site1][rot1][6],sc[site1][rot1][5],120.,120.)+
        HbondEnergyAtomAndAtom(sc[site1][rot1][5],sc[site1][rot1][6],sc[site2][rot2][2],sc[site2][rot2][1],120.,120.);
    }
    else if(seq[site2]=='S'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][6],sc[site1][rot1][5],109.5,120.);
    }
    else if(seq[site2]=='T'){
      energy = HbondEnergyAtomAndAtom(stru[site2].xyz[4],sc[site2][rot2][0],sc[site1][rot1][6],sc[site1][rot1][5],109.5,120.);
    }
    else if(seq[site2]=='Y'){
      energy = HbondEnergyAtomAndAtom(sc[site2][rot2][5],sc[site2][rot2][6],sc[site1][rot1][6],sc[site1][rot1][5],120.,120.);
    }
  }
  return energy;
}

void SelfEnergy::EnergySidechainAndBackbone(int site,FV1 &ener)
{
  ener.assign(nrots[site],0.);
  //residue internal energy
  for(int j=0;j<4;j++){
    for(int k=6;k<static_cast<int>(stru[site].atTypes.size());k++){
      float eij=sqrt(depth[site][k]*depth[site][j]);
      for(int r=0;r<nrots[site];r++){
        float dist=Distance(stru[site].xyz[j],sc[site][r][k-5]);
        //if(dist>VDW_DIST_CUT) continue;
        float rij=VDWType(atomIdx[site][k],atomIdx[site][j],radius[site][k]+radius[site][j],dist);
        ener[r]+=VDWEnergyAtomAndAtom(dist/rij,eij);
      }
    }
  }

  for(int i=0;i<static_cast<int>(conMap[site].size());i++){
    int ipair=conMap[site][i];
    //side-chain vs. main-chain vdW interactions
    int n=seq[ipair]=='G'?4:5;
    for(int j=0;j<n;j++){
      for(int k=5;k<static_cast<int>(stru[site].atTypes.size());k++){
        float eij=sqrt(depth[site][k]*depth[ipair][j]);
        for(int r=0;r<nrots[site];r++){
          float dist=Distance(stru[ipair].xyz[j],sc[site][r][k-5]);
          //if(dist>VDW_DIST_CUT) continue;
          float rij=VDWType(atomIdx[site][k],atomIdx[ipair][j],radius[site][k]+radius[ipair][j],dist);
          ener[r]+=VDWEnergyAtomAndAtom(dist/rij,eij);
        }
      }
    }
    //side-chain vs. main-chain Hbond
    for(int r=0;r<nrots[site];r++){
      ener[r]+=EnergyPolarSidechainAndBackbone(site,r,ipair);
    }
  }
}

void SelfEnergy::EnergyRotamerSidechainAndFixedSidechain(int site,FV1 &ener)
{
  for(int i=0;i<static_cast<int>(conMap[site].size());i++){
    int ipair=conMap[site][i];
    if(bestrot[ipair]==-1){
      continue;
    }
    for(int j=5;j<static_cast<int>(stru[ipair].atTypes.size());j++){
      for(int k=5;k<static_cast<int>(stru[site].atTypes.size());k++){
        float eij=sqrt(depth[site][k]*depth[ipair][j]);
        for(int r=0;r<nrots[site];r++){
          float dist=Distance(sc[ipair][0][j-5],sc[site][r][k-5]);
          //if(dist>VDW_DIST_CUT) continue;
          float rij=VDWType(atomIdx[site][k],atomIdx[ipair][j],radius[site][k]+radius[ipair][j],dist);
          ener[r]+=VDWEnergyAtomAndAtom(dist/rij,eij);
        }
      }
    }
    //side-chain vs. side-chain polar energy
    for(int r=0;r<nrots[site];r++){
      ener[r]+=EnergyPolarSidechainAndSidechain(site,r,ipair,0);
    }
  }
}




void SelfEnergy::CalcSelfEnergy(voronota::scripting::StandardOutputMockup& som)
{
  SetVdwPar();
  AssignConMap();
  
  FV1 ftmp;
  eTableSelf.assign(nres,ftmp);
  //calculate self-energy
  for(int i=0;i<nres;i++){
    if(nrots[i]<2){
      continue;
    }
    EnergySidechainAndBackbone(i,eTableSelf[i]);
  }

  int nPosFixedNonAlaGly=0;
  for(int i=0;i<nres;i++){
    if(nrots[i]==1){
      nPosFixedNonAlaGly++;
    }
    if(nrots[i]<2){
      continue;
    }
    float emin=1e8;
    for(int j=0;j<nrots[i];j++){
      if(eTableSelf[i][j]<emin){
        emin=eTableSelf[i][j];
      }
    }
    for(int j=0;j<nrots[i];j++){
      if(eTableSelf[i][j]>emin+SEC_CUT){
        nrots[i]--;
        eTableSelf[i].erase(eTableSelf[i].begin()+j);
        probRot[i].erase(probRot[i].begin()+j);
        chi[i].erase(chi[i].begin()+j);
        sc[i].erase(sc[i].begin()+j);
        j--;
      }
    }
    if(nrots[i]==1){
      //cout<<"fixed residue index: "<<i<<endl;
      bestrot[i]=0;
      nPosFixedNonAlaGly++;
    }
  }
  som.cout() <<"#residues fixed during self-energy-check: "<<nPosFixedNonAlaGly<<endl;
  
  //update the self-energy table
  for(int i=0;i<nres;i++){
    if(nrots[i]<2){
      continue;
    }
    for(int j=0;j<nrots[i];j++){
      eTableSelf[i][j]+=RotamerPreferenceEnergy(i,j);
    }
    EnergyRotamerSidechainAndFixedSidechain(i,eTableSelf[i]);
  }
}
