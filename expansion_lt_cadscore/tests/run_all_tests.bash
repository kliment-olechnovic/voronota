#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

cd ../

rm -f "./cadscore-lt"

g++ -std=c++17 -Ofast -march=native -fopenmp -I "../expansion_lt/src" -o ./cadscore-lt ./src/cadscore_lt.cpp

export PATH="$(pwd):${PATH}"

cd - &> /dev/null

################################################################################

./run_basic_tests.bash

git status -s ./output/

################################################################################

