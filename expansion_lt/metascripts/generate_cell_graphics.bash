#!/bin/bash

CELLID="$1"

{
cat << 'EOF'
from pymol.cgo import *
from pymol import cmd
cellCELLID  = [COLOR, 1.0, 1.0, 0.0,
EOF

cat \
| ./voronota-lt -output-csa-with-graphics -probe 1.4 \
| egrep '^csa ' \
| awk -v cellid="$CELLID" '{if($2==cellid || $3==cellid){print $7}}' \
| sed 's/NORMAL/\nNORMAL/g' \
| sed 's/END/\nEND,/g' \
| sed 's/,/, /g'

cat << 'EOF'
]
cmd.load_cgo(cellCELLID, 'cellCELLID')
cmd.set('two_sided_lighting', 1)
EOF
} \
| sed "s/CELLID/${CELLID}/g"