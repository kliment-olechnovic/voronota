#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/balls_querying
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTA_RADII_FILE --include-heteroatoms --annotated \
> $SUBDIR/balls

cat $SUBDIR/balls \
| $VORONOTA query-balls --match 'r<3:7,9>&A<CA,CB>' \
| column -t \
> $SUBDIR/match_basic

cat $SUBDIR/balls \
| $VORONOTA query-balls --match-adjuncts 'tf=45.0:50.0' --match-tags 'el=N|el=O' \
| column -t \
> $SUBDIR/match_adjuncts_and_tags

cat $SUBDIR/balls \
| $VORONOTA query-balls --match 'A<OE1>&r<1:32>' --whole-residues \
| column -t \
> $SUBDIR/match_whole_residues

cat $SUBDIR/balls \
| $VORONOTA query-balls --seq-output $SUBDIR/sequence \
> /dev/null

cat $SUBDIR/balls \
| $VORONOTA query-balls \
  --set-ref-seq-num-adjunct <(cut -c10-100 < $SUBDIR/sequence ; cut -c150-190 < $SUBDIR/sequence) \
  --ref-seq-alignment $SUBDIR/sequence_alignment \
| $VORONOTA query-balls --renumber-from-adjunct refseq \
> $SUBDIR/balls_renumbered_by_alignment

cat $SUBDIR/balls \
| $VORONOTA query-balls --drop-altloc-indicators --drop-atom-serials --set-dssp-tags <(dssp $INPUTDIR/single/structure.pdb) \
> $SUBDIR/balls_with_dssp_info
