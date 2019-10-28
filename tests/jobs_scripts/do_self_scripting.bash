#!/bin/bash

SUBDIR=$OUTPUTDIR/self_scripting
mkdir -p $SUBDIR

{
cat << EOF

# Note the same use of both "-" and "--"

set-alias printseq "print-sequence -secondary-structure"

import --file $INPUTDIR/single/structure.cif
import -file "$INPUTDIR/single/structure.pdb"
import $INPUTDIR/single/structure.pdb --include-heteroatoms
printseq
print-atoms [--tags het --adjuncts tf:0:10]
restrict-atoms [--tags-not het]
printseq
construct-contacts --no-calculate-volumes
construct-contacts
make-drawable-contacts '[--atom1 [r<83>] -min-seq-sep 1]'
make-drawable-contacts '[--atom1 [r<83>]]'
make-undrawable-contacts '[-max-seq-sep 0]'
export-atoms --file '$SUBDIR/plain_atoms'
export-atoms --file '$SUBDIR/pdb_atoms' --as-pdb --pdb-b-factor volume
export-contacts '$SUBDIR/plain_contacts'

list-objects
delete-objects
import --file '$SUBDIR/plain_atoms' --format plain
list-objects

select-contacts
import-contacts --file '$SUBDIR/plain_contacts'

select-contacts [--atom1 [R<PHE>] -atom2 [R<PHE>] -min-area 5.0 --min-seq-sep 1] --name cs1

export-selection-of-contacts [cs1] -file '$SUBDIR/saved_selection_cs1'
import-selection-of-contacts -file '$SUBDIR/saved_selection_cs1' -name cs1_copy
delete-selections-of-contacts cs1_copy

set-alias multitag "set-tag-of-contacts [--min-area \${1}] med; delete-tags-of-contacts [--min-area \${2}] --tags med"

multitag 6.0 8.0 # Arguments to an alias are unnamed

set-alias pc-cs1 "print-contacts [cs1]"
set-alias alt-pc-cs1 "pc-cs1"
alt-pc-cs1
alt-pc-cs1 --inter-residue

select-atoms [r<64> & A<C,N,O,CA,CB>] as1

export-selection-of-atoms [as1] -file '$SUBDIR/saved_selection_as1'
import-selection-of-atoms -file '$SUBDIR/saved_selection_as1' -name as1_copy
delete-selections-of-atoms as1_copy

set-tag-of-atoms [A<C,N,O,CA,CB>] --tag pept
delete-tags-of-atoms [A<CA,CB>] --tags pept
print-atoms [as1]
print-atoms [r<64> & A<C,N,O,CA,CB>]

rename-selection-of-atoms nosel1 nodel2
delete-selections-of-contacts nosel1
list-selections-of-atoms
list-selections-of-contacts

color-atoms 0xFFFF00
color-atoms [A<C,CA,N,O>] 0xFF0000
color-atoms (not [A<C,CA,N,O>]) 0x00FF00
show-atoms
export-atoms-as-pymol-cgo [r<70:90>] -file '$SUBDIR/cgo_atoms.py' -name atoms
export-atoms-as-pymol-cgo [r<70:90>] -wireframe -file '$SUBDIR/cgo_atoms_wf.py' -name atoms_wf

spectrum-atoms
export-cartoon-as-pymol-cgo [r<45:60>&A<CA>] -file '$SUBDIR/cgo_cartoon.py' -name cartoon
export-cartoon-as-pymol-cgo [r<45:60>&A<CA>] -wireframe -file '$SUBDIR/cgo_cartoon_wf.py' -name cartoon_wf

color-contacts 0x00FF00
color-contacts [--a1 [A<C,CA,N,O>] --a2 [A<C,CA,N,O>]] 0x00FFFF
color-contacts [-a1 [-m! A<C,CA,N,O>] -a2! [A<C,CA,N,O>]] 0xFFFF00
show-contacts
export-contacts-as-pymol-cgo --file '$SUBDIR/cgo_contacts.py' --name contacts

color-contacts 0xFF00FF
export-contacts-as-pymol-cgo --wireframe --file '$SUBDIR/cgo_contacts_wf.py' --name contacts_wf

export-atoms-and-contacts '$SUBDIR/plain_atoms_and_contacts'
import '$SUBDIR/plain_atoms_and_contacts' -format plain
import '$SUBDIR/plain_atoms' -format plain

construct-triangulation
export-triangulation '_virtual/plain_triangulation' -link
list-virtual-files
download-virtual-file -name '_virtual/plain_triangulation' -file '$SUBDIR/plain_triangulation'
delete-virtual-files '_virtual/plain_triangulation'
list-virtual-files

explain-command color-contacts
list-commands

EOF
} \
> $SUBDIR/script_input

{
cat << EOF
source '$SUBDIR/script_input'
list-objects
delete-objects -names plain_atoms
list-objects
set-alias quit exit
quit
list-objects
EOF
} \
| $VORONOTA run-script \
> $SUBDIR/script_output \
2> $SUBDIR/script_errors

rm "$SUBDIR/plain_atoms_and_contacts"

