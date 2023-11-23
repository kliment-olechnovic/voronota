%module voronotalt

%{
#include "voronotalt.h"
%}

%include <std_vector.i>

%template(VectorBall) std::vector<Ball>;
%template(VectorContacts) std::vector<Contact>;
%template(VectorCell) std::vector<Cell>;

%include "voronotalt.h"
