#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/contacts_scoring
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	cat $INFILE \
	| $VORONOTA get-balls-from-atoms-file \
	  --radii-file $VORONOTADIR/radii \
	  --include-heteroatoms \
	  --annotated \
	  --multimodel-chains \
	| $VORONOTA query-balls \
	  --rename-chains \
	  --renumber-positively \
	  --reset-serials \
	| $VORONOTA query-balls \
	  --drop-atom-serials \
	  --drop-altloc-indicators \
	| $VORONOTA query-balls \
	  --chains-summary-output $SUBDIR/$INFILEBASENAME.chaincount \
	| $VORONOTA calculate-contacts \
	  --annotated \
	| $VORONOTA query-contacts --match-min-seq-sep 1 \
	| $VORONOTA query-contacts \
	  --match-first 'A<C>' \
	  --match-second 'A<N>' \
	  --match-max-seq-sep 1 \
	  --match-max-dist 1.6 \
	  --invert \
	| $VORONOTA query-contacts \
	  --match-min-seq-sep 1 \
	  --match-max-seq-sep 1 \
	  --set-tags 'sep1' \
	| $VORONOTA query-contacts \
	  --match-min-seq-sep 2 \
	  --no-solvent \
	  --set-tags 'sep2' \
	| awk '{print $1 " " $2 " " $5 " " $3}' \
	| tee $SUBDIR/$INFILEBASENAME.contacts \
	| $VORONOTA score-contacts-potential \
	  --multiply-areas 0.5 \
	> $SUBDIR/$INFILEBASENAME.summary
done

find $SUBDIR/ -type f -name "*.summary" \
|$VORONOTA score-contacts-potential \
  --input-file-list \
  --contributions-file $SUBDIR/contributions \
  --potential-file $SUBDIR/potential \
  --probabilities-file $SUBDIR/probabilities \
  --single-areas-file $SUBDIR/singleareas \
  --toggling-list 'hb;ds' \
> $SUBDIR/summary

for INFILE in $SUBDIR/*.contacts
do
	INFILEBASENAME=$(basename $INFILE .contacts)
	cat $INFILE \
	| $VORONOTA score-contacts-energy \
	  --potential-file $SUBDIR/potential \
	  --ignorable-max-seq-sep 1 \
	  --depth 2 \
	  --inter-atom-scores-file $SUBDIR/$INFILEBASENAME.interatomenergies \
	  --atom-scores-file $SUBDIR/$INFILEBASENAME.atomenergies \
	> $SUBDIR/$INFILEBASENAME.globalenergy
	
	cat $SUBDIR/$INFILEBASENAME.atomenergies \
	| $VORONOTA score-contacts-quality \
	  --default-mean -0.5 \
	  --default-sd 0.5 \
	  --mean-shift 0.0 \
	  --smoothing-window 5 \
	  --atom-scores-file $SUBDIR/$INFILEBASENAME.atomqscores \
	  --residue-scores-file $SUBDIR/$INFILEBASENAME.residueqscores \
	> $SUBDIR/$INFILEBASENAME.globalqscore
done

for INFILE in $SUBDIR/model*.contacts
do
	INFILEBASENAME=$(basename $INFILE .contacts)
	cat $INFILE \
	| awk '{print $1 " " $2 " " $4}' \
	| $VORONOTA compare-contacts \
	  --target-contacts-file <(cat $SUBDIR/target.contacts | awk '{print $1 " " $2 " " $4}') \
	  --atom-scores-file $SUBDIR/$INFILEBASENAME.atomcadscores \
	  --residue-scores-file $SUBDIR/$INFILEBASENAME.residuecadscores \
	  --depth 0 \
	> $SUBDIR/$INFILEBASENAME.globalcadscore
done

$VORONOTA score-scores \
  --reference-threshold 0.5 \
  --testable-step 0.02 \
  --outcomes-file $SUBDIR/localclassification \
  --ROC-curve-file $SUBDIR/roccurve \
  --PR-curve-file $SUBDIR/prcurve \
> $SUBDIR/scorescoringsummary << EOF
$SUBDIR/model1.residuecadscores $SUBDIR/model1.residueqscores
$SUBDIR/model2.residuecadscores $SUBDIR/model2.residueqscores
