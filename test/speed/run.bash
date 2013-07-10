#!/bin/bash

TEST_SUBJECT=$1
WORKING_DIR=$2
PDB_FILE=$3

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

zcat "$PDB_FILE" | awk '/^END/{exit}1' | egrep '^ATOM' | egrep -v '.{76} H' | $TEST_SUBJECT --mode get-balls-from-pdb-file > $INPUT_FILE

if [ ! -s "$INPUT_FILE" ]
then
	exit 1
fi

( time -p ($TEST_SUBJECT --mode calculate-triangulation --clog-file $RAW_LOG_FILE --print-log --skip-output < $INPUT_FILE > /dev/null) ) 2> $RAW_TIME_FILE

echo "input $PDB_FILE_BASENAME" > $LOG_LIST_FILE
cat $RAW_LOG_FILE >> $LOG_LIST_FILE
cat $RAW_TIME_FILE | sed 's/^/time_/' >> $LOG_LIST_FILE

( cat $LOG_LIST_FILE | cut --delimiter " " --fields 1 | paste -s ; cat $LOG_LIST_FILE | cut --delimiter " " --fields 2 | paste -s ) | column -t > $LOG_TABLE_FILE

rm -r "$TMP_DIR"

cat $LOG_LIST_FILE
