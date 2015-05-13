#!/bin/bash

set +e

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

SUBDIR=$OUTPUTDIR/setting_contacts_hbonds_tags
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/radii --include-heteroatoms --annotated \
> $TMPDIR/balls

cat $TMPDIR/balls \
| $VORONOTA query-balls --pdb-output $TMPDIR/refined.pdb \
> /dev/null

cd $TMPDIR
hbplus ./refined.pdb > /dev/null
cd - &> /dev/null

mv $TMPDIR/refined.hb2 $SUBDIR/hbplus_output

cat $TMPDIR/balls \
| $VORONOTA calculate-contacts --annotated \
| $VORONOTA query-contacts --set-hbplus-tags $SUBDIR/hbplus_output \
| $VORONOTA query-contacts --match-tags hb \
> $SUBDIR/contacts_with_hbonds_tags
