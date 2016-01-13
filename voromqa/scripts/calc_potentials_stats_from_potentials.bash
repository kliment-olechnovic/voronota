#!/bin/bash

INPUT_SEARCH_DIR=""
OUTPUTDIR=""

while getopts "i:o:" OPTION
do
	case $OPTION in
	i)
		INPUT_SEARCH_DIR=$OPTARG
		;;
    o)
		OUTPUTDIR=$OPTARG
		;;
	esac
done

mkdir -p $OUTPUTDIR

find $INPUT_SEARCH_DIR -type f -name potential -not -empty \
| $BINDIR/voronota x-score-contacts-potentials-stats \
> $OUTPUTDIR/potential_means_and_sds
