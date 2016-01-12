#!/bin/bash

SUBDIR=$OUTPUTDIR/cadscore_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  -r $SUBDIR/$INFILEBASENAME/residue_scores \
	  -s 1 \
	  -C $SUBDIR/cache
done > $SUBDIR/global_scores

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  -c "--no-same-chain" \
	  -r $SUBDIR/$INFILEBASENAME/interface_residue_scores \
	  -s 1 \
	  -C $SUBDIR/cache
done > $SUBDIR/interface_global_scores

rm -r $SUBDIR/cache
