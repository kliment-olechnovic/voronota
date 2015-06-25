#!/bin/bash

set +e

WORKDIR=""
CENTRALITY_OPTION=""
WITH_MOCK_SOLVENT=false

while getopts "d:cs" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	c)
		CENTRALITY_OPTION="--tag-centrality"
		;;
	s)
		WITH_MOCK_SOLVENT=true
		;;
	esac
done

if $WITH_MOCK_SOLVENT
then
	cat $WORKDIR/balls \
	| $BINDIR/voronota calculate-mock-solvent \
	  --solvent-distance 0.8 \
	| $BINDIR/voronota calculate-contacts \
	  --annotated $CENTRALITY_OPTION \
	  --probe 3.0 \
	> $WORKDIR/raw_contacts_with_mock_solvent
else
	cat $WORKDIR/balls \
	| $BINDIR/voronota calculate-contacts \
	  --annotated $CENTRALITY_OPTION \
	> $WORKDIR/raw_contacts
fi
