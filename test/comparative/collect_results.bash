#!/bin/bash

WORKING_DIR=$1
OUTPUT_FILE=$2

cat $(find $WORKING_DIR -type f -name "*.summary.table" | head -1) | head -1 > $OUTPUT_FILE

( find $WORKING_DIR -type f -name "*.summary.table" | while read ROW_FILE ; do cat $ROW_FILE | tail -1 ; done ) >> $OUTPUT_FILE
