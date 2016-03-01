#!/bin/bash

INFILE=$1

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

voronota-voromqa \
  -i $INFILE \
  -a $TMPDIR/iscores \
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
  -g '--drawing-name faces --adjunct-gradient em --adjunct-gradient-blue 1 --adjunct-gradient-red 0' \
  -d $TMPDIR/ifaces.py \
  -q \
> /dev/null

cat > $TMPDIR/script.pml << EOF
spectrum b, red_white_blue, minimum=0, maximum=1
EOF

pymol $TMPDIR/ifaces.py $TMPDIR/struct.pdb $TMPDIR/script.pml
