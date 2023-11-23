#!/bin/bash

cd "$(dirname $0)"

rm -rf "./voronotalt_python_wrap.cxx" "./_voronotalt_python.so" "./voronotalt_python.py" "./__pycache__"

swig -python -c++ voronotalt_python.i

g++ -fPIC -shared -Ofast -march=native -fopenmp voronotalt_python_wrap.cxx -o _voronotalt_python.so -I/usr/include/python3.11

python3 -B "./test_run_voronotalt_python.py"
