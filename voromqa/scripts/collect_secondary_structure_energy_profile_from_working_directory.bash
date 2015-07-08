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

cat $WORKDIR/balls \
| $BINDIR/voronota query-balls --drop-tags --drop-adjuncts --set-dssp-info $WORKDIR/dssp_info \
| $BINDIR/voronota query-balls --match-tags 'dssp=H|dssp=G|dssp=I' --set-tags 'SS=H' \
| $BINDIR/voronota query-balls --match-tags 'dssp=B|dssp=E' --set-tags 'SS=S' \
| $BINDIR/voronota query-balls --match-tags-not 'SS=H|SS=S' --set-tags 'SS=L' \
| $BINDIR/voronota query-balls --set-external-adjuncts <(cat $WORKDIR/atom_energies | awk '{print $1 " " ($4/$2)}') --set-external-adjuncts-name qenergy \
| egrep 'SS=.*phi=.*psi=.*qenergy=.*' \
| sed 's|^\(\S\+\)\s.*SS=\(.\).*phi=\(.\+\);psi=\(.\+\);qenergy=\(.\+\)|\1 \2 \3 \4 \5|' \
| sed 's|^\S*\(R<.\+>A<.\+>\)\s|\1 |' \
> $WORKDIR/secondary_structure_energy_profile
