#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/voromqa_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-voromqa \
	  -i $INFILE \
	  -a $SUBDIR/$INFILEBASENAME/atom_scores.pdb \
	  -r $SUBDIR/$INFILEBASENAME/residue_scores.pdb \
	  -c $SUBDIR/$INFILEBASENAME/cameo_residue_scores.pdb \
	  -s 5 \
	| sed "s/^VoroMQA global score =/$INFILEBASENAME/"
done > $SUBDIR/global_scores
