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
	  --smoothing-window 0 \
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
	  --output-residue-scores-pdb-t $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_target.pdb \
	  --output-residue-scores-pdb-m $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_model.pdb \
	  --smoothing-window 0 \
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
	  --contacts-query "--match-tags MM" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_MM

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags AM" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_AM

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags AS" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_AS

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags MS" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_MS

rm -r $SUBDIR/cache

#####################################################

SUBDIR=$SUBDIR/old_contacts_mode
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --old-contacts-mode \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/residue_scores \
	  --smoothing-window 0 \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --old-contacts-mode \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--no-same-chain" \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/interface_residue_scores \
	  --output-residue-scores-pdb-t $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_target.pdb \
	  --output-residue-scores-pdb-m $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_model.pdb \
	  --smoothing-window 0 \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/interface_global_scores

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --old-contacts-mode \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags SS" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_SS

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --old-contacts-mode \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags MM" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_MM

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --old-contacts-mode \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags AM" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_AM

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --old-contacts-mode \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags AS" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_AS

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	$VORONOTADIR/voronota-cadscore \
	  --old-contacts-mode \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--match-tags MS" \
	  --cache-dir $SUBDIR/cache
done > $SUBDIR/global_scores_MS

rm -r $SUBDIR/cache
