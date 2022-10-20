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
#ifndef STRUCTURE_H
#define STRUCTURE_H


#include "Utility.h"
#include "AAName.h"
#include <iomanip>
#include <fstream>

#include "../../../../src/scripting/io_selectors.h"

using namespace std;

struct Residue
{
  string name;
  char chID;
  int pos;
  char ins;
  SV1 atNames;
  string atTypes;
  FV2 xyz;
};

typedef vector<Residue> PV1;
typedef vector<vector<Residue> > PV2;

class Structure
{
public:
  string seq;
  PV1 pdb;
  int nres;

  ~Structure();
  void Pdb2Fas();
  void ReadPDB(string &pdbfile, voronota::scripting::StandardOutputMockup& som);
  void OutputPDB(PV1 &pdb, voronota::scripting::StandardOutputMockup& som);
  void OutputPDB(PV1 &pdb,string &pdbfile);
  void WritePDB(string &pdbfile, voronota::scripting::StandardOutputMockup& som);
};


#endif
