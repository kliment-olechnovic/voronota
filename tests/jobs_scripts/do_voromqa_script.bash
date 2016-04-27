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
	  --output-selected-scores $SUBDIR/$INFILEBASENAME/selected_atom_scores \
	  --output-balls $SUBDIR/$INFILEBASENAME/balls

done > $SUBDIR/global_scores

cat $INPUTDIR/complex/target.pdb \
| $VORONOTA get-balls-from-atoms-file --annotated \
| $VORONOTA query-balls --seq-output $SUBDIR/reference_sequence \
> /dev/null

fold -w 50 $SUBDIR/reference_sequence > $SUBDIR/reference_sequence_wrapped
mv $SUBDIR/reference_sequence_wrapped $SUBDIR/reference_sequence

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)

	$VORONOTADIR/voronota-voromqa \
	  --input $INFILE \
	  --reference-sequence $SUBDIR/reference_sequence \
	  --rebuild-sidechains Scwrl4 \
	  --output-sequence-alignment $SUBDIR/$INFILEBASENAME/sequence_alignment \
	  --output-scores-for-CASP $SUBDIR/$INFILEBASENAME/casp_qa_scores
	
	cat $SUBDIR/$INFILEBASENAME/casp_qa_scores | tr '\t' '\n' > $SUBDIR/$INFILEBASENAME/casp_qa_scores_wrapped
	mv $SUBDIR/$INFILEBASENAME/casp_qa_scores_wrapped $SUBDIR/$INFILEBASENAME/casp_qa_scores

done > $SUBDIR/global_scores_using_reference_sequence
