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

CASEID="4"

{
cat << 'EOF'
from pymol.cgo import *
from pymol import cmd
caseCASEID  = [COLOR, 1.0, 1.0, 0.0,
EOF

{
cat << 'EOF'
0 0 0 1
0 0 0 0.5
1 0 0 1
EOF
} \
| ./voronota-lt -output-csa-with-graphics -probe 0.5 \
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

CASEID="5"

{
cat << 'EOF'
from pymol.cgo import *
from pymol import cmd
caseCASEID  = [COLOR, 1.0, 1.0, 0.0,
EOF

{
cat << 'EOF'
0 0 2 1
0 1 0 0.5
0.38268343236509 0.923879532511287 0 0.5
0.707106781186547 0.707106781186548 0 0.5
0.923879532511287 0.38268343236509 0 0.5
1 6.12323399573677e-17 0 0.5
0.923879532511287 -0.38268343236509 0 0.5
0.707106781186548 -0.707106781186547 0 0.5
0.38268343236509 -0.923879532511287 0 0.5
1.22464679914735e-16 -1 0 0.5
-0.38268343236509 -0.923879532511287 0 0.5
-0.707106781186547 -0.707106781186548 0 0.5
-0.923879532511287 -0.38268343236509 0 0.5
-1 -1.83697019872103e-16 0 0.5
-0.923879532511287 0.38268343236509 0 0.5
-0.707106781186548 0.707106781186547 0 0.5
-0.38268343236509 0.923879532511287 0 0.5
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

