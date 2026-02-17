#!/bin/bash

cd $(dirname "$0")

MODE="$1"

rm -rf "./output"
mkdir -p "./output"

cd ../

if [ ! -f "./cadscore-lt" ] || [ "$MODE" != "skip-building" ]
then
	rm -f "./cadscore-lt"
	g++ -std=c++17 -Ofast -march=native -fopenmp -I "../expansion_lt/src" -o ./cadscore-lt ./src/cadscore_lt.cpp
fi

export PATH="$(pwd):${PATH}"

cd - &> /dev/null

################################################################################

./run_basic_tests.bash

./run_tutorial_part1_tests.bash

./run_tutorial_part2_tests.bash

git status -s ./output/

################################################################################

