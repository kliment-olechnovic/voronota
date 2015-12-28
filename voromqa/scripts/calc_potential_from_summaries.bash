#!/bin/bash

INPUT_FILE_LIST=""
RANDOMIZE=""
INPUT_CONTRIBUTIONS=""
FIXED_TYPES_OPTION=""
OUTPUTDIR=""

while getopts "i:r:c:t:o:" OPTION
do
	case $OPTION in
	i)
		INPUT_FILE_LIST=$OPTARG
		;;
	r)
		RANDOMIZE=$OPTARG
		;;
	c)
		INPUT_CONTRIBUTIONS="--input-contributions $OPTARG"
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
| $BINDIR/voronota score-contacts-potential \
  --input-file-list \
  --contributions-file $OUTPUTDIR/contributions \
  --single-areas-file $OUTPUTDIR/single_areas \
| tee $OUTPUTDIR/summary \
| $BINDIR/voronota score-contacts-potential $INPUT_CONTRIBUTIONS $FIXED_TYPES_OPTION \
  --potential-file $OUTPUTDIR/potential \
  --probabilities-file $OUTPUTDIR/probabilities \
> /dev/null
