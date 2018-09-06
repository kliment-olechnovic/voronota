#!/bin/bash

cd $(dirname "$0")

if [ ! -d "./voronota_package" ]
then
	../package.bash voronota_package
	mv ../voronota_package.tar.gz ./voronota_package.tar.gz
	tar -xf ./voronota_package.tar.gz
	rm ./voronota_package.tar.gz
else
	cp ../voronota ../voronota-* ./voronota_package/
fi

export VORONOTA=./voronota_package/voronota
export VORONOTADIR=./voronota_package/
export INPUTDIR=./input
export OUTPUTDIR=./jobs_output

mkdir -p $VORONOTADIR/resources
$VORONOTADIR/voronota-resources voromqa_v1_energy_means_and_sds > $VORONOTADIR/resources/voromqa_v1_energy_means_and_sds
$VORONOTADIR/voronota-resources voromqa_v1_energy_potential > $VORONOTADIR/resources/voromqa_v1_energy_potential
$VORONOTADIR/voronota-resources radii > $VORONOTADIR/resources/radii

for JOBSCRIPT in "$@"
do
	bash "./jobs_scripts/do_${JOBSCRIPT}.bash"
done
