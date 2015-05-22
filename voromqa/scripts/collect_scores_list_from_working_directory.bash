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

QSCORE1="QSCORE1_NA"
QSCORE2="QSCORE2_NA"
CADSCORE1="CADSCORE1_NA"
CADSCORE2="CADSCORE2_NA"
GOAPSCORE1="GOAPSCORE1_NA"
GOAPSCORE2="GOAPSCORE2_NA"
GOAPSCORE3="GOAPSCORE3_NA"
TMSCORE="TMSCORE_NA"

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

if [ -s "$WORKDIR/goap_scores" ]
then
	GOAPSCORE1=$(cat $WORKDIR/goap_scores | awk '{print $3}')
	GOAPSCORE2=$(cat $WORKDIR/goap_scores | awk '{print $4}')
	GOAPSCORE3=$(cat $WORKDIR/goap_scores | awk '{print $5}')
fi

if [ -s "$WORKDIR/tmscore_output" ]
then
	TMSCORE=$(cat $WORKDIR/tmscore_output | egrep '^TM-score' | awk '{print $3}')
fi

echo "$WORKDIR $QSCORE1 $QSCORE2 $CADSCORE1 $CADSCORE2 $GOAPSCORE1 $GOAPSCORE2 $GOAPSCORE3 $TMSCORE" > $WORKDIR/scores_list
