#!/bin/bash

SUBDIR=$OUTPUTDIR/interface_contacts_comparison_and_drawing
mkdir -p $SUBDIR

cat $INPUTDIR/complex/target.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --annotated \
| $VORONOTA query-balls --drop-altloc-indicators --drop-atom-serials \
| $VORONOTA calculate-contacts --annotated --draw \
| $VORONOTA query-contacts --no-same-chain --no-solvent --preserve-graphics \
> $SUBDIR/target_iface

cat $INPUTDIR/complex/model1.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --annotated \
| $VORONOTA query-balls --drop-altloc-indicators --drop-atom-serials \
| $VORONOTA calculate-contacts --annotated --draw \
| $VORONOTA query-contacts --no-same-chain --no-solvent --preserve-graphics \
> $SUBDIR/model1_iface

cat $INPUTDIR/complex/model2.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --annotated \
| $VORONOTA query-balls --drop-altloc-indicators --drop-atom-serials \
| $VORONOTA calculate-contacts --annotated --draw \
| $VORONOTA query-contacts --no-same-chain --no-solvent --preserve-graphics \
> $SUBDIR/model2_iface

cat $SUBDIR/model1_iface \
| $VORONOTA compare-contacts \
  --detailed-output \
  --target-contacts-file <(cat $SUBDIR/target_iface) \
  --inter-residue-scores-file $SUBDIR/model1_iface_cad_score_inter_residue \
  --inter-atom-scores-file $SUBDIR/model1_iface_cad_score_inter_atom \
> $SUBDIR/model1_iface_cad_score_global

cat $SUBDIR/model2_iface \
| $VORONOTA compare-contacts \
  --detailed-output \
  --target-contacts-file <(cat $SUBDIR/target_iface) \
  --inter-residue-scores-file $SUBDIR/model2_iface_cad_score_inter_residue \
  --inter-atom-scores-file $SUBDIR/model2_iface_cad_score_inter_atom \
> $SUBDIR/model2_iface_cad_score_global

cat $SUBDIR/target_iface \
| $VORONOTA query-contacts --inter-residue \
  --set-external-adjuncts <(cat $SUBDIR/model1_iface_cad_score_inter_residue | awk '{print $1 " " $2 " " $3}') \
  --set-external-adjuncts-name irs \
  --preserve-graphics \
| $VORONOTA draw-contacts \
  --drawing-for-pymol $SUBDIR/model1_iface_cad_score_inter_residue_drawing.py \
  --drawing-for-chimera $SUBDIR/model1_iface_cad_score_inter_residue_drawing_for_chimera.bild \
  --drawing-name model1_iface_cad_score \
  --adjunct-gradient irs \
  --adjunct-gradient-blue 1.0 \
  --adjunct-gradient-red 0.0 \
> /dev/null

cat $SUBDIR/target_iface \
| $VORONOTA query-contacts --inter-residue \
  --set-external-adjuncts <(cat $SUBDIR/model2_iface_cad_score_inter_residue | awk '{print $1 " " $2 " " $3}') \
  --set-external-adjuncts-name irs \
  --preserve-graphics \
| $VORONOTA draw-contacts \
  --drawing-for-pymol $SUBDIR/model2_iface_cad_score_inter_residue_drawing.py \
  --drawing-for-chimera $SUBDIR/model2_iface_cad_score_inter_residue_drawing_for_chimera.bild \
  --drawing-name model2_iface_cad_score \
  --adjunct-gradient irs \
  --adjunct-gradient-blue 1.0 \
  --adjunct-gradient-red 0.0 \
> /dev/null
