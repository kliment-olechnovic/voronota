#!/bin/bash

MAXCUT=$1
STEPCUT=$2
INFILE=$3

ALLOWED_SCORE_ERROR="0.01"

readonly TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

cat $INFILE \
| ./voronota get-balls-from-atoms-file --annotated \
| ./voronota query-balls --seq-output "$TMPLDIR/sequence" \
> /dev/null

CUT1="1"
CUT2="1"
cat "$TMPLDIR/sequence" | cut -c${CUT1}- | rev | cut -c${CUT2}- | rev > "$TMPLDIR/sequence_cut_00"
SCORE_00=$(./voronota-voromqa -i "$INFILE" --reference-sequence "$TMPLDIR/sequence_cut_00" | awk '{print $2}')

MAX_SCORE="$SCORE_00"
MAX_SCORE_CUT1="$CUT1"
MAX_SCORE_CUT2="$CUT2"

PROGRESS=true
while [ "$CUT1" -le "$MAXCUT" ] && [ "$CUT2" -le "$MAXCUT" ] && $PROGRESS
do
	echo $CUT1 $CUT2 $SCORE_00
	
	cat "$TMPLDIR/sequence" | cut -c$(echo ${CUT1}+${STEPCUT} | bc)- | rev | cut -c${CUT2}- | rev > "$TMPLDIR/sequence_cut_10"
	SCORE_10=$(./voronota-voromqa -i "$INFILE" --reference-sequence "$TMPLDIR/sequence_cut_10" | awk '{print $2}')
	
	cat "$TMPLDIR/sequence" | cut -c${CUT1}- | rev | cut -c$(echo ${CUT2}+${STEPCUT} | bc)- | rev > "$TMPLDIR/sequence_cut_01"
	SCORE_01=$(./voronota-voromqa -i "$INFILE" --reference-sequence "$TMPLDIR/sequence_cut_01" | awk '{print $2}')
	
	cat "$TMPLDIR/sequence" | cut -c$(echo ${CUT1}+${STEPCUT} | bc)- | rev | cut -c$(echo ${CUT2}+${STEPCUT} | bc)- | rev > "$TMPLDIR/sequence_cut_11"
	SCORE_11=$(./voronota-voromqa -i "$INFILE" --reference-sequence "$TMPLDIR/sequence_cut_11" | awk '{print $2}')
	
	if [ "$(echo ${SCORE_10}+${ALLOWED_SCORE_ERROR}'>='${MAX_SCORE} | bc -l)" == "1" ] && [ "$(echo ${SCORE_10}'>='${SCORE_01} | bc -l)" == "1" ] && [ "$(echo ${SCORE_10}'>='${SCORE_11} | bc -l)" == "1" ]
	then
		SCORE_00="$SCORE_10"
		CUT1=$(echo ${CUT1}+${STEPCUT} | bc)
	elif [ "$(echo ${SCORE_01}+${ALLOWED_SCORE_ERROR}'>='${MAX_SCORE} | bc -l)" == "1" ] && [ "$(echo ${SCORE_01}'>='${SCORE_10} | bc -l)" == "1" ] && [ "$(echo ${SCORE_01}'>='${SCORE_11} | bc -l)" == "1" ]
	then
		SCORE_00="$SCORE_01"
		CUT2=$(echo ${CUT2}+${STEPCUT} | bc)
	elif [ "$(echo ${SCORE_11}+${ALLOWED_SCORE_ERROR}'>='${MAX_SCORE} | bc -l)" == "1" ] && [ "$(echo ${SCORE_11}'>='${SCORE_10} | bc -l)" == "1" ] && [ "$(echo ${SCORE_11}'>='${SCORE_01} | bc -l)" == "1" ]
	then
		SCORE_00="$SCORE_11"
		CUT1=$(echo ${CUT1}+${STEPCUT} | bc)
		CUT2=$(echo ${CUT2}+${STEPCUT} | bc)
	else
		PROGRESS=false
	fi
	
	if [ "$(echo ${SCORE_00}'>'${MAX_SCORE} | bc -l)" == "1" ]
	then
		MAX_SCORE="$SCORE_00"
		MAX_SCORE_CUT1="$CUT1"
		MAX_SCORE_CUT2="$CUT2"
	fi
done

echo $MAX_SCORE_CUT1 $MAX_SCORE_CUT2 $MAX_SCORE
