#!/bin/bash

cd $(dirname "$0")

export CORETESTSDIR="../../tests"

export VORONOTAJSDIR="${CORETESTSDIR}/voronota_package/expansion_js"
export INPUTDIR=./input
export OUTPUTDIR=./jobs_output

if [ ! -d "$VORONOTAJSDIR" ]
then
	echo "No package saved to run partial testing, rerun the full testing to prepare for partial testing."
	exit 1
else
	cp ../voronota-js ../voronota-js-* "${VORONOTAJSDIR}/"
fi

mkdir -p $OUTPUTDIR

for JOBSCRIPT in "$@"
do
	bash "./jobs_scripts/do_${JOBSCRIPT}.bash"
done
