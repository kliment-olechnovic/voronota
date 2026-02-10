%module cadscorelt_python

%{
#include "cadscorelt.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include <std_except.i>

%template(VectorMolecularAtomBall) std::vector<MolecularAtomBall>;
%template(VectorGlobalScore) std::vector<GlobalScore>;
%template(VectorAtomScore) std::vector<AtomScore>;
%template(VectorResidueScore) std::vector<ResidueScore>;
%template(VectorChainScore) std::vector<ChainScore>;
%template(VectorAtomAtomScore) std::vector<AtomAtomScore>;
%template(VectorResidueResidueScore) std::vector<ResidueResidueScore>;
%template(VectorChainChainScore) std::vector<ChainChainScore>;

%exception
{
    try
    {
        $action
    }
    catch (const std::exception& e)
    {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

%include "cadscorelt.h"
