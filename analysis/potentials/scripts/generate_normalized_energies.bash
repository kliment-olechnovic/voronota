#!/bin/bash

BIN_DIR=$1
POTENTIAL_FILE=$2
OUTPUT_DIR=$3
INPUT_FILE=$4

mkdir -p $OUTPUT_DIR

TMP_DIR=$(mktemp -d)

cat $INPUT_FILE \
| sed 's/hb;//' \
| sed 's/rh/./' \
| $BIN_DIR/voronota query-contacts --no-solvent --match-max-seq-sep 5 --set-tags sd \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| $BIN_DIR/voronota score-contacts --detailed-output --potential-file $POTENTIAL_FILE --atom-scores-file $TMP_DIR/scores \
> /dev/null

cat $TMP_DIR/scores | awk '{print $1 " " $3}' | sed 's/.*\(R<.*>A<.*>\)/\1/' > $OUTPUT_DIR/$(basename $INPUT_FILE)

rm -r $TMP_DIR
