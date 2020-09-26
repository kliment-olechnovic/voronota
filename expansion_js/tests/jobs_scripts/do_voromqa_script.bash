#!/bin/bash

SUBDIR=$OUTPUTDIR/voromqa_script
mkdir -p $SUBDIR

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-voromqa \
  --input _list \
  --processors 4 \
  --select-contacts '[-a1 [-chain A] -a2 [-chain B]]' \
  --tour-sort '-columns full_dark_score sel_energy -multipliers 1 -1 -tolerances 0.02 0.0' \
| column -t \
> "$SUBDIR/global_scores"

