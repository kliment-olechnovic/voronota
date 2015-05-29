#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

INPUTFILE=""
OUTDIR=""

while getopts "i:o:" OPTION
do
	case $OPTION in
	i)
		INPUTFILE=$OPTARG
		;;
    o)
		OUTDIR=$OPTARG
		;;
	esac
done

INPUTBASENAME=$(basename $INPUTFILE .tar.bz2)

cp $INPUTFILE $TMPDIR/input.tar.bz2

cd $TMPDIR
tar -xjf $TMPDIR/input.tar.bz2
cd - &> /dev/null

if [ ! -d "$TMPDIR/$INPUTBASENAME" ] || [ ! -s "$TMPDIR/$INPUTBASENAME/nat.pdb" ] || [ ! -s "$TMPDIR/$INPUTBASENAME/min.1.pdb" ]
then
	echo "Invalid input archive." 1>&2
	exit 1
fi

OUTDIR=$OUTDIR/$INPUTBASENAME
mkdir -p $OUTDIR/target
mkdir -p $OUTDIR/models

cat $TMPDIR/$INPUTBASENAME/nat.pdb \
| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated \
| grep -f $BINDIR/standard_names \
| $BINDIR/voronota query-balls --rename-chains --drop-atom-serials --drop-altloc-indicators \
| sort -V \
> $OUTDIR/target/balls

cat $OUTDIR/target/balls | awk '{print $1}' > $TMPDIR/filter

find $TMPDIR/$INPUTBASENAME/ -type f -name min.*.pdb -not -empty | while read MODEL
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
