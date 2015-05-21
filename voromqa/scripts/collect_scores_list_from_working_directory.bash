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

QSCORE1="NA"
QSCORE2="NA"
CADSCORE1="NA"
CADSCORE2="NA"

if [ -s "$WORKDIR/global_quality_score" ]
then
	QSCORE1=$(cat $WORKDIR/global_quality_score | awk '{print $1}')
	QSCORE2=$(cat $WORKDIR/global_quality_score | awk '{print $2}')
fi

if [ -s "$WORKDIR/global_cad_score" ]
then
	CADSCORE1=$(cat $WORKDIR/global_cad_score | grep atom | awk '{print $2}')
	CADSCORE2=$(cat $WORKDIR/global_cad_score | grep residue | awk '{print $2}')
fi

echo "$WORKDIR $QSCORE1 $QSCORE2 $CADSCORE1 $CADSCORE2" > $WORKDIR/scores_list
