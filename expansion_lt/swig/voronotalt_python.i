%module voronotalt_python

%{
#include "voronotalt.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include <std_except.i>

%template(VectorBall) std::vector<Ball>;
%template(VectorContacts) std::vector<Contact>;
%template(VectorCell) std::vector<Cell>;
%template(VectorMolecularAtomBall) std::vector<MolecularAtomBall>;
%template(VectorMolecularInterAtomContactSummary) std::vector<MolecularInterAtomContactSummary>;
%template(VectorMolecularInterResidueContactSummary) std::vector<MolecularInterResidueContactSummary>;
%template(VectorMolecularInterChainContactSummary) std::vector<MolecularInterChainContactSummary>;
%template(VectorMolecularAtomCellSummary) std::vector<MolecularAtomCellSummary>;
%template(VectorMolecularResidueCellSummary) std::vector<MolecularResidueCellSummary>;
%template(VectorMolecularChainCellSummary) std::vector<MolecularChainCellSummary>;

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

%include "voronotalt.h"
