#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

SUBDIR=$OUTPUTDIR/setting_contacts_hbonds_tags
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --include-heteroatoms --annotated \
> $TMPDIR/balls

cat $TMPDIR/balls \
| $VORONOTA x-write-balls-to-atoms-file --pdb-output $TMPDIR/refined.pdb \
> /dev/null

cd $TMPDIR
hbplus ./refined.pdb > /dev/null
cd - &> /dev/null

cat $TMPDIR/refined.hb2 | tail -n +2 > $SUBDIR/hbplus_output

cat $TMPDIR/balls \
| $VORONOTA calculate-contacts --annotated \
> $TMPDIR/contacts

cat $TMPDIR/contacts \
| $VORONOTA query-contacts --set-hbplus-tags $TMPDIR/refined.hb2 \
| $VORONOTA query-contacts --match-tags hb \
> $SUBDIR/contacts_with_hbonds_tags

cat $TMPDIR/contacts \
| $VORONOTA query-contacts --set-hbplus-tags $TMPDIR/refined.hb2 --inter-residue-hbplus-tags \
| $VORONOTA query-contacts --match-tags rhb \
> $SUBDIR/contacts_with_inter_residue_hbonds_tags
