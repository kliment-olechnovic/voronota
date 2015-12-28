#!/bin/bash

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

SUBDIR=$OUTPUTDIR/memory_check
mkdir -p $SUBDIR

g++ -g -O1 -o $TMPDIR/voronota_raw $VORONOTADIR/src/*.cpp

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --include-heteroatoms \
> $TMPDIR/balls

valgrind --tool=memcheck $TMPDIR/voronota_raw calculate-vertices < $TMPDIR/balls > $SUBDIR/triangulation 2> $TMPDIR/log

(cat $TMPDIR/log | grep -A 3 "LEAK SUMMARY" ; cat $TMPDIR/log | grep "ERROR SUMMARY") \
| sed 's/^==\S\+== //' \
> $SUBDIR/valgrind_memcheck_log
