#!/bin/bash

MAXCUT=$1
STEPCUT=$2
INFILE=$3

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

cat $INFILE \
| ./voronota get-balls-from-atoms-file --annotated \
| ./voronota query-balls --seq-output "$TMPLDIR/sequence" \
> /dev/null

for CUT1 in $(seq 1 $STEPCUT $MAXCUT)
do
	for CUT2 in $(seq 1 $STEPCUT $MAXCUT)
	do
		cat "$TMPLDIR/sequence" | cut -c${CUT1}- | rev | cut -c${CUT2}- | rev > "$TMPLDIR/sequence_cut__${CUT1}__${CUT2}"
	done
done

find "$TMPLDIR/" -type f -name 'sequence_cut__*' -not -empty | sort -V | while read SEQFILE
do
	./voronota-voromqa --input "$INFILE" --reference-sequence "$SEQFILE" \
	| sed "s|^|$(basename $SEQFILE | sed 's|^sequence_cut__||' | sed 's|__| |g') |" \
	| awk '{print $1 " " $2 " " $4 " " $5 " " $6}'
done
