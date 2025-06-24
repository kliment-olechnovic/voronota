#!/bin/bash

SUBDIR=$OUTPUTDIR/receptor_data_graph_script
mkdir -p $SUBDIR

$VORONOTAJSDIR/voronota-js-receptor-data-graph \
  --input "${INPUTDIR}/complex/target.pdb" \
  --output-dir "${SUBDIR}/output_graph_default" 

$VORONOTAJSDIR/voronota-js-receptor-data-graph \
  --input "${INPUTDIR}/complex/target.pdb" \
  --output-dir "${SUBDIR}/output_graph_custom1" \
  --probe-min 1 \
  --probe-max 5 \
  --buriedness-core 0.8 \
  --buriedness-rim 0.7 \
  --subpockets 1

  