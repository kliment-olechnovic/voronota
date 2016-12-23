#!/bin/bash

SUBDIR=$OUTPUTDIR/volumes_script
mkdir -p $SUBDIR

$VORONOTADIR/voronota-volumes \
  -i $INPUTDIR/single/structure.pdb \
  --input-filter-query "--match r<1:50>" \
  --cache-dir $SUBDIR/cache \
> $SUBDIR/volumes0

$VORONOTADIR/voronota-volumes \
  --input $INPUTDIR/single/structure.pdb \
  --input-filter-query "--match r<1:50>" \
  --atoms-query "--match R<LYS>" \
  --cache-dir $SUBDIR/cache \
  --sum-at-end \
> $SUBDIR/volumes1

$VORONOTADIR/voronota-volumes \
  --input $INPUTDIR/single/structure.pdb \
  --input-filter-query "--match r<1:50>" \
  --atoms-query "--match R<LYS>" \
  --cache-dir $SUBDIR/cache \
  --sum-at-end \
  --per-residue \
> $SUBDIR/volumes2

rm -r $SUBDIR/cache
