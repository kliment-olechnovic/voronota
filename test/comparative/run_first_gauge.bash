#!/bin/bash

FIRST_GAUGE=$1
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

INPUT_FILE="$TMP_DIR/input.pdb"
RAW_OUTPUT_FILE="$TMP_DIR/input.a.qtf"
RAW_TIME_FILE="$TMP_DIR/raw_time"
BALLS_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.first_gauge.balls"
QUADRUPLES_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.first_gauge.quadruples"
LOG_FILE="$WORKING_DIR/$PDB_FILE_BASENAME.first_gauge.log"

zcat "$PDB_FILE" | awk '/^END/{exit}1' | egrep '^ATOM' | egrep -v '.{76} H' > $INPUT_FILE

if [ ! -s "$INPUT_FILE" ]
then
	exit 1
fi

( time -p (timeout 600 $FIRST_GAUGE $INPUT_FILE $TMP_DIR/ &> /dev/null) ) 2> $RAW_TIME_FILE

if [ ! -s "$RAW_OUTPUT_FILE" ]
then
	exit 1
fi

cat $RAW_OUTPUT_FILE | egrep '^QTVTX' | awk '{print $4 " " $5 " " $6 " " $7}' > $BALLS_FILE

cat $RAW_OUTPUT_FILE | egrep '^QTCELL' | awk '{print ($3-1) " " ($4-1) " " ($5-1) " " ($6-1)}' | awk '{split($0,array," "); asort(array); printf array[1] " " array[2] " " array[3] " " array[4] "\n"}' | sort > $QUADRUPLES_FILE

true > $LOG_FILE
cat $RAW_OUTPUT_FILE | egrep '^MODEL' | awk '{print "atoms " $3}' >> $LOG_FILE
cat $RAW_OUTPUT_FILE | egrep '^MODEL' | awk '{print "vertices " $4}' >> $LOG_FILE
cat $RAW_TIME_FILE | sed 's/^/time_/' >> $LOG_FILE

rm -r "$TMP_DIR"

gzip -f $BALLS_FILE $QUADRUPLES_FILE &> /dev/null
