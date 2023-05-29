#!/bin/bash

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

SUBDIR=$OUTPUTDIR/compilation_directly
mkdir -p $SUBDIR

cp -r $VORONOTADIR $TMPDIR/package
rm $TMPDIR/package/voronota
cd $TMPDIR/package
g++ -O3 -o voronota $(find ./src/ -name '*.cpp') &> /dev/null
cd - &> /dev/null

$TMPDIR/package/voronota --help > $SUBDIR/full_help
