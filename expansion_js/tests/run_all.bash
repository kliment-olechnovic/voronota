#!/bin/bash

cd $(dirname "$0")

rm -rf ./voronota-js_package

../package.bash voronota-js_package
mv ../voronota-js_package.tar.gz ./voronota-js_package.tar.gz
tar -xf ./voronota-js_package.tar.gz
rm ./voronota-js_package.tar.gz

export VORONOTAJSDIR=./voronota-js_package/
export INPUTDIR=./input
export OUTPUTDIR=./jobs_output

rm -r -f $OUTPUTDIR
mkdir $OUTPUTDIR

find ./jobs_scripts/ -type f -name "*.bash" | xargs -L 1 -P 4 bash

rm -rf ./voronota-js_package
