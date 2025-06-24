#!/bin/bash

SUBDIR=$OUTPUTDIR/lt_data_graph_script
mkdir -p $SUBDIR

$VORONOTAJSDIR/voronota-js-lt-data-graph --input "${INPUTDIR}/complex/target.pdb" --output-dir "${SUBDIR}/output_graph_default" 

$VORONOTAJSDIR/voronota-js-lt-data-graph --input "${INPUTDIR}/complex/target.pdb" --output-dir "${SUBDIR}/output_graph_custom1" --probe 2.8

