#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/voromqa_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voromqa $INFILE $SUBDIR/$INFILEBASENAME | sed "s/^VoroMQA global score =/$INFILEBASENAME/"
done > $SUBDIR/global_scores
