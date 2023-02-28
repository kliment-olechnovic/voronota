#!/bin/bash

SUBDIR=$OUTPUTDIR/fast_iface_cadscore_matrix_script
mkdir -p $SUBDIR

mkdir -p "${SUBDIR}/augmented_input"

find "$INPUTDIR/complex/" -type f \
| while read -r FNAME
do
	for NUM in 1 2 3 4
	do
		cp "$FNAME" "${SUBDIR}/augmented_input/copy${NUM}_$(basename ${FNAME})"
	done
done

find "${SUBDIR}/augmented_input" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-cadscore-matrix \
  --processors 4 \
  --output-table-file "$SUBDIR/global_scores"

find "${SUBDIR}/augmented_input" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-cadscore-matrix \
| column -t \
> "$SUBDIR/global_scores_formatted"

find "${SUBDIR}/augmented_input" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-cadscore-matrix \
  --processors 4 \
  --remap-chains \
| column -t \
> "$SUBDIR/global_scores_remapped_formatted"

find "${SUBDIR}/augmented_input" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-cadscore-matrix \
  --processors 4 \
  --remap-chains \
  --crude \
| column -t \
> "$SUBDIR/global_scores_crude_remapped_formatted"

rm -r "${SUBDIR}/augmented_input"

