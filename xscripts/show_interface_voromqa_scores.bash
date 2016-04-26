#!/bin/bash

INFILE=$1

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

voronota-voromqa \
  --input $INFILE \
  --output-atom-scores $TMPDIR/iscores \
  --output-atom-scores-pdb $TMPDIR/struct.pdb \
  --multiple-models

voronota-contacts \
  --input $INFILE \
  --contacts-query-additional "--set-external-means $TMPDIR/iscores" \
  --contacts-query '--no-same-chain --no-solvent' \
  --drawing-parameters '--drawing-name faces --adjunct-gradient em --adjunct-gradient-blue 1 --adjunct-gradient-red 0' \
  --output-drawing $TMPDIR/ifaces.py \
  --multiple-models \
> /dev/null

cat > $TMPDIR/script.pml << EOF
spectrum b, red_white_blue, minimum=0, maximum=1
EOF

pymol $TMPDIR/ifaces.py $TMPDIR/struct.pdb $TMPDIR/script.pml
