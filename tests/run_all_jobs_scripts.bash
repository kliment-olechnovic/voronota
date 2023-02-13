#!/bin/bash

cd $(dirname "$0")

./prepare_package_for_testing.bash

export VORONOTA=./voronota_package/voronota
export VORONOTADIR=./voronota_package/
export INPUTDIR=./input
export OUTPUTDIR=./jobs_output

mkdir $VORONOTADIR/resources
$VORONOTADIR/voronota-resources voromqa_v1_energy_means_and_sds > $VORONOTADIR/resources/voromqa_v1_energy_means_and_sds
$VORONOTADIR/voronota-resources voromqa_v1_energy_potential > $VORONOTADIR/resources/voromqa_v1_energy_potential
$VORONOTADIR/voronota-resources radii > $VORONOTADIR/resources/radii

rm -r -f $OUTPUTDIR
mkdir $OUTPUTDIR

find ./jobs_scripts/ -type f -name "*.bash" | xargs -L 1 -P 4 bash
