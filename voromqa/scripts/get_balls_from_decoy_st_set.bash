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

INPUTBASENAME=$(basename $INPUTFILE .tar.gz)

cp $INPUTFILE $TMPDIR/input.tar.gz

cd $TMPDIR
tar -xzf ./input.tar.gz
rm ./input.tar.gz
cd - &> /dev/null

find $TMPDIR -mindepth 2 -maxdepth 2 -type f -name "*.pdb.gz" > $TMPDIR/target_filename
find $TMPDIR -mindepth 3 -maxdepth 3 -type f -name "*.pdb.gz" > $TMPDIR/models_filenames

if [ ! -s "$TMPDIR/target_filename" ] || [ ! -s "$TMPDIR/models_filenames" ]
then
	echo "Invalid input archive." 1>&2
	exit 1
fi

$BINDIR/get_balls_from_target_and_models_list.bash -t $(head -1 $TMPDIR/target_filename) -m $TMPDIR/models_filenames -o $OUTDIR/decoy_st_set/$INPUTBASENAME -z
