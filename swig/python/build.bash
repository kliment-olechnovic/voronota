#!/bin/bash

cd "$(dirname "$0")"

rm -rf src module

cp -r ../../src ./
cp ../voronota_scripting.i ./src

cd ./src

rm main.cpp mode_*.cpp

swig -c++ -python voronota_scripting.i 

g++ -fpic -c voronota_scripting.cpp -O3

g++ -fpic -c voronota_scripting_wrap.cxx -I/usr/include/python2.7/ -O3

gcc -shared voronota_scripting.o voronota_scripting_wrap.o -o _voronota_scripting.so -lstdc++

cd ..

mkdir -p module

cp src/*.so src/*.py ./module
