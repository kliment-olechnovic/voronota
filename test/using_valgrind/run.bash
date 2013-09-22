#!/bin/bash

cd $(dirname "$0")

INPUT_FILE=$1

if [ ! -s "$INPUT_FILE" ]
then
	exit 1
fi

TEST_SUBJECT_SOURCE_DIR=../../src/
RADII_FILE=../../resources/radii

TMP_DIR=$(mktemp -d)

TEST_SUBJECT="$TMP_DIR/executable"
BALLS_FILE="$TMP_DIR/balls"
TRIANGULATION_FILE="$TMP_DIR/triangulation"

g++ -g -O1 -o $TEST_SUBJECT $TEST_SUBJECT_SOURCE_DIR/*.cpp

echo
echo "Preparing balls:"
valgrind --tool=memcheck $TEST_SUBJECT --mode get-balls-from-atoms-file --include-heteroatoms --radii-file $RADII_FILE < $INPUT_FILE > $BALLS_FILE

echo
echo "Calculating triangulation:"
valgrind --tool=memcheck $TEST_SUBJECT --mode calculate-triangulation --print-log --epsilon 0.00000001 < $BALLS_FILE > $TRIANGULATION_FILE

echo
echo "Comparing triangulations:"
valgrind --tool=memcheck $TEST_SUBJECT --mode compare-triangulations --first-triangulation-file $TRIANGULATION_FILE --second-triangulation-file $TRIANGULATION_FILE < $BALLS_FILE

rm -r "$TMP_DIR"
