#!/bin/bash

BINDIR=""
SCHEDULER=""
CPUCOUNT=""
COMMANDBUNDLE=""
ARGLIST=""
DEPENDENCIES_FILE=""
LOG_OUTPUT_DIR=""
WRAP_COMMANDBUNDLE=false

while getopts "b:s:p:c:a:d:l:w" OPTION
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
	l)
		LOG_OUTPUT_DIR=$OPTARG
		;;
	w)
		WRAP_COMMANDBUNDLE=true
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
		DEPENDENCIES_OPTION="--dependency=afterok:$DEPENDENCIES_STRING"
	fi
fi

LOG_OUTPUT_OPTION=""
if [ -n "$LOG_OUTPUT_DIR" ] && [ "$SCHEDULER" != "bash" ]
then
	mkdir -p $LOG_OUTPUT_DIR
	LOG_OUTPUT_OPTION="--output=$LOG_OUTPUT_DIR/log_%j"
fi

if $WRAP_COMMANDBUNDLE
then
$SCHEDULER $DEPENDENCIES_OPTION $LOG_OUTPUT_OPTION <<EOF
#!/bin/bash
$COMMANDBUNDLE
EOF
exit
fi

if [ -z "$ARGLIST" ]
then
	$SCHEDULER $DEPENDENCIES_OPTION $LOG_OUTPUT_OPTION $BINDIR/run_jobs.bash $BINDIR "$COMMANDBUNDLE"
else
	INCOUNT=$(cat $ARGLIST | wc -l)
	CHUNKSIZE=$(echo "$INCOUNT/$CPUCOUNT" | bc)
	
	if [ "$CHUNKSIZE" -le "0" ]
	then
		CHUNKSIZE="1"
	fi
	
	cat $ARGLIST \
	| xargs -L $CHUNKSIZE \
	$SCHEDULER $DEPENDENCIES_OPTION $LOG_OUTPUT_OPTION $BINDIR/run_jobs.bash $BINDIR "$COMMANDBUNDLE"
fi
