#!/bin/bash

TEST_SUBJECT=$1
OUTPUT_DIR=$2
INCLUDE_HYDROGENS=$3
EXCLUDE_HIDDEN=$4
IN_PARALLEL=$5
PDB_FILE=$6

RAW_PDB_FILE_BASENAME=$(basename $PDB_FILE)
PDB_FILE_BASENAME=$(basename $PDB_FILE .ent.gz)

mkdir -p $OUTPUT_DIR

TMP_DIR=$(mktemp -d)

INPUT_BALLS_FILE="$TMP_DIR/input.balls"
RAW_LOG_FILE="$TMP_DIR/raw_log"
RAW_TIME_FILE="$TMP_DIR/raw_time"
LOG_LIST_FILE="$OUTPUT_DIR/$PDB_FILE_BASENAME.summary.list"
LOG_TABLE_FILE="$OUTPUT_DIR/$PDB_FILE_BASENAME.summary.table"

INCLUDE_HYDROGENS_FLAG=""
if [ "$INCLUDE_HYDROGENS" == "yes" ]
then
	INCLUDE_HYDROGENS_FLAG="--include-hydrogens"
fi

EXCLUDE_HIDDEN_FLAG=""
if [ "$EXCLUDE_HIDDEN" == "yes" ]
then
	EXCLUDE_HIDDEN_FLAG="--exclude-hidden-balls"
fi

PRITING_APP="zcat"
if [ "$RAW_PDB_FILE_BASENAME" == "$PDB_FILE_BASENAME" ]
then
	PRITING_APP="cat"
fi
$PRITING_APP $PDB_FILE | $TEST_SUBJECT --mode get-balls-from-atoms-file $INCLUDE_HYDROGENS_FLAG > $INPUT_BALLS_FILE

if [ "$IN_PARALLEL" == "yes" ]
then
	( time -p ($TEST_SUBJECT --mode calculate-vertices-in-parallel --method openmp --parts 4 --clog-file $RAW_LOG_FILE --print-log < $INPUT_BALLS_FILE > /dev/null 2> /dev/null) ) 2> $RAW_TIME_FILE
else
	( time -p ($TEST_SUBJECT --mode calculate-vertices --clog-file $RAW_LOG_FILE --print-log $EXCLUDE_HIDDEN_FLAG < $INPUT_BALLS_FILE > /dev/null 2> /dev/null) ) 2> $RAW_TIME_FILE
fi

echo "input $PDB_FILE_BASENAME" > $LOG_LIST_FILE
cat $RAW_LOG_FILE >> $LOG_LIST_FILE
cat $RAW_TIME_FILE | sed 's/^/time_/' >> $LOG_LIST_FILE

( cat $LOG_LIST_FILE | cut --delimiter " " --fields 1 | paste -s ; cat $LOG_LIST_FILE | cut --delimiter " " --fields 2 | paste -s ) | column -t > $LOG_TABLE_FILE

rm -r "$TMP_DIR"
