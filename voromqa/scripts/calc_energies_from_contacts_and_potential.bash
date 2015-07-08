#!/bin/bash

set +e

WORKDIR=""
POTENTIAL=""

while getopts "d:p:" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	p)
		POTENTIAL=$OPTARG
		;;
	esac
done

cat $WORKDIR/contacts \
| $BINDIR/voronota score-contacts-energy \
  --potential-file $POTENTIAL \
  --atom-scores-file $WORKDIR/atom_energies \
> $WORKDIR/global_energy
