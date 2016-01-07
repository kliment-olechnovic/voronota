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

VORONOTA=$VORONOTADIR/voronota
for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	cat $INFILE \
	| $VORONOTA get-balls-from-atoms-file --annotated \
	| $VORONOTA calculate-contacts --annotated \
	| $VORONOTA query-contacts --no-same-chain --no-solvent \
	> $SUBDIR/$INFILEBASENAME/interface_contacts
done

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  -T $SUBDIR/target/interface_contacts \
	  -M $SUBDIR/$INFILEBASENAME/interface_contacts \
	  -a $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_target.pdb \
	  -b $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_model.pdb \
	  -s 1
done > /dev/null

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  -T $SUBDIR/target/interface_contacts \
	  -M $SUBDIR/$INFILEBASENAME/interface_contacts \
	| sed "s/^CAD-score global score =/$INFILEBASENAME/"
done > $SUBDIR/interface_global_scores
