#!/bin/bash

SUBDIR=$OUTPUTDIR/fast_iface_cadscore_script
mkdir -p $SUBDIR

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --processors 4 \
  --output-table-file "$SUBDIR/global_scores"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
| column -t \
> "$SUBDIR/global_scores_formatted"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --processors 4 \
  --remap-chains \
| column -t \
> "$SUBDIR/global_scores_remapped_formatted"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --processors 4 \
  --remap-chains \
  --crude \
| column -t \
> "$SUBDIR/global_scores_crude_remapped_formatted"
