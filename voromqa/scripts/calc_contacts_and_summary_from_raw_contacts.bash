#!/bin/bash

WORKDIR=""
MULTAREAS=false
WITH_MOCK_SOLVENT=false

while getopts "d:ms" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	m)
		MULTAREAS=true
		;;
	s)
		WITH_MOCK_SOLVENT=true
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

RAW_CONTACTS_FILE="$WORKDIR/raw_contacts"
if $WITH_MOCK_SOLVENT
then
	RAW_CONTACTS_FILE="$WORKDIR/raw_contacts_with_mock_solvent"
fi

cat $RAW_CONTACTS_FILE \
| $BINDIR/voronota query-contacts --match-min-seq-sep 1 \
| $BINDIR/voronota query-contacts \
  --match-first 'A<C>' \
  --match-second 'A<N>' \
  --match-max-seq-sep 1 \
  --match-max-dist 1.6 \
  --invert \
> ${RAW_CONTACTS_FILE}_filtered

cat ${RAW_CONTACTS_FILE}_filtered \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 1 \
  --match-max-seq-sep 1 \
  --set-tags 'sep1' \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 2 \
  --no-solvent \
  --set-tags 'sep2' \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| tr ';' '_' \
> $WORKDIR/contacts

cat ${RAW_CONTACTS_FILE}_filtered \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 1 \
  --match-max-seq-sep 6 \
  --set-tags 'sep1' \
| $BINDIR/voronota query-contacts \
  --match-min-seq-sep 7 \
  --no-solvent \
  --set-tags 'sep2' \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| tr ';' '_' \
| $BINDIR/voronota score-contacts-potential $MULTAREAS_OPTION \
  --contributions-file $WORKDIR/contributions \
  --single-areas-file $WORKDIR/single_areas \
> $WORKDIR/summary
