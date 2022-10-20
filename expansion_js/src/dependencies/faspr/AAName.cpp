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
#include "AAName.h"


char Three2One(string aa3)
{
  if(aa3=="ALA")return 'A';
  else if(aa3=="CYS")return 'C';
  else if(aa3=="ASP")return 'D';
  else if(aa3=="GLU")return 'E';
  else if(aa3=="PHE")return 'F';
  else if(aa3=="GLY")return 'G';
  else if(aa3=="HIS")return 'H';
  else if(aa3=="ILE")return 'I';
  else if(aa3=="LYS")return 'K';
  else if(aa3=="LEU")return 'L';
  else if(aa3=="MET")return 'M';
  else if(aa3=="ASN")return 'N';
  else if(aa3=="PRO")return 'P';
  else if(aa3=="GLN")return 'Q';
  else if(aa3=="ARG")return 'R';
  else if(aa3=="SER")return 'S';
  else if(aa3=="THR")return 'T';
  else if(aa3=="VAL")return 'V';
  else if(aa3=="TRP")return 'W';
  else if(aa3=="TYR")return 'Y';

  else if(aa3=="AYA")return 'A';
  else if(aa3=="CAS")return 'C';
  else if(aa3=="CAY")return 'C';
  else if(aa3=="CEA")return 'C';
  else if(aa3=="CME")return 'C';
  else if(aa3=="CMT")return 'C';
  else if(aa3=="CSB")return 'C';
  else if(aa3=="CSD")return 'C';
  else if(aa3=="CSE")return 'C';
  else if(aa3=="CSO")return 'C';
  else if(aa3=="CSP")return 'C';
  else if(aa3=="CSS")return 'C';
  else if(aa3=="CSW")return 'C';
  else if(aa3=="CSX")return 'C';
  else if(aa3=="CYG")return 'C';
  else if(aa3=="CYM")return 'C';
  else if(aa3=="NPH")return 'C';
  else if(aa3=="OCS")return 'C';
  else if(aa3=="OCY")return 'C';
  else if(aa3=="SNC")return 'C';
  else if(aa3=="PYX")return 'C';
  else if(aa3=="SMC")return 'C';
  else if(aa3=="SNN")return 'D';
  else if(aa3=="ASQ")return 'D';
  else if(aa3=="BHD")return 'D';
  else if(aa3=="DOH")return 'D';
  else if(aa3=="PHD")return 'D';
  else if(aa3=="CGU")return 'E';
  else if(aa3=="EHP")return 'F';
  else if(aa3=="ACY")return 'G';
  else if(aa3=="GL3")return 'G';
  else if(aa3=="HSD")return 'H';
  else if(aa3=="HSE")return 'H';
  else if(aa3=="HSP")return 'H';
  else if(aa3=="MHS")return 'H';
  else if(aa3=="NEP")return 'H';
  else if(aa3=="H2P")return 'H';
  else if(aa3=="HIC")return 'H';
  else if(aa3=="HIP")return 'H';
  else if(aa3=="INI")return 'K';
  else if(aa3=="MLY")return 'K';
  else if(aa3=="MLZ")return 'K';
  else if(aa3=="KCX")return 'K';
  else if(aa3=="LLP")return 'K';
  else if(aa3=="LLY")return 'K';
  else if(aa3=="LYZ")return 'K';
  else if(aa3=="M3L")return 'K';
  else if(aa3=="CXM")return 'M';
  else if(aa3=="FME")return 'M';
  else if(aa3=="MHO")return 'M';
  else if(aa3=="MSE")return 'M';
  else if(aa3=="OMT")return 'M';
  else if(aa3=="SME")return 'M';
  else if(aa3=="ASX")return 'N';
  else if(aa3=="MEN")return 'N';
  else if(aa3=="HYP")return 'P';
  else if(aa3=="PRS")return 'P';
  else if(aa3=="GLX")return 'Q';
  else if(aa3=="MGN")return 'Q';
  else if(aa3=="PCA")return 'Q';
  else if(aa3=="AAR")return 'R';
  else if(aa3=="AGM")return 'R';
  else if(aa3=="OPR")return 'R';
  else if(aa3=="MIS")return 'S';
  else if(aa3=="SEP")return 'S';
  else if(aa3=="SVA")return 'S';
  else if(aa3=="AEI")return 'T';
  else if(aa3=="TPO")return 'T';
  else if(aa3=="FTR")return 'W';
  else if(aa3=="HTR")return 'W';
  else if(aa3=="TRF")return 'W';
  else if(aa3=="TRN")return 'W';
  else if(aa3=="TRO")return 'W';
  else if(aa3=="ACE")return 'X';
  else if(aa3=="UNK")return 'X';
  else if(aa3=="FOR")return 'X';
  else if(aa3=="TPQ")return 'Y';
  else if(aa3=="TYI")return 'Y';
  else if(aa3=="TYN")return 'Y';
  else if(aa3=="TYQ")return 'Y';
  else if(aa3=="TYS")return 'Y';
  else if(aa3=="TYY")return 'Y';
  else if(aa3=="YOF")return 'Y';
  else if(aa3=="PAQ")return 'Y';
  else if(aa3=="PTH")return 'Y';
  return 'X';
}

void One2Three(char aa1, string &aa3)
{
  switch(aa1){
  case'A':aa3="ALA";break;
  case'C':aa3="CYS";break;
  case'D':aa3="ASP";break;
  case'E':aa3="GLU";break;
  case'F':aa3="PHE";break;
  case'G':aa3="GLY";break;
  case'H':aa3="HIS";break;
  case'I':aa3="ILE";break;
  case'K':aa3="LYS";break;
  case'L':aa3="LEU";break;
  case'M':aa3="MET";break;
  case'N':aa3="ASN";break;
  case'P':aa3="PRO";break;
  case'Q':aa3="GLN";break;
  case'R':aa3="ARG";break;
  case'S':aa3="SER";break;
  case'T':aa3="THR";break;
  case'V':aa3="VAL";break;
  case'W':aa3="TRP";break;
  case'Y':aa3="TYR";break;
  default:break;
  }
}
