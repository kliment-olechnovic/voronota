#!/bin/bash

set +e

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

INPUT_FILE_PATH=""
INPUT_URL=false
INPUT_PRINT_COMMAND="cat"
MULTIMODEL_CHAINS_OPTION=""
OUTPUTBASEDIR=""

while getopts "i:uzmo:" OPTION
do
	case $OPTION in
	i)
		INPUT_FILE_PATH=$OPTARG
		;;
	u)
		INPUT_URL=true
		;;
	z)
		INPUT_PRINT_COMMAND="zcat"
		;;
	m)
		MULTIMODEL_CHAINS_OPTION="--multimodel-chains"
		;;
    o)
		OUTPUTBASEDIR=$OPTARG
		;;
	esac
done

if $INPUT_URL
then
	wget -O $TMPDIR/download "$INPUT_FILE_PATH" &> /dev/null
	$INPUT_PRINT_COMMAND $TMPDIR/download > $TMPDIR/input.pdb
else
	$INPUT_PRINT_COMMAND $INPUT_FILE_PATH > $TMPDIR/input.pdb
fi

if [ ! -s "$TMPDIR/input.pdb" ]
then
	echo "Failed to get input file." 1>&2
	exit 1
fi

WORKDIR=$OUTPUTBASEDIR/$(basename $INPUT_FILE_PATH)
mkdir -p $WORKDIR

cat $TMPDIR/input.pdb \
| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated $MULTIMODEL_CHAINS_OPTION \
| grep -f $BINDIR/standard_names \
| $BINDIR/voronota query-balls --drop-altloc-indicators \
> $WORKDIR/balls
