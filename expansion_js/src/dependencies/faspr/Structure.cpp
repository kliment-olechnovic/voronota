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
#include "Structure.h"

void Structure::ReadPDB(string &pdbfile, voronota::scripting::StandardOutputMockup& som)
{
  string buf,stmp1,stmp2;
  Residue rtmp;

  int nAlaGly=0;

  {
  voronota::scripting::InputSelector input_selector(pdbfile);
  istream& infile=input_selector.stream();

  if(!infile){
    //cerr<<"error! can not open pdb file "<<pdbfile<<" for reading"<<endl;
    //exit(0);
	throw std::runtime_error(std::string("error! can not open pdb file ")+pdbfile+" for reading");
  }

  while(!infile.eof()){
    buf.clear();
    getline(infile,buf);
    if(buf.empty())break;
    if(buf.substr(0,4)=="END ") break;
    if(buf.substr(0,4)=="ATOM"){
      stmp1=buf.substr(22,5);
      if(stmp1!=stmp2){//a new residue
        if(!stmp2.empty()){//store the last complete residue
          pdb.push_back(rtmp);
          rtmp.atNames.clear();
          rtmp.atTypes.clear();
          rtmp.xyz.clear();
        }
        rtmp.name=buf.substr(17,3);
        if(rtmp.name=="ALA" || rtmp.name=="GLY"){
          nAlaGly++;
        }
        rtmp.chID=buf[21];
        rtmp.pos=atoi(buf.substr(22,4).c_str());
        rtmp.ins=buf[26];
        stmp2=stmp1;
      }  
      if(buf[16]>'A')continue;
      rtmp.atNames.push_back(buf.substr(12,4));
      rtmp.atTypes.push_back(buf[13]);
      FV1 xyz;
      for(int i=30;i<47;i+=8){
        xyz.push_back(atof(buf.substr(i,8).c_str()));
      }
      rtmp.xyz.push_back(xyz);
      xyz.clear();
    }
  };
  }

  pdb.push_back(rtmp);
  rtmp.atNames.clear();
  rtmp.atTypes.clear();
  rtmp.xyz.clear();
  
  nres=pdb.size();
  som.cout() <<"#residues in pdb: "<<nres<<endl;
  som.cout() <<"#alanines and glycines: "<<nAlaGly<<endl;
}

void Structure::WritePDB(string &pdbfile, voronota::scripting::StandardOutputMockup& som)
{
  if(pdbfile=="" || pdbfile=="void"){
    OutputPDB(pdb, som);
  }
  else{
    OutputPDB(pdb,pdbfile);
  }
}

void Structure::OutputPDB(PV1 &pdb, voronota::scripting::StandardOutputMockup& som)
{
  som.cout() <<setiosflags(ios::fixed)<<setprecision(3);
  int i,j,k,atomindex=1;
  som.cout() <<"REMARK repacked structure by FASPR"<<endl;
  for(i=0;i<nres;i++){
    for(j=0;j<static_cast<int>(pdb[i].atNames.size());j++){
      som.cout() <<"ATOM  "<<setw(5)<<atomindex<<" ";
      som.cout() <<pdb[i].atNames[j]<<" "<<pdb[i].name<<" "<<pdb[i].chID<<setw(4)<<pdb[i].pos<<pdb[i].ins<<"   ";
      for(k=0;k<3;k++) som.cout() <<setw(8)<<pdb[i].xyz[j][k];
      som.cout() <<"  1.00  0.00           "<<pdb[i].atTypes[j]<<"  "<<endl;
      atomindex++;
    }
    if(i==nres-1){
      goto TER;
    }
    else{
      if(pdb[i].chID!=pdb[i+1].chID){
TER:        som.cout() <<"TER   "<<setw(5)<<atomindex<<"      "<<pdb[i].name<<" "<<pdb[i].chID<<setw(4)<<pdb[i].pos<<endl;
      }
    }
  }
}

void Structure::OutputPDB(PV1 &pdb,string &pdbfile)
{
  voronota::scripting::OutputSelector output_selector(pdbfile);
  ostream& ofile=output_selector.stream();

  if(!ofile){
    //cerr<<"\ncan not open "<<pdbfile<<" for writing result"<<endl;
    //exit(0);
	throw std::runtime_error(std::string("can not open ")+pdbfile+" for writing result");
  }
  ofile<<setiosflags(ios::fixed)<<setprecision(3);
  int i,j,k,atomindex=1;
  ofile<<"REMARK repacked structure by FASPR"<<endl;
  for(i=0;i<nres;i++){
    for(j=0;j<static_cast<int>(pdb[i].atNames.size());j++){
      ofile<<"ATOM  "<<setw(5)<<atomindex<<" ";
      ofile<<pdb[i].atNames[j]<<" "<<pdb[i].name<<" "<<pdb[i].chID<<setw(4)<<pdb[i].pos<<pdb[i].ins<<"   ";
      for(k=0;k<3;k++)
      ofile<<setw(8)<<pdb[i].xyz[j][k];
      ofile<<"  1.00  0.00           "<<pdb[i].atTypes[j]<<"  "<<endl;
      atomindex++;
    }
    if(i==nres-1){
      goto TER;
    }
    else{
      if(pdb[i].chID!=pdb[i+1].chID){
TER:        ofile<<"TER   "<<setw(5)<<atomindex<<"      "<<pdb[i].name<<" "<<pdb[i].chID<<setw(4)<<pdb[i].pos<<endl;
      }
    }
  }
}


void Structure::Pdb2Fas()
{
  seq.clear();
  for(int i=0;i<nres;i++){
    seq.push_back(Three2One(pdb[i].name));
  }
}


Structure::~Structure()
{
  nres=0;
  pdb.clear();
  seq.clear();
}
