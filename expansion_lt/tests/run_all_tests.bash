#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

cd ../

rm -f "./voronota-lt"

g++ -std=c++14 -Ofast -march=native -fopenmp -o ./voronota-lt ./src/voronota_lt.cpp

export PATH="$(pwd):${PATH}"

cd - &> /dev/null

################################################################################

./run_basic_tests.bash

./run_tutorial_tests.bash

git status -s ./output/

################################################################################

