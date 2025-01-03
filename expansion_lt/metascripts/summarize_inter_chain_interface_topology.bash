#!/bin/bash

readonly ZEROARG=$0

OUTDIR="$1"
INFILE="$2"
EXTRAOPTION="$3"

if [ -z "$OUTDIR" ]
then
	echo >&2 "Error: no output directory provided"
	exit 1
fi

if [ -z "$INFILE" ]
then
	echo >&2 "Error: no input file provided"
	exit 1
fi

if [ -n "$EXTRAOPTION" ]
then
	echo >&2 "Error: too many input arguments provided, must be only two (output directory, input file path)"
	exit 1
fi

if [ "$OUTDIR" == "_stdout" ] && [ "$INFILE" == "_list" ]
then
	readonly TMPLDIR="$(mktemp -d)"
	trap "rm -r $TMPLDIR" EXIT
	
	cat > "${TMPLDIR}/input"
	
	if [ -z "${TMPLDIR}/input" ]
	then
		echo >&2 "Error: no input files list provided to stdin"
		exit 1
	fi
	
	cat "${TMPLDIR}/input" \
	| egrep -v '^\s*list_\s*$' \
	| xargs -L 1 -P 20 $ZEROARG "${TMPLDIR}/output"
	
	find "${TMPLDIR}/output" -type f \
	| xargs -L 100 cat \
	| awk '{if(NR==1 || $1!="input"){print $0}}' \
	> "${TMPLDIR}/table.tsv"
	
	{
		head -1 "${TMPLDIR}/table.tsv"
		tail -n +2 "${TMPLDIR}/table.tsv" | sort -n -k2,2
	} \
	> "${TMPLDIR}/sorted_table.tsv"
	
	cat "${TMPLDIR}/sorted_table.tsv"
	
	exit 0
fi

if [ ! -s "$INFILE" ]
then
	echo >&2 "Error: input structure file '$INFILE' does not exist"
	exit 1
fi

command -v voronota-lt &> /dev/null || { echo >&2 "Error: 'voronota-lt' executable not in binaries path"; exit 1; }

command -v voronota-js-pdb-utensil-filter-atoms &> /dev/null || { echo >&2 "Error: 'voronota-js-pdb-utensil-filter-atoms' executable not in binaries path"; exit 1; }

mkdir -p "$OUTDIR"

cat "$INFILE" \
| voronota-js-pdb-utensil-filter-atoms '[-aname C,N,CA]' \
| voronota-lt \
  --compute-only-inter-chain-contacts \
  --probe 9.0 \
  --mesh-print-topology-summary \
2> >(egrep -v '^log_' >&2) \
| sed 's|^meshinfo_header|input|' \
| sed "s|^meshinfo|$(basename ${INFILE})|" \
> "${OUTDIR}/$(basename ${INFILE}).txt"

