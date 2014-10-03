#!/bin/bash

cd $(dirname "$0")

../../package.bash voronota_package
mv ../../voronota_package.tar.gz ./voronota_package.tar.gz
tar -xf ./voronota_package.tar.gz
rm ./voronota_package.tar.gz

TEST_SUBJECT=./voronota_package/voronota
RADII_FILE=./voronota_package/radii

INPUT_DIR=./input/

OUTPUT_DIR=./output/
rm -r -f $OUTPUT_DIR
mkdir $OUTPUT_DIR

$TEST_SUBJECT 2> $OUTPUT_DIR/help_message
$TEST_SUBJECT --help 2> $OUTPUT_DIR/help_full_message

############################

OUTPUT_SUBDIR=$OUTPUT_DIR/p1
mkdir $OUTPUT_SUBDIR

$TEST_SUBJECT get-balls-from-atoms-file --radii-file $RADII_FILE < $INPUT_DIR/structure.pdb > $OUTPUT_SUBDIR/balls

$TEST_SUBJECT calculate-vertices --print-log --check < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/triangulation 2> $OUTPUT_SUBDIR/log_triangulation

$TEST_SUBJECT calculate-vertices-in-parallel --print-log --method simulated --parts 8 < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/triangulation_p 2> $OUTPUT_SUBDIR/log_triangulation_p

$TEST_SUBJECT calculate-contacts < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/contacts

############################

OUTPUT_SUBDIR=$OUTPUT_DIR/p2
mkdir $OUTPUT_SUBDIR

$TEST_SUBJECT get-balls-from-atoms-file --radii-file $RADII_FILE --include-heteroatoms < $INPUT_DIR/structure.pdb > $OUTPUT_SUBDIR/balls

$TEST_SUBJECT calculate-vertices --print-log --check < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/triangulation 2> $OUTPUT_SUBDIR/log_triangulation

$TEST_SUBJECT calculate-vertices-in-parallel --print-log --method simulated --parts 8 < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/triangulation_p 2> $OUTPUT_SUBDIR/log_triangulation_p

$TEST_SUBJECT calculate-contacts < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/contacts

############################

OUTPUT_SUBDIR=$OUTPUT_DIR/p3
mkdir $OUTPUT_SUBDIR

$TEST_SUBJECT get-balls-from-atoms-file < $INPUT_DIR/structure.pdb > $OUTPUT_SUBDIR/balls

$TEST_SUBJECT calculate-vertices --print-log --check < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/triangulation 2> $OUTPUT_SUBDIR/log_triangulation

$TEST_SUBJECT calculate-contacts < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/contacts

############################

OUTPUT_SUBDIR=$OUTPUT_DIR/p4
mkdir $OUTPUT_SUBDIR

$TEST_SUBJECT get-balls-from-atoms-file --mmcif --radii-file $RADII_FILE --include-heteroatoms --annotated < $INPUT_DIR/structure.cif > $OUTPUT_SUBDIR/balls

$TEST_SUBJECT calculate-contacts --annotated < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/contacts

($TEST_SUBJECT query-contacts --match-first 'r[3:7,9]&an[CA,CB]' --match-min-seq-sep 1 | column -t) < $OUTPUT_SUBDIR/contacts > $OUTPUT_SUBDIR/contacts_query1
($TEST_SUBJECT query-contacts --inter-residue --match-first 'rn[THR]' --match-second 'rn[ARG]' < $OUTPUT_SUBDIR/contacts | column -t) > $OUTPUT_SUBDIR/contacts_query2
cat $OUTPUT_SUBDIR/balls | $TEST_SUBJECT calculate-contacts --annotated --draw | $TEST_SUBJECT query-contacts --match-first 'rn[CRO]' --match-min-seq-sep 1 --preserve-graphics --drawing-for-pymol $OUTPUT_SUBDIR/contacts_query3_drawing_for_pymol --drawing-for-jmol $OUTPUT_SUBDIR/contacts_query3_drawing_for_jmol --drawing-for-scenejs $OUTPUT_SUBDIR/contacts_query3_drawing_for_scenejs > $OUTPUT_SUBDIR/contacts_query3
($TEST_SUBJECT query-contacts --no-solvent --match-min-area 10.0 --match-min-dist 1.5 --match-max-dist 4.0 --match-min-seq-sep 1 | column -t) < $OUTPUT_SUBDIR/contacts > $OUTPUT_SUBDIR/contacts_query4
($TEST_SUBJECT query-contacts --match-external-annotations $OUTPUT_SUBDIR/contacts_query2 | column -t) < $OUTPUT_SUBDIR/contacts > $OUTPUT_SUBDIR/contacts_query5
($TEST_SUBJECT query-contacts --match-first-not 'rn[VAL]' --match-second-not 'rn[VAL]' --invert | column -t) < $OUTPUT_SUBDIR/contacts_query4 > $OUTPUT_SUBDIR/contacts_query6
($TEST_SUBJECT query-contacts --match-first 'an[O]' --set-tags 'withO' | $TEST_SUBJECT query-contacts --match-first 'an[NZ]' --set-tags 'withNZ' | column -t) < $OUTPUT_SUBDIR/contacts_query4 > $OUTPUT_SUBDIR/contacts_query7
($TEST_SUBJECT query-contacts --match-tags 'withO' --match-tags-not 'withNZ' | column -t) < $OUTPUT_SUBDIR/contacts_query7 > $OUTPUT_SUBDIR/contacts_query8
($TEST_SUBJECT query-contacts --match-first 'an[O,NZ]' --set-adjuncts 'b=10.0;a=1.0' | column -t) < $OUTPUT_SUBDIR/contacts_query7 > $OUTPUT_SUBDIR/contacts_query9

($TEST_SUBJECT query-contacts --match-min-seq-sep 1 | $TEST_SUBJECT score-contacts-potential --output-summed-areas | column -t) < $OUTPUT_SUBDIR/contacts > $OUTPUT_SUBDIR/contacts_scores_potential_areas
($TEST_SUBJECT query-contacts --match-min-seq-sep 1 | $TEST_SUBJECT score-contacts-potential | column -t) < $OUTPUT_SUBDIR/contacts > $OUTPUT_SUBDIR/contacts_scores_potential_values
($TEST_SUBJECT query-contacts --match-min-seq-sep 1 | $TEST_SUBJECT score-contacts --potential-file $OUTPUT_SUBDIR/contacts_scores_potential_values --inter-atom-scores-file $OUTPUT_SUBDIR/contacts_scores_inter_atom --inter-residue-scores-file $OUTPUT_SUBDIR/contacts_scores_inter_residue --atom-scores-file $OUTPUT_SUBDIR/contacts_scores_atom --residue-scores-file $OUTPUT_SUBDIR/contacts_scores_residue) < $OUTPUT_SUBDIR/contacts > $OUTPUT_SUBDIR/contacts_scores_global
($TEST_SUBJECT query-contacts --match-min-seq-sep 10 | $TEST_SUBJECT query-contacts --set-external-adjuncts $OUTPUT_SUBDIR/contacts_scores_inter_atom | column -t) < $OUTPUT_SUBDIR/contacts > $OUTPUT_SUBDIR/contacts_scores_injected

($TEST_SUBJECT query-contacts --match-min-seq-sep 1 | $TEST_SUBJECT compare-contacts --target-contacts-file <(cat $OUTPUT_SUBDIR/contacts | $TEST_SUBJECT query-contacts --match-min-seq-sep 1) --inter-atom-scores-file $OUTPUT_SUBDIR/contacts_comparison_inter_atom --inter-residue-scores-file $OUTPUT_SUBDIR/contacts_comparison_inter_residue --atom-scores-file $OUTPUT_SUBDIR/contacts_comparison_atom --residue-scores-file $OUTPUT_SUBDIR/contacts_comparison_residue) < $OUTPUT_SUBDIR/contacts > $OUTPUT_SUBDIR/contacts_comparison_global

($TEST_SUBJECT query-balls --match 'r[3:7,9]&an[CA,CB]' | column -t) < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/balls_query1
($TEST_SUBJECT query-balls --match-adjuncts 'tf=45.0:50.0' --match-tags 'el=N|el=O' | column -t) < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/balls_query2
($TEST_SUBJECT query-balls --match-external-annotations $OUTPUT_SUBDIR/contacts_query2 | column -t) < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/balls_query3
($TEST_SUBJECT query-balls --match 'an[OE1]' --whole-residues | column -t) < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/balls_query4

($TEST_SUBJECT query-balls --set-external-adjuncts $OUTPUT_SUBDIR/contacts_scores_atom --set-external-adjuncts-name qsa | $TEST_SUBJECT query-balls --set-external-adjuncts $OUTPUT_SUBDIR/contacts_scores_residue --set-external-adjuncts-name qsr | column -t) < $OUTPUT_SUBDIR/balls > $OUTPUT_SUBDIR/balls_scores1
$TEST_SUBJECT query-balls --set-external-adjuncts <(awk '{ print $1 " " (100-$2*100) }' < $OUTPUT_SUBDIR/contacts_scores_atom) --set-external-adjuncts-name qsa --pdb-output $OUTPUT_SUBDIR/balls_scores1_pdb1 --pdb-output-b-factor qsa < $OUTPUT_SUBDIR/balls > /dev/null
$TEST_SUBJECT query-balls --pdb-output $OUTPUT_SUBDIR/balls_scores1_pdb2 --pdb-output-b-factor qsa --pdb-output-template $INPUT_DIR/structure.pdb < $OUTPUT_SUBDIR/balls_scores1 > /dev/null

############################

OUTPUT_SUBDIR=$OUTPUT_DIR/p5
mkdir $OUTPUT_SUBDIR

$TEST_SUBJECT get-balls-from-atoms-file --radii-file $RADII_FILE --annotated < $INPUT_DIR/complex/target.pdb | $TEST_SUBJECT calculate-contacts --annotated --draw | $TEST_SUBJECT query-contacts --no-same-chain --no-solvent --preserve-graphics > $OUTPUT_SUBDIR/target_iface
$TEST_SUBJECT get-balls-from-atoms-file --radii-file $RADII_FILE --annotated < $INPUT_DIR/complex/model1.pdb | $TEST_SUBJECT calculate-contacts --annotated | $TEST_SUBJECT query-contacts --no-same-chain --no-solvent > $OUTPUT_SUBDIR/model1_iface
$TEST_SUBJECT get-balls-from-atoms-file --radii-file $RADII_FILE --annotated < $INPUT_DIR/complex/model2.pdb | $TEST_SUBJECT calculate-contacts --annotated | $TEST_SUBJECT query-contacts --no-same-chain --no-solvent > $OUTPUT_SUBDIR/model2_iface

$TEST_SUBJECT compare-contacts --target-contacts-file <(cat $OUTPUT_SUBDIR/target_iface) --inter-residue-scores-file $OUTPUT_SUBDIR/model1_iface_inter_residue_cadscores < $OUTPUT_SUBDIR/model1_iface > $OUTPUT_SUBDIR/model1_iface_global_cadscores
$TEST_SUBJECT compare-contacts --target-contacts-file <(cat $OUTPUT_SUBDIR/target_iface) --inter-residue-scores-file $OUTPUT_SUBDIR/model2_iface_inter_residue_cadscores < $OUTPUT_SUBDIR/model2_iface > $OUTPUT_SUBDIR/model2_iface_global_cadscores

$TEST_SUBJECT query-contacts --inter-residue --set-external-adjuncts <(cat $OUTPUT_SUBDIR/model1_iface_inter_residue_cadscores | awk '{print $1 " " $2 " " (1-$3)}') --set-external-adjuncts-name irs --preserve-graphics < $OUTPUT_SUBDIR/target_iface | $TEST_SUBJECT query-contacts --drawing-for-pymol $OUTPUT_SUBDIR/model1_iface_scores_drawing.py --drawing-name model1_iface_scores --drawing-adjunct-gradient irs > /dev/null
$TEST_SUBJECT query-contacts --inter-residue --set-external-adjuncts <(cat $OUTPUT_SUBDIR/model2_iface_inter_residue_cadscores | awk '{print $1 " " $2 " " (1-$3)}') --set-external-adjuncts-name irs --preserve-graphics < $OUTPUT_SUBDIR/target_iface | $TEST_SUBJECT query-contacts --drawing-for-pymol $OUTPUT_SUBDIR/model2_iface_scores_drawing.py --drawing-name model2_iface_scores --drawing-adjunct-gradient irs > /dev/null

############################

rm -r ./voronota_package

hg status ./
