#!/bin/bash

INFILE=$1

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

voronota-voromqa \
  -i $INFILE \
  -a $TMPDIR/iscores

voronota-contacts \
  -i $INFILE \
  -m "--set-external-means $TMPDIR/iscores" \
  -c '--no-same-chain --no-solvent' \
  -g '--drawing-name faces --adjunct-gradient em --adjunct-gradient-blue 1 --adjunct-gradient-red 0' \
  -d $TMPDIR/ifaces.py \
> /dev/null

pymol $TMPDIR/ifaces.py
