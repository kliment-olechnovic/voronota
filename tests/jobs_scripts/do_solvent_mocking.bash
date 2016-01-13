#!/bin/bash

SUBDIR=$OUTPUTDIR/solvent_mocking
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --annotated \
| $VORONOTA x-calculate-mock-solvent --solvent-radius 1.4 --solvent-distance 0.5 --sih-depth 1 \
| $VORONOTA x-write-balls-to-atoms-file --pdb-output $SUBDIR/balls_with_mock_solvent.pdb \
> $SUBDIR/balls_with_mock_solvent
