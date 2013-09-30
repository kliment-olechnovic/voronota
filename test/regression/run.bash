#!/bin/bash

cd $(dirname "$0")

../../package.bash apollota_package
mv ../../apollota_package.tar.gz ./apollota_package.tar.gz
tar -xf ./apollota_package.tar.gz
rm ./apollota_package.tar.gz

TEST_SUBJECT=./apollota_package/apollota
RADII_FILE=./apollota_package/radii

INPUT_FILE=./input.pdb
INPUT_FILE_MMCIF=./input.cif
OUTPUT_DIR=./output/

rm -r -f $OUTPUT_DIR
mkdir $OUTPUT_DIR

$TEST_SUBJECT --help 2> $OUTPUT_DIR/help_message
$TEST_SUBJECT --help-full 2> $OUTPUT_DIR/help_full_message

$TEST_SUBJECT --mode get-balls-from-atoms-file --radii-file $RADII_FILE < $INPUT_FILE > $OUTPUT_DIR/balls1
$TEST_SUBJECT --mode get-balls-from-atoms-file --radii-file $RADII_FILE --include-heteroatoms < $INPUT_FILE > $OUTPUT_DIR/balls2
$TEST_SUBJECT --mode get-balls-from-atoms-file < $INPUT_FILE > $OUTPUT_DIR/balls3
$TEST_SUBJECT --mode get-balls-from-atoms-file --mmcif --radii-file $RADII_FILE --include-heteroatoms < $INPUT_FILE_MMCIF > $OUTPUT_DIR/balls4

$TEST_SUBJECT --mode calculate-triangulation --print-log --clog-file $OUTPUT_DIR/log_triangulation1 --check < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/triangulation1
$TEST_SUBJECT --mode calculate-triangulation --print-log --clog-file $OUTPUT_DIR/log_triangulation2 --check < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/triangulation2
$TEST_SUBJECT --mode calculate-triangulation --print-log --clog-file $OUTPUT_DIR/log_triangulation3 --check < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/triangulation3
$TEST_SUBJECT --mode calculate-triangulation-in-parallel --print-log --clog-file $OUTPUT_DIR/log_triangulation1p --method simulated --parts 8 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/triangulation1p
$TEST_SUBJECT --mode calculate-triangulation-in-parallel --print-log --clog-file $OUTPUT_DIR/log_triangulation2p --method simulated --parts 8 < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/triangulation2p

$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation2 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_2_on_1
$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation2 < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/comparison_1_to_2_on_2
$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation3 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_3_on_1
$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation3 < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/comparison_1_to_3_on_3
$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation1p < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_1p
$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation2 --second-triangulation-file $OUTPUT_DIR/triangulation2p < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_2_to_2p

rm -r ./apollota_package

hg status ./
