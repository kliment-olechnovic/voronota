#!/bin/bash

BINDIR=""
SCHEDULER=""
CPUCOUNT=""
COMMANDBUNDLE=""
ARGLIST=""

while getopts "b:s:p:c:a:" OPTION
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
	esac
done

if [ -z "$BINDIR" ] || [ -z "$SCHEDULER" ] || [ -z "$CPUCOUNT" ] || [ -z "$COMMANDBUNDLE" ] || [ -z "$ARGLIST" ]
then
	echo "Missing required scheduling parameters." 1>&2
	exit 1
fi

INCOUNT=$(cat $ARGLIST | wc -l)
CHUNKSIZE=$(echo "$INCOUNT/$CPUCOUNT" | bc)

if [ "$CHUNKSIZE" -le "0" ]
then
	CHUNKSIZE="1"
fi

cat $ARGLIST \
| xargs -L $CHUNKSIZE \
$SCHEDULER $BINDIR/run_jobs.bash $BINDIR "$COMMANDBUNDLE"
