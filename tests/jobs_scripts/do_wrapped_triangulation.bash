#!/bin/bash

SUBDIR=$OUTPUTDIR/wrapped_triangulation

mkdir -p $SUBDIR

$VORONOTA x-wrapped-get-balls-from-atoms-file-and-calculate-vertices \
  --input-file $INPUTDIR/single/structure.pdb \
  > $SUBDIR/balls_and_vertices

$VORONOTA x-wrapped-get-balls-from-atoms-file-and-calculate-vertices \
  --input-file <(cat $INPUTDIR/single/structure.pdb | egrep 'GLU A   5') \
  --sketch-file $SUBDIR/part_sketch.py \
  > $SUBDIR/part_balls_and_vertices
