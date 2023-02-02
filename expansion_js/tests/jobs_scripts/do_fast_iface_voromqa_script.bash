#!/bin/bash

SUBDIR=$OUTPUTDIR/fast_iface_voromqa_script
mkdir -p $SUBDIR

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-voromqa \
  --input _list \
  --processors 4 \
  --subselect-contacts '[-a1 [-chain A] -a2 [-chain B]]' \
  --output-ia-contacts-file "$SUBDIR/inter_atom_contacts_for_-BASENAME-.txt" \
  --output-ir-contacts-file "$SUBDIR/inter_residue_contacts_for_-BASENAME-.txt" \
| column -t \
> "$SUBDIR/global_scores"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-voromqa \
  --input _list \
  --processors 4 \
  --subselect-contacts '[-a1 [-chain A] -a2 [-chain B]]' \
  --blanket \
  --score-symmetry \
  --output-ir-contacts-file "$SUBDIR/inter_residue_contacts_blanket_for_-BASENAME-.txt" \
| column -t \
> "$SUBDIR/global_scores_blanket"

find "$INPUTDIR/complex/" -type f \
| xargs -L 1 -P 4 $VORONOTAJSDIR/voronota-js-fast-iface-voromqa \
  --subselect-contacts '[-a1 [-chain A] -a2 [-chain B]]' \
  --output-table-file "$SUBDIR/global_scores_for_-BASENAME-.txt" \
  --input

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-voromqa \
  --input _list \
  --processors 4 \
  --run-faspr "${HOME}/software/FASPR/dun2010bbdep.bin" \
| column -t \
> "$SUBDIR/global_scores_after_faspr"
