#!/bin/bash

TEST_SUBJECT=$1
EPSILON=$2
INPUT_DIR=$3
OUTPUT_DIR=$4
PDB_FILE=$5

PDB_FILE_BASENAME=$(basename $PDB_FILE .ent.gz)
PDB_FILE_DOMAIN=$(echo $PDB_FILE_BASENAME | sed 's/pdb.\(..\)./\1/')
INPUT_DIR="$INPUT_DIR/$PDB_FILE_DOMAIN"

mkdir -p $OUTPUT_DIR

TMP_DIR=$(mktemp -d)

ZIPPED_INPUT_FILE="$INPUT_DIR/$PDB_FILE_BASENAME.first_gauge.balls.gz"
SEQUENTIAL_LOG_FILE="$INPUT_DIR/$PDB_FILE_BASENAME.test_subject.log"
INPUT_FILE="$TMP_DIR/input.balls"
RAW_LOG_FILE="$TMP_DIR/raw_log"
RAW_TIME_FILE="$TMP_DIR/raw_time"
LOG_LIST_FILE="$OUTPUT_DIR/$PDB_FILE_BASENAME.parallel.log.list"
LOG_TABLE_FILE="$OUTPUT_DIR/$PDB_FILE_BASENAME.parallel.log.table"

zcat $ZIPPED_INPUT_FILE > $INPUT_FILE

if [ ! -s "$INPUT_FILE" ]
then
	exit 1
fi

( time -p ($TEST_SUBJECT --mode calculate-triangulation-in-parallel --method openmp --parts 4 --clog-file $RAW_LOG_FILE --epsilon $EPSILON --print-log < $INPUT_FILE > /dev/null 2> /dev/null) ) 2> $RAW_TIME_FILE

echo "input $PDB_FILE_BASENAME" > $LOG_LIST_FILE
cat $SEQUENTIAL_LOG_FILE | sed 's/^/sequential_/' >> $LOG_LIST_FILE
cat $RAW_LOG_FILE | sed 's/^/parallel_/' >> $LOG_LIST_FILE
cat $RAW_TIME_FILE | sed 's/^/parallel_time_/' >> $LOG_LIST_FILE

( cat $LOG_LIST_FILE | cut --delimiter " " --fields 1 | paste -s ; cat $LOG_LIST_FILE | cut --delimiter " " --fields 2 | paste -s ) | column -t > $LOG_TABLE_FILE

rm -r "$TMP_DIR"
