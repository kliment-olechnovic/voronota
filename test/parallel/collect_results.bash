#!/bin/bash

WORKING_DIR=$1
OUTPUT_FILE=$2

TMP_FILE=$(mktemp)

( find $WORKING_DIR -type f -name "*.log.table" |  while read ROW_FILE; do cat $ROW_FILE ; done ) > $TMP_FILE

( cat $TMP_FILE | head -1 ; cat $TMP_FILE | egrep -v 'input' | sort ) | column -t > $OUTPUT_FILE
	
rm $TMP_FILE
