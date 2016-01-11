#!/bin/bash

SUBDIR=$OUTPUTDIR/wrapped_triangulation

mkdir -p $SUBDIR

$VORONOTA x-wrapped-get-balls-from-atoms-file-and-calculate-vertices --input-file $INPUTDIR/single/structure.pdb > $SUBDIR/balls_and_vertices
