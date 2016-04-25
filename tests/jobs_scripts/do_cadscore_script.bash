#!/bin/bash

SUBDIR=$OUTPUTDIR/cadscore_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/residue_scores \
	  --smoothing-window 1 \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--no-same-chain" \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/interface_residue_scores \
	  --smoothing-window 1 \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/interface_global_scores

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags SS" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_SS

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags-not MM" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_AS

rm -r $SUBDIR/cache
