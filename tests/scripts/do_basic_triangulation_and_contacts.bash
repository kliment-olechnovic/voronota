#!/bin/bash

set +e

DIR=$OUTPUTDIR/basic_triangulation_and_contacts

SUBDIR=$DIR/default_radii
mkdir -p $SUBDIR
cat $INPUTDIR/single/structure.pdb | $VORONOTA get-balls-from-atoms-file > $SUBDIR/balls
cat $SUBDIR/balls | $VORONOTA calculate-vertices --print-log --check > $SUBDIR/triangulation 2> $SUBDIR/triangulation_log
cat $SUBDIR/balls | $VORONOTA calculate-vertices-in-parallel --print-log --method simulated --parts 8 > $SUBDIR/triangulation_simulated_parallel 2> $SUBDIR/triangulation_simulated_parallel_log
cat $SUBDIR/balls | $VORONOTA calculate-contacts > $SUBDIR/contacts

SUBDIR=$DIR/custom_radii
mkdir -p $SUBDIR
cat $INPUTDIR/single/structure.pdb | $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTA_RADII_FILE > $SUBDIR/balls
cat $SUBDIR/balls | $VORONOTA calculate-vertices --print-log --check > $SUBDIR/triangulation 2> $SUBDIR/triangulation_log
cat $SUBDIR/balls | $VORONOTA calculate-vertices-in-parallel --print-log --method simulated --parts 8 > $SUBDIR/triangulation_simulated_parallel 2> $SUBDIR/triangulation_simulated_parallel_log
cat $SUBDIR/balls | $VORONOTA calculate-contacts > $SUBDIR/contacts

SUBDIR=$DIR/custom_radii_with_heteroatoms
mkdir -p $SUBDIR
cat $INPUTDIR/single/structure.pdb | $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTA_RADII_FILE --include-heteroatoms > $SUBDIR/balls
cat $SUBDIR/balls | $VORONOTA calculate-vertices --print-log --check > $SUBDIR/triangulation 2> $SUBDIR/triangulation_log
cat $SUBDIR/balls | $VORONOTA calculate-vertices-in-parallel --print-log --method simulated --parts 8 > $SUBDIR/triangulation_simulated_parallel 2> $SUBDIR/triangulation_simulated_parallel_log
cat $SUBDIR/balls | $VORONOTA calculate-contacts > $SUBDIR/contacts

SUBDIR=$DIR/mmcif_custom_radii_with_heteroatoms
mkdir -p $SUBDIR
cat $INPUTDIR/single/structure.cif | $VORONOTA get-balls-from-atoms-file --mmcif --radii-file $VORONOTA_RADII_FILE --include-heteroatoms > $SUBDIR/balls
cat $SUBDIR/balls | $VORONOTA calculate-vertices --print-log --check > $SUBDIR/triangulation 2> $SUBDIR/triangulation_log
cat $SUBDIR/balls | $VORONOTA calculate-vertices-in-parallel --print-log --method simulated --parts 8 > $SUBDIR/triangulation_simulated_parallel 2> $SUBDIR/triangulation_simulated_parallel_log
cat $SUBDIR/balls | $VORONOTA calculate-contacts > $SUBDIR/contacts
