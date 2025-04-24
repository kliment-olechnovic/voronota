#!/bin/bash

cd "$(dirname $0)"

rm -rf \
  "./voronotalt_python_wrap.cxx" \
  "./_voronotalt_python.so" "./voronotalt_python.py" \
  "./voronotalt/_voronotalt_python.so" "./voronotalt/voronotalt_python.py" \
  "./__pycache__" \

swig -python -c++ voronotalt_python.i

g++ -std=c++11 -fPIC -shared -Ofast -march=native -fopenmp -I ../src/ voronotalt_python_wrap.cxx -o _voronotalt_python.so $(python3-config --includes)

mkdir -p "./voronotalt"
mv "./voronotalt_python.py" "./voronotalt/voronotalt_python.py"
mv "./_voronotalt_python.so" "./voronotalt/_voronotalt_python.so"
