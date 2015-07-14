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
| $VORONOTA query-balls --set-dssp-info <(dssp $INPUTDIR/single/structure.pdb) \
| $VORONOTA query-balls --match-tags 'dssp=H|dssp=G|dssp=I' --set-tags 'helix' \
| $VORONOTA query-balls --match-tags 'dssp=B|dssp=E' --set-tags 'sheet' \
| $VORONOTA query-balls --set-adjuncts 'ssc=100' \
| $VORONOTA query-balls --match-tags 'helix' --set-adjuncts 'ssc=50' \
| $VORONOTA query-balls --match-tags 'sheet' --set-adjuncts 'ssc=0' \
| $VORONOTA write-balls-to-atoms-file \
  --pdb-output $SUBDIR/balls_with_dssp_info.pdb \
  --pdb-output-b-factor ssc \
  --pdb-output-template $INPUTDIR/single/structure.pdb \
> $SUBDIR/balls_with_dssp_info

$VORONOTA query-balls --rename-chains --renumber-positively --reset-serials \
> $SUBDIR/normalize_naming << EOF
c<U>r<5>a<1>R<SER>A<N> 28.888 9.409 52.301 1.7 . .
c<U>r<6>a<2>R<SER>A<CA> 27.638 10.125 52.516 1.9 . .
c<U>r<7>a<3>R<SER>A<C> 26.499 9.639 51.644 1.75 . .
c<U>r<8>a<4>R<SER>A<O> 26.606 8.656 50.915 1.49 . .
c<E>r<-2>a<1764>R<ILE>A<N> 42.803 22.231 49.842 1.7 . .
c<E>r<-1>a<1765>R<ILE>A<CA> 43.006 21.375 50.998 1.9 . .
c<E>r<0>a<1766>R<ILE>A<C> 44.016 20.291 50.633 1.75 . .
c<E>r<1>a<1767>R<ILE>A<O> 45.09 20.176 51.246 1.49 . .
EOF

echo $SUBDIR/balls \
| $VORONOTA query-balls-sequences-pairings-stats \
> $SUBDIR/sequences_pairings_stats

cat $SUBDIR/balls \
| $VORONOTA query-balls --match 'R<CRO>' \
| $VORONOTA draw-balls \
  --representation vdw \
  --drawing-for-pymol $SUBDIR/drawing_matched_balls_for_pymol.py \
  --drawing-for-scenejs $SUBDIR/drawing_matched_balls_for_scenejs \
  --drawing-name CRO_balls \
  --default-color '0x00FFFF' \
  --use-labels \
| $VORONOTA query-balls --match-tags 'el=C' --set-adjuncts 'r=0.8;g=0.8;b=0.8' \
| $VORONOTA query-balls --match-tags 'el=N' --set-adjuncts 'r=0.2;g=0.2;b=1.0' \
| $VORONOTA query-balls --match-tags 'el=O' --set-adjuncts 'r=1.0;g=0.2;b=0.2' \
| $VORONOTA draw-balls \
  --representation sticks \
  --drawing-for-pymol $SUBDIR/drawing_matched_sticks_for_pymol.py \
  --drawing-for-scenejs $SUBDIR/drawing_matched_sticks_for_scenejs \
  --drawing-name CRO_sticks \
  --default-color '0x00FF00' \
  --adjuncts-rgb \
  --use-labels \
> /dev/null
