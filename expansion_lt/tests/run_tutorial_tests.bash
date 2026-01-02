#!/bin/bash

cd $(dirname "$0")

####################################################################

mkdir -p "./output/tutorial/part1"

voronota-lt ./input/7DFR.cif \
&> ./output/tutorial/part1/part1step01.txt

{
voronota-lt --input ./input/7DFR.cif \
--restrict-atom-descriptors-for-output "[-rname FOL -aname O2]" \
--restrict-contacts-for-output "[-a1 [-rname FOL -aname O2]]"
} &> ./output/tutorial/part1/part1step02.txt

{
voronota-lt --input ./input/7DFR.cif \
--restrict-atom-descriptors-for-output "[-rname FOL -aname O2]" \
--restrict-contacts-for-output "[-a1 [-rname FOL -aname O2]]" \
--graphics-restrict-representations faces wireframe sas \
--graphics-title "7DFR_FOL_O2" \
--graphics-output-file-for-pymol "./output/tutorial/part1/draw_7DFR_FOL_O2_contacts.py"
} &> ./output/tutorial/part1/part1step03.txt

{
voronota-lt --input ./input/7DFR.cif \
--restrict-contacts-for-output "[-a1 [-rname FOL -aname O2]]" \
--sites-view-script-for-pymol "./output/tutorial/part1/show_7DFR_FOL_O2_site.pml"
} &> ./output/tutorial/part1/part1step04.txt

{
voronota-lt --quiet --input ./input/7DFR.cif \
--restrict-atom-descriptors-for-output "[-rname FOL -aname O2]" \
--restrict-contacts-for-output "[-a1 [-rname FOL -aname O2]]" \
--print-contacts \
--print-cells \
| column -t
} &> ./output/tutorial/part1/part1step05.txt

{
voronota-lt ./input/7DFR.cif \
--restrict-atom-descriptors-for-output "[-rname FOL]" \
--restrict-contacts-for-output "[-a1 [-rname FOL] -a2 [-protein]]" \
--graphics-restrict-representations faces wireframe sas \
--graphics-title "7DFR_FOL" \
--graphics-output-file-for-pymol "./output/tutorial/part1/draw_7DFR_FOL_contacts.py"
} &> ./output/tutorial/part1/part1step06.txt

{
voronota-lt --input ./input/7DFR.cif \
--restrict-contacts-for-output "[-a1 [-rname FOL] -a2 [-protein]]" \
--sites-view-script-for-pymol "./output/tutorial/part1/show_7DFR_FOL_site.pml"
} &> ./output/tutorial/part1/part1step07.txt

{
voronota-lt --input ./input/7DFR.cif \
--restrict-atom-descriptors-for-output "[-rname FOL]" \
--print-cells \
| csvsort -t -c sas_area -r \
| head -n 15 \
| tr "," "\t" \
| column -t
} &> ./output/tutorial/part1/part1step08.txt

{
voronota-lt --input ./input/7DFR.cif \
--restrict-contacts "[-a1 [-rname FOL,NAP] -a2 [-protein]]" \
--graphics-title "7DFR_ligands_raw_collisions" \
--graphics-output-file-for-pymol "./output/tutorial/part1/draw_7DFR_ligands_raw_collisions.py" \
--graphics-restrict-representations collisions
} &> ./output/tutorial/part1/part1step09.txt

{
voronota-lt --input ./input/7DFR.cif \
--restrict-contacts "[-a1 [-rname FOL,NAP] -a2 [-protein]]" \
--graphics-title "7DFR_ligands_contacts" \
--graphics-output-file-for-pymol "./output/tutorial/part1/draw_7DFR_ligands_contacts.py" \
--graphics-restrict-representations faces wireframe
} &> ./output/tutorial/part1/part1step10.txt

{
voronota-lt --quiet --input ./input/7DFR.cif \
--restrict-contacts "[-a1 [-rname FOL,NAP] -a2 [-protein]]" \
--print-contacts-residue-level \
| csvsort -t -c area -r \
| head -n 6 \
| tr "," "\t" \
| column -t
} &> ./output/tutorial/part1/part1step11.txt

{
voronota-lt --quiet --input ./input/7DFR.cif \
--restrict-contacts "[-a1 [-rname FOL,NAP] -a2 [-protein]]" \
--restrict-atom-descriptors-for-output "[-protein]" \
--print-sites-residue-level \
| csvsort -t -c area -r \
| head -n 6 \
| tr "," "\t" \
| column -t
} &> ./output/tutorial/part1/part1step12.txt

{
voronota-lt --input ./input/7DFR.cif \
--restrict-contacts "[-a1 [-rname FOL] -a2 [-rname NAP]]" \
--graphics-title "7DFR_inter_ligand" \
--graphics-output-file-for-pymol "./output/tutorial/part1/draw_7DFR_inter_ligand_contacts.py"
} &> ./output/tutorial/part1/part1step13.txt

{
voronota-lt --input ./input/7DFR.cif \
--write-input-balls-to-file "./output/tutorial/part1/processed_atomic_balls.tsv" \
--write-contacts-to-file "./output/tutorial/part1/atom_atom_contacts.tsv" \
--write-contacts-residue-level-to-file "./output/tutorial/part1/residue_residue_contact_summaries.tsv" \
--write-cells-to-file "./output/tutorial/part1/per_atom_cells.tsv" \
--write-cells-residue-level-to-file "./output/tutorial/part1/per_residue_cells_summaries.tsv" \
--write-sites-to-file "./output/tutorial/part1/per_atom_binding_site.tsv" \
--write-sites-residue-level-to-file "./output/tutorial/part1/per_residue_binding_site_summaries.tsv"
} &> ./output/tutorial/part1/part1step14.txt

####################################################################

mkdir -p "./output/tutorial/part2"

{
voronota-lt --input ./input/4UN3.cif \
--restrict-contacts "[-a1 [-protein] -a2 [-nucleic-dna]]" \
--print-contacts-chain-level \
| column -t
} &> ./output/tutorial/part2/part2step01.txt


{
voronota-lt --quiet --input ./input/4UN3.cif \
--restrict-contacts "[-a1 [-protein] -a2 [-nucleic-dna-sidechain]]" \
--print-contacts-chain-level \
| column -t
} &> ./output/tutorial/part2/part2step02.txt

{
cat > "./output/tutorial/part2/coloring_rules.txt" << EOF
contact faces 71A4C1 [-a1 [-protein] -a2 [-nucleic-dna-backbone]]
contact faces FFFF00 [-a1 [-protein] -a2 [-nucleic-dna-sidechain]]
EOF

voronota-lt --input ./input/4UN3.cif \
--restrict-contacts "[-a1 [-chain B] -a2 [-chain C,D]]" \
--graphics-coloring-config-file "./output/tutorial/part2/coloring_rules.txt" \
--graphics-title "protein_dna" \
--graphics-output-file-for-pymol ./output/tutorial/part2/draw_protein_dna_contacts.py
} &> ./output/tutorial/part2/part2step03.txt

{
voronota-lt --quiet --input ./input/4UN3.cif \
--restrict-contacts \
"[-a1 [-protein -elem N,O] -a2 [-nucleic-dna-sidechain -elem N,O] -max-dist 3.5]" \
--print-contacts \
| column -t
} &> ./output/tutorial/part2/part2step04.txt

{
voronota-lt --quiet --input ./input/4UN3.cif \
--restrict-contacts \
"[-a1 [-protein -elem N,O] -a2 [-nucleic-dna-sidechain -elem N,O] -max-dist 3.5]" \
--print-contacts-residue-level \
--print-sites-residue-level \
| column -t
} &> ./output/tutorial/part2/part2step05.txt

{
cat > "./output/tutorial/part2/detailed_coloring_rules.txt" << EOF
contact faces 71A4C1 [-a1 [-protein] -a2 [-nucleic-dna-backbone]]
contact faces FFFF00 [-a1 [-protein] -a2 [-nucleic-dna-sidechain]]
contact faces FF00FF [-a1 [-protein -elem N,O] -a2 [-nucleic -elem N,O] -max-dist 3.5]
EOF

voronota-lt --input ./input/4UN3.cif \
--restrict-contacts "[-a1 [-chain B -rnum 1107,1333,1335] -a2 [-nucleic-dna]]" \
--graphics-coloring-config-file "./output/tutorial/part2/detailed_coloring_rules.txt" \
--graphics-title "focused_protein_dna" \
--graphics-output-file-for-pymol "./output/tutorial/part2/draw_focused_protein_dna_contacts.py" \
--sites-view-script-for-pymol "./output/tutorial/part2/show_site_atoms.pml"
} &> ./output/tutorial/part2/part2step06.txt

{
voronota-lt --input ./input/4UN3.cif \
--restrict-contacts "[-a1 [-chain B -rnum 1107,1333,1335] -a2 [-nucleic-dna]]" \
--graphics-coloring-config-file "./output/tutorial/part2/detailed_coloring_rules.txt" \
--plot-config-flags compact labeled colored \
--plot-contacts-to-file "./output/tutorial/part2/plot_protein_dna_contacts.svg" \
--plot-contacts-residue-level-to-file "./output/tutorial/part2/plot_protein_dna_contacts_residue_level.svg"
} &> ./output/tutorial/part2/part2step07.txt

