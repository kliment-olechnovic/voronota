#!/bin/bash

SCRIPTDIR=$(dirname $0)
POTENTIAL_FILE1=$1
POTENTIAL_FILE2=$2
TAG1=$3
TAG2=$4
OUTDIR=$5

mkdir -p $OUTDIR

R --vanilla --args \
  V-input1 $POTENTIAL_FILE1 \
  V-input2 $POTENTIAL_FILE2 \
  V-filter1 V3 $TAG1 \
  V-filter2 V3 $TAG2 \
  V-column1 V4.x \
  V-column2 V4.y \
  V-sds-column V5 \
  V-mergingA V1 \
  V-mergingB V2 \
  V-output-image $OUTDIR/plot_for__${TAG1}__and__${TAG2}.png \
< $SCRIPTDIR/compare_two_tables_values.R
