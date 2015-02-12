#!/bin/bash

readonly BINDIR=$1
readonly OUTPUTDIRBASE=$2
readonly INPUTNAME=$3

#####################################################

readonly OUTPUTDIR=$OUTPUTDIRBASE/$(basename $INPUTNAME)

if [ ! -d $OUTPUTDIR ]
then
	exit 1
fi

#####################################################

cat $OUTPUTDIR/atom_energies \
| $BINDIR/voronota score-contacts-quality \
  --default-mean 0.32 \
  --default-sd 0.15 \
  --means-and-sds-file $BINDIR/normalized_atom_energies_distributions_parameters \
  --mean-shift 0 \
  --smoothing-window 5 \
  --atom-scores-file $OUTPUTDIR/atom_qscores \
  --residue-scores-file $OUTPUTDIR/residue_qscores \
> $OUTPUTDIR/atom_qscores_average

#####################################################

cat $OUTPUTDIR/atom_energies_whb \
| $BINDIR/voronota score-contacts-quality \
  --default-mean 0.14 \
  --default-sd 0.19 \
  --means-and-sds-file $BINDIR/normalized_atom_energies_whb_distributions_parameters \
  --mean-shift 0 \
  --smoothing-window 5 \
  --atom-scores-file $OUTPUTDIR/atom_qscores_whb \
  --residue-scores-file $OUTPUTDIR/residue_qscores_whb \
> $OUTPUTDIR/atom_qscores_whb_average
