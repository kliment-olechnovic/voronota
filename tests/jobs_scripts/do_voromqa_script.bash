#!/bin/bash

SUBDIR=$OUTPUTDIR/voromqa_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)

	$VORONOTADIR/voronota-voromqa \
	  --input $INFILE \
	  --output-atom-scores $SUBDIR/$INFILEBASENAME/atom_scores \
	  --output-atom-scores-pdb $SUBDIR/$INFILEBASENAME/atom_scores.pdb \
	  --smoothing-window 5 \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/residue_scores \
	  --output-residue-scores-pdb $SUBDIR/$INFILEBASENAME/residue_scores.pdb \
	  --output-residue-scores-for-CAMEO $SUBDIR/$INFILEBASENAME/cameo_residue_scores.pdb \
	  --output-contacts-map-svg $SUBDIR/$INFILEBASENAME/colored_map.svg \
	  --contacts-query '--no-same-chain --no-solvent' \
	  --output-selected-scores $SUBDIR/$INFILEBASENAME/selected_atom_scores

done > $SUBDIR/global_scores
