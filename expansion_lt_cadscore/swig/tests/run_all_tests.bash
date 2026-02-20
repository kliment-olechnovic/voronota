#!/bin/bash

cd $(dirname "$0")

MODE="$1"

rm -rf "./output"
mkdir -p "./output"

if [ ! -d "./testvenv" ]
then
	./setup_python_venv.bash
fi

cd ../

if [ ! -f "./cadscorelt/_cadscorelt_python.so" ] || [ "$MODE" != "skip-building" ]
then
	./compile_cadscorelt_python.bash
fi

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

