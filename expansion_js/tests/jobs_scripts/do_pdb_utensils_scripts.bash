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

