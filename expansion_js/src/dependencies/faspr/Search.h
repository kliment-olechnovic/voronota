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
#ifndef SEARCH_H
#define SEARCH_H

#include "PairEnergy.h"
#include <set>
#include <stack>
#include <cstring>
#include <ctime>

using namespace std;

#define DEE_THRESHOLD 0.0

/**************************************************************************
                     RankElement and RankSize
***************************************************************************/
template <class Elem> class RankElement{
public:
  RankElement(int a,Elem b):idx(a),element(b){}
  int idx;
  Elem element;
  bool operator < (const RankElement &m)const {
    return element<m.element;
  }
  bool operator > (const RankElement &m)const {
    return element>m.element;
  }
};


//sort elements according to the size
template <class Elem> class RankSize{
public:
  RankSize(int a,Elem b):idx(a),element(b){}
  int idx;
  Elem element;
  bool operator < (const RankSize &m)const {
    return element.size()<m.element.size();
  }
  bool operator > (const RankSize &m)const {
    return element.size()>m.element.size();
  }
};

/**************************************************************************
                                 Graph
The whole residue interaction graph consists of several subgraphs/clusters, 
each cluster can be represented as a map:
  map(current Vertex)->neighbor Vertices
***************************************************************************/
typedef map<int, set <int> > Graph;
void ShowGraph(Graph &graph);


/**************************************************************************
                                 Bag
***************************************************************************/
typedef enum{
  Root,
  Inner,
  Leaf,
  None
}BagType;

class Bag{
public:
  set<int> left;
  set<int> right;
  set<int> total;
  int parentBagIdx;
  set<int> childBagIdx;
  int type;
  int childCounter;

  /* data structure to record solution*/
  IV1 lsites;
  IV1 rsites;
  IV1 tsites;
  IV2 lrots;
  IV2 rrots;
  FV2 Etcom;
  IV2 Rtcom;
  IV1 indices;
  bool deployFlag;

  /* for backtrack*/
  float EGMEC;
  IV1 RLGMEC;
  IV1 RRGMEC;

  Bag(){
    type=Leaf;
    deployFlag=false;
  }
  void ShowBag(voronota::scripting::StandardOutputMockup& som);
};


/**************************************************************************
                         Tree Decomposition
***************************************************************************/
class TreeDecomposition{
public:
  vector <Bag> bags;
  vector <Bag> connBags;

  void Subgraph2TreeDecomposition(int index,Graph &graph, voronota::scripting::StandardOutputMockup& som);
  void MergeBags(int depth, voronota::scripting::StandardOutputMockup& som);
  int CheckTreewidth();

};


class Solution:public PairEnergy{
public:
  ~Solution();
  IV1 unfixres;
  bool DEESearch(IV1 &pos, voronota::scripting::StandardOutputMockup& som);
  int DEEGoldstein(IV1& pos);
  int DEEsplit(IV1& pos);
  void Pick(int site,int rot);

  vector< Graph > graphs;
  void ConstructAdjMatrix(int nunfix,IV2 &adjMatrix, voronota::scripting::StandardOutputMockup& som);
  void ConstructSubgraphs(int nunfix,IV1 &visited,IV2 &adjMatrix,IV2 &flagMatrix);
  void FindSubgraphByDFS(Graph &graph,int u,IV1 &visited,IV2 &adjMatrix,IV2 &flagMatrix,stack<int> &vertices);
  void ShowGraphs(voronota::scripting::StandardOutputMockup& som);
  void GraphEdgeDecomposition(IV2 &adjMatrix,float threshold, voronota::scripting::StandardOutputMockup& som);

  TreeDecomposition tree;
  void TreeDecompositionBottomToTopCalcEnergy();
  void CalcRightBagRotamerCombinationEnergy(Bag &leafbag,int depth,float &Etmp,IV1 &Rtmp,FV1 &Ercom,IV2 &Rrcom);
  void CalcLeftBagRotamerCombinationEnergy(Bag &rootbag,int depth,float &Etmp,IV1 &Rtmp,FV1 &Elcom,IV2 &Rlcom);
  void GetLeftBagRotamerCombination(Bag &leafbag,int depth,IV1 &Rtmp,IV2 &Rlcom);
  void BagDeploySites(Bag &leafbag);
  void LeafBagCalcEnergy(Bag &leafbag,IV2 &Rlcom);
  void CombineChildIntoParentBag(Bag &leafbag,Bag &parbag,IV2 &Rclcom);
  void SubsetCheck(IV1 &subset,IV1 &fullset, IV1 &indices);
  void RootBagFindGMEC(Bag &rootbag);
  void TreeDecompositionTopToBottomAssignRotamer(Bag &parbag,Bag &childbag);
  void TreeDecompositionRelease();
  void Search(voronota::scripting::StandardOutputMockup& som);
};

#endif
