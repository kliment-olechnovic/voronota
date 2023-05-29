#!/bin/bash

SUBDIR=$OUTPUTDIR/compilation_directly
mkdir -p $SUBDIR

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

cp -r "$CORETESTSDIR/voronota_package" "$TMPLDIR/package"

rm "$TMPLDIR/package/expansion_js/voronota-js"
cd $TMPLDIR/package/expansion_js
g++ -std=c++14 -I"./src/dependencies" -O3 -o "./voronota-js" $(find ./src/ -name '*.cpp') &> /dev/null
cd - &> /dev/null

$TMPLDIR/package/expansion_js/voronota-js --version > $SUBDIR/version_output

