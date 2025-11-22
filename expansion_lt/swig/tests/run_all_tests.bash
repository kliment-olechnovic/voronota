#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

if [ ! -d "./testvenv" ]
then
	./setup_python_venv.bash
fi

cd ../

./compile_voronotalt_python.bash

SWIGDIR="$(pwd)"

cd - &> /dev/null

####################################################################

source ./testvenv/bin/activate

export PYTHONPATH="${PYTHONPATH}:${SWIGDIR}"

python3 -B "./input/run_voronotalt_python.py" > "./output/run_voronotalt_python.py.output.txt"

python3 -B "./input/run_voronotalt_python_molecular.py" > "./output/run_voronotalt_python_molecular.py.output.txt"

python3 -B "./input/run_voronotalt_python_molecular_biotite.py" > "./output/run_voronotalt_python_molecular_biotite.py.output.txt"

################################################################################

git status -s ./output/

