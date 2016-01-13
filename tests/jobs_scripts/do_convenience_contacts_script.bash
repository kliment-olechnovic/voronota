#!/bin/bash

SUBDIR=$OUTPUTDIR/convenience_contacts_script
mkdir -p $SUBDIR

$VORONOTADIR/voronota-contacts \
  -i $INPUTDIR/single/structure.pdb \
  -a "--match r<1:50>" \
  -c "--inter-residue" \
  -C $SUBDIR/cache \
> $SUBDIR/contacts0

$VORONOTADIR/voronota-contacts \
  -i $INPUTDIR/single/structure.pdb \
  -a "--match r<1:50>" \
  -c "--inter-residue --match-first R<LYS>" \
  -C $SUBDIR/cache \
> $SUBDIR/contacts1

$VORONOTADIR/voronota-contacts \
  -i $INPUTDIR/single/structure.pdb \
  -a "--match r<1:50>" \
  -c "--inter-residue --match-first R<LEU>" \
  -C $SUBDIR/cache \
> $SUBDIR/contacts2

rm -r $SUBDIR/cache
