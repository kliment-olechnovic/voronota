#!/bin/bash

SUBDIR=$OUTPUTDIR/cadscore_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  -a $SUBDIR/$INFILEBASENAME/residue_scores_on_target.pdb \
	  -b $SUBDIR/$INFILEBASENAME/residue_scores_on_model.pdb \
	  -s 1 \
	| sed "s/^CAD-score global score =/$INFILEBASENAME/"
done > $SUBDIR/global_scores

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  -q "--no-same-chain" \
	  -a $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_target.pdb \
	  -b $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_model.pdb \
	  -s 1 \
	| sed "s/^CAD-score global score =/$INFILEBASENAME/"
done > $SUBDIR/interface_global_scores
