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

if [ -z "$CASPNAME" ] || [ -z "$TARGETNAME" ] || [ -z "$OUTDIR" ] 
then
	echo "Missing arguments." 1>&2
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

cd -

OUTDIR=$OUTDIR/$CASPNAME$STAGENUM
mkdir -p $OUTDIR

mv $TMPDIR/$TARGETNAME $OUTDIR/$TARGETNAME
mv $TMPDIR/$TARGETNAME.pdb $OUTDIR/$TARGETNAME/$TARGETNAME.pdb
