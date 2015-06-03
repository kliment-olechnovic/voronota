#!/bin/bash

BINDIR=""
SCHEDULER=""
CPUCOUNT=""
COMMANDBUNDLE=""
ARGLIST=""
DEPENDENCIES_FILE=""

while getopts "b:s:p:c:a:d:" OPTION
do
	case $OPTION in
	b)
		BINDIR=$OPTARG
		;;
	s)
		SCHEDULER=$OPTARG
		;;
	p)
		CPUCOUNT=$OPTARG
		;;
	c)
		COMMANDBUNDLE=$OPTARG
		;;
	a)
		ARGLIST=$OPTARG
		;;
	d)
		DEPENDENCIES_FILE=$OPTARG
		;;
	esac
done

if [ -z "$BINDIR" ] || [ -z "$SCHEDULER" ] || [ -z "$CPUCOUNT" ] || [ -z "$COMMANDBUNDLE" ]
then
	echo "Missing required scheduling parameters." 1>&2
	exit 1
fi

DEPENDENCIES_OPTION=""
if [ -n "$DEPENDENCIES_FILE" ] && [ -s "$DEPENDENCIES_FILE" ] && [ "$SCHEDULER" != "bash" ]
then
	DEPENDENCIES_STRING=$(cat $DEPENDENCIES_FILE | egrep "^Submitted batch job" | awk '{print $4}' | tr '\n' ':' | sed 's/:$//')
	if [ -n "$DEPENDENCIES_STRING" ]
	then
		DEPENDENCIES_OPTION="--dependency=afterok:<$DEPENDENCIES_STRING>"
	fi
fi

if [ -z "$ARGLIST" ]
then
	$SCHEDULER $DEPENDENCIES_OPTION $BINDIR/run_jobs.bash $BINDIR "$COMMANDBUNDLE"
else
	INCOUNT=$(cat $ARGLIST | wc -l)
	CHUNKSIZE=$(echo "$INCOUNT/$CPUCOUNT" | bc)
	
	if [ "$CHUNKSIZE" -le "0" ]
	then
		CHUNKSIZE="1"
	fi
	
	cat $ARGLIST \
	| xargs -L $CHUNKSIZE \
	$SCHEDULER $DEPENDENCIES_OPTION $BINDIR/run_jobs.bash $BINDIR "$COMMANDBUNDLE"
fi
