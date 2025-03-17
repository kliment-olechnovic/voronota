#!/bin/bash

SUBDIR=$OUTPUTDIR/pdb_utensils_scripts
mkdir -p $SUBDIR

{
cat "$INPUTDIR/complex/target.pdb" \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-renumber-by-sequence --strict --report-alignment --chain-id A --sequence-str VTDRIGQMILEMFRTGMCLFSVRSPGGVAELYGGEARKVEITGTSLTIEREDWHLHCKLET \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-renumber-by-sequence --strict --report-alignment --chain-id B --sequence-str ------VTDRIGQMILEMFRTGMCL---------FSVRSPGGVAELYGGEARKVEITGTS----------LTIEREDWHLHCKLET------ \
> "${SUBDIR}/complex_renumbered1.pdb"
} 2> "${SUBDIR}/complex_alignment1.txt"

{
cat "$INPUTDIR/complex/target.pdb" \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-renumber-by-sequence --report-alignment --chain-id A --sequence-str VTDRIGQMILEMFRTGMCLFSVRSPGGVAELYGGEARKVEITGTSLTIEREDWHLHCKLET \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-renumber-by-sequence --report-alignment --chain-id B --sequence-str ------VTDRIGQMILEMFRTGMCL---------FSVRSPGGVAELYGGEARKVEITGTS----------LTIEREDWHLHCKLET------ \
> "${SUBDIR}/complex_renumbered2.pdb"
} 2> "${SUBDIR}/complex_alignment2.txt"

cat "$INPUTDIR/complex/target.pdb" | $VORONOTAJSDIR/voronota-js-pdb-utensil-filter-atoms '[-chain A]' > "${SUBDIR}/separate_chain_A.pdb"
cat "$INPUTDIR/complex/target.pdb" | $VORONOTAJSDIR/voronota-js-pdb-utensil-filter-atoms '[-chain B]' > "${SUBDIR}/separate_chain_B.pdb"

find "${SUBDIR}/" -type f -name 'separate_chain_*.pdb' \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-detect-inter-structure-contacts --probe 1.4 --output-header \
> "${SUBDIR}/inter_chain_detection_results.txt"

find "${SUBDIR}/" -type f -name 'separate_chain_*.pdb' | xargs rm

cat "$INPUTDIR/complex/target.pdb" \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-filter-atoms '[-rnum 30]' \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-rename-chains '_invert_case' \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-rename-chains 'a=X,b=X2' \
> "${SUBDIR}/filtered.pdb"

cat "$INPUTDIR/complex/target.pdb" \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-print-sequence-from-structure \
> "${SUBDIR}/sequences_all.fasta"

cat "$INPUTDIR/complex/target.pdb" \
| $VORONOTAJSDIR/voronota-js-pdb-utensil-print-sequence-from-structure '[-rnum 30:50]' \
> "${SUBDIR}/sequences_of_selection.fasta"

$VORONOTAJSDIR/voronota-js-pdb-utensil-summarize-ensemble \
  --restrict-input "[-chain A]" \
  --layering-mode "outward" \
  --directing-mode "none" \
  --input-directory "$INPUTDIR/complex" \
  --atom-atom-output-file "${SUBDIR}/ensemble_summary_inter_atom_with_layering_outward.txt" \
> "${SUBDIR}/ensemble_summary_inter_residue_with_layering_outward.txt"

$VORONOTAJSDIR/voronota-js-pdb-utensil-summarize-ensemble \
  --restrict-input "[-chain A]" \
  --layering-mode "inward" \
  --directing-mode "none" \
  --input-directory "$INPUTDIR/complex" \
  --atom-atom-output-file "${SUBDIR}/ensemble_summary_inter_atom_with_layering_inward.txt" \
> "${SUBDIR}/ensemble_summary_inter_residue_with_layering_inward.txt"

$VORONOTAJSDIR/voronota-js-pdb-utensil-summarize-ensemble \
  --restrict-input "[-chain A]" \
  --layering-mode "outward" \
  --directing-mode "d4" \
  --input-directory "$INPUTDIR/complex" \
  --atom-atom-output-file "${SUBDIR}/ensemble_summary_inter_atom_with_layering_outward_directing_d4.txt" \
> "${SUBDIR}/ensemble_summary_inter_residue_with_layering_outward_directing_d4.txt"

$VORONOTAJSDIR/voronota-js-pdb-utensil-summarize-ensemble \
  --restrict-input "[-chain A]" \
  --layering-mode "inward" \
  --directing-mode "d4" \
  --input-directory "$INPUTDIR/complex" \
  --atom-atom-output-file "${SUBDIR}/ensemble_summary_inter_atom_with_layering_inward_directing_d4.txt" \
> "${SUBDIR}/ensemble_summary_inter_residue_with_layering_inward_directing_d4.txt"

$VORONOTAJSDIR/voronota-js-pdb-utensil-summarize-ensemble-aa \
  --restrict-input "[-chain A]" \
  --layering-mode "inward" \
  --directing-mode "none" \
  --input-directory "$INPUTDIR/complex" \
  --atom-atom-output-file "${SUBDIR}/ensemble_summary_aa_inter_atom_with_layering_inward.txt"

$VORONOTAJSDIR/voronota-js-pdb-utensil-summarize-ensemble-aa \
  --restrict-input "[-chain A]" \
  --layering-mode "inward" \
  --directing-mode "d4" \
  --input-directory "$INPUTDIR/complex" \
  --atom-atom-output-file "${SUBDIR}/ensemble_summary_aa_inter_atom_with_layering_inward_directing_d4.txt"

$VORONOTAJSDIR/voronota-js-pdb-utensil-summarize-ensemble-aa \
  --restrict-input "[-chain A]" \
  --layering-mode "inward" \
  --directing-mode "none" \
  --input-directory "$INPUTDIR/complex" \
  --on-residue-level "true" \
  --atom-atom-output-file "${SUBDIR}/ensemble_summary_aa_inter_residue_with_layering_inward.txt"

$VORONOTAJSDIR/voronota-js-pdb-utensil-summarize-ensemble-aa \
  --restrict-input "[-chain A]" \
  --layering-mode "inward" \
  --directing-mode "d4" \
  --input-directory "$INPUTDIR/complex" \
  --on-residue-level "true" \
  --atom-atom-output-file "${SUBDIR}/ensemble_summary_aa_inter_residue_with_layering_inward_directing_d4.txt"

