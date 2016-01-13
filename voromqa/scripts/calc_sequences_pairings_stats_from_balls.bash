#!/bin/bash

INPUT_FILE_LIST=""
OUTPUTDIR=""

while getopts "i:o:" OPTION
do
	case $OPTION in
	i)
		INPUT_FILE_LIST=$OPTARG
		;;
    o)
		OUTPUTDIR=$OPTARG
		;;
	esac
done

mkdir -p $OUTPUTDIR

cat $INPUT_FILE_LIST \
| $BINDIR/voronota x-query-balls-sequences-pairings-stats \
> $OUTPUTDIR/pairings_probabilities
