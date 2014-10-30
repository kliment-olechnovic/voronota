#!/bin/bash

BIN_DIR=$1
OUTPUT_DIR=$2
PDBID=$3
CHAINS=$4

TMP_DIR=$(mktemp -d)
cd $TMP_DIR

wget "http://www.pdb.org/pdb/files/$PDBID.gz" &> /dev/null
gunzip "$PDBID.gz"

$BIN_DIR/voronota get-balls-from-atoms-file --radii-file $BIN_DIR/radii --annotated < $PDBID \
| $BIN_DIR/voronota calculate-contacts --annotated \
| $BIN_DIR/voronota query-contacts --match-first "c<$CHAINS>" --match-min-seq-sep 2 > ./contacts

mkdir -p $OUTPUT_DIR

$BIN_DIR/voronota score-contacts-potential < ./contacts > $OUTPUT_DIR/$PDBID.summary
wc -l < ./contacts | sed "s/^/$PDBID.summary /" > $OUTPUT_DIR/$PDBID.count

rm -r $TMP_DIR
