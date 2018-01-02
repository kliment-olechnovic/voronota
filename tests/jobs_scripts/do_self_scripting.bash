#!/bin/bash

SUBDIR=$OUTPUTDIR/self_scripting
mkdir -p $SUBDIR

{
cat << EOF

load-atoms --file $INPUTDIR/single/structure.cif --format mmcif
load-atoms --file $INPUTDIR/single/structure.pdb
load-atoms $INPUTDIR/single/structure.pdb --include-heteroatoms
print-atoms {--tags het --adjuncts tf=0:10}
restrict-atoms {--tags-not het}
construct-contacts --calculate-volumes --render-use '{--atom1 {r<83>} --min-seq-sep 1}'
save-atoms --file '$SUBDIR/plain_atoms'
save-contacts '$SUBDIR/plain_contacts'
load-atoms --file '$SUBDIR/plain_atoms' --format plain
select-contacts
load-contacts --file '$SUBDIR/plain_contacts'

select-contacts {--atom1 {R<PHE>} --atom2 {R<PHE>} --min-area 5.0 --min-seq-sep 1} --name cs1
print-contacts {--sel cs1} --desc --sort area --file '$SUBDIR/printed_contacts'
print-contacts {--no-solvent --min-seq-sep 2} --desc --sort area --limit 3 --expand
print-contacts {--no-solvent --min-seq-sep 2} --desc --sort area --limit 3 --expand --inter-residue

select-atoms {r<64>&A<C,N,O,CA,CB>} --name as1
print-atoms {--sel as1} --sort tags --file '$SUBDIR/printed_atoms'
print-atoms {r<64>&A<C,N,O,CA,CB>} --sort atmn --expand

rename-selection-of-atoms nosel1 nodel2
delete-selections-of-contacts nosel1
list-selections-of-atoms
list-selections-of-contacts

color-contacts 0x00FF00
color-contacts {--atom1 {--match A<C,CA,N,O>} --atom2 {--match A<C,CA,N,O>}} 0x00FFFF
color-contacts {--atom1 {--match-not A<C,CA,N,O>} --atom2 {--match-not A<C,CA,N,O>}} 0xFFFF00
show-contacts
write-contacts-as-pymol-cgo --file '$SUBDIR/cgo_contacts.py' --name contacts

color-contacts 0xFF00FF
write-contacts-as-pymol-cgo --wireframe --file '$SUBDIR/cgo_contacts_wf.py' --name contacts_wf

save-atoms-and-contacts '$SUBDIR/plain_atoms_and_contacts'
load-atoms-and-contacts '$SUBDIR/plain_atoms_and_contacts'
load-atoms-and-contacts '$SUBDIR/plain_atoms'

print-history

EOF
} \
| $VORONOTA x-run-script \
> $SUBDIR/script_output \
2> $SUBDIR/script_errors

rm "$SUBDIR/plain_atoms_and_contacts"

