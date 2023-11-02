#!/bin/bash

####################################################################

CASEID="1"

{
cat << 'EOF'
from pymol.cgo import *
from pymol import cmd
caseCASEID  = [COLOR, 1.0, 1.0, 0.0,
EOF

{
cat << 'EOF'
0.0 0.0 0.0 1.0
0.5 0.0 0.0 1.0
1.0 0.0 0.0 1.0
EOF
} \
| ./voronota-lt -output-csa-with-graphics -probe 1.0 \
| egrep '^csa ' \
| awk '{print $7}' \
| sed 's/NORMAL/\nNORMAL/g' \
| sed 's/END/\nEND,/g' \
| sed 's/,/, /g'

cat << 'EOF'
]
cmd.load_cgo(caseCASEID, 'caseCASEID')
cmd.set('two_sided_lighting', 1)
EOF
} \
| sed "s/CASEID/${CASEID}/g" \
> "./tcd${CASEID}.py"

####################################################################

CASEID="2"

{
cat << 'EOF'
from pymol.cgo import *
from pymol import cmd
caseCASEID  = [COLOR, 1.0, 1.0, 0.0,
EOF

{
cat << 'EOF'
0 0 0 1
0 0 1 1
0 1 0 1
0 1 1 1
EOF
} \
| ./voronota-lt -output-csa-with-graphics -probe 2.0 \
| egrep '^csa ' \
| awk '{print $7}' \
| sed 's/NORMAL/\nNORMAL/g' \
| sed 's/END/\nEND,/g' \
| sed 's/,/, /g'

cat << 'EOF'
]
cmd.load_cgo(caseCASEID, 'caseCASEID')
cmd.set('two_sided_lighting', 1)
EOF
} \
| sed "s/CASEID/${CASEID}/g" \
> "./tcd${CASEID}.py"

####################################################################

CASEID="3"

{
cat << 'EOF'
from pymol.cgo import *
from pymol import cmd
caseCASEID  = [COLOR, 1.0, 1.0, 0.0,
EOF

{
cat << 'EOF'
0 0 0 1
0 0 1 1
0 1 0 1
0 1 1 1
1 0 0 1
1 0 1 1
1 1 0 1
1 1 1 1
EOF
} \
| ./voronota-lt -output-csa-with-graphics -probe 2.0 \
| egrep '^csa ' \
| awk '{print $7}' \
| sed 's/NORMAL/\nNORMAL/g' \
| sed 's/END/\nEND,/g' \
| sed 's/,/, /g'

cat << 'EOF'
]
cmd.load_cgo(caseCASEID, 'caseCASEID')
cmd.set('two_sided_lighting', 1)
EOF
} \
| sed "s/CASEID/${CASEID}/g" \
> "./tcd${CASEID}.py"

####################################################################

