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

(cat $INPUT_FILE_LIST | xargs -L 100 -P 1 cat) \
| $BINDIR/voronota x-score-contacts-energy-stats \
> $OUTPUTDIR/energy_means_and_sds
