#!/bin/bash

set +e

WORKDIR=""
CENTRALITY_OPTION=""

while getopts "d:c" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	c)
		CENTRALITY_OPTION="--tag-centrality"
		;;
	esac
done

cat $WORKDIR/balls \
| $BINDIR/voronota calculate-contacts \
  --annotated $CENTRALITY_OPTION \
> $WORKDIR/raw_contacts
