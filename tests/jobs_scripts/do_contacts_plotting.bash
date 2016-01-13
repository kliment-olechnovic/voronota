#!/bin/bash

SUBDIR=$OUTPUTDIR/contacts_plotting
mkdir -p $SUBDIR

cat $INPUTDIR/complex/target.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --annotated \
| $VORONOTA calculate-contacts --annotated \
| $VORONOTA query-contacts --inter-residue --no-solvent \
| $VORONOTA query-contacts --match-min-seq-sep 0 --match-max-seq-sep 1 --set-adjuncts 'r=1;g=1;b=0' \
| $VORONOTA query-contacts --match-min-seq-sep 2 --match-max-seq-sep 10 --set-adjuncts 'r=0;g=1;b=1' \
| $VORONOTA query-contacts --match-min-seq-sep 11 --set-adjuncts 'r=1;g=0;b=0' \
| $VORONOTA query-contacts --no-same-chain --set-adjuncts 'r=1;g=0;b=1' \
| $VORONOTA x-plot-contacts \
  --background-color black \
  --default-color white \
  --adjuncts-rgb \
  --no-contraction \
  --svg-output $SUBDIR/plot.svg \
  --axis-output $SUBDIR/plot_axis \
  --points-output $SUBDIR/plot_points \
  --patterns-output $SUBDIR/plot_patterns \
  --binarize-patterns
