#!/bin/bash

set +e

WORKDIR=""
ENERGIES_STATS_FILE=""

while getopts "d:e:" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	e)
		ENERGIES_STATS_FILE=$OPTARG
		;;
	esac
done

cat $WORKDIR/atom_energies \
| $BINDIR/voronota score-contacts-quality \
  --default-mean -0.34 \
  --default-sd 0.19 \
  --means-and-sds-file $ENERGIES_STATS_FILE \
  --smoothing-window 5 \
  --atom-scores-file $WORKDIR/atom_quality_scores \
  --residue-scores-file $WORKDIR/residue_quality_scores \
> $WORKDIR/global_quality_score
