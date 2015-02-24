#!/bin/bash

set +e

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

BINDIR=""
INPUT_FILE_LIST=""
RANDOMIZE=false
INPUT_SIZE=""
OUTPUTDIR=""
MIN_SEQSEP_OPTION=""
MAX_SEQSEP_OPTION=""

while getopts "b:i:rs:o:x:y:" OPTION
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
	x)
		MIN_SEQSEP_OPTION="--match-min-seq-sep $OPTARG"
		;;
	y)
		MAX_SEQSEP_OPTION="--match-max-seq-sep $OPTARG"
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
fi

if $RANDOMIZE
then
	cat $INPUT_FILE_LIST | shuf > $TMPDIR/list
else
	cp $INPUT_FILE_LIST $TMPDIR/list
fi

if [ -z "$INPUT_SIZE" ]
then
	mv $TMPDIR/list $OUTPUTDIR/list_in
	true > $OUTPUTDIR/list_not_in
else
	cat $TMPDIR/list | head -n $INPUT_SIZE > $OUTPUTDIR/list_in
	cat $TMPDIR/list | tail -n "+$(($INPUT_SIZE+1))" > $OUTPUTDIR/list_in
fi

if [ ! -s "$OUTPUTDIR/list_in" ]
then
	echo "Empty input list." 1>&2
	exit 1
fi

cat $OUTPUTDIR/list_in | while read IFILE
do
	cat $IFILE \
	| $BINDIR/voronota query-contacts $MIN_SEQSEP_OPTION $MAX_SEQSEP_OPTION \
	| awk '{print $1 " " $2 " " $5 " " $3}' \
	| $BINDIR/voronota score-contacts-potential
done \
| $BINDIR/voronota score-contacts-potential \
> $OUTPUTDIR/summary

cat $OUTPUTDIR/summary \
| awk '{print $1 " " $2 " . " $4}' \
| $BINDIR/voronota score-contacts-potential --potential-file $OUTPUTDIR/potential_without_tags --solvent-factor 2 \
> /dev/null

cat $OUTPUTDIR/summary \
| $BINDIR/voronota score-contacts-potential --potential-file $OUTPUTDIR/potential_with_tags --solvent-factor 2 \
> /dev/null
