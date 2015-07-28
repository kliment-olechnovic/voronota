#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/contacts_vectorizing
mkdir -p $SUBDIR

for INFILE in $INPUTDIR/complex/*.pdb
do
	INFILEBASENAME=$(basename $INFILE .pdb)
	cat $INFILE \
	| $VORONOTA get-balls-from-atoms-file \
	  --radii-file $VORONOTADIR/resources/radii \
	  --annotated \
	| $VORONOTA calculate-contacts \
	  --annotated \
	| $VORONOTA query-contacts \
	  --inter-residue \
	  --no-same-chain \
	  --no-solvent \
	> $SUBDIR/$INFILEBASENAME.contacts
done

find $SUBDIR/ -type f -name "*.contacts" \
| sort \
| $VORONOTA vectorize-contacts \
> $SUBDIR/contacts_vectors

rm $SUBDIR/*.contacts
