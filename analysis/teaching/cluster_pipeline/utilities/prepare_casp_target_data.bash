#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

BINDIR=""
CASPNAME=""
TARGETNAME=""
STAGENUM=""
OUTDIR=""

while getopts "b:c:t:s:o:" OPTION
do
	case $OPTION in
	b)
		BINDIR=$OPTARG
		;;
	c)
		CASPNAME="CASP$OPTARG"
		;;
	t)
		TARGETNAME=$OPTARG
		;;
	s)
		STAGENUM=".stage$OPTARG"
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

if [ -z "$BINDIR" ] || [ -z "$CASPNAME" ] || [ -z "$TARGETNAME" ] || [ -z "$OUTDIR" ] 
then
	echo "Missing arguments." 1>&2
	exit 1
fi

if [ ! -s "$BINDIR/voronota" ]
then
	echo "Invalid binaries directory." 1>&2
	exit 1
fi

TARGETSURL=""
MODELSURL=""

if [ "$CASPNAME" == "CASP10" ]
then
	TARGETSURL="http://predictioncenter.org/download_area/CASP10/targets/casp10.targets_unsplitted.noT0695T0739.tgz"
	MODELSURL="http://predictioncenter.org/download_area/CASP10/server_predictions/$TARGETNAME$STAGENUM.3D.srv.tar.gz"
fi

if [ "$CASPNAME" == "CASP11" ]
then
	TARGETSURL="http://predictioncenter.org/download_area/CASP11/targets/casp11.targets_unsplitted.release11242014.tgz"
	MODELSURL="http://predictioncenter.org/download_area/CASP11/server_predictions/$TARGETNAME$STAGENUM.3D.srv.tar.gz"
fi

if [ -z "$TARGETSURL" ] || [ -z "$MODELSURL" ]
then
	echo "Unsupported CASP number." 1>&2
	exit 1
fi

cd $TMPDIR

TARGETSARCHIVE=$(basename $TARGETSURL)
MODELSARCHIVE=$(basename $MODELSURL)

wget "$TARGETSURL" &> $TMPDIR/$TARGETSARCHIVE.log
wget "$MODELSURL" &> $TMPDIR/$MODELSARCHIVE.log

if [ ! -f "$TARGETSARCHIVE" ] || [ ! -f "$MODELSARCHIVE" ]
then
	echo "Failed to download data." 1>&2
	cat $TMPDIR/$TARGETSARCHIVE.log 1>&2
	cat $TMPDIR/$MODELSARCHIVE.log 1>&2
	exit 1
fi

tar -xzf $TARGETSARCHIVE
tar -xzf $MODELSARCHIVE

if [ ! -f "$TARGETNAME.pdb" ] || [ ! -d "$TARGETNAME" ]
then
	echo "Failed to extract data." 1>&2
	exit 1
fi

cd - &> /dev/null

OUTDIR=$OUTDIR/$CASPNAME$STAGENUM/$TARGETNAME
mkdir -p $OUTDIR/target
mkdir -p $OUTDIR/models

cat $TMPDIR/$TARGETNAME.pdb \
| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated \
| grep -f $BINDIR/standard_atom_names \
| $BINDIR/voronota query-balls --rename-chains --drop-atom-serials --drop-altloc-indicators \
| sort -V | $BINDIR/voronota query-balls --reset-serials \
> $OUTDIR/target/balls

cat $OUTDIR/target/balls | $BINDIR/voronota calculate-contacts --annotated > $OUTDIR/target/contacts

cat $OUTDIR/target/balls | $BINDIR/voronota query-balls --drop-atom-serials | awk '{print $1}' > $TMPDIR/filter

find $TMPDIR/$TARGETNAME -type f -not -empty | while read MODEL
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
