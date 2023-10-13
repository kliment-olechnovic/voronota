#!/bin/bash

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

SUBDIR=$OUTPUTDIR/parallel_vertices_calculations_using_openmp
mkdir -p $SUBDIR

g++ -O3 -std=c++11 -fopenmp -o $TMPDIR/voronota_openmp $VORONOTADIR/src/voronota.cpp $VORONOTADIR/src/modes/*.cpp &> /dev/null

cat $INPUTDIR/single/structure.pdb \
| $TMPDIR/voronota_openmp get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --include-heteroatoms \
> $TMPDIR/balls

cat $TMPDIR/balls | $TMPDIR/voronota_openmp calculate-vertices-in-parallel --method openmp --parts 4 --print-log > /dev/null 2> $SUBDIR/openmp_4p_log
cat $TMPDIR/balls | $TMPDIR/voronota_openmp calculate-vertices-in-parallel --method openmp --parts 8 --print-log > /dev/null 2> $SUBDIR/openmp_8p_log
cat $TMPDIR/balls | $TMPDIR/voronota_openmp calculate-vertices-in-parallel --method openmp --parts 16 --print-log > /dev/null 2> $SUBDIR/openmp_16p_log
cat $TMPDIR/balls | $TMPDIR/voronota_openmp calculate-vertices-in-parallel --method openmp --parts 32 --print-log > /dev/null 2> $SUBDIR/openmp_32p_log
