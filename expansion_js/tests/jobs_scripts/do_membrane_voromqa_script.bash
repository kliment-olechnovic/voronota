#!/bin/bash

SUBDIR=$OUTPUTDIR/membrane_voromqa_script
mkdir -p $SUBDIR

$VORONOTAJSDIR/voronota-js-membrane-voromqa \
  --input "$INPUTDIR/membrane/structure.pdb" \
  --membrane-width 20,30 \
  --output-local-scores "$SUBDIR/local_scores/" \
| column -t \
> "$SUBDIR/global_scores"

