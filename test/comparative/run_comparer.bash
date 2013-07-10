#!/bin/bash

COMPARER=$1
EPSILON=$2
WORKING_DIR=$3
PDB_FILE=$4

PDB_FILE_BASENAME=$(basename $PDB_FILE .ent.gz)

BALLS_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.first_gauge.balls"
FIRST_GAUGE_QUADRUPLES_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.first_gauge.quadruples"
SECOND_GAUGE_QUADRUPLES_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.second_gauge.quadruples"
TEST_SUBJECT_QUADRUPLES_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.test_subject.quadruples"

if [ -s "$BALLS_FILE" ] && [ -s "$TEST_SUBJECT_QUADRUPLES_FILE" ]
then
	if [ -s "$FIRST_GAUGE_QUADRUPLES_FILE" ]
	then
		FIRST_RESULTS_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.comparison.first"
		$COMPARER --mode compare-triangulations --epsilon $EPSILON --first-triangulation-file $FIRST_GAUGE_QUADRUPLES_FILE --second-triangulation-file $TEST_SUBJECT_QUADRUPLES_FILE < $BALLS_FILE > $FIRST_RESULTS_FILE	
	fi
	if [ -s "$SECOND_GAUGE_QUADRUPLES_FILE" ]
	then
		SECOND_RESULTS_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.comparison.second"
		$COMPARER --mode compare-triangulations --epsilon $EPSILON --first-triangulation-file $SECOND_GAUGE_QUADRUPLES_FILE --second-triangulation-file $TEST_SUBJECT_QUADRUPLES_FILE < $BALLS_FILE > $SECOND_RESULTS_FILE	
	fi
fi
