#!/bin/bash

BIN_DIR=$1
OUTPUT_DIR=$2

mkdir -p $OUTPUT_DIR

$BIN_DIR/voronota score-contacts-potential --input-file-list \
> $OUTPUT_DIR/summary__full

cat $OUTPUT_DIR/summary__full \
| egrep -v 'sd0to0|sd1to1' \
| sed 's/hb;//' \
| sed 's/sd2to5/sd/' \
| $BIN_DIR/voronota score-contacts-potential --potential-file $OUTPUT_DIR/potential__rh_sd \
> $OUTPUT_DIR/summary__rh_sd

cat $OUTPUT_DIR/summary__rh_sd \
| sed 's/rh;sd/rh/' | sed 's/sd/./' \
| $BIN_DIR/voronota score-contacts-potential --potential-file $OUTPUT_DIR/potential__rh\
> $OUTPUT_DIR/summary__rh

cat $OUTPUT_DIR/summary__rh_sd \
| sed 's/rh;sd/sd/' | sed 's/rh/./' \
| $BIN_DIR/voronota score-contacts-potential --potential-file $OUTPUT_DIR/potential__sd \
> $OUTPUT_DIR/summary__sd

cat $OUTPUT_DIR/summary__rh_sd \
| awk '{print $1 " " $2 " . " $4}' \
| $BIN_DIR/voronota score-contacts-potential --potential-file $OUTPUT_DIR/potential__simple \
> $OUTPUT_DIR/global_summary_simple
