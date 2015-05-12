#!/bin/bash

set +e

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

SUBDIR=$OUTPUTDIR/compilation_using_cmake
mkdir -p $SUBDIR

cp -r $VORONOTADIR $TMPDIR/package
mkdir $TMPDIR/package/build
cd $TMPDIR/package/build
cmake ../ &> /dev/null
make &> /dev/null
cd - &> /dev/null

$TMPDIR/package/build/voronota --help &> $SUBDIR/full_help
