#!/bin/bash

set +e

SUBDIR=$OUTPUTDIR/contacts_querying
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/radii --include-heteroatoms --annotated \
| tee $SUBDIR/balls \
| $VORONOTA calculate-contacts --annotated \
> $SUBDIR/contacts

cat $SUBDIR/contacts \
| $VORONOTA query-contacts \
  --match-first 'r<3:7,9>&A<CA,CB>' \
  --match-min-seq-sep 1 \
  --match-max-seq-sep 20 \
| column -t \
> $SUBDIR/match_first_and_seqsep

cat $SUBDIR/contacts \
| $VORONOTA query-contacts --inter-residue --match-first 'R<THR>' --match-second 'R<ARG>' \
| column -t \
> $SUBDIR/match_first_and_second_resnames

cat $SUBDIR/contacts \
| $VORONOTA query-contacts \
  --no-solvent \
  --match-min-area 10.0 \
  --match-max-area 13.0 \
  --match-min-dist 1.5 \
  --match-max-dist 4.0 \
  --match-min-seq-sep 1 \
| column -t \
> $SUBDIR/match_nosolvent_minarea_maxdist_seqsep

cat $SUBDIR/contacts \
| $VORONOTA query-contacts --match-external-pairs $SUBDIR/match_first_and_second_resnames \
| column -t \
> $SUBDIR/match_external_pairs

cat $SUBDIR/contacts \
| $VORONOTA query-contacts --match-first-not 'R<VAL>' --match-second-not 'R<VAL>' --invert \
| column -t \
> $SUBDIR/match_not_and_invert

cat $SUBDIR/contacts \
| $VORONOTA query-contacts --match-first 'A<O>' --set-tags 'withO' \
| $VORONOTA query-contacts --match-first 'A<NZ>' --set-tags 'withNZ' \
| column -t \
> $SUBDIR/match_first_and_set_tags

cat $SUBDIR/match_first_and_set_tags \
| $VORONOTA query-contacts --summarize \
> $SUBDIR/match_first_and_set_tags_summary

cat $SUBDIR/match_first_and_set_tags \
| $VORONOTA query-contacts --match-tags 'withO' --match-tags-not 'withNZ' \
| column -t \
> $SUBDIR/match_tags

cat $SUBDIR/contacts \
| $VORONOTA query-contacts --match-first 'A<O,NZ>' --set-adjuncts 'b=10.0;a=1.0' \
| column -t \
> $SUBDIR/match_first_and_set_adjuncts

cat $SUBDIR/match_first_and_set_adjuncts \
| $VORONOTA query-contacts --match-adjuncts 'a=0:2' --match-adjuncts-not 'b=0:2' --drop-tags --set-tags 'ta;tb' \
| $VORONOTA query-contacts --drop-adjuncts \
| column -t \
> $SUBDIR/match_adjuncts_and_drop_adjuncts

cat $SUBDIR/contacts \
| column -t \
| $VORONOTA query-contacts \
  --match-external-first <(head -30 $SUBDIR/balls | awk '{print $1}') \
  --match-external-second <(head -30 $SUBDIR/balls | awk '{print $1}') \
> $SUBDIR/match_external_first_and_second

cat $SUBDIR/balls \
| $VORONOTA calculate-contacts --annotated --draw \
| $VORONOTA query-contacts \
  --match-first 'R<CRO>' \
  --match-min-seq-sep 1 \
  --preserve-graphics \
  --drawing-for-pymol $SUBDIR/drawing_matched_residue_for_pymol.py \
  --drawing-for-jmol $SUBDIR/drawing_matched_residue_for_jmol \
  --drawing-for-scenejs $SUBDIR/drawing_matched_residue_for_scenejs \
  --drawing-name 'CRO_contacts' \
  --drawing-color 0xFFFF00 \
  --drawing-alpha 0.75 \
  --drawing-labels \
| $VORONOTA query-contacts \
  --inter-residue \
  --drawing-for-pymol $SUBDIR/drawing_matched_residue_randomly_colored_for_pymol.py \
  --drawing-name 'CRO_contacts_random_colors' \
  --drawing-random-colors \
> /dev/null

cat $SUBDIR/balls \
| $VORONOTA query-balls --match-external-annotations <(cat $SUBDIR/match_first_and_second_resnames | awk '{print $1 " " $2}' | tr ' ' '\n') \
> $SUBDIR/balls_matched_by_external_annotations

cat $SUBDIR/balls \
| $VORONOTA calculate-contacts --annotated --tag-centrality \
| $VORONOTA query-contacts \
  --match-min-seq-sep 7 \
  --match-tags 'central' \
| $VORONOTA query-contacts \
  --set-distance-bins-tags '2.4;2.6;2.8;3.0;3.2;3.4;3.6;3.8;4.0;4.5;5.0;5.5;6.0' \
> $SUBDIR/match_far_central_contacts

cat $SUBDIR/contacts \
| $VORONOTA query-contacts-simulating-unfolding --max-seq-sep 6 \
> $SUBDIR/roughly_unfolded_contacts
