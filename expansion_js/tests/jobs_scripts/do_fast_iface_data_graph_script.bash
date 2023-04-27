#!/bin/bash

SUBDIR=$OUTPUTDIR/fast_iface_data_graph_script
mkdir -p $SUBDIR

find "$INPUTDIR/complex/" -type f \
| while read -r INFILE
do
	$VORONOTAJSDIR/voronota-js-fast-iface-data-graph \
	  --coarse-grained \
	  --input "$INFILE" \
	  --restrict-input "[]" \
	  --subselect-contacts "[]" \
	  --processors "1" \
	  --output-data-prefix "${SUBDIR}/rr_" \
	> /dev/null
	
	$VORONOTAJSDIR/voronota-js-fast-iface-data-graph \
	  --input "$INFILE" \
	  --restrict-input "[]" \
	  --subselect-contacts "[]" \
	  --processors "1" \
	  --output-data-prefix "${SUBDIR}/aa_" \
	> /dev/null
done

