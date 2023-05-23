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

