#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

BINDIR=""
TARGETDIR=""
OUTDIR=""

while getopts "b:t:o:" OPTION
do
	case $OPTION in
	b)
		BINDIR=$OPTARG
		;;
	t)
		TARGETDIR=$OPTARG
		;;
	o)
		OUTDIR=$OPTARG
		;;
	?)
		echo "Unrecognized option." 1>&2
		exit 1
		;;
	esac
done

if [ -z "$BINDIR" ] || [ -z "$TARGETDIR" ] || [ -z "$OUTDIR" ]
then
	echo "Missing arguments." 1>&2
	exit 1
fi

if [ ! -s "$BINDIR/voronota" ]
then
	echo "Invalid binaries directory." 1>&2
	exit 1
fi

TARGETNAME=$(basename $TARGETDIR)
SETNAME=$(basename $(dirname $TARGETDIR))

if [ ! -s "$TARGETDIR/$TARGETNAME.pdb" ]
then
	echo "Invalid target directory." 1>&2
	exit 1
fi

OUTDIR=$OUTDIR/$SETNAME/$TARGETNAME
mkdir -p $OUTDIR/target
mkdir -p $OUTDIR/models

cat $TARGETDIR/$TARGETNAME.pdb \
| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated \
| grep -f $BINDIR/standard_atom_names \
| $BINDIR/voronota query-balls --rename-chains --drop-atom-serials --drop-altloc-indicators \
| sort -V | $BINDIR/voronota query-balls --reset-serials \
> $OUTDIR/target/balls

cat $OUTDIR/target/balls | $BINDIR/voronota calculate-contacts --annotated > $OUTDIR/target/contacts

cat $OUTDIR/target/balls | $BINDIR/voronota query-balls --drop-atom-serials | awk '{print $1}' > $TMPDIR/filter

find $TARGETDIR -type f -name "*.pdb" -not -empty | grep -v "$TARGETDIR/$TARGETNAME.pdb" | while read MODEL
do
	cat $MODEL \
	| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated \
	| grep -f $BINDIR/standard_atom_names \
	| $BINDIR/voronota query-balls --rename-chains --drop-atom-serials --drop-altloc-indicators \
	| $BINDIR/voronota query-balls --match-external-annotations $TMPDIR/filter \
	| sort -V | $BINDIR/voronota query-balls --reset-serials \
	> $TMPDIR/filtered
	
	if [ -s "$TMPDIR/filtered" ] && [ "$(cat $OUTDIR/target/balls | wc -l)" -eq "$(cat $TMPDIR/filtered | wc -l)" ]
	then
		MODELNAME=$(basename $MODEL)
		mkdir -p $OUTDIR/models/$MODELNAME
		mv $TMPDIR/filtered $OUTDIR/models/$MODELNAME/balls
		cat $OUTDIR/models/$MODELNAME/balls | $BINDIR/voronota calculate-contacts --annotated > $OUTDIR/models/$MODELNAME/contacts
	fi
done
