%module voronotalt_python

%{
#include "voronotalt.h"
%}

%include <std_vector.i>
%include <std_except.i>

%template(VectorBall) std::vector<Ball>;
%template(VectorContacts) std::vector<Contact>;
%template(VectorCell) std::vector<Cell>;

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
