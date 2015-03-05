#!/bin/bash

LISTFILE=""
ENDING=""
OUTFILE=""

while getopts "l:e:o:" OPTION
do
	case $OPTION in
	l)
		LISTFILE=$OPTARG
		;;
	e)
		ENDING=$OPTARG
		;;
	o)
		OUTFILE=$OPTARG
		;;
	?)
		echo "Unrecognized option." 1>&2
		exit 1
		;;
	esac
done

mkdir -p $(dirname $OUTFILE)

cat $(cat $LISTFILE | awk -v ending="$ENDING" '{print $1 $ending}') > $OUTFILE
