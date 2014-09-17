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

$TEST_SUBJECT get-balls-from-atoms-file --radii-file $RADII_FILE < $INPUT_FILE > $OUTPUT_DIR/balls1
$TEST_SUBJECT get-balls-from-atoms-file --radii-file $RADII_FILE --include-heteroatoms < $INPUT_FILE > $OUTPUT_DIR/balls2
$TEST_SUBJECT get-balls-from-atoms-file < $INPUT_FILE > $OUTPUT_DIR/balls3
$TEST_SUBJECT get-balls-from-atoms-file --mmcif --radii-file $RADII_FILE --include-heteroatoms --annotated < $INPUT_FILE_MMCIF > $OUTPUT_DIR/balls4

$TEST_SUBJECT calculate-vertices --print-log --check < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/triangulation1 2> $OUTPUT_DIR/log_triangulation1
$TEST_SUBJECT calculate-vertices --print-log --check < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/triangulation2 2> $OUTPUT_DIR/log_triangulation2
$TEST_SUBJECT calculate-vertices --print-log --check < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/triangulation3 2> $OUTPUT_DIR/log_triangulation3
$TEST_SUBJECT calculate-vertices-in-parallel --print-log --method simulated --parts 8 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/triangulation1p 2> $OUTPUT_DIR/log_triangulation1p
$TEST_SUBJECT calculate-vertices-in-parallel --print-log --method simulated --parts 8 < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/triangulation2p 2> $OUTPUT_DIR/log_triangulation2p

$TEST_SUBJECT calculate-contacts < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/contacts1
$TEST_SUBJECT calculate-contacts < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/contacts2
$TEST_SUBJECT calculate-contacts < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/contacts3
$TEST_SUBJECT calculate-contacts --annotated < $OUTPUT_DIR/balls4 > $OUTPUT_DIR/contacts4

($TEST_SUBJECT query-contacts --match-first 'r[3:7,9]&an[CA,CB]' --match-min-seq-sep 1 | column -t) < $OUTPUT_DIR/contacts4 > $OUTPUT_DIR/contacts4_query1
($TEST_SUBJECT query-contacts --inter-residue --match-first 'rn[THR]' --match-second 'rn[ARG]' < $OUTPUT_DIR/contacts4) | awk '{print $1 " " $2}' | column -t > $OUTPUT_DIR/contacts4_query2
cat $OUTPUT_DIR/balls4 | $TEST_SUBJECT calculate-contacts --annotated --draw | $TEST_SUBJECT query-contacts --match-first 'rn[CRO]' --match-min-seq-sep 1 --preserve-graphics --drawing-for-pymol $OUTPUT_DIR/contacts4_query3_drawing_for_pymol --drawing-for-jmol $OUTPUT_DIR/contacts4_query3_drawing_for_jmol --drawing-for-scenejs $OUTPUT_DIR/contacts4_query3_drawing_for_scenejs > $OUTPUT_DIR/contacts4_query3
($TEST_SUBJECT query-contacts --no-solvent --match-min-area 10.0 --match-min-dist 1.5 --match-max-dist 4.0 --match-min-seq-sep 1 | column -t) < $OUTPUT_DIR/contacts4 > $OUTPUT_DIR/contacts4_query4
($TEST_SUBJECT query-contacts --match-external-annotations $OUTPUT_DIR/contacts4_query2 | column -t) < $OUTPUT_DIR/contacts4 > $OUTPUT_DIR/contacts4_query5
($TEST_SUBJECT query-contacts --match-first-not 'rn[VAL]' --match-second-not 'rn[VAL]' --invert | column -t) < $OUTPUT_DIR/contacts4_query4 > $OUTPUT_DIR/contacts4_query6
($TEST_SUBJECT query-contacts --match-first 'an[O]' --set-tags 'withO' | $TEST_SUBJECT query-contacts --match-first 'an[NZ]' --set-tags 'withNZ' | column -t) < $OUTPUT_DIR/contacts4_query4 > $OUTPUT_DIR/contacts4_query7
($TEST_SUBJECT query-contacts --match-tags 'withO' --match-tags-not 'withNZ' | column -t) < $OUTPUT_DIR/contacts4_query7 > $OUTPUT_DIR/contacts4_query8
($TEST_SUBJECT query-contacts --match-first 'an[O,NZ]' --set-adjuncts 'b=10.0;a=1.0' | column -t) < $OUTPUT_DIR/contacts4_query7 > $OUTPUT_DIR/contacts4_query9

($TEST_SUBJECT query-balls --match 'r[3:7,9]&an[CA,CB]' | column -t) < $OUTPUT_DIR/balls4 > $OUTPUT_DIR/balls4_query1
($TEST_SUBJECT query-balls --match-adjuncts 'tf=45.0:50.0' --match-tags 'N|O' | column -t) < $OUTPUT_DIR/balls4 > $OUTPUT_DIR/balls4_query2
($TEST_SUBJECT query-balls --match-external-annotations $OUTPUT_DIR/contacts4_query2 | column -t) < $OUTPUT_DIR/balls4 > $OUTPUT_DIR/balls4_query3
($TEST_SUBJECT query-balls --match 'an[OE1]' --whole-residues | column -t) < $OUTPUT_DIR/balls4 > $OUTPUT_DIR/balls4_query4

rm -r ./voronota_package

hg status ./
