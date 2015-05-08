#!/bin/bash

set +e

cd $(dirname "$0")

../package.bash voronota_package
mv ../voronota_package.tar.gz ./voronota_package.tar.gz
tar -xf ./voronota_package.tar.gz
rm ./voronota_package.tar.gz

trap "rm -r ./voronota_package" EXIT

export VORONOTA=./voronota_package/voronota
export VORONOTA_RADII_FILE=./voronota_package/radii
export INPUTDIR=./input
export OUTPUTDIR=./output

rm -r -f $OUTPUTDIR
mkdir $OUTPUTDIR

for SCRIPT in ./scripts/*
do
	$SCRIPT
done
