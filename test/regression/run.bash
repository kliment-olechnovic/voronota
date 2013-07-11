#!/bin/bash

cd $(dirname "$0")

TEST_SUBJECT=../../Release/apollota
RADII_FILE=../../resources/radii
INPUT_FILE=./input.pdb
OUTPUT_DIR=./output/

rm -r -f $OUTPUT_DIR
mkdir $OUTPUT_DIR

$TEST_SUBJECT --help 2> $OUTPUT_DIR/help_message
$TEST_SUBJECT --help-full 2> $OUTPUT_DIR/help_full_message

set -e

$TEST_SUBJECT --mode get-balls-from-pdb-file --radii-file $RADII_FILE --output-comments < $INPUT_FILE > $OUTPUT_DIR/balls1
$TEST_SUBJECT --mode get-balls-from-pdb-file --radii-file $RADII_FILE --output-comments --include-heteroatoms < $INPUT_FILE > $OUTPUT_DIR/balls2
$TEST_SUBJECT --mode get-balls-from-pdb-file --output-comments < $INPUT_FILE > $OUTPUT_DIR/balls3

$TEST_SUBJECT --mode calculate-triangulation --print-log --clog-file $OUTPUT_DIR/log_triangulation1 --check < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/triangulation1
$TEST_SUBJECT --mode calculate-triangulation --print-log --clog-file $OUTPUT_DIR/log_triangulation2 --check < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/triangulation2
$TEST_SUBJECT --mode calculate-triangulation --print-log --clog-file $OUTPUT_DIR/log_triangulation3 --check < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/triangulation3

$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation2 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_2_on_1
$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation2 < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/comparison_1_to_2_on_2
$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation3 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_3_on_1
$TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation3 < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/comparison_1_to_3_on_3

hg status ./
