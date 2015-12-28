#!/bin/bash

WORKDIR=""

while getopts "d:" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	esac
done

TARGET_WORKDIR=$(dirname $(dirname $WORKDIR))/target

cat $WORKDIR/contacts | grep -v "solvent" | awk '{print $1 " " $2 " " $4}' \
| $BINDIR/voronota compare-contacts \
  --target-contacts-file <(cat $TARGET_WORKDIR/contacts | grep -v "solvent" | awk '{print $1 " " $2 " " $4}') \
  --atom-scores-file $WORKDIR/atom_cad_scores \
  --residue-scores-file $WORKDIR/residue_cad_scores \
  --smoothing-window 3 \
  --smoothed-scores-file $WORKDIR/smoothed_residue_cad_scores \
  --depth 0 \
> $WORKDIR/global_cad_score
