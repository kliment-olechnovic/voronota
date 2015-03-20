#!/bin/bash

set +e

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

BINDIR=""
WORKDIR=""
WITH_TAGS=false
POTENTIAL=""
IGNORABLE_MAX_SEQSEP="1"
DEPTH="2"
OUTPUT_NAME="energy"
QUALITY_PARAMS=""
STATISTICAL=false

while getopts "b:d:tp:y:e:n:q:s" OPTION
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
	t)
		WITH_TAGS=true
		;;
	p)
		POTENTIAL=$OPTARG
		;;
	y)
		IGNORABLE_MAX_SEQSEP=$OPTARG
		;;
	e)
		DEPTH=$OPTARG
		;;
	n)
		OUTPUT_NAME=$OPTARG
		;;
	q)
		QUALITY_PARAMS=$OPTARG
		;;
	s)
		STATISTICAL=true
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

if [ -z "$POTENTIAL" ]
then
	echo "Missing potential file." 1>&2
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

if [ ! -s "$POTENTIAL" ]
then
	echo "Invalid potential file." 1>&2
	exit 1
fi

if $WITH_TAGS
then
	cat $WORKDIR/contacts | awk '{print $1 " " $2 " " $5 " " $3}' > $TMPDIR/contacts
else
	cat $WORKDIR/contacts | awk '{print $1 " " $2 " . " $3}' > $TMPDIR/contacts
fi

cat $TMPDIR/contacts \
| $BINDIR/voronota score-contacts-energy \
--potential-file $POTENTIAL \
--ignorable-max-seq-sep $IGNORABLE_MAX_SEQSEP \
--depth $DEPTH \
--atom-scores-file $WORKDIR/$OUTPUT_NAME"_atoms" \
> $WORKDIR/$OUTPUT_NAME"_global"

if [ -n "$QUALITY_PARAMS" ]
then
	cat $WORKDIR/$OUTPUT_NAME"_atoms" \
	| $BINDIR/voronota score-contacts-quality \
	--means-and-sds-file $QUALITY_PARAMS \
	--smoothing-window 5 \
	--residue-scores-file $WORKDIR/$OUTPUT_NAME"_residues_quality" \
	> $WORKDIR/$OUTPUT_NAME"_global_quality"
fi

if $STATISTICAL
then
	cat $WORKDIR/balls | $BINDIR/voronota query-balls --chains-summary-output $TMPDIR/chains_counts > /dev/null
	cat $TMPDIR/chains_counts | tail -n +2 | awk '{print "c<" $1 ">"}' > $TMPDIR/representative_chains
	
	cat $WORKDIR/$OUTPUT_NAME"_atoms" \
	| grep -f $TMPDIR/representative_chains \
	| sed 's/.*\(R<.*>A<.*>\)/\1/' \
	| awk '{print $1 " " ($4/$2)}' \
	> $TMPDIR/normalized_energy_atoms
	
	mv $TMPDIR/normalized_energy_atoms $WORKDIR/$OUTPUT_NAME"_atoms"
	
	if [ -n "$QUALITY_PARAMS" ]
	then
		cat $WORKDIR/$OUTPUT_NAME"_residues_quality" \
		| sed 's/.*\(R<.*>\)/\1/' \
		> $TMPDIR/unnumbered_residues_quality
		
		mv $TMPDIR/unnumbered_residues_quality $WORKDIR/$OUTPUT_NAME"_residues_quality"
	fi
fi
