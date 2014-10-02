#!/bin/bash

cd $(dirname "$0")

../../package.bash voronota_package
mv ../../voronota_package.tar.gz ./voronota_package.tar.gz
tar -xf ./voronota_package.tar.gz
rm ./voronota_package.tar.gz

TEST_SUBJECT=./voronota_package/voronota
RADII_FILE=./voronota_package/radii

INPUT_FILE=./input.pdb
INPUT_FILE_MMCIF=./input.cif
OUTPUT_DIR=./output/

rm -r -f $OUTPUT_DIR
mkdir $OUTPUT_DIR

$TEST_SUBJECT 2> $OUTPUT_DIR/help_message
$TEST_SUBJECT --help 2> $OUTPUT_DIR/help_full_message

############################

mkdir $OUTPUT_DIR/p1

$TEST_SUBJECT get-balls-from-atoms-file --radii-file $RADII_FILE < $INPUT_FILE > $OUTPUT_DIR/p1/balls

$TEST_SUBJECT calculate-vertices --print-log --check < $OUTPUT_DIR/p1/balls > $OUTPUT_DIR/p1/triangulation 2> $OUTPUT_DIR/p1/log_triangulation

$TEST_SUBJECT calculate-vertices-in-parallel --print-log --method simulated --parts 8 < $OUTPUT_DIR/p1/balls > $OUTPUT_DIR/p1/triangulation_p 2> $OUTPUT_DIR/p1/log_triangulation_p

$TEST_SUBJECT calculate-contacts < $OUTPUT_DIR/p1/balls > $OUTPUT_DIR/p1/contacts

############################

mkdir $OUTPUT_DIR/p2

$TEST_SUBJECT get-balls-from-atoms-file --radii-file $RADII_FILE --include-heteroatoms < $INPUT_FILE > $OUTPUT_DIR/p2/balls

$TEST_SUBJECT calculate-vertices --print-log --check < $OUTPUT_DIR/p2/balls > $OUTPUT_DIR/p2/triangulation 2> $OUTPUT_DIR/p2/log_triangulation

$TEST_SUBJECT calculate-vertices-in-parallel --print-log --method simulated --parts 8 < $OUTPUT_DIR/p2/balls > $OUTPUT_DIR/p2/triangulation_p 2> $OUTPUT_DIR/p2/log_triangulation_p

$TEST_SUBJECT calculate-contacts < $OUTPUT_DIR/p2/balls > $OUTPUT_DIR/p2/contacts

############################

mkdir $OUTPUT_DIR/p3

$TEST_SUBJECT get-balls-from-atoms-file < $INPUT_FILE > $OUTPUT_DIR/p3/balls

$TEST_SUBJECT calculate-vertices --print-log --check < $OUTPUT_DIR/p3/balls > $OUTPUT_DIR/p3/triangulation 2> $OUTPUT_DIR/p3/log_triangulation

$TEST_SUBJECT calculate-contacts < $OUTPUT_DIR/p3/balls > $OUTPUT_DIR/p3/contacts

############################

mkdir $OUTPUT_DIR/p4

$TEST_SUBJECT get-balls-from-atoms-file --mmcif --radii-file $RADII_FILE --include-heteroatoms --annotated < $INPUT_FILE_MMCIF > $OUTPUT_DIR/p4/balls

$TEST_SUBJECT calculate-contacts --annotated < $OUTPUT_DIR/p4/balls > $OUTPUT_DIR/p4/contacts

($TEST_SUBJECT query-contacts --match-first 'r[3:7,9]&an[CA,CB]' --match-min-seq-sep 1 | column -t) < $OUTPUT_DIR/p4/contacts > $OUTPUT_DIR/p4/contacts_query1
($TEST_SUBJECT query-contacts --inter-residue --match-first 'rn[THR]' --match-second 'rn[ARG]' < $OUTPUT_DIR/p4/contacts | column -t) > $OUTPUT_DIR/p4/contacts_query2
cat $OUTPUT_DIR/p4/balls | $TEST_SUBJECT calculate-contacts --annotated --draw | $TEST_SUBJECT query-contacts --match-first 'rn[CRO]' --match-min-seq-sep 1 --preserve-graphics --drawing-for-pymol $OUTPUT_DIR/p4/contacts_query3_drawing_for_pymol --drawing-for-jmol $OUTPUT_DIR/p4/contacts_query3_drawing_for_jmol --drawing-for-scenejs $OUTPUT_DIR/p4/contacts_query3_drawing_for_scenejs > $OUTPUT_DIR/p4/contacts_query3
($TEST_SUBJECT query-contacts --no-solvent --match-min-area 10.0 --match-min-dist 1.5 --match-max-dist 4.0 --match-min-seq-sep 1 | column -t) < $OUTPUT_DIR/p4/contacts > $OUTPUT_DIR/p4/contacts_query4
($TEST_SUBJECT query-contacts --match-external-annotations $OUTPUT_DIR/p4/contacts_query2 | column -t) < $OUTPUT_DIR/p4/contacts > $OUTPUT_DIR/p4/contacts_query5
($TEST_SUBJECT query-contacts --match-first-not 'rn[VAL]' --match-second-not 'rn[VAL]' --invert | column -t) < $OUTPUT_DIR/p4/contacts_query4 > $OUTPUT_DIR/p4/contacts_query6
($TEST_SUBJECT query-contacts --match-first 'an[O]' --set-tags 'withO' | $TEST_SUBJECT query-contacts --match-first 'an[NZ]' --set-tags 'withNZ' | column -t) < $OUTPUT_DIR/p4/contacts_query4 > $OUTPUT_DIR/p4/contacts_query7
($TEST_SUBJECT query-contacts --match-tags 'withO' --match-tags-not 'withNZ' | column -t) < $OUTPUT_DIR/p4/contacts_query7 > $OUTPUT_DIR/p4/contacts_query8
($TEST_SUBJECT query-contacts --match-first 'an[O,NZ]' --set-adjuncts 'b=10.0;a=1.0' | column -t) < $OUTPUT_DIR/p4/contacts_query7 > $OUTPUT_DIR/p4/contacts_query9

($TEST_SUBJECT query-contacts --match-min-seq-sep 1 | $TEST_SUBJECT score-contacts-potential --output-summed-areas | column -t) < $OUTPUT_DIR/p4/contacts > $OUTPUT_DIR/p4/contacts_scores_potential_areas
($TEST_SUBJECT query-contacts --match-min-seq-sep 1 | $TEST_SUBJECT score-contacts-potential | column -t) < $OUTPUT_DIR/p4/contacts > $OUTPUT_DIR/p4/contacts_scores_potential_values
($TEST_SUBJECT query-contacts --match-min-seq-sep 1 | $TEST_SUBJECT score-contacts --potential-file $OUTPUT_DIR/p4/contacts_scores_potential_values --inter-atom-scores-file $OUTPUT_DIR/p4/contacts_scores_inter_atom --inter-residue-scores-file $OUTPUT_DIR/p4/contacts_scores_inter_residue --atom-scores-file $OUTPUT_DIR/p4/contacts_scores_atom --residue-scores-file $OUTPUT_DIR/p4/contacts_scores_residue) < $OUTPUT_DIR/p4/contacts > $OUTPUT_DIR/p4/contacts_scores_global
($TEST_SUBJECT query-contacts --match-min-seq-sep 10 | $TEST_SUBJECT query-contacts --set-external-adjuncts $OUTPUT_DIR/p4/contacts_scores_inter_atom | column -t) < $OUTPUT_DIR/p4/contacts > $OUTPUT_DIR/p4/contacts_scores_injected

($TEST_SUBJECT query-contacts --match-min-seq-sep 1 | $TEST_SUBJECT compare-contacts --target-contacts-file <(cat $OUTPUT_DIR/p4/contacts | $TEST_SUBJECT query-contacts --match-min-seq-sep 1) --inter-atom-scores-file $OUTPUT_DIR/p4/contacts_comparison_inter_atom --inter-residue-scores-file $OUTPUT_DIR/p4/contacts_comparison_inter_residue --atom-scores-file $OUTPUT_DIR/p4/contacts_comparison_atom --residue-scores-file $OUTPUT_DIR/p4/contacts_comparison_residue) < $OUTPUT_DIR/p4/contacts > $OUTPUT_DIR/p4/contacts_comparison_global

($TEST_SUBJECT query-balls --match 'r[3:7,9]&an[CA,CB]' | column -t) < $OUTPUT_DIR/p4/balls > $OUTPUT_DIR/p4/balls_query1
($TEST_SUBJECT query-balls --match-adjuncts 'tf=45.0:50.0' --match-tags 'el=N|el=O' | column -t) < $OUTPUT_DIR/p4/balls > $OUTPUT_DIR/p4/balls_query2
($TEST_SUBJECT query-balls --match-external-annotations $OUTPUT_DIR/p4/contacts_query2 | column -t) < $OUTPUT_DIR/p4/balls > $OUTPUT_DIR/p4/balls_query3
($TEST_SUBJECT query-balls --match 'an[OE1]' --whole-residues | column -t) < $OUTPUT_DIR/p4/balls > $OUTPUT_DIR/p4/balls_query4

($TEST_SUBJECT query-balls --set-external-adjuncts $OUTPUT_DIR/p4/contacts_scores_atom --set-external-adjuncts-name qsa | $TEST_SUBJECT query-balls --set-external-adjuncts $OUTPUT_DIR/p4/contacts_scores_residue --set-external-adjuncts-name qsr | column -t) < $OUTPUT_DIR/p4/balls > $OUTPUT_DIR/p4/balls_scores1
$TEST_SUBJECT query-balls --set-external-adjuncts <(awk '{ print $1 " " (100-$2*100) }' < $OUTPUT_DIR/p4/contacts_scores_atom) --set-external-adjuncts-name qsa --pdb-output $OUTPUT_DIR/p4/balls_scores1_pdb1 --pdb-output-b-factor qsa < $OUTPUT_DIR/p4/balls > /dev/null
$TEST_SUBJECT query-balls --pdb-output $OUTPUT_DIR/p4/balls_scores1_pdb2 --pdb-output-b-factor qsa --pdb-output-template $INPUT_FILE < $OUTPUT_DIR/p4/balls_scores1 > /dev/null

############################

rm -r ./voronota_package

hg status ./
