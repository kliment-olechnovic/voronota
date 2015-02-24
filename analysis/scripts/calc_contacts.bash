#!/bin/bash

set +e

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

BINDIR=""
INPUT_FILE_PATH=""
INPUT_URL=false
INPUT_PRINT_COMMAND="cat"
OUTPUTDIR=""
RUN_HBPLUS=false

while getopts "b:i:uzo:p" OPTION
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
		INPUT_FILE_PATH=$OPTARG
		;;
	u)
		INPUT_URL=true
		;;
	z)
		INPUT_PRINT_COMMAND="zcat"
		;;
    o)
		OUTPUTDIR=$OPTARG
		;;
	p)
		RUN_HBPLUS=true
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

if [ -z "$INPUT_FILE_PATH" ]
then
	echo "Missing input file path." 1>&2
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

if $INPUT_URL
then
	wget -O $TMPDIR/download "$INPUT_FILE_PATH" &> /dev/null
	$INPUT_PRINT_COMMAND $TMPDIR/download > $TMPDIR/input.pdb
else
	$INPUT_PRINT_COMMAND $INPUT_FILE_PATH > $TMPDIR/input.pdb
fi

if [ ! -s "$TMPDIR/input.pdb" ]
then
	echo "Failed to extract input file." 1>&2
	exit 1
fi

mkdir -p "$OUTPUTDIR"

if [ ! -d "$OUTPUTDIR" ]
then
	echo "Could not find or create output directory." 1>&2
fi

cat $TMPDIR/input.pdb \
| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated \
| $BINDIR/voronota query-balls --drop-altloc-indicators --drop-atom-serials --drop-adjuncts --drop-tags \
| sed 's/A<OXT>/A<O>/g' \
| grep -f $BINDIR/standard_atom_names \
> $OUTPUTDIR/balls

if [ ! -s "$OUTPUTDIR/balls" ]
then
	echo "Input set of atoms is empty." 1>&2
	exit 1
fi

cat $OUTPUTDIR/balls \
| $BINDIR/voronota calculate-contacts --annotated \
> $OUTPUTDIR/contacts

if $RUN_HBPLUS
then
	cp $BINDIR/hbplus $TMPDIR/hbplus
	cd $TMPDIR
	./hbplus ./input.pdb > /dev/null 2> /dev/null
	cd - &> /dev/null
	
	cp $TMPDIR/input.hb2 $OUTPUTDIR/hbplus_output

	cat $OUTPUTDIR/contacts \
	| $BINDIR/voronota query-contacts --set-hbplus-tags $OUTPUTDIR/hbplus_output --inter-residue-hbplus-tags \
	> $OUTPUTDIR/contacts_whb

	mv $OUTPUTDIR/contacts_whb $OUTPUTDIR/contacts
fi
