#!/bin/bash

set +e

INPUT_FILE_LIST=""
RANDOMIZE=""
FIXED_TYPES_OPTION=""
OUTPUTDIR=""

while getopts "i:r:t:o:" OPTION
do
	case $OPTION in
	i)
		INPUT_FILE_LIST=$OPTARG
		;;
	r)
		RANDOMIZE=$OPTARG
		;;
	t)
		FIXED_TYPES_OPTION="--input-fixed-types $OPTARG"
		;;
    o)
		OUTPUTDIR=$OPTARG
		;;
	esac
done

mkdir -p $OUTPUTDIR

if [ -n "$RANDOMIZE" ]
then
	OUTPUTDIR=$(mktemp --tmpdir=$OUTPUTDIR -d)
	cat $INPUT_FILE_LIST | shuf | head -n $RANDOMIZE | sort > $OUTPUTDIR/list
else
	cat $INPUT_FILE_LIST | sort > $OUTPUTDIR/list
fi

cat $OUTPUTDIR/list \
| $BINDIR/voronota score-contacts-potential --input-file-list \
| $BINDIR/voronota score-contacts-potential \
  --potential-file $OUTPUTDIR/potential $FIXED_TYPES_OPTION \
  --contributions-file $OUTPUTDIR/contributions \
  --probabilities-file $OUTPUTDIR/probabilities \
  --single-areas-file $OUTPUTDIR/single_areas \
> $OUTPUTDIR/summary
