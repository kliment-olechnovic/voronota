#!/bin/bash

cd "$(dirname $0)"

rm -rf \
  "./cadscorelt_python_wrap.cxx" \
  "./_cadscorelt_python.so" "./cadscorelt_python.py" \
  "./cadscorelt/_cadscorelt_python.so" "./cadscorelt/cadscorelt_python.py" \
  "./__pycache__" \

swig -python -c++ cadscorelt_python.i

g++ -std=c++17 -fPIC -shared -Ofast -march=native -fopenmp -I ../src/ -I ../../expansion_lt/src/ cadscorelt_python_wrap.cxx -o _cadscorelt_python.so $(python3-config --includes)

mkdir -p "./cadscorelt"
mv "./cadscorelt_python.py" "./cadscorelt/cadscorelt_python.py"
mv "./_cadscorelt_python.so" "./cadscorelt/_cadscorelt_python.so"
