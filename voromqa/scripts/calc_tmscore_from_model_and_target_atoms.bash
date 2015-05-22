#!/bin/bash

WORKDIR=""

while getopts "d:" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	esac
done

TARGET_WORKDIR=$(dirname $(dirname $WORKDIR))/target

$BINDIR/TMscore $WORKDIR/atoms.pdb $TARGET_WORKDIR/atoms.pdb > $WORKDIR/tmscore_output
