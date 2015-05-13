#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/balls_querying
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/radii --include-heteroatoms --annotated \
> $SUBDIR/balls

cat $SUBDIR/balls \
| $VORONOTA query-balls --match 'r<3:7,9>&A<CA,CB>' \
| column -t \
> $SUBDIR/match_basic

cat $SUBDIR/balls \
| $VORONOTA query-balls --match-not 'r<3:7,9>&A<CA,CB>' --invert \
| column -t \
> $SUBDIR/match_basic_not_and_invert

cat $SUBDIR/balls \
| $VORONOTA query-balls --match-adjuncts 'tf=45.0:50.0' --match-tags 'el=N|el=O' \
| column -t \
> $SUBDIR/match_adjuncts_and_tags

cat $SUBDIR/balls \
| $VORONOTA query-balls --match-adjuncts-not 'tf=45.0:50.0' --invert \
| $VORONOTA query-balls --match-tags-not 'el=N|el=O' --invert \
| column -t \
> $SUBDIR/match_adjuncts_not_and_tags_not_and_invert

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
| $VORONOTA query-balls --set-dssp-tags <(dssp $INPUTDIR/single/structure.pdb) \
| $VORONOTA query-balls --match-tags 'dssp=H|dssp=G|dssp=I' --set-tags 'helix' \
| $VORONOTA query-balls --match-tags 'dssp=B|dssp=E' --set-tags 'sheet' \
| $VORONOTA query-balls --set-adjuncts 'ssc=100' \
| $VORONOTA query-balls --match-tags 'helix' --set-adjuncts 'ssc=50' \
| $VORONOTA query-balls --match-tags 'sheet' --set-adjuncts 'ssc=0' \
| $VORONOTA query-balls \
  --pdb-output $SUBDIR/balls_with_dssp_info.pdb \
  --pdb-output-b-factor ssc \
  --pdb-output-template $INPUTDIR/single/structure.pdb \
> $SUBDIR/balls_with_dssp_info
