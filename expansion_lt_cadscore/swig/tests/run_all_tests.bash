#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

if [ ! -d "./testvenv" ]
then
	./setup_python_venv.bash
fi

cd ../

./compile_cadscorelt_python.bash

SWIGDIR="$(pwd)"

cd - &> /dev/null

####################################################################

source ./testvenv/bin/activate

export PYTHONPATH="${PYTHONPATH}:${SWIGDIR}"

find ./input/ -type f -name '*.py' \
| while read -r SCRIPTFILE
do
	SCRIPTNAME="$(basename ${SCRIPTFILE})"
	
	python3 -B "${SCRIPTFILE}" > "./output/${SCRIPTNAME}.output.txt"
done

################################################################################

git status -s ./output/

