#!/bin/bash

SUBDIR=$OUTPUTDIR/global_cadscore_script
mkdir -p $SUBDIR

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-global-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --processors 4 \
  --output-table-file "$SUBDIR/global_scores"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-global-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --processors 4 \
  --lt \
  --output-table-file "$SUBDIR/global_scores_lt"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-global-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
| column -t \
> "$SUBDIR/global_scores_formatted"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-global-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --lt \
| column -t \
> "$SUBDIR/global_scores_formatted_lt"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-global-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --lt \
  --subselect-contacts '[-inter-chain]' \
| column -t \
> "$SUBDIR/global_scores_interchain_formatted_lt"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-global-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --lt \
  --subselect-contacts '[-a1 [-chain A] -a2 [-chain B]]' \
| column -t \
> "$SUBDIR/global_scores_interpart_formatted_lt"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-global-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --processors 4 \
  --remap-chains \
  --crude \
| column -t \
> "$SUBDIR/global_scores_crude_remapped_formatted"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-global-cadscore \
  --target "$INPUTDIR/complex/target.pdb" \
  --model _list \
  --processors 4 \
  --remap-chains \
  --crude \
  --lt \
| column -t \
> "$SUBDIR/global_scores_crude_remapped_formatted_lt"

