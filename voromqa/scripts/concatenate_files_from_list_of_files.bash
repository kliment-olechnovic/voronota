#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

INPUT_FILE_LIST=""
OUTPUT_FILE=""
HEADER=false
SORT=false
TABULATE=false
COMPRESS=false

while getopts "i:o:hstz" OPTION
do
	case $OPTION in
	i)
		INPUT_FILE_LIST=$OPTARG
		;;
	o)
		OUTPUT_FILE=$OPTARG
		;;
	h)
		HEADER=true
		;;
	s)
		SORT=true
		;;
	t)
		TABULATE=true
		;;
	z)
		COMPRESS=true
		;;
	esac
done

(cat $INPUT_FILE_LIST | xargs -L 100 -P 1 cat) > $TMPDIR/output

if $HEADER
then
	head -1 $TMPDIR/output > $TMPDIR/header
	cat $TMPDIR/output | egrep -v "$(cat $TMPDIR/header)" > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

if $SORT
then
	cat $TMPDIR/output | sort | uniq > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

if $HEADER
then
	cat $TMPDIR/header $TMPDIR/output > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

if $TABULATE
then
	cat $TMPDIR/output | column -t > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

mv $TMPDIR/output $OUTPUT_FILE

if $COMPRESS
then
	bzip2 $OUTPUT_FILE
fi
