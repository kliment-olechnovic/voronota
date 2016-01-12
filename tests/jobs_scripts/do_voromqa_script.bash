#!/bin/bash

SUBDIR=$OUTPUTDIR/voromqa_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-voromqa \
	  -i $INFILE \
	  -a $SUBDIR/$INFILEBASENAME/atom_scores \
	  -r $SUBDIR/$INFILEBASENAME/residue_scores \
	  -c $SUBDIR/$INFILEBASENAME/cameo_residue_scores.pdb \
	  -s 5
done > $SUBDIR/global_scores
