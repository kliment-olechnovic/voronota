#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

INPUT_FILE_LIST=""
OUTPUT_FILE=""
HEADER_START=""
SORT=false
TABULATE=false

while getopts "i:o:h:sut" OPTION
do
	case $OPTION in
	i)
		INPUT_FILE_LIST=$OPTARG
		;;
	o)
		OUTPUT_FILE=$OPTARG
		;;
	h)
		HEADER_START=$OPTARG
		;;
	s)
		SORT=true
		;;
	t)
		TABULATE=true
		;;
	esac
done

(cat $INPUT_FILE_LIST | xargs -L 100 -P 1 cat) > $TMPDIR/output

if $SORT
then
	cat $TMPDIR/output | sort | uniq > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

if [ "$HEADER_START" != "" ]
then
	{
		cat $TMPDIR/output | egrep "^$HEADER_START" | uniq
		cat $TMPDIR/output | egrep -v "^$HEADER_START"
	} > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

if $TABULATE
then
	cat $TMPDIR/output | column -t > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

mv $TMPDIR/output $OUTPUT_FILE
