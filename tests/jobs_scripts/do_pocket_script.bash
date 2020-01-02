#!/bin/bash

SUBDIR=$OUTPUTDIR/pocket_script
mkdir -p $SUBDIR

$VORONOTADIR/voronota-pocket \
  -i $INPUTDIR/membrane/structure.pdb \
  --input-filter-query "--match c<B>" \
  --probe-min 4.0 \
  --probe-max 30.0 \
  --buriedness-core 0.7 \
  --buriedness-rim 0.4 \
  --voxelization-factor 1.0 \
  --output-atoms $SUBDIR/atoms1 \
  --output-pocketness-pdb $SUBDIR/pocketness1.pdb \
  --output-vertices $SUBDIR/vertices1 \
  --output-voxels-pdb $SUBDIR/voxels1.pdb \
  --draw-tetrahedrons $SUBDIR/tetrahedrons1.py \
  --draw-spheres $SUBDIR/spheres1.py \
  --output-log $SUBDIR/log1 \
  --output-header \
| column -t \
> $SUBDIR/scores1

cat $SUBDIR/log1 \
| sed 's|/tmp/tmp\.[[:alnum:]]\+/|/tmp/tmp.randomid/|g' \
> $SUBDIR/log1_no_tmp_dir_name

rm $SUBDIR/log1
