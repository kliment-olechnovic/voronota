#!/bin/bash

set +e

cd $(dirname "$0")

../package.bash voronota_package
mv ../voronota_package.tar.gz ./voronota_package.tar.gz
tar -xf ./voronota_package.tar.gz
rm ./voronota_package.tar.gz

trap "rm -r ./voronota_package" EXIT

export VORONOTA=./voronota_package/voronota
export VORONOTADIR=./voronota_package/
export INPUTDIR=./input
export OUTPUTDIR=./output

rm -r -f $OUTPUTDIR
mkdir $OUTPUTDIR

find ./scripts/ -type f -name "*.bash" | xargs -L 1 -P 4 bash
