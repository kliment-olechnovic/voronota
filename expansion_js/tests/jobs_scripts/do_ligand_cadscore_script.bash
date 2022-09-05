#!/bin/bash

SUBDIR=$OUTPUTDIR/ligand_cadscore_script
mkdir -p $SUBDIR

$VORONOTAJSDIR/voronota-js-ligand-cadscore \
  --casp15-target "$INPUTDIR/protein_ligand/T1118v1LG035_1" \
  --casp15-target-pose 1 \
  --casp15-model "$INPUTDIR/protein_ligand/T1118v1LG046_1" \
  --casp15-model-pose 2 \
  --table-dir "$SUBDIR/table" \
  --details-dir "$SUBDIR/details" \
  --drawing-dir "$SUBDIR/details/drawings" \
  --and-swap true \
| column -t \
> "$SUBDIR/stdout.txt"

