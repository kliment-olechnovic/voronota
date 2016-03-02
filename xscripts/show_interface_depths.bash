#!/bin/bash

INFILE=$1

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

voronota-voromqa \
  -i $INFILE \
  -d $TMPDIR/iscores \
  -q

voronota-bfactor \
  -p $INFILE \
  -s $TMPDIR/iscores \
  -q \
> $TMPDIR/struct.pdb

voronota-contacts \
  -i $INFILE \
  -m "--set-external-means $TMPDIR/iscores" \
  -c '--no-same-chain --no-solvent' \
  -g '--drawing-name faces --adjunct-gradient em --adjunct-gradient-blue 1 --adjunct-gradient-red 3' \
  -d $TMPDIR/ifaces.py \
  -q \
> /dev/null

cat > $TMPDIR/script.pml << EOF
spectrum b, blue_white_red, minimum=1, maximum=3
EOF

pymol $TMPDIR/ifaces.py $TMPDIR/struct.pdb $TMPDIR/script.pml
