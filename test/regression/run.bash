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

$TEST_SUBJECT --mode get-balls-from-atoms-file --radii-file $RADII_FILE < $INPUT_FILE > $OUTPUT_DIR/balls1
$TEST_SUBJECT --mode get-balls-from-atoms-file --radii-file $RADII_FILE --include-heteroatoms < $INPUT_FILE > $OUTPUT_DIR/balls2
$TEST_SUBJECT --mode get-balls-from-atoms-file < $INPUT_FILE > $OUTPUT_DIR/balls3
$TEST_SUBJECT --mode get-balls-from-atoms-file --mmcif --radii-file $RADII_FILE --include-heteroatoms --enhanced-comments < $INPUT_FILE_MMCIF > $OUTPUT_DIR/balls4

$TEST_SUBJECT --mode calculate-vertices --print-log --clog-file $OUTPUT_DIR/log_triangulation1 --check < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/triangulation1
$TEST_SUBJECT --mode calculate-vertices --print-log --clog-file $OUTPUT_DIR/log_triangulation2 --check < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/triangulation2
$TEST_SUBJECT --mode calculate-vertices --print-log --clog-file $OUTPUT_DIR/log_triangulation3 --check < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/triangulation3
$TEST_SUBJECT --mode calculate-vertices-in-parallel --print-log --clog-file $OUTPUT_DIR/log_triangulation1p --method simulated --parts 8 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/triangulation1p
$TEST_SUBJECT --mode calculate-vertices-in-parallel --print-log --clog-file $OUTPUT_DIR/log_triangulation2p --method simulated --parts 8 < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/triangulation2p

$TEST_SUBJECT --mode compare-quadruples-sets --first-quadruples-file $OUTPUT_DIR/triangulation1 --second-quadruples-file $OUTPUT_DIR/triangulation2 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_2_on_1
$TEST_SUBJECT --mode compare-quadruples-sets --first-quadruples-file $OUTPUT_DIR/triangulation1 --second-quadruples-file $OUTPUT_DIR/triangulation2 < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/comparison_1_to_2_on_2
$TEST_SUBJECT --mode compare-quadruples-sets --first-quadruples-file $OUTPUT_DIR/triangulation1 --second-quadruples-file $OUTPUT_DIR/triangulation3 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_3_on_1
$TEST_SUBJECT --mode compare-quadruples-sets --first-quadruples-file $OUTPUT_DIR/triangulation1 --second-quadruples-file $OUTPUT_DIR/triangulation3 < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/comparison_1_to_3_on_3
$TEST_SUBJECT --mode compare-quadruples-sets --first-quadruples-file $OUTPUT_DIR/triangulation1 --second-quadruples-file $OUTPUT_DIR/triangulation1p < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_1p
$TEST_SUBJECT --mode compare-quadruples-sets --first-quadruples-file $OUTPUT_DIR/triangulation2 --second-quadruples-file $OUTPUT_DIR/triangulation2p < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_2_to_2p

$TEST_SUBJECT --mode calculate-contacts --print-log --clog-file $OUTPUT_DIR/log_contacts1 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/contacts1
$TEST_SUBJECT --mode calculate-contacts --print-log --clog-file $OUTPUT_DIR/log_contacts2 < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/contacts2
$TEST_SUBJECT --mode calculate-contacts --print-log --clog-file $OUTPUT_DIR/log_contacts3 < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/contacts3
$TEST_SUBJECT --mode calculate-contacts --print-log --clog-file $OUTPUT_DIR/log_contacts4 --annotate < $OUTPUT_DIR/balls4 > $OUTPUT_DIR/contacts4

$TEST_SUBJECT --mode calculate-contacts-query --match-first 'r[3:7,9]&an[CA,CB]' < $OUTPUT_DIR/contacts4 > $OUTPUT_DIR/contacts4_query1
$TEST_SUBJECT --mode calculate-contacts-query --inter-residue --match-first 'rn[THR]' --match-second 'rn[ARG]' < $OUTPUT_DIR/contacts4 > $OUTPUT_DIR/contacts4_query2
cat $OUTPUT_DIR/balls4 | $TEST_SUBJECT --mode calculate-contacts --annotate --draw | $TEST_SUBJECT --mode calculate-contacts-query --match-first 'rn[CRO]' --preserve-graphics --drawing-for-pymol $OUTPUT_DIR/contacts4_query3_drawing_for_pymol --drawing-for-jmol $OUTPUT_DIR/contacts4_query3_drawing_for_jmol > $OUTPUT_DIR/contacts4_query3
$TEST_SUBJECT --mode calculate-contacts-query --no-solvent --match-min-area 10.0 < $OUTPUT_DIR/contacts4 > $OUTPUT_DIR/contacts4_query4

rm -r ./voronota_package

hg status ./
