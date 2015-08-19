#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/points_vectorizing
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	cat $INFILE \
	| $VORONOTA get-balls-from-atoms-file --annotated \
	> $SUBDIR/$INFILEBASENAME.points
done

find $SUBDIR/ -type f -name "*.points" \
| sort \
| $VORONOTA vectorize-points \
  --RMSD-matrix $SUBDIR/rmsd_matrix \
  --consensus-list $SUBDIR/consensus_list \
  --clustering-output $SUBDIR/clusters \
  --clustering-threshold 25.0 \
> /dev/null

rm $SUBDIR/*.points
