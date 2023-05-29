#!/bin/bash

cd $(dirname "$0")

export CORETESTSDIR="../../tests"

${CORETESTSDIR}/prepare_package_for_testing.bash

export VORONOTAJSDIR="${CORETESTSDIR}/voronota_package/expansion_js"
export INPUTDIR=./input
export OUTPUTDIR=./jobs_output

rm -r -f $OUTPUTDIR
mkdir $OUTPUTDIR

find ./jobs_scripts/ -type f -name "*.bash" | xargs -L 1 -P 4 bash
