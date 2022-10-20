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

#include "faspr_config.h"

#include "Search.h"
#include <sstream>

#include "../../../../src/scripting/io_selectors.h"

namespace FASPR
{

using namespace std;

int main_of_faspr(int argc, const char** argv, const FASPRConfig& faspr_config, voronota::scripting::StandardOutputMockup& som)
{
  som.cout() <<"###########################################################################"<<endl;
  som.cout() <<"                    FASPR (Version 20200309)                 "<<endl;
  som.cout() <<"  A method for fast and accurate protein side-chain packing, "<<endl;
  som.cout() <<"which is an important problem in protein structure prediction"<<endl;
  som.cout() <<"and protein design."<<endl;
  som.cout() <<endl;
  som.cout() <<"Copyright (c) 2020 Xiaoqiang Huang"<<endl;
  som.cout() <<"Yang Zhang Lab"<<endl;
  som.cout() <<"Dept. of Computational Medicine and Bioinformatics"<<endl;
  som.cout() <<"Medical School"<<endl;
  som.cout() <<"University of Michigan"<<endl;
  som.cout() <<"Email:tommyhuangthu@foxmail.com, xiaoqiah@umich.edu"<<endl;
  som.cout() <<"###########################################################################"<<endl;
  clock_t start,finish;
  float duration;
  start = clock();

  if(argc<2){
    som.cout() <<"Usage: ./FASPR -i input.pdb -o output.pdb\n";
    som.cout() <<"[-s sequence.txt] to load a sequence file\n";
    return 0;
  }

//  string rotfile=faspr_config.PROGRAM_PATH+"/"+faspr_config.ROTLIB2010;
//  fstream infile(rotfile.c_str(),ios::in|ios::binary);
//  if(!infile){
//    //cerr<<"error! cannot find rotamer library "<<faspr_config.ROTLIB2010<<endl;
//    //exit(0);
//    throw std::runtime_error(std::string("error! cannot find rotamer library ")+faspr_config.ROTLIB2010);
//  }
//  else{
//    infile.close();
//  }

  string pdbin=(string)"example/1mol.pdb";
  string pdbout=(string)"example/1mol_FASPR.pdb";
  string seqfile=(string)"void";

  bool sflag=false;
  int i;
  for(i=1;i<argc-1;i++){
   if(argv[i][0]=='-'){
     if(argv[i][1]=='i'){
       i++;
       pdbin=argv[i];
     }
     else if(argv[i][1]=='o'){
       i++;
       pdbout=argv[i];
     }
     else if(argv[i][1]=='s'){
       i++;
       seqfile=argv[i];
       sflag=true;
     }
   }
  }
  
  Solution faspr;
  faspr.ReadPDB(pdbin, som);
  if(sflag) faspr.LoadSeq(seqfile, som);
  else faspr.LoadSeq(som);
  faspr.BuildSidechain(faspr_config);
  faspr.CalcSelfEnergy(som);
  faspr.CalcPairEnergy();
  faspr.Search(som);
  faspr.WritePDB(pdbout, som);
  finish = clock();
  duration = (float)(finish-start)/CLOCKS_PER_SEC;
  som.cout() <<"#computational time: "<<duration<<" seconds"<<endl;

  return 0;
}

}

