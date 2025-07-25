#!/bin/bash

cd "$(dirname $0)"

# building with OpenMP and with CPU architecture matching
# g++ -std=c++14 -Ofast -march=native -fopenmp -o ./sihsolvexpand ./sihsolvexpand.cpp -I ../../src/

# building without OpenMP in a more portable way
g++ -std=c++14 -O3 -o ./sihsolvexpand ./sihsolvexpand.cpp -I ../../src/
