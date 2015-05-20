#!/bin/bash

set +e

WORKDIR=""
MULTAREAS=false

while getopts "d:m" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	m)
		MULTAREAS=true
		;;
	esac
done

MULTAREAS_OPTION=""
if $MULTAREAS
then
	cat $WORKDIR/balls \
	| $BINDIR/voronota query-balls --chains-summary-output $WORKDIR/chains_counts \
	> /dev/null
	MULTAREAS_OPTION="--multiply-areas $(cat $WORKDIR/chains_counts | head -1 | awk '{print (24.0/$1*$2)}')"
fi

cat $WORKDIR/raw_contacts \
| $BINDIR/voronota query-contacts --match-min-seq-sep 1 \
| $BINDIR/voronota query-contacts --drop-tags \
| $BINDIR/voronota query-contacts \
  --match-first 'A<C>' \
  --match-second 'A<N>' \
  --match-max-seq-sep 1 \
  --match-max-dist 1.6 \
  --invert \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 1 \
  --match-max-seq-sep 1 \
  --set-tags 'sep1' \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 2 \
  --no-solvent \
  --set-tags 'sep2' \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| tee $WORKDIR/contacts \
| $BINDIR/voronota score-contacts-potential $MULTAREAS_OPTION \
  --contributions-file $WORKDIR/contributions \
  --single-areas-file $WORKDIR/single_areas \
> $WORKDIR/summary
