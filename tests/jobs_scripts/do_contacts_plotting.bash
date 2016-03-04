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
  --binarize-patterns \
| $VORONOTA query-contacts --drop-adjuncts \
| $VORONOTA query-contacts --match-max-seq-sep 1 --set-adjuncts 'gv=0' \
| $VORONOTA query-contacts --match-min-seq-sep 2 --match-max-seq-sep 5 --set-adjuncts 'gv=5' \
| $VORONOTA query-contacts --match-min-seq-sep 6 --match-max-seq-sep 10 --set-adjuncts 'gv=10' \
| $VORONOTA query-contacts --match-min-seq-sep 11 --match-max-seq-sep 15 --set-adjuncts 'gv=15' \
| $VORONOTA query-contacts --match-min-seq-sep 16 --match-max-seq-sep 20 --set-adjuncts 'gv=20' \
| $VORONOTA query-contacts --match-min-seq-sep 21 --match-max-seq-sep 25 --set-adjuncts 'gv=25' \
| $VORONOTA query-contacts --match-min-seq-sep 26 --match-max-seq-sep 30 --set-adjuncts 'gv=30' \
| $VORONOTA query-contacts --match-min-seq-sep 31 --match-max-seq-sep 35 --set-adjuncts 'gv=35' \
| $VORONOTA query-contacts --match-min-seq-sep 36 --match-max-seq-sep 40 --set-adjuncts 'gv=40' \
| $VORONOTA query-contacts --match-min-seq-sep 41 --match-max-seq-sep 45 --set-adjuncts 'gv=45' \
| $VORONOTA x-plot-contacts \
  --background-color black \
  --default-color yellow \
  --adjunct-gradient gv \
  --adjunct-gradient-blue 45 \
  --adjunct-gradient-red 0 \
  --no-contraction \
  --svg-output $SUBDIR/plot_with_gradient_colors.svg \
> /dev/null
