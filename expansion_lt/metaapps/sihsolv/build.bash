#!/bin/bash

cd "$(dirname $0)"

g++ -std=c++14 -Ofast -march=native -fopenmp -o ./sihsolv ./sihsolv.cpp -I ../../src/
