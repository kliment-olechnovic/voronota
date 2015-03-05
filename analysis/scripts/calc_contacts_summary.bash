#!/bin/bash

set +e

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

BINDIR=""
WORKDIR=""
MIN_SEQSEP_OPTION=""
MAX_SEQSEP_OPTION=""
WEIGHTED=false
SUMMARY_NAME="summary"

while getopts "b:d:x:y:wn:" OPTION
do
	case $OPTION in
	h)
		echo "No help available, read sript file." 1>&2
		exit 0
		;;
	b)
		BINDIR=$OPTARG
		;;
	d)
		WORKDIR=$OPTARG
		;;
	x)
		MIN_SEQSEP_OPTION="--match-min-seq-sep $OPTARG"
		;;
	y)
		MAX_SEQSEP_OPTION="--match-max-seq-sep $OPTARG"
		;;
	w)
		WEIGHTED=true
		;;
	n)
		SUMMARY_NAME=$OPTARG
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

if [ -z "$WORKDIR" ]
then
	echo "Missing working directory." 1>&2
	exit 1
fi

if [ ! -s "$BINDIR/voronota" ]
then
	echo "Invalid binaries directory." 1>&2
	exit 1
fi

if [ ! -s "$WORKDIR/contacts" ]
then
	echo "Invalid working directory." 1>&2
	exit 1
fi

MULTCOEFF="24.0"
if $WEIGHTED
then
	cat $WORKDIR/balls | $BINDIR/voronota query-balls --chains-summary-output $TMPDIR/chains_counts > /dev/null
	MULTCOEFF=$(cat $TMPDIR/chains_counts | head -1 | awk '{print (24.0/$1*$2)}')
fi

cat $WORKDIR/contacts \
| $BINDIR/voronota query-contacts $MIN_SEQSEP_OPTION $MAX_SEQSEP_OPTION \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| $BINDIR/voronota score-contacts-potential --multiply-areas $MULTCOEFF \
> $WORKDIR/$SUMMARY_NAME
