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
	  -z $SUBDIR/$INFILEBASENAME/colored_map.svg \
	  -s 5 \
	  -m '--no-same-chain --no-solvent'

	$VORONOTADIR/voronota-bfactor \
	  -p $INFILE \
	  -s $SUBDIR/$INFILEBASENAME/atom_scores \
	> $SUBDIR/$INFILEBASENAME/atom_scores.pdb

	$VORONOTADIR/voronota-bfactor \
	  -p $INFILE \
	  -s $SUBDIR/$INFILEBASENAME/residue_scores \
	> $SUBDIR/$INFILEBASENAME/residue_scores.pdb

done > $SUBDIR/global_scores
