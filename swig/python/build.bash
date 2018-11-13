#!/bin/bash

cd "$(dirname "$0")"

rm -rf src module

cp -r ../../src ./
cp ../voronota_scripting.i ./src

cd ./src

rm main.cpp mode_*.cpp

swig -c++ -python voronota_scripting.i 

g++ -O3 -fPIC -shared -static-libgcc -static-libstdc++ \
  voronota_scripting.cpp \
  voronota_scripting_wrap.cxx -I/usr/include/python2.7/ \
  -o _voronota_scripting.so

cd ..

mkdir -p module

cp src/*.so src/*.py ./module
cp test.py ./module
