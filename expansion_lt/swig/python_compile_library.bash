#!/bin/bash

cd "$(dirname $0)"

rm -rf "./voronotalt_wrap.cxx" "./_voronotalt.so" "./voronotalt.py" "./__pycache__"

swig -python -c++ voronotalt.i

g++ -fPIC -shared -Ofast -march=native -fopenmp voronotalt_wrap.cxx -o _voronotalt.so -I/usr/include/python3.11

python3 "./python_run_usage_example.py"
