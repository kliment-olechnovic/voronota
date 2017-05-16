#!/bin/bash

SUBDIR=$OUTPUTDIR/cadscore_script
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	mkdir -p $SUBDIR/$INFILEBASENAME
	$VORONOTADIR/voronota-cadscore \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/residue_scores \
	  --smoothing-window 0 \
	  --cache-dir $SUBDIR/cache \
	  --use-all-query-codes \
	  --enable-site-based-scoring \
	| column -t \
	| tee $SUBDIR/$INFILEBASENAME/coded_global_scores \
	| grep ' AA '
done > $SUBDIR/global_scores

cat > $SUBDIR/input_model_chains_renaming << 'EOF'
A B
B A
EOF

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	mkdir -p $SUBDIR/$INFILEBASENAME
	$VORONOTADIR/voronota-cadscore \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query "--no-same-chain" \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/interface_residue_scores \
	  --output-residue-scores-pdb-t $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_target.pdb \
	  --output-residue-scores-pdb-m $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_model.pdb \
	  --smoothing-window 0 \
	  --cache-dir $SUBDIR/cache \
	  --use-all-query-codes \
	  --input-model-chains-renaming $SUBDIR/input_model_chains_renaming \
	  --remap-chains \
	  --remap-chains-output $SUBDIR/$INFILEBASENAME/interface_chains_remapping \
	  --enable-site-based-scoring \
	| column -t \
	| tee $SUBDIR/$INFILEBASENAME/interface_coded_global_scores \
	| grep ' AA '
done > $SUBDIR/interface_global_scores

rm -r $SUBDIR/cache

#####################################################

SUBDIR=$SUBDIR/old_contacts_mode
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	mkdir -p $SUBDIR/$INFILEBASENAME
	$VORONOTADIR/voronota-cadscore \
	  --old-regime \
	  -t $INPUTDIR/complex/target.pdb \
	  -m $INFILE \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/residue_scores \
	  --smoothing-window 0 \
	  --cache-dir $SUBDIR/cache \
	  --use-all-query-codes \
	  --enable-site-based-scoring \
	| column -t \
	| tee $SUBDIR/$INFILEBASENAME/coded_global_scores \
	| grep ' AA '
done > $SUBDIR/global_scores

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	mkdir -p $SUBDIR/$INFILEBASENAME
	$VORONOTADIR/voronota-cadscore \
	  --old-regime \
	  --input-target $INPUTDIR/complex/target.pdb \
	  --input-model $INFILE \
	  --contacts-query-inter-chain \
	  --output-residue-scores $SUBDIR/$INFILEBASENAME/interface_residue_scores \
	  --output-residue-scores-pdb-t $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_target.pdb \
	  --output-residue-scores-pdb-m $SUBDIR/$INFILEBASENAME/interface_residue_scores_on_model.pdb \
	  --smoothing-window 0 \
	  --cache-dir $SUBDIR/cache \
	  --use-all-query-codes \
	  --enable-site-based-scoring \
	| column -t \
	| tee $SUBDIR/$INFILEBASENAME/interface_coded_global_scores \
	| grep ' AA '
done > $SUBDIR/interface_global_scores

rm -r $SUBDIR/cache
