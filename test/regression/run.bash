#!/bin/bash

set -e

cd $(dirname "$0")

APOLLOTA=../../Release/apollota
RADII_FILE=../../resources/radii
INPUT_FILE=./input.pdb
OUTPUT_DIR=./output/

rm -r -f $OUTPUT_DIR
mkdir $OUTPUT_DIR

$APOLLOTA --mode get-balls-from-pdb-file --radii-file $RADII_FILE < $INPUT_FILE > $OUTPUT_DIR/balls1
$APOLLOTA --mode get-balls-from-pdb-file --radii-file $RADII_FILE --include-heteroatoms < $INPUT_FILE > $OUTPUT_DIR/balls2
$APOLLOTA --mode get-balls-from-pdb-file < $INPUT_FILE > $OUTPUT_DIR/balls3

$APOLLOTA --mode calculate-triangulation --print-log --clog-file $OUTPUT_DIR/log_triangulation1 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/triangulation1
$APOLLOTA --mode calculate-triangulation --print-log --clog-file $OUTPUT_DIR/log_triangulation2 < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/triangulation2
$APOLLOTA --mode calculate-triangulation --print-log --clog-file $OUTPUT_DIR/log_triangulation3 < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/triangulation3

$APOLLOTA --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation2 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_2_on_1
$APOLLOTA --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation2 < $OUTPUT_DIR/balls2 > $OUTPUT_DIR/comparison_1_to_2_on_2
$APOLLOTA --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation3 < $OUTPUT_DIR/balls1 > $OUTPUT_DIR/comparison_1_to_3_on_1
$APOLLOTA --mode compare-triangulations --first-triangulation-file $OUTPUT_DIR/triangulation1 --second-triangulation-file $OUTPUT_DIR/triangulation3 < $OUTPUT_DIR/balls3 > $OUTPUT_DIR/comparison_1_to_3_on_3

hg status ./
