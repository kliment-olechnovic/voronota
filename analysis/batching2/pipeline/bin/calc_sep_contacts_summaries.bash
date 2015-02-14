#!/bin/bash

readonly BINDIR=$1
readonly OUTPUTDIRBASE=$2
readonly CONTACTSNAME=$3
readonly INPUTNAME=$4

#####################################################

readonly OUTPUTDIR=$OUTPUTDIRBASE/$(basename $INPUTNAME)

if [ ! -d $OUTPUTDIR ]
then
	exit 1
fi

#####################################################

for SEQSEP in {1..10}
do
cat $OUTPUTDIR/$CONTACTSNAME \
| $BINDIR/voronota query-contacts --match-min-seq-sep $SEQSEP --match-max-seq-sep $SEQSEP \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| $BINDIR/voronota score-contacts-potential \
> "${OUTPUTDIR}/${CONTACTSNAME}_seqsep_${SEQSEP}_summary"
done

cat $OUTPUTDIR/$CONTACTSNAME \
| $BINDIR/voronota query-contacts --match-min-seq-sep 11 \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| $BINDIR/voronota score-contacts-potential \
> "${OUTPUTDIR}/${CONTACTSNAME}_seqsep_11_summary"
