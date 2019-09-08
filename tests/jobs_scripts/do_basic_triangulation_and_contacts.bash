#!/bin/bash

DIR=$OUTPUTDIR/basic_triangulation_and_contacts

##############################################################

SUBDIR=$DIR/default_radii
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb | $VORONOTA get-balls-from-atoms-file > $SUBDIR/balls

cat $SUBDIR/balls | $VORONOTA calculate-vertices --print-log --check > $SUBDIR/triangulation 2> $SUBDIR/triangulation_log

cat $SUBDIR/balls | $VORONOTA calculate-vertices-in-parallel --print-log --method simulated --parts 8 > $SUBDIR/triangulation_simulated_parallel 2> $SUBDIR/triangulation_simulated_parallel_log

cat $SUBDIR/balls | $VORONOTA calculate-contacts --volumes-output $SUBDIR/volumes --old-contacts-output $SUBDIR/oldcontacts > $SUBDIR/contacts

##############################################################

SUBDIR=$DIR/custom_radii
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb | $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii > $SUBDIR/balls

cat $SUBDIR/balls | $VORONOTA calculate-vertices --print-log --check > $SUBDIR/triangulation 2> $SUBDIR/triangulation_log

cat $SUBDIR/balls | $VORONOTA calculate-vertices-in-parallel --print-log --method simulated --parts 8 > $SUBDIR/triangulation_simulated_parallel 2> $SUBDIR/triangulation_simulated_parallel_log

cat $SUBDIR/balls | $VORONOTA calculate-contacts --volumes-output $SUBDIR/volumes --old-contacts-output $SUBDIR/oldcontacts > $SUBDIR/contacts

##############################################################

SUBDIR=$DIR/custom_radii_with_heteroatoms
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb | $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --include-heteroatoms > $SUBDIR/balls

cat $SUBDIR/balls | $VORONOTA calculate-vertices --print-log --check > $SUBDIR/triangulation 2> $SUBDIR/triangulation_log

cat $SUBDIR/balls | $VORONOTA calculate-vertices-in-parallel --print-log --method simulated --parts 8 > $SUBDIR/triangulation_simulated_parallel 2> $SUBDIR/triangulation_simulated_parallel_log

cat $SUBDIR/balls | $VORONOTA calculate-contacts --volumes-output $SUBDIR/volumes --old-contacts-output $SUBDIR/oldcontacts > $SUBDIR/contacts

##############################################################

SUBDIR=$DIR/mmcif_custom_radii_with_heteroatoms_with_calculation_enhancements
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.cif \
| $VORONOTA get-balls-from-atoms-file \
  --input-format mmcif \
  --radii-file $VORONOTADIR/resources/radii \
  --include-heteroatoms \
  --hull-offset 3.0 \
> $SUBDIR/balls

cat $SUBDIR/balls \
| $VORONOTA calculate-vertices \
  --exclude-hidden-balls \
  --include-surplus-quadruples \
  --link \
  --print-log \
  --check \
> $SUBDIR/triangulation \
2> $SUBDIR/triangulation_log

cat $SUBDIR/balls \
| $VORONOTA calculate-vertices-in-parallel \
  --include-surplus-quadruples \
  --link \
  --print-log \
  --method simulated \
  --parts 8 \
> $SUBDIR/triangulation_simulated_parallel \
2> $SUBDIR/triangulation_simulated_parallel_log

cat $SUBDIR/balls \
| $VORONOTA calculate-contacts \
  --exclude-hidden-balls \
> $SUBDIR/contacts

##############################################################

SUBDIR=$DIR/with_secondary_options
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file \
  --radii-file $VORONOTADIR/resources/radii \
  --include-heteroatoms \
  --include-hydrogens \
  --multimodel-chains \
  --default-radius 1.70 \
  --only-default-radius \
  --hull-offset 3.0 \
> $SUBDIR/balls

cat $SUBDIR/balls \
| $VORONOTA calculate-vertices \
  --exclude-hidden-balls \
  --include-surplus-quadruples \
  --link \
  --init-radius-for-BSH 3.5 \
  --print-log \
  --check \
 > $SUBDIR/triangulation \
 2> $SUBDIR/triangulation_log
 
cat $SUBDIR/balls \
| $VORONOTA calculate-vertices-in-parallel \
  --include-surplus-quadruples \
  --init-radius-for-BSH 3.5 \
  --link \
  --print-log \
  --method simulated \
  --parts 8 \
> $SUBDIR/triangulation_simulated_parallel \
2> $SUBDIR/triangulation_simulated_parallel_log

cat $SUBDIR/balls \
| $VORONOTA calculate-contacts \
  --exclude-hidden-balls \
  --add-mirrored \
  --probe 1.4 \
  --step 0.2 \
  --projections 5 \
  --sih-depth 3 \
> $SUBDIR/contacts
