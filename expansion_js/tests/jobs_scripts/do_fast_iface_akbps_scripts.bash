#!/bin/bash

SUBDIR=$OUTPUTDIR/fast_iface_akbps_scripts
mkdir -p $SUBDIR

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-areas-layered \
  --layering-mode "inward" \
  --directing-mode "none" \
  --input _list \
  --processors 4 \
  --output-table-file "$SUBDIR/areas_layered_inward"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-areas-layered \
  --layering-mode "outward" \
  --directing-mode "none" \
  --input _list \
  --processors 4 \
  --output-table-file "$SUBDIR/areas_layered_outward"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-areas-layered \
  --layering-mode "inward" \
  --directing-mode "d4" \
  --input _list \
  --processors 4 \
  --output-table-file "$SUBDIR/areas_layered_inward_d4"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-areas-layered \
  --layering-mode "outward" \
  --directing-mode "d4" \
  --input _list \
  --processors 4 \
  --output-table-file "$SUBDIR/areas_layered_outward_d4"
  
find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-akbps \
  --akbps-lib ${INPUTDIR}/../../../resources/voromqalike_area_kbps_raw \
  --input _list \
  --processors 4 \
  --output-table-file "$SUBDIR/akbps_scores"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-akbps-layered \
  --akbps-layered-lib ${INPUTDIR}/../../../resources/voromqalike_area_kbps_layered_collapsed_raw \
  --input _list \
  --processors 4 \
  --output-table-file "$SUBDIR/akbps_layered_collapsed_scores"

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-akbps-layered \
  --akbps-layered-lib ${INPUTDIR}/../../../resources/voromqalike_area_kbps_layered_inward_raw \
  --input _list \
  --processors 4 \
  --output-table-file "$SUBDIR/akbps_layered_inward_scores"
