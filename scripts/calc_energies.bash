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

cat $OUTPUTDIR/contacts \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| $BINDIR/voronota score-contacts-energy --potential-file $BINDIR/potential --atom-scores-file $OUTPUTDIR/atom_energies \
> $OUTPUTDIR/global_energy

cat $OUTPUTDIR/atom_energies \
| sed 's/.*\(R<.*>A<.*>\)/\1/' \
| awk '{print $1 " " ($4/$2)}' \
> $OUTPUTDIR/normalized_atom_energies

#####################################################

cat $OUTPUTDIR/contacts_whb \
| awk '{print $1 " " $2 " " $5 " " $3}' \
| $BINDIR/voronota score-contacts-energy --potential-file $BINDIR/potential_whb --atom-scores-file $OUTPUTDIR/atom_energies_whb \
> $OUTPUTDIR/global_energy_whb

cat $OUTPUTDIR/atom_energies_whb \
| sed 's/.*\(R<.*>A<.*>\)/\1/' \
| awk '{print $1 " " ($4/$2)}' \
> $OUTPUTDIR/normalized_atom_energies_whb
