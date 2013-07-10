#!/bin/bash

SECOND_GAUGE=$1
WORKING_DIR=$2
PDB_FILE=$3

PDB_FILE_BASENAME=$(basename $PDB_FILE .ent.gz)
PDB_FILE_DOMAIN=$(echo $PDB_FILE_BASENAME | sed 's/pdb.\(..\)./\1/')
WORKING_DIR="$WORKING_DIR/$PDB_FILE_DOMAIN"

TMP_DIR=$(mktemp -d)

INPUT_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.first_gauge.balls"
RAW_OUTPUT_FILE="$TMP_DIR/awvoronoi_result.xml"
RAW_TIME_FILE="$TMP_DIR/raw_time"
QUADRUPLES_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.second_gauge.quadruples"
LOG_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.second_gauge.log"

gunzip -f $INPUT_FILE.gz &> /dev/null

if [ ! -s "$INPUT_FILE" ]
then
	exit 1
fi

( time -p (timeout 600 java -Xms1246m -jar $SECOND_GAUGE/awvoronoi-run-1.0.0.jar $INPUT_FILE $RAW_OUTPUT_FILE &> /dev/null) ) 2> $RAW_TIME_FILE

if [ ! -s "$RAW_OUTPUT_FILE" ]
then
	exit 1
fi

cat $RAW_OUTPUT_FILE | grep '<cell id' | grep -v 'INF' | tr '<' ' ' | tr '>' ' ' | awk '{print ($4-1) " " ($5-1) " " ($6-1) " " ($7-1)}' | awk '{split($0,array," "); asort(array); printf array[1] " " array[2] " " array[3] " " array[4] "\n"}' | sort > $QUADRUPLES_FILE

true > $LOG_FILE
cat $RAW_OUTPUT_FILE | grep '<vertex id' | grep -v 'INF' | wc -l | sed 's/^/atoms /' >> $LOG_FILE
cat $RAW_OUTPUT_FILE | grep '<cell id' | grep -v 'INF' | wc -l | sed 's/^/vertices /' >> $LOG_FILE
cat $RAW_TIME_FILE | sed 's/^/time_/' >> $LOG_FILE

rm -r "$TMP_DIR"

gzip -f $INPUT_FILE $QUADRUPLES_FILE &> /dev/null
