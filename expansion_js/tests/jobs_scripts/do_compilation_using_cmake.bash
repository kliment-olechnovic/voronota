#!/bin/bash

SUBDIR=$OUTPUTDIR/compilation_using_cmake
mkdir -p $SUBDIR

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

cp -r "$CORETESTSDIR/voronota_package" "$TMPLDIR/package"

rm "$TMPLDIR/package/expansion_js/voronota-js"
mkdir $TMPLDIR/package/expansion_js/build
cd $TMPLDIR/package/expansion_js/build
cmake ../ &> /dev/null
make &> /dev/null
cd - &> /dev/null

$TMPLDIR/package/expansion_js/build/voronota-js --version > $SUBDIR/version_output

