#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

TARGETFILE=""
MODELSLISTFILE=""
OUTDIR=""

while getopts "t:m:o:" OPTION
do
	case $OPTION in
	t)
		TARGETFILE=$OPTARG
		;;
	m)
		MODELSLISTFILE=$OPTARG
		;;
    o)
		OUTDIR=$OPTARG
		;;
	esac
done

if [ ! -s "$TARGETFILE" ] || [ ! -s "$MODELSLISTFILE" ]
then
	echo "Invalid target and models list input." 1>&2
	exit 1
fi

mkdir -p $OUTDIR/target
mkdir -p $OUTDIR/models

cat $TARGETFILE \
| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated \
| grep -f $BINDIR/standard_names \
| $BINDIR/voronota query-balls --rename-chains --drop-atom-serials --drop-altloc-indicators \
| sort -V \
> $OUTDIR/target/balls

cat $OUTDIR/target/balls | awk '{print $1}' > $TMPDIR/filter

cat $MODELSLISTFILE | while read MODEL
do
	cat $MODEL \
	| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated \
	| grep -f $BINDIR/standard_names \
	| $BINDIR/voronota query-balls --rename-chains --drop-atom-serials --drop-altloc-indicators \
	| $BINDIR/voronota query-balls --match-external-annotations $TMPDIR/filter \
	| sort -V \
	> $TMPDIR/filtered
	
	if [ -s "$TMPDIR/filtered" ] && [ "$(cat $OUTDIR/target/balls | wc -l)" -eq "$(cat $TMPDIR/filtered | wc -l)" ]
	then
		MODELNAME=$(basename $MODEL)
		mkdir -p $OUTDIR/models/$MODELNAME
		mv $TMPDIR/filtered $OUTDIR/models/$MODELNAME/balls
	fi
done
