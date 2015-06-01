#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

CASPNAME=""
TARGETNAME=""
STAGENUM=""
OUTDIR=""

while getopts "c:t:s:o:" OPTION
do
	case $OPTION in
	c)
		CASPNAME=$OPTARG
		;;
	t)
		TARGETNAME=$OPTARG
		;;
	s)
		STAGENUM=$OPTARG
		;;
    o)
		OUTDIR=$OPTARG
		;;
	esac
done

if [ -z "$CASPNAME" ] || [ -z "$TARGETNAME" ] || [ -z "$OUTDIR" ] 
then
	echo "Missing arguments." 1>&2
	exit 1
fi

TARGETSURL=""
MODELSURL=""

if [ "$CASPNAME" == "CASP8" ]
then
	TARGETSURL="http://predictioncenter.org/download_area/CASP8/targets/casp8.targ_unsplit.tar.gz"
	MODELSURL="http://predictioncenter.org/download_area/CASP8/server_predictions/$TARGETNAME$STAGENUM.3D.srv.tar.gz"
fi

if [ "$CASPNAME" == "CASP9" ]
then
	TARGETSURL="http://predictioncenter.org/download_area/CASP9/targets/casp9.targ_unsplit.tgz"
	MODELSURL="http://predictioncenter.org/download_area/CASP9/server_predictions/$TARGETNAME$STAGENUM.3D.srv.tar.gz"
fi

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

find $TMPDIR/$TARGETNAME -type f -not -empty > $TMPDIR/models_list

$BINDIR/get_balls_from_target_and_models_list.bash -t $TMPDIR/$TARGETNAME.pdb -m $TMPDIR/models_list -o $OUTDIR/$CASPNAME$STAGENUM/$TARGETNAME
