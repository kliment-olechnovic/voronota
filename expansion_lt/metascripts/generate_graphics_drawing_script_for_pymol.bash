#!/bin/bash

PROBE="$1"

if [ -z "$PROBE" ]
then
	PROBE="$1.4"
fi

readonly TMPLDIR="$(mktemp -d)"
trap "rm -r $TMPLDIR" EXIT

cat > "${TMPLDIR}/balls"

cat "${TMPLDIR}/balls" \
| voronota-lt -probe "$PROBE" -write-contacts-graphics-to-file "${TMPLDIR}/graphics" > /dev/null

CASEID="$(cat ${TMPLDIR}/balls | md5sum | awk '{print $1}')"

{
cat << 'EOF'
from pymol.cgo import *
from pymol import cmd
caseCASEID  = [COLOR, 0.0, 1.0, 1.0,
EOF

cat "${TMPLDIR}/balls" \
| awk '{print "SPHERE, " $1 ", " $2 ", " $3 ", " $4 ","}'

echo "COLOR, 1.0, 1.0, 0.0,"

cat "${TMPLDIR}/graphics" \
| egrep '^[A-Z]'

cat << 'EOF'
]
cmd.load_cgo(caseCASEID, 'caseCASEID')
cmd.set('two_sided_lighting', 1)
EOF
} \
| sed "s/CASEID/${CASEID}/g"

