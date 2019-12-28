#!/bin/bash

SUBDIR=$OUTPUTDIR/membrane_script
mkdir -p $SUBDIR

$VORONOTADIR/voronota-membrane \
  -i $INPUTDIR/membrane/structure.pdb \
  --input-filter-query "--match c<B>" \
  --membrane-width 20 \
  --output-atoms $SUBDIR/atoms1 \
  --output-membraneness-pdb $SUBDIR/membraneness1.pdb \
  --output-log $SUBDIR/log1 \
  --output-header \
| column -t \
> $SUBDIR/scores1

cat $SUBDIR/log1 \
| sed 's|/tmp/tmp\.[[:alnum:]]\+/|/tmp/tmp.randomid/|g' \
> $SUBDIR/log1_no_tmp_dir_name

rm $SUBDIR/log1
