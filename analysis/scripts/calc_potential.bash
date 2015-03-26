#!/bin/bash

set +e

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

BINDIR=""
INPUT_FILE_LIST=""
RANDOMIZE=false
INPUT_SIZE=""
OUTPUTDIR=""
FIXED_TYPES_WITHOUT_TAGS_OPTION=""
FIXED_TYPES_WITH_TAGS_OPTION=""

while getopts "b:i:rs:o:t" OPTION
do
	case $OPTION in
	h)
		echo "No help available, read sript file." 1>&2
		exit 0
		;;
	b)
		BINDIR=$OPTARG
		;;
	i)
		INPUT_FILE_LIST=$OPTARG
		;;
	r)
		RANDOMIZE=true
		;;
	s)
		INPUT_SIZE=$OPTARG
		;;
    o)
		OUTPUTDIR=$OPTARG
		;;
	t)
		FIXED_TYPES_WITHOUT_TAGS_OPTION="--input-fixed-types $BINDIR/standard_interactions_without_tags"
		FIXED_TYPES_WITH_TAGS_OPTION="--input-fixed-types $BINDIR/standard_interactions_with_tags"
		;;
	?)
		echo "Unrecognized option." 1>&2
		exit 1
		;;
	esac
done

if [ -z "$BINDIR" ]
then
	echo "Missing binaries directory." 1>&2
	exit 1
fi

if [ -z "$INPUT_FILE_LIST" ]
then
	echo "Missing input file list." 1>&2
	exit 1
fi

if [ -z "$OUTPUTDIR" ]
then
	echo "Missing output directory." 1>&2
	exit 1
fi

if [ ! -s "$BINDIR/voronota" ]
then
	echo "Invalid binaries directory." 1>&2
	exit 1
fi

if [ ! -s "$INPUT_FILE_LIST" ]
then
	echo "Could not find input file list." 1>&2
	exit 1
fi

mkdir -p "$OUTPUTDIR"

if [ ! -d "$OUTPUTDIR" ]
then
	echo "Could not find or create output directory." 1>&2
	exit 1
fi

if $RANDOMIZE
then
	RANDOMDIR=$(mktemp --tmpdir=$OUTPUTDIR -d)
	if [ ! -d "$RANDOMDIR" ]
	then
		echo "Could not create random output directory." 1>&2
		exit 1
	fi
	OUTPUTDIR=$RANDOMDIR
	cat $INPUT_FILE_LIST | shuf > $TMPDIR/list
else
	cp $INPUT_FILE_LIST $TMPDIR/list
fi

if [ -z "$INPUT_SIZE" ]
then
	cat $TMPDIR/list | sort > $OUTPUTDIR/list_in
	true > $OUTPUTDIR/list_not_in
else
	cat $TMPDIR/list | head -n $INPUT_SIZE | sort > $OUTPUTDIR/list_in
	cat $TMPDIR/list | tail -n "+$(($INPUT_SIZE+1))" | sort > $OUTPUTDIR/list_not_in
fi

if [ ! -s "$OUTPUTDIR/list_in" ]
then
	echo "Empty input list." 1>&2
	exit 1
fi

cat $OUTPUTDIR/list_in \
| $BINDIR/voronota score-contacts-potential --input-file-list \
> $OUTPUTDIR/summary

cat $OUTPUTDIR/summary \
| $BINDIR/voronota score-contacts-potential --potential-file $OUTPUTDIR/potential_with_tags $FIXED_TYPES_WITH_TAGS_OPTION \
> $OUTPUTDIR/summary_with_tags

rm $OUTPUTDIR/summary
