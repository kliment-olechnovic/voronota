#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/cadscore_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/cadscore \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  -r $SUBDIR/$INFILEBASENAME/residue_scores_on_model.pdb \
	| sed "s/^CAD-score global score =/$INFILEBASENAME/"
done > $SUBDIR/global_scores
