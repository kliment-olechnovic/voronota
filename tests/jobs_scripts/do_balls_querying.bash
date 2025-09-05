#!/bin/bash

SUBDIR=$OUTPUTDIR/balls_querying
mkdir -p $SUBDIR

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --include-heteroatoms --annotated \
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
| $VORONOTA query-balls --set-dssp-info <(dssp --output-format=dssp $INPUTDIR/single/structure.pdb) \
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
| $VORONOTA x-query-balls-sequences-pairings-stats \
> $SUBDIR/sequences_pairings_stats

cat $SUBDIR/balls \
| $VORONOTA query-balls --match 'R<CRO>' \
| $VORONOTA x-draw-balls \
  --representation vdw \
  --drawing-for-pymol $SUBDIR/drawing_matched_balls_for_pymol.py \
  --drawing-for-scenejs $SUBDIR/drawing_matched_balls_for_scenejs \
  --drawing-name CRO_balls \
  --default-color '0x00FFFF' \
  --use-labels \
| $VORONOTA x-draw-balls \
  --representation vdw \
  --drawing-for-pymol $SUBDIR/drawing_matched_balls_colored_randomly_for_pymol.py \
  --drawing-name CRO_balls_colored_randomly \
  --random-colors \
| $VORONOTA query-balls --match-tags 'el=C' --set-adjuncts 'r=0.8;g=0.8;b=0.8' \
| $VORONOTA query-balls --match-tags 'el=N' --set-adjuncts 'r=0.2;g=0.2;b=1.0' \
| $VORONOTA query-balls --match-tags 'el=O' --set-adjuncts 'r=1.0;g=0.2;b=0.2' \
| $VORONOTA x-draw-balls \
  --representation sticks \
  --drawing-for-pymol $SUBDIR/drawing_matched_sticks_for_pymol.py \
  --drawing-for-scenejs $SUBDIR/drawing_matched_sticks_for_scenejs \
  --drawing-name CRO_sticks \
  --default-color '0x00FF00' \
  --adjuncts-rgb \
  --use-labels \
| $VORONOTA x-draw-balls \
  --representation sticks \
  --drawing-for-pymol $SUBDIR/drawing_matched_sticks_colored_by_tf_for_pymol.py \
  --drawing-name CRO_sticks_colored_by_tf \
  --adjunct-gradient tf \
  --adjunct-gradient-blue 0.0 \
  --adjunct-gradient-red 20.0 \
> /dev/null

cat $SUBDIR/balls \
| $VORONOTA query-balls \
  --set-seq-pos-adjunct \
  --set-dssp-info <(dssp --output-format=dssp $INPUTDIR/single/structure.pdb) \
| $VORONOTA x-draw-balls \
  --representation cartoon \
  --drawing-for-pymol $SUBDIR/cartoon_for_pymol.py \
  --drawing-name cartoon \
  --use-labels \
  --random-colors-by-chain \
  --adjunct-gradient seqpos \
  --rainbow-gradient \
> /dev/null

cat $SUBDIR/balls \
| $VORONOTA query-balls-clashes \
  --clash-distance 2.2 \
  --init-radius-for-BSH 3.5 \
  --min-seq-sep 2 \
> $SUBDIR/balls_clashes

{
cat << 'EOF'
MODEL        1
ATOM     25  N   VAL A  23      34.647  24.659 104.854  1.00 37.32           N  
ATOM     26  CA  VAL A  23      33.353  25.304 105.184  1.00 30.15           C  
ATOM     27  C   VAL A  23      32.461  25.468 103.960  1.00 27.72           C  
ATOM     28  O   VAL A  23      32.884  25.999 102.917  1.00 27.64           O  
ATOM     29  CB  VAL A  23      33.559  26.673 105.868  1.00 28.16           C  
ATOM     30  CG1 VAL A  23      32.238  27.414 106.098  1.00 27.02           C  
ATOM     31  CG2 VAL A  23      34.293  26.485 107.187  1.00 29.70           C  
ATOM     32  N   LEU A  24      31.211  25.029 104.098  1.00 22.93           N  
ATOM     33  CA  LEU A  24      30.243  25.151 103.028  1.00 20.72           C  
ATOM     34  C   LEU A  24      29.559  26.500 103.248  1.00 24.20           C  
ATOM     35  O   LEU A  24      29.076  26.756 104.350  1.00 23.10           O  
ATOM     36  CB  LEU A  24      29.245  24.014 103.083  1.00 22.26           C  
ATOM     37  CG  LEU A  24      29.807  22.621 102.885  1.00 22.78           C  
ATOM     38  CD1 LEU A  24      28.818  21.585 103.406  1.00 24.90           C  
ATOM     39  CD2 LEU A  24      30.078  22.410 101.402  1.00 23.56           C
ENDMDL                                                                          
MODEL        2                                                                  
ATOM    136  N   THR A  37      15.765  -1.125 116.204  1.00 18.84           N  
ATOM    137  CA  THR A  37      15.928  -2.515 116.621  1.00 18.88           C  
ATOM    138  C   THR A  37      16.177  -3.359 115.406  1.00 19.06           C  
ATOM    139  O   THR A  37      16.290  -2.845 114.283  1.00 19.19           O  
ATOM    140  CB  THR A  37      14.688  -2.991 117.424  1.00 22.18           C  
ATOM    141  OG1 THR A  37      14.994  -4.243 118.097  1.00 22.52           O  
ATOM    142  CG2 THR A  37      13.494  -3.170 116.500  1.00 24.26           C  
ATOM    143  N   VAL A  38      16.317  -4.669 115.633  1.00 20.57           N  
ATOM    144  CA  VAL A  38      16.605  -5.614 114.588  1.00 22.49           C  
ATOM    145  C   VAL A  38      15.664  -6.780 114.808  1.00 21.42           C  
ATOM    146  O   VAL A  38      15.619  -7.341 115.900  1.00 24.84           O  
ATOM    147  CB  VAL A  38      18.074  -6.093 114.630  1.00 21.65           C  
ATOM    148  CG1 VAL A  38      18.354  -7.041 113.472  1.00 25.79           C  
ATOM    149  CG2 VAL A  38      19.053  -4.900 114.625  1.00 24.15           C  
ENDMDL
EOF
} \
| $VORONOTA get-balls-from-atoms-file \
  --annotated \
  --multimodel-chains \
| $VORONOTA query-balls \
  --match 'A<C,CA,N,O>' \
| $VORONOTA write-balls-to-atoms-file \
  --pdb-output $SUBDIR/multimodel_query_result.pdb \
> $SUBDIR/multimodel_query_result
