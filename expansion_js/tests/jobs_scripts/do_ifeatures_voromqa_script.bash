#!/bin/bash

SUBDIR=$OUTPUTDIR/ifeatures_voromqa_script
mkdir -p $SUBDIR

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-ifeatures-voromqa \
  --input _list \
  --processors 4 \
  --as-assembly \
| column -t \
> "$SUBDIR/global_scores"

