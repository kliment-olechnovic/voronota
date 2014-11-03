#!/bin/bash

BINDIR=$(dirname "$0")

INPUTFILE=$1
OUTPUTDIR=$2

if [ ! -s "$INPUTFILE" ] || [ -z "$OUTPUTDIR" ]
then
	echo "Invalid arguments"
	exit 1
fi

mkdir -p $OUTPUTDIR

cat $INPUTFILE \
| $BINDIR/voronota get-balls-from-atoms-file --radii-file $BINDIR/radii --annotated | tee $OUTPUTDIR/balls \
| $BINDIR/voronota calculate-contacts --annotated | tee $OUTPUTDIR/contacts \
| $BINDIR/voronota score-contacts --potential-file $BINDIR/potential --atom-scores-file $OUTPUTDIR/atom_scores --residue-scores-file $OUTPUTDIR/residue_scores \
> $OUTPUTDIR/global_scores

cat $OUTPUTDIR/atom_scores | awk '{ print $1 " " (100-$2*100) }' > $OUTPUTDIR/atom_errors
cat $OUTPUTDIR/residue_scores | awk '{ print $1 " " (100-$2*100) }' > $OUTPUTDIR/residue_errors

cat $OUTPUTDIR/balls \
| $BINDIR/voronota query-balls --set-external-adjuncts $OUTPUTDIR/atom_errors --set-external-adjuncts-name ae --pdb-output-b-factor ae \
--pdb-output-template $INPUTFILE --pdb-output $OUTPUTDIR/atom_errors.pdb > /dev/null

cat $OUTPUTDIR/balls \
| $BINDIR/voronota query-balls --set-external-adjuncts $OUTPUTDIR/residue_errors --set-external-adjuncts-name re --pdb-output-b-factor re \
--pdb-output-template $INPUTFILE --pdb-output $OUTPUTDIR/residue_errors.pdb > /dev/null
