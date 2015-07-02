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
	  --drop-adjuncts \
	  --drop-tags \
	  --drop-atom-serials \
	  --drop-altloc-indicators \
	  --chains-summary-output $SUBDIR/$INFILEBASENAME.chaincount \
	  --chains-seq-identity 0.9 \
	| tee $SUBDIR/$INFILEBASENAME.balls \
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
| $VORONOTA score-contacts-potential \
  --input-file-list \
  --contributions-file $SUBDIR/contributions \
  --potential-file $SUBDIR/potential \
  --probabilities-file $SUBDIR/probabilities \
  --single-areas-file $SUBDIR/singleareas \
  --toggling-list 'hb;ds' \
> $SUBDIR/summary

cat $SUBDIR/summary \
| $VORONOTA score-contacts-potential \
  --input-contributions $SUBDIR/contributions \
  --input-fixed-types <(cat $SUBDIR/potential | awk '{print $1 " " $2 " " $3}') \
  --potential-file $SUBDIR/potentialfromsummary \
> /dev/null

echo $SUBDIR/*.balls \
| $VORONOTA query-balls-sequences-pairings-stats \
> $SUBDIR/sequences_pairings_stats

cat $SUBDIR/summary \
| $VORONOTA score-contacts-potential \
  --input-contributions $SUBDIR/contributions \
  --input-seq-pairs-stats $SUBDIR/sequences_pairings_stats \
  --potential-file $SUBDIR/potentialadjustedtoseqpairingsstat \
> /dev/null

echo "$SUBDIR/potential $SUBDIR/potentialfromsummary" \
| $VORONOTA score-contacts-potentials-stats \
> $SUBDIR/potentials_stats

for INFILE in $SUBDIR/*.contacts
do
	INFILEBASENAME=$(basename $INFILE .contacts)
	cat $INFILE \
	| $VORONOTA score-contacts-energy \
	  --potential-file $VORONOTADIR/energy_potential \
	  --ignorable-max-seq-sep 1 \
	  --depth 2 \
	  --inter-atom-scores-file $SUBDIR/$INFILEBASENAME.interatomenergies \
	  --atom-scores-file $SUBDIR/$INFILEBASENAME.atomenergies \
	> $SUBDIR/$INFILEBASENAME.globalenergy
	
	cat $INFILE \
	| $VORONOTA query-contacts-depth-values \
	> $SUBDIR/$INFILEBASENAME.depthvalues
	
	cat $SUBDIR/$INFILEBASENAME.atomenergies \
	| $VORONOTA score-contacts-quality \
	  --default-mean -0.32 \
	  --default-sd 0.17 \
	  --mean-shift 0.0 \
	  --means-and-sds-file $VORONOTADIR/energy_means_and_sds \
	  --external-weights-file $SUBDIR/$INFILEBASENAME.depthvalues \
	  --smoothing-window 5 \
	  --atom-scores-file $SUBDIR/$INFILEBASENAME.atomqscores \
	  --residue-scores-file $SUBDIR/$INFILEBASENAME.residueqscores \
	> $SUBDIR/$INFILEBASENAME.globalqscore
done

cat $SUBDIR/*.atomenergies \
| $VORONOTA score-contacts-energy-stats \
> $SUBDIR/means_and_sds

for INFILE in $SUBDIR/model*.contacts
do
	INFILEBASENAME=$(basename $INFILE .contacts)
	cat $INFILE \
	| awk '{print $1 " " $2 " " $4}' \
	| $VORONOTA compare-contacts \
	  --target-contacts-file <(cat $SUBDIR/target.contacts | awk '{print $1 " " $2 " " $4}') \
	  --atom-scores-file $SUBDIR/$INFILEBASENAME.atomcadscores \
	  --residue-scores-file $SUBDIR/$INFILEBASENAME.residuecadscores \
	  --smoothing-window 3 \
	  --smoothed-scores-file $SUBDIR/$INFILEBASENAME.smoothedresiduecadscores \
	  --depth 0 \
	> $SUBDIR/$INFILEBASENAME.globalcadscore
done

$VORONOTA score-scores \
  --reference-threshold 0.4 \
  --testable-step 0.01 \
  --outcomes-file $SUBDIR/localclassification \
  --ROC-curve-file $SUBDIR/roccurve \
  --PR-curve-file $SUBDIR/prcurve \
> $SUBDIR/scorescoringsummary << EOF
$SUBDIR/model1.residuecadscores $SUBDIR/model1.residueqscores
$SUBDIR/model2.residuecadscores $SUBDIR/model2.residueqscores
EOF

for INFILE in $SUBDIR/*.balls
do
	INFILEBASENAME=$(basename $INFILE .balls)
	
	cat $INFILE \
	| $VORONOTA query-balls \
	  --set-external-adjuncts $SUBDIR/$INFILEBASENAME.residueqscores \
	  --set-external-adjuncts-name rqscore \
	| $VORONOTA query-balls \
	  --set-external-adjuncts $SUBDIR/$INFILEBASENAME.atomqscores \
	  --set-external-adjuncts-name aqscore \
	| $VORONOTA write-balls-to-atoms-file \
	  --pdb-output $SUBDIR/$INFILEBASENAME.residueqscores.pdb \
	  --pdb-output-b-factor rqscore \
	| $VORONOTA write-balls-to-atoms-file \
	  --pdb-output $SUBDIR/$INFILEBASENAME.atomqscores.pdb \
	  --pdb-output-b-factor aqscore \
	> /dev/null
done

cat $SUBDIR/target.balls \
| $VORONOTA query-balls \
  --set-external-adjuncts $SUBDIR/model1.residuecadscores \
  --set-external-adjuncts-name m1rcadscores \
| $VORONOTA query-balls \
  --set-external-adjuncts $SUBDIR/model2.residuecadscores \
  --set-external-adjuncts-name m2rcadscores \
| $VORONOTA write-balls-to-atoms-file \
  --pdb-output $SUBDIR/target.m1residuecadscores.pdb \
  --pdb-output-b-factor m1rcadscores \
| $VORONOTA write-balls-to-atoms-file \
  --pdb-output $SUBDIR/target.m2residuecadscores.pdb \
  --pdb-output-b-factor m2rcadscores \
> /dev/null
