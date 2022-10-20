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
#ifndef ROTAMER_BUILDER_H
#define ROTAMER_BUILDER_H

#include "Structure.h"
#include <fstream>
#include <map>
#include <cstring>

#include "faspr_config.h"

using namespace std;

struct Topology
{
  int nchi;
  SV1 atnames;
  IV2 former3AtomIdx;
  FV2 ic;
};

class RotamerBuilder:public Structure
{
public:
  ~RotamerBuilder();
  void LoadSeq(voronota::scripting::StandardOutputMockup& som);
  void LoadSeq(string &seqfile, voronota::scripting::StandardOutputMockup& som);
  void LoadParameter();
  void LoadBBdepRotlib2010(const FASPRConfig& faspr_config);
  void BuildSidechain(const FASPRConfig& faspr_config);
  void RotlibFromBinary2Text(string binlibfile,string &txtlibfile, const FASPRConfig& faspr_config);
  void RotlibFromText2Binary(string &fulltextlib,string &binlibfile, const FASPRConfig& faspr_config);
  void PhiPsi(voronota::scripting::StandardOutputMockup& som);
  void AssignSidechainTopology();
  int LoadBackbone(int site);
  void SideChain(int site,int rot,FV2& rxyz);

  IV1 subStat;
  IV1 nrots;
  FV1 phi,psi;
  FV3 chi;
  FV2 probRot;
  FV1 maxProb;
  PV1 stru;
  FV4 sc;
  map<char,float> wRotlib;
  map<char,Topology> sidechainTopo;
};

#endif
