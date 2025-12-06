#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

if [ ! -d "./testvenv" ]
then
	./setup_python_venv.bash
fi

cd ../

#./compile_voronotalt_python.bash

SWIGDIR="$(pwd)"

cd - &> /dev/null

####################################################################

source ./testvenv/bin/activate

export PYTHONPATH="${PYTHONPATH}:${SWIGDIR}"

python3 -B "./input/run_voronotalt_python_simple.py" > "./output/run_voronotalt_python_simple.py.output.txt"

python3 -B "./input/run_voronotalt_python_simple_biotite.py" > "./output/run_voronotalt_python_simple_biotite.py.output.txt"

python3 -B "./input/run_voronotalt_python_simple_pandas.py" > "./output/run_voronotalt_python_simple_pandas.py.output.txt"

python3 -B "./input/run_voronotalt_python_molecular.py" > "./output/run_voronotalt_python_molecular.py.output.txt"

python3 -B "./input/run_voronotalt_python_molecular_biotite.py" > "./output/run_voronotalt_python_molecular_biotite.py.output.txt"

python3 -B "./input/run_voronotalt_python_molecular_gemmi.py" > "./output/run_voronotalt_python_molecular_gemmi.py.output.txt"

python3 -B "./input/run_voronotalt_python_molecular_biopython.py" > "./output/run_voronotalt_python_molecular_biopython.py.output.txt"

python3 -B "./input/run_voronotalt_python_molecular_pandas.py" > "./output/run_voronotalt_python_molecular_pandas.py.output.txt"

################################################################################

git status -s ./output/

