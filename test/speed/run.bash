#!/bin/bash

TEST_SUBJECT=$1
RADII_FILE=$2
WORKING_DIR=$3
PDB_FILE=$4

if [ ! -s "$PDB_FILE" ]
then
	exit 1
fi

PDB_FILE_BASENAME=$(basename $PDB_FILE .ent.gz)
PDB_FILE_DOMAIN=$(echo $PDB_FILE_BASENAME | sed 's/pdb.\(..\)./\1/')
WORKING_DIR="$WORKING_DIR/$PDB_FILE_DOMAIN"

mkdir -p $WORKING_DIR

TMP_DIR=$(mktemp -d)

INPUT_FILE="$TMP_DIR/input.balls"
RAW_LOG_FILE="$TMP_DIR/raw_log"
RAW_TIME_FILE="$TMP_DIR/raw_time"
LOG_LIST_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.log.list"
LOG_TABLE_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.log.table"
LOG_ERRORS_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.log.errors"

zcat "$PDB_FILE" | $TEST_SUBJECT --mode get-balls-from-pdb-file --include-heteroatoms --radii-file $RADII_FILE > $INPUT_FILE 2> /dev/null

if [ ! -s "$INPUT_FILE" ]
then
	echo "$PDB_FILE_BASENAME has no acceptable atoms"
	exit 1
fi

( time -p ($TEST_SUBJECT --mode calculate-triangulation --clog-file $RAW_LOG_FILE --print-log --skip-output < $INPUT_FILE > /dev/null 2> $LOG_ERRORS_FILE) ) 2> $RAW_TIME_FILE

if [ ! -s "$LOG_ERRORS_FILE" ]
then
	rm -f $LOG_ERRORS_FILE
fi

echo "input $PDB_FILE_BASENAME" > $LOG_LIST_FILE
cat $RAW_LOG_FILE >> $LOG_LIST_FILE
cat $RAW_TIME_FILE | sed 's/^/time_/' >> $LOG_LIST_FILE

( cat $LOG_LIST_FILE | cut --delimiter " " --fields 1 | paste -s ; cat $LOG_LIST_FILE | cut --delimiter " " --fields 2 | paste -s ) | column -t > $LOG_TABLE_FILE

rm -r "$TMP_DIR"

cat $LOG_LIST_FILE
