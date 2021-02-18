#!/bin/bash

readonly ZEROARG=$0

readonly infile="$1"

if [ -z "$infile" ] || [ ! -s "$infile" ]
then
	echo >&2 "No input file provided."
	exit 1
fi

if [[ $ZEROARG == *"/"* ]]
then
	cd $(dirname $ZEROARG)
	export PATH=$(pwd):$PATH
	cd - &> /dev/null
fi

command -v voronota &> /dev/null || { echo >&2 "Error: 'voronota' executable not in binaries path"; exit 1; }
command -v voronota-resources &> /dev/null || { echo >&2 "Error: 'voronota-resources' executable not in binaries path"; exit 1; }
command -v voronota-volumes &> /dev/null || { echo >&2 "Error: 'voronota-volumes' executable not in binaries path"; exit 1; }

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

{
echo "chain res_number res_name volume_struct volume_free"

cat "$infile" | voronota get-balls-from-atoms-file --annotated | voronota expand-descriptors | awk '{print $1 " " $2}' | uniq \
| while read -r chain resnum
do
	resnum_prev=$((resnum-1))
	resnum_next=$((resnum+1))
	
	full_results="$(voronota-volumes -i "$infile" --per-residue --atoms-query "--match c<${chain}>&r<${resnum}>" --cache-dir "${TMPLDIR}/vorocache" | voronota expand-descriptors | awk '{print $1 " " $2 " " $6 " " $8}')"

	partial_results="$(voronota-volumes -i "$infile" --per-residue --atoms-query "--match c<${chain}>&r<${resnum}>" --input-filter-query "--match c<${chain}>&r<${resnum_prev}:${resnum_next}>" | voronota expand-descriptors | awk '{print $8}')"
	
	echo "$full_results $partial_results"
done
} | tr ' ' '\t'

