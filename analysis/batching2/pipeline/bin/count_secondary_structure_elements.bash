#!/bin/bash

readonly BINDIR=$1
readonly OUTPUTDIRBASE=$2
readonly INPUTFILE=$3

#####################################################

readonly INPUTBASENAME=$(basename $INPUTFILE)
readonly OUTPUTDIR=$OUTPUTDIRBASE/$INPUTBASENAME

if [ ! -d $OUTPUTDIR ]
then
	exit 1
fi

#####################################################

readonly TMPDIR=$(mktemp -d)

$BINDIR/dssp $INPUTFILE | grep -A 9999999 "#  RESIDUE AA STRUCTURE" | tail -n +2 | egrep -v '!' | cut -c 17-17 > $TMPDIR/classes

readonly ALPHACOUNT=$(cat $TMPDIR/classes | egrep 'H|G|I' | wc -l)
readonly BETACOUNT=$(cat $TMPDIR/classes | egrep 'B|E' | wc -l)
readonly LOOPSCOUNT=$(cat $TMPDIR/classes | egrep -v 'H|G|I|B|E' | wc -l)

echo $ALPHACOUNT $BETACOUNT $LOOPSCOUNT > $OUTPUTDIR/secondary_structure_element_counts

rm -r $TMPDIR
