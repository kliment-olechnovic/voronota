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
//#pragma warning(disable:4305)

#include "RotamerBuilder.h"
#include <stdio.h>

#define WGT_CYS  5.5
#define WGT_ASP  2.0
#define WGT_GLU  1.0
#define WGT_PHE  1.5
#define WGT_HIS  3.0
#define WGT_ILE  1.0
#define WGT_LYS  2.0
#define WGT_LEU  2.0
#define WGT_MET  1.5
#define WGT_ASN  2.0
#define WGT_PRO  1.5
#define WGT_GLN  2.5
#define WGT_ARG  1.5
#define WGT_SER  1.5
#define WGT_THR  2.0
#define WGT_VAL  2.0
#define WGT_TRP  3.5
#define WGT_TYR  1.5
#define ROT_PROB_CUT_MIN  0.01
#define ROT_PROB_CUT_ACC  0.97

void RotamerBuilder::LoadSeq(voronota::scripting::StandardOutputMockup& som)
{
  Pdb2Fas();
  subStat.assign(nres,1);
  PhiPsi(som);
}

void RotamerBuilder::LoadSeq(string &seqfile, voronota::scripting::StandardOutputMockup& som)
{
  LoadSeq(som);
  string stmp=seq;
  string buf,tmp;

  {
  voronota::scripting::InputSelector input_selector(seqfile);
  istream& infile=input_selector.stream();

  if(!infile){
    //cerr<<"error! cannot open sequence file: "<<seqfile<<endl;
    //exit(0);
    throw std::runtime_error(std::string("error! cannot open sequence file: ")+seqfile);
  }
  while(!infile.eof()){
    buf.clear();
    getline(infile,buf);
    if(buf.empty())break;
    if(buf[0]=='>')continue;
    tmp.append(buf);
  }
  }
  
  seq.clear();
  int i;
  char aa;
  for(i=0;i<static_cast<int>(tmp.size());i++){
    aa=tmp[i];
    if((aa>='a'&&aa<'z')||(aa>='A'&&aa<'Z'))
    seq.push_back(aa);
  }
  if(static_cast<int>(seq.size())!=nres){
    //cout<<"error! the sequence length from file is different from the sequence length from structure"<<endl;
    //exit(0);
    throw std::runtime_error(std::string("error! the sequence length from file is different from the sequence length from structure"));
  }
  for(i=0;i<nres;i++){
    if(seq[i]>='a'){
      seq[i]-=32;
      if(stmp[i]==seq[i]) subStat[i]=0;
    }
    if(stmp[i]!=seq[i])One2Three(seq[i],pdb[i].name);
  }
  PhiPsi(som);
}
void RotamerBuilder::LoadParameter()
{
  wRotlib['C']=WGT_CYS;
  wRotlib['D']=WGT_ASP;
  wRotlib['E']=WGT_GLU;
  wRotlib['F']=WGT_PHE;
  wRotlib['H']=WGT_HIS;
  wRotlib['I']=WGT_ILE;
  wRotlib['K']=WGT_LYS;
  wRotlib['L']=WGT_LEU;
  wRotlib['M']=WGT_MET;
  wRotlib['N']=WGT_ASN;
  wRotlib['P']=WGT_PRO;
  wRotlib['Q']=WGT_GLN;
  wRotlib['R']=WGT_ARG;
  wRotlib['S']=WGT_SER;
  wRotlib['T']=WGT_THR;
  wRotlib['V']=WGT_VAL;
  wRotlib['W']=WGT_TRP;
  wRotlib['Y']=WGT_TYR;

}

void RotamerBuilder::LoadBBdepRotlib2010(const FASPRConfig& faspr_config)
{
  map<char,int> Chin;//number of chi for the residue
  map<char,int> Rotn;//rotamer number of a residue
  map<char,int> Rotl;//skipped line in reading rotamer library
  Chin['R']=4;Chin['N']=2;Chin['D']=2;Chin['C']=1;Chin['Q']=3;Chin['E']=3;Chin['H']=2;Chin['I']=2;
  Chin['L']=2;Chin['K']=4;Chin['M']=3;Chin['F']=2;Chin['P']=2;Chin['S']=1;Chin['T']=1;Chin['W']=2;
  Chin['Y']=2;Chin['V']=1;

  Rotn['R']=75;Rotn['N']=36;Rotn['D']=18;Rotn['C']=3;Rotn['Q']=108;Rotn['E']=54;Rotn['H']=36;
  Rotn['I']=9;Rotn['L']=9;Rotn['K']=73;Rotn['M']=27;Rotn['F']=18;Rotn['P']=2;Rotn['S']=3;
  Rotn['T']=3;Rotn['W']=36;Rotn['Y']=18;Rotn['V']=3;
  Rotl['R']=0;Rotl['N']=75;Rotl['D']=111;Rotl['C']=129;Rotl['Q']=132;Rotl['E']=240;Rotl['H']=294;
  Rotl['I']=330;Rotl['L']=339;Rotl['K']=348;Rotl['M']=421;Rotl['F']=448;Rotl['P']=466;Rotl['S']=468;
  Rotl['T']=471;Rotl['W']=474;Rotl['Y']=510;Rotl['V']=528;

  int i,j,k;
  int phin,psin,Xn;
  float p,mp,ap;
  string buf;
  FV1 ftmp,stmp;
  FV2 ctmp;
  fstream infile;
  short sht;
  string rotfile=faspr_config.PROGRAM_PATH+"/"+faspr_config.ROTLIB2010;
  infile.open(rotfile.c_str(),ios::in|ios::binary);
  if(!infile){
    //cerr<<"error! cannot open rotamer library "<<rotfile<<endl;
    //exit(0);
    throw std::runtime_error(std::string("error! cannot open rotamer library ")+rotfile);
  }
  for(i=0;i<nres;i++){
    mp=0.;
    if(subStat[i]==0||seq[i]=='A'||seq[i]=='G'){
      goto CT;
    }

    p=phi[i]+180.;
    phin=(int)floor((p+5.)/10.);
    if(phin>=36){
      phin-=36;
    }
    p=psi[i]+180.;
    psin=(int)floor((p+5.)/10.);
    if(psin>=36){
      psin-=36;
    }
    Xn=Chin[seq[i]];
    infile.seekg((1296*Rotl[seq[i]]+(36*phin+psin)*Rotn[seq[i]])*20,ios::beg);
    ap=0.;
    for(j=0;j<Rotn[seq[i]];j++){
      infile.read((char*)&p,4);
      if(p<ROT_PROB_CUT_MIN)break;
      if(p==0.){
        p=1e-7;
      }
      if(p>mp){
        mp=p;
      }
      stmp.push_back(p);
      for(k=0;k<Xn;k++){
        infile.read((char*)&sht,2);
        ftmp.push_back(((float)sht)/10.);
      }
      infile.seekg((8-Xn)*2,ios::cur);//skip the other fields
      ctmp.push_back(ftmp);
      ftmp.clear();
      ap+=p;
      if(ap>ROT_PROB_CUT_ACC)break;
    }
CT:    chi.push_back(ctmp);
    ctmp.clear();
    nrots.push_back(stmp.size());
    probRot.push_back(stmp);
    stmp.clear();
    maxProb.push_back(mp);
  }
  infile.close();
  Rotn.clear();
  Rotl.clear();
  Chin.clear();
}


void RotamerBuilder::RotlibFromBinary2Text(string binlibfile,string &txtlibfile, const FASPRConfig& faspr_config)
{
  fstream infile;
  string pp=faspr_config.PROGRAM_PATH+binlibfile;
  infile.open(pp.c_str(),ios::in|ios::binary);
  if(!infile){
    //cerr<<"error! cannot open rotamer library "<<pp<<endl;exit(0);
    throw std::runtime_error(std::string("error! cannot open rotamer library ")+pp);
  }
  ofstream outfile;
  outfile.open(txtlibfile.c_str());
  outfile<<setiosflags(ios::fixed)<<setprecision(6);
  while(!infile.eof()){
    float p;
    infile.read((char*)&p,4);
    outfile<<setprecision(6)<<setw(8)<<p;
    short val;
    for(int i=0;i<8;++i){
      infile.read((char*)&val,2);
      outfile<<setprecision(1)<<setw(7)<<(float)val/10.0;
    }
    outfile<<endl;
  }
  infile.close();
  outfile.close();
}

/**************************************************************
RotlibFromText2Binary() converts an standard Dunbrack library
into a binary rotamer library for fast access
***************************************************************/
void RotamerBuilder::RotlibFromText2Binary(string &fulltextlib,string &binlibfile, const FASPRConfig& faspr_config)
{
  ofstream outfile;
  outfile.open(binlibfile.c_str(),ios::binary);

  char inpath[2048];
  snprintf(inpath,2048,"%s%s",faspr_config.PROGRAM_PATH.c_str(),fulltextlib.c_str());
  FILE* infile=fopen(inpath,"r");
  if(infile==NULL){
    //cerr<<"error! cannot open rotamer library "<<inpath<<endl;
    //exit(0);
	  throw std::runtime_error(std::string("error! cannot open rotamer library ")+inpath);
  }
  char line[2048];
  while(fgets(line,2048,infile)){
    if(line[0]=='#') continue;
    char aaname[4];
    int phi,psi;
    int a,b,c,d,e;
    float prob,x1,x2,x3,x4,v1,v2,v3,v4;
    sscanf(line,"%s %d %d %d %d %d %d %d %f %f %f %f %f %f %f %f %f\n",
      aaname,&phi,&psi,&a,&b,&c,&d,&e,&prob,&x1,&x2,&x3,&x4,&v1,&v2,&v3,&v4);
    if(phi==180 || psi==180) continue;
    outfile.write((char*)&prob,sizeof(float));
    short s1=(short)(10*x1);
    short s2=(short)(10*x2);
    short s3=(short)(10*x3);
    short s4=(short)(10*x4);
    short s5=(short)(10*v1);
    short s6=(short)(10*v2);
    short s7=(short)(10*v3);
    short s8=(short)(10*v4);
    outfile.write((char*)&s1,sizeof(short));
    outfile.write((char*)&s2,sizeof(short));
    outfile.write((char*)&s3,sizeof(short));
    outfile.write((char*)&s4,sizeof(short));
    outfile.write((char*)&s5,sizeof(short));
    outfile.write((char*)&s6,sizeof(short));
    outfile.write((char*)&s7,sizeof(short));
    outfile.write((char*)&s8,sizeof(short));
  }
  fclose(infile);
  outfile.close();
}

int RotamerBuilder::LoadBackbone(int site)
{
  if(subStat[site]==0)stru[site]=pdb[site];
  else if(seq[site]=='G')return 0;
  else{
    int i;
    SV1 atmp=sidechainTopo[seq[site]].atnames;
    for(i=0;i<static_cast<int>(atmp.size());i++){
      stru[site].atNames.push_back(atmp[i]);
      stru[site].atTypes.push_back(atmp[i][1]);
    }
    FV1 p,xyz;
    p=sidechainTopo[seq[site]].ic[0];
    Internal2Cartesian(stru[site].xyz[2],stru[site].xyz[0],stru[site].xyz[1],p,xyz);
    stru[site].xyz.push_back(xyz);
  }
  return 0;
}

void RotamerBuilder::SideChain(int site,int rot,FV2& rxyz)
{
  rxyz.clear();
  FV2 ftmp2=stru[site].xyz;
  int i,chipos=0;
  IV1 Inx;
  FV1 p,xyz,dihe=chi[site][rot];
  //starting from i=1 to skip the CB atoms
  for(i=1;i<static_cast<int>(sidechainTopo[seq[site]].atnames.size());i++){
    Inx=sidechainTopo[seq[site]].former3AtomIdx[i];
    p=sidechainTopo[seq[site]].ic[i];
    if(p[2]==181.0){
      p[2]=dihe[chipos];chipos++;
    }
    Internal2Cartesian(ftmp2[Inx[0]],ftmp2[Inx[1]],ftmp2[Inx[2]],p,xyz);
    ftmp2.push_back(xyz);
    rxyz.push_back(xyz);
    xyz.clear();
  }
}

void RotamerBuilder::PhiPsi(voronota::scripting::StandardOutputMockup& som)
{
  stru.clear();
  map<char,int> atNum;
  atNum['C']=atNum['S']=6;atNum['P']=atNum['T']=atNum['V']=7;atNum['M']=atNum['N']=atNum['D']=atNum['I']=atNum['L']=8;
  atNum['Q']=atNum['E']=atNum['K']=9;atNum['H']=10;atNum['R']=atNum['F']=11;atNum['Y']=12;atNum['W']=14;atNum['A']=5;
  int i,j,k,l,Inx[4];
  SV1 stmp(4," N  ");stmp[1]=" CA ";stmp[2]=" C  ";stmp[3]=" O  ";
  //default, set phi as -60 and psi as 60 (see SCWRL4 paper)
  phi.assign(nres,-60.);
  psi.assign(nres,60.);
  Residue rtmp;
  for(i=0;i<nres;i++){
    if(pdb[i].atNames.size()<3){
      //cout<<"error! backbone is incomplete at residue: "<<pdb[i].name<<pdb[i].pos<<", please check!"<<endl;
      //exit(0);
      throw std::runtime_error(std::string("error! backbone is incomplete at residue: ")+pdb[i].name+", please check!");
    }
    l=0;
    for(j=0;j<static_cast<int>(pdb[i].atNames.size());j++){
      for(k=0;k<4;k++){
        if(pdb[i].atNames[j]==stmp[k]){
          Inx[k]=j;
          l++;
        }
      }
      if(l==4)break;
    }
    if(l<4){
      if(l==3){
        l=0;
        for(j=0;j<static_cast<int>(pdb[i].atNames.size());j++){
          for(k=0;k<3;k++){
            if(pdb[i].atNames[j]==stmp[k]){
              Inx[k]=j;l++;
            }
          }
          if(l==3) goto ADDO;
        }
      }
      //cout<<"error: backbone is incomplete at residue: "<<pdb[i].name<<pdb[i].pos<<", please check!"<<endl;
      //exit(0);
      throw std::runtime_error(std::string("error: backbone is incomplete at residue: ")+pdb[i].name+", please check!");
ADDO:      Inx[3]=pdb[i].atNames.size();
      FV1 p(3,1.234),xyz;p[1]=120;p[2]=180;
      if(i==nres-1){
        Internal2Cartesian(pdb[i].xyz[Inx[0]],pdb[i].xyz[Inx[1]],pdb[i].xyz[Inx[2]],p,xyz);
      }
      else if(Distance(pdb[i].xyz[2],pdb[i+1].xyz[0])>BONDDIST){
        Internal2Cartesian(pdb[i].xyz[Inx[0]],pdb[i].xyz[Inx[1]],pdb[i].xyz[Inx[2]],p,xyz);
      }
      else{
        Internal2Cartesian(pdb[i+1].xyz[0],pdb[i].xyz[Inx[1]],pdb[i].xyz[Inx[2]],p,xyz);
      }
      pdb[i].xyz.push_back(xyz);
    }
    rtmp.chID=pdb[i].chID;rtmp.ins=pdb[i].ins;rtmp.name=pdb[i].name;rtmp.pos=pdb[i].pos;
    rtmp.atNames=stmp;rtmp.atTypes="NCCO";
    for(j=0;j<4;j++){
      rtmp.xyz.push_back(pdb[i].xyz[Inx[j]]);
    }
    stru.push_back(rtmp);
    rtmp.atNames.clear();
    rtmp.atTypes.clear();
    rtmp.xyz.clear();
    if(subStat[i]==0){
      if(atNum[seq[i]]>static_cast<int>(pdb[i].atNames.size())){
        som.cout() <<"warning! incomplete residue "<<pdb[i].name<<pdb[i].pos<<endl;
        subStat[i]=1;
      }
    }
    for(j=0;j<3;j++){
      if(Distance(stru[i].xyz[j],stru[i].xyz[j+1])>BONDDIST){
        som.cout() <<"warning! chain is discontinuous at residue "<<pdb[i].name<<stru[i].pos<<endl;
      }
    }
    if(i==0)continue;
    if(Distance(stru[i-1].xyz[2],stru[i].xyz[0])>BONDDIST){
      som.cout() <<"warning! chain is discontinuous at ";
      som.cout() <<seq[i-1]<<stru[i-1].pos<<" and "<<seq[i]<<stru[i].pos<<endl;
      continue;
    }
    psi[i-1]=Dihedral(stru[i-1].xyz[0],stru[i-1].xyz[1],stru[i-1].xyz[2],stru[i].xyz[0]);
    phi[i]=Dihedral(stru[i-1].xyz[2],stru[i].xyz[0],stru[i].xyz[1],stru[i].xyz[2]);
  }
  atNum.clear();
}

void RotamerBuilder::AssignSidechainTopology()
{
  Topology ttmp;
  IV1 itmp;
  FV1 ftmp;

  //Ala
  ttmp.nchi=0;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['A']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Arg
  ttmp.nchi=4;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.52);ftmp.push_back(114.1);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD ");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.52);ftmp.push_back(111.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" NE ");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.461);ftmp.push_back(112);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CZ ");
  itmp.push_back(5);itmp.push_back(6);itmp.push_back(7);itmp.push_back(8);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.33);ftmp.push_back(124.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" NH1");
  itmp.push_back(6);itmp.push_back(7);itmp.push_back(8);itmp.push_back(9);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.326);ftmp.push_back(120);ftmp.push_back(0);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" NH2");
  itmp.push_back(9);itmp.push_back(7);itmp.push_back(8);itmp.push_back(10);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.326);ftmp.push_back(120);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['R']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Asn
  ttmp.nchi=2;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.516);ftmp.push_back(112.7);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" OD1");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.231);ftmp.push_back(120.8);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" ND2");
  itmp.push_back(6);itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.328);ftmp.push_back(116.5);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['N']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Asp
  ttmp.nchi=2;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.516);ftmp.push_back(112.7);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" OD1");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.25);ftmp.push_back(118.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" OD2");
  itmp.push_back(6);itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.25);ftmp.push_back(118.5);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['D']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Cys
  ttmp.nchi=1;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" SG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.807);ftmp.push_back(114);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['C']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Gln
  ttmp.nchi=3;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.52);ftmp.push_back(114.1);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD ");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.516);ftmp.push_back(112.7);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" OE1");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.231);ftmp.push_back(120.8);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" NE2");
  itmp.push_back(7);itmp.push_back(5);itmp.push_back(6);itmp.push_back(8);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.328);ftmp.push_back(116.5);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['Q']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Glu
  ttmp.nchi=3;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.52);ftmp.push_back(114.1);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD ");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.516);ftmp.push_back(112.7);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" OE1");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.25);ftmp.push_back(118.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" OE2");
  itmp.push_back(7);itmp.push_back(5);itmp.push_back(6);itmp.push_back(8);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.25);ftmp.push_back(118.5);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['E']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //His
  ttmp.nchi=2;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.5);ftmp.push_back(113.8);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" ND1");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.378);ftmp.push_back(122.7);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD2");
  itmp.push_back(6);itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.354);ftmp.push_back(131);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CE1");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);itmp.push_back(8);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.32);ftmp.push_back(109.2);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" NE2");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);itmp.push_back(9);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.374);ftmp.push_back(107.2);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['H']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Ile
  ttmp.nchi=2;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.546);ftmp.push_back(111.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG1");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.3);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG2");
  itmp.push_back(5);itmp.push_back(1);itmp.push_back(4);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.521);ftmp.push_back(110.5);ftmp.push_back(-122.6);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD1");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.516);ftmp.push_back(114);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['I']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Leu
  ttmp.nchi=2;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(116.3);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD1");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.521);ftmp.push_back(110.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD2");
  itmp.push_back(6);itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.521);ftmp.push_back(110.5);ftmp.push_back(122.6);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['L']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Lys
  ttmp.nchi=4;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.52);ftmp.push_back(114.1);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD ");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.52);ftmp.push_back(111.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CE ");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.52);ftmp.push_back(111.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" NZ ");
  itmp.push_back(5);itmp.push_back(6);itmp.push_back(7);itmp.push_back(8);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.489);ftmp.push_back(112);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['K']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Met
  ttmp.nchi=3;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.52);ftmp.push_back(114.1);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" SD ");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.807);ftmp.push_back(112.7);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CE ");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.789);ftmp.push_back(100.8);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['M']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Phe
  ttmp.nchi=2;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.5);ftmp.push_back(113.8);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD1");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.391);ftmp.push_back(120.7);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD2");
  itmp.push_back(6);itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.391);ftmp.push_back(120.7);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CE1");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);itmp.push_back(8);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.393);ftmp.push_back(120.7);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CE2");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);itmp.push_back(9);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.393);ftmp.push_back(120.7);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CZ ");
  itmp.push_back(5);itmp.push_back(6);itmp.push_back(8);itmp.push_back(10);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.39);ftmp.push_back(120);ftmp.push_back(0);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['F']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Pro
  ttmp.nchi=2;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(103.2);ftmp.push_back(-120);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.495);ftmp.push_back(104.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD ");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.507);ftmp.push_back(105.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['P']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Ser
  ttmp.nchi=1;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" OG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.417);ftmp.push_back(110.8);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['S']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Thr
  ttmp.nchi=1;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.542);ftmp.push_back(111.5);ftmp.push_back(-122);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" OG1");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.433);ftmp.push_back(109.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG2");
  itmp.push_back(5);itmp.push_back(1);itmp.push_back(4);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.521);ftmp.push_back(110.5);ftmp.push_back(-120);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['T']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Trp
  ttmp.nchi=2;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.5);ftmp.push_back(113.8);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD1");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.365);ftmp.push_back(126.9);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD2");
  itmp.push_back(6);itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.433);ftmp.push_back(126.7);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" NE1");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);itmp.push_back(8);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.375);ftmp.push_back(110.2);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CE2");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);itmp.push_back(9);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.413);ftmp.push_back(107.2);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CE3");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);itmp.push_back(10);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.4);ftmp.push_back(133.9);ftmp.push_back(0);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CZ2");
  itmp.push_back(5);itmp.push_back(7);itmp.push_back(9);itmp.push_back(11);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.399);ftmp.push_back(122.4);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CZ3");
  itmp.push_back(5);itmp.push_back(7);itmp.push_back(10);itmp.push_back(12);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.392);ftmp.push_back(118.7);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CH2");
  itmp.push_back(7);itmp.push_back(9);itmp.push_back(11);itmp.push_back(13);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.372);ftmp.push_back(117.5);ftmp.push_back(0);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['W']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Tyr
  ttmp.nchi=2;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.53);ftmp.push_back(110.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG ");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.511);ftmp.push_back(113.8);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD1");
  itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.394);ftmp.push_back(120.8);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CD2");
  itmp.push_back(6);itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.394);ftmp.push_back(120.8);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CE1");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(6);itmp.push_back(8);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.392);ftmp.push_back(121.1);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CE2");
  itmp.push_back(4);itmp.push_back(5);itmp.push_back(7);itmp.push_back(9);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.392);ftmp.push_back(121.1);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CZ ");
  itmp.push_back(5);itmp.push_back(6);itmp.push_back(8);itmp.push_back(10);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.385);ftmp.push_back(119.5);ftmp.push_back(0);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" OH ");
  itmp.push_back(6);itmp.push_back(8);itmp.push_back(10);itmp.push_back(11);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.376);ftmp.push_back(119.7);ftmp.push_back(180);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['Y']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
  //Val
  ttmp.nchi=1;
  ttmp.atnames.push_back(" CB ");
  itmp.push_back(2);itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.546);ftmp.push_back(111.5);ftmp.push_back(-122.5);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG1");
  itmp.push_back(0);itmp.push_back(1);itmp.push_back(4);itmp.push_back(5);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.521);ftmp.push_back(110.5);ftmp.push_back(181);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  ttmp.atnames.push_back(" CG2");
  itmp.push_back(5);itmp.push_back(1);itmp.push_back(4);itmp.push_back(6);
  ttmp.former3AtomIdx.push_back(itmp);itmp.clear();
  ftmp.push_back(1.521);ftmp.push_back(110.5);ftmp.push_back(122.6);
  ttmp.ic.push_back(ftmp);ftmp.clear();
  sidechainTopo['V']=ttmp;
  ttmp.atnames.clear();ttmp.former3AtomIdx.clear();ttmp.ic.clear();
}


RotamerBuilder::~RotamerBuilder()
{
  stru.clear();
  sc.clear();
  phi.clear();
  psi.clear();
  chi.clear();
  probRot.clear();
  maxProb.clear();
  nrots.clear();
  subStat.clear();
  sidechainTopo.clear();
}


void RotamerBuilder::BuildSidechain(const FASPRConfig& faspr_config)
{
  LoadParameter();
  LoadBBdepRotlib2010(faspr_config);
  AssignSidechainTopology();
  int i,j;
  for(i=0;i<nres;i++){
    LoadBackbone(i);
  }

  FV2 rxyz;
  FV3 rtmp;
  for(i=0;i<nres;i++){
    if(nrots[i]!=0){
      for(j=0;j<nrots[i];j++){
        SideChain(i,j,rxyz);
        rtmp.push_back(rxyz);
      }
    }
    sc.push_back(rtmp);
    rtmp.clear();
  }  
  sidechainTopo.clear();
  subStat.clear();
}

