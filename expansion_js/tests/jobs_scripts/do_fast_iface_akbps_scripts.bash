#!/bin/bash

SUBDIR=$OUTPUTDIR/fast_iface_akbps_scripts
mkdir -p $SUBDIR

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-akbps \
  --akbps-lib ${INPUTDIR}/../../../resources/voromqalike_area_kbps_raw \
  --input _list \
  --processors 4 \
  --output-table-file "$SUBDIR/akbps_scores"

