#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

cd ../

./compile_voronotalt_python.bash

export PYTHONPATH="${PYTHONPATH}:$(pwd)"

cd - &> /dev/null

####################################################################

python3 -B "./input/run_voronotalt_python.py" > "./output/run_voronotalt_python.py.output.txt"

################################################################################

git status -s ./output/

