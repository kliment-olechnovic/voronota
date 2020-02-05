#!/bin/bash

cd "$(dirname "$0")"

rm -rf src module

cp -r ../../src ./
cp ../voronota_scripting.i ../voronota_scripting.h ../voronota_scripting.cpp ./src

cd ./src

rm ./voronota.cpp
rm -r ./modes

swig -c++ -python -py3 voronota_scripting.i 

g++ -O3 -fPIC -shared \
  -pedantic -pedantic-errors -Wall -Werror -Wextra \
  voronota_scripting.cpp \
  voronota_scripting_wrap.cxx -I/usr/include/python3.6m/ \
  -o _voronota_scripting.so

cd ..

mkdir -p module

cp src/*.so src/*.py ./module
cp test.py launch_pymol.py ./module
cp ../../tests/input/single/structure.pdb ./module
