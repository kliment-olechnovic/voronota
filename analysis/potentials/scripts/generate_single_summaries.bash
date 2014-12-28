#!/bin/bash

BIN_DIR=$1
OUTPUT_DIR=$2
INPUT_FILE=$3

mkdir -p $OUTPUT_DIR

cat $INPUT_FILE \
| $BIN_DIR/voronota query-contacts --no-solvent --match-min-seq-sep 0 --match-max-seq-sep 0 --set-tags sd0to0 \
| $BIN_DIR/voronota query-contacts --no-solvent --match-min-seq-sep 1 --match-max-seq-sep 1 --set-tags sd1to1 \
| $BIN_DIR/voronota query-contacts --no-solvent --match-min-seq-sep 2 --match-max-seq-sep 5 --set-tags sd2to5 \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| $BIN_DIR/voronota score-contacts-potential \
> $OUTPUT_DIR/$(basename $INPUT_FILE)
