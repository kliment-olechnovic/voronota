#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/cadscore_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/cadscore.bash \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  -T $SUBDIR/$INFILEBASENAME/residue_scores_on_target.pdb \
	  -M $SUBDIR/$INFILEBASENAME/residue_scores_on_model.pdb \
	  -s 1 \
	| sed "s/^CAD-score global score =/$INFILEBASENAME/"
done > $SUBDIR/global_scores
