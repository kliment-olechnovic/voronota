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

INPUTDIR=$(dirname $INPUTFILE)
INPUTBASENAME=$(basename $INPUTFILE .pdb)

find $INPUTDIR -type f -name "${INPUTBASENAME}*.pdb" -not -empty | grep -v $INPUTFILE > $TMPDIR/models_list

$BINDIR/get_balls_from_target_and_models_list.bash -t $INPUTFILE -m $TMPDIR/models_list -o $OUTDIR/$INPUTBASENAME
