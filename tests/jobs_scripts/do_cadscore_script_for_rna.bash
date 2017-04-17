#!/bin/bash

SUBDIR=$OUTPUTDIR/cadscore_script_for_rna
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/rna/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	mkdir -p $SUBDIR/$INFILEBASENAME
	$VORONOTADIR/voronota-cadscore \
	  -t $INPUTDIR/rna/target.pdb \
	  -m $INFILE \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/residue_scores \
	  --smoothing-window 0 \
	  --use-all-query-codes \
	| column -t \
	| tee $SUBDIR/$INFILEBASENAME/coded_global_scores \
	| grep ' AA '
done > $SUBDIR/global_scores

#####################################################

SUBDIR=$SUBDIR/old_contacts_mode
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/rna/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	mkdir -p $SUBDIR/$INFILEBASENAME
	$VORONOTADIR/voronota-cadscore \
	  --old-regime \
	  -t $INPUTDIR/rna/target.pdb \
	  -m $INFILE \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/residue_scores \
	  --smoothing-window 0 \
	  --use-all-query-codes \
	| column -t \
	| tee $SUBDIR/$INFILEBASENAME/coded_global_scores \
	| grep ' AA '
done > $SUBDIR/global_scores
