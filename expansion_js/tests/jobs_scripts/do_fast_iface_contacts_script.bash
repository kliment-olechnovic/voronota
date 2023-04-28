#!/bin/bash

SUBDIR=$OUTPUTDIR/fast_iface_contacts_script
mkdir -p $SUBDIR

find "$INPUTDIR/complex/" -type f \
| while read -r INFILE
do
	$VORONOTAJSDIR/voronota-js-fast-iface-contacts \
	  --input "$INFILE" \
	  --restrict-input "[]" \
	  --subselect-contacts "[]" \
	  --processors "1" \
	  --expand-ids \
	  --output-drawing-script "${SUBDIR}/draw_aa_sas_-BASENAME-.py" \
	| column -t \
	> "${SUBDIR}/aa_nosas_$(basename ${INFILE}).tsv"
	
	cat "$INFILE" \
	| $VORONOTAJSDIR/voronota-js-fast-iface-contacts \
	  --input _stream \
	  --coarse-grained \
	  --with-sas-areas \
	  --og-pipeable \
	| wc -l \
	> "${SUBDIR}/rr_count_$(basename ${INFILE}).txt"
done

find "$INPUTDIR/complex/" -type f \
| $VORONOTAJSDIR/voronota-js-fast-iface-contacts \
  --input _list \
  --restrict-input "[]" \
  --subselect-contacts "[]" \
  --processors "4" \
  --coarse-grained \
  --with-sas-areas \
  --og-pipeable \
  --output-contacts-file "${SUBDIR}/rr_sas_-BASENAME-.tsv"

