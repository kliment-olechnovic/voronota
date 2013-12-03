#!/bin/bash

TEST_SUBJECT=$1
EPSILON=$2
WORKING_DIR=$3
PDB_FILE=$4

PDB_FILE_BASENAME=$(basename $PDB_FILE .ent.gz)
PDB_FILE_DOMAIN=$(echo $PDB_FILE_BASENAME | sed 's/pdb.\(..\)./\1/')
WORKING_DIR="$WORKING_DIR/$PDB_FILE_DOMAIN"

TMP_DIR=$(mktemp -d)

INPUT_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.first_gauge.balls"
RAW_OUTPUT_FILE="$TMP_DIR/triangulation"
BALLS_GRAPH_OUTPUT_FILE="$TMP_DIR/balls_graph"
VERTICES_GRAPH_OUTPUT_FILE="$TMP_DIR/vertices_graph"
RAW_TIME_FILE="$TMP_DIR/raw_time"
QUADRUPLES_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.test_subject.quadruples"
LOG_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.test_subject.log"

gunzip -f $INPUT_FILE.gz &> /dev/null

if [ ! -s "$INPUT_FILE" ]
then
	exit 1
fi

( time -p ($TEST_SUBJECT --mode calculate-triangulation --clog-file $LOG_FILE --epsilon $EPSILON --print-log --output-balls-graph $BALLS_GRAPH_OUTPUT_FILE --output-vertices-graph $VERTICES_GRAPH_OUTPUT_FILE < $INPUT_FILE > $RAW_OUTPUT_FILE) ) 2> $RAW_TIME_FILE

if [ ! -s "$RAW_OUTPUT_FILE" ]
then
	exit 1
fi

cat $RAW_OUTPUT_FILE | awk '{print $1 " " $2 " " $3 " " $4}' | awk '{split($0,array," "); asort(array); printf array[1] " " array[2] " " array[3] " " array[4] "\n"}' | sort > $QUADRUPLES_FILE

cat $RAW_TIME_FILE | sed 's/^/time_/' >> $LOG_FILE

rm -r "$TMP_DIR"

gzip -f $INPUT_FILE $QUADRUPLES_FILE &> /dev/null
