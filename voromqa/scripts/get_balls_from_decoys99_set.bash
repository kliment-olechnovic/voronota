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

find $TMPDIR/$INPUTBASENAME/ -type f -name min.*.pdb -not -empty > $TMPDIR/models_list

$BINDIR/get_balls_from_target_and_models_list.bash -t $TMPDIR/$INPUTBASENAME/nat.pdb -m $TMPDIR/models_list -o $OUTDIR/decoys99/$INPUTBASENAME
