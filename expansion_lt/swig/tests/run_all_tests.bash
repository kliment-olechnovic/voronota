#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

cd ../

if [ ! -s "./_voronotalt_python.so" ] || [ ! -s "./voronotalt_python.py" ]
then
	./compile_voronotalt_python.bash
fi

export PYTHONPATH="${PYTHONPATH}:$(pwd)"

cd - &> /dev/null

####################################################################

python3 -B "./input/run_voronotalt_python.py" > "./output/run_voronotalt_python.py.output.txt"

################################################################################

git status -s ./output/

