#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

cd ../

rm -f "./voronota-lt"

g++ -std=c++14 -Ofast -march=native -fopenmp -o ./voronota-lt ./src/voronota_lt.cpp

export PATH="$(pwd):${PATH}"

cd - &> /dev/null

####################################################################

voronota-lt --help &> "./output/help_message.txt"

####################################################################

voronota-lt -processors 40 -probe 1.4 -print-everything < "./input/balls_2zsk.xyzr" 2> "./output/contacts_2zsk_summary.txt" > "./output/contacts_2zsk_tables.txt"

voronota-lt -processors 40 -probe 1.4  < "./input/balls_3dlb.xyzr" 2> "./output/contacts_3dlb_summary.txt"

voronota-lt -processors 40 -probe 1.4  < "./input/balls_7br8.xyzr" 2> "./output/contacts_7br8_summary.txt"

####################################################################

voronota-lt -processors 40 -print-everything -i "./input/balls_2zsk.xyzr" 2> "./output/contacts_2zsk_from_file_summary.txt" > /dev/null

voronota-lt -processors 40 --input "./input/balls_3dlb.xyzr" 2> "./output/contacts_3dlb_from_file_summary.txt"

voronota-lt -processors 40 --input "./input/balls_7br8.xyzr" 2> "./output/contacts_7br8_from_file_summary.txt"

####################################################################

voronota-lt \
< "./input/assembly_1ctf.pdb1" \
2> "./output/contacts_1ctf_pdb_summary.txt"

voronota-lt \
  --pdb-or-mmcif-heteroatoms \
< "./input/assembly_1ctf.pdb1" \
2> "./output/contacts_1ctf_pdb_with_heteroatoms_summary.txt"

voronota-lt \
  --pdb-or-mmcif-join-models \
< "./input/assembly_1ctf.pdb1" \
2> "./output/contacts_1ctf_pdb_as_assembly_summary.txt"

voronota-lt \
  --pdb-or-mmcif-join-models \
  --pdb-or-mmcif-heteroatoms \
  --write-input-balls-to-file "./output/contacts_1ctf_pdb_as_assembly_with_heteroatoms_input_balls.txt" \
< "./input/assembly_1ctf.pdb1" \
2> "./output/contacts_1ctf_pdb_as_assembly_with_heteroatoms_summary.txt"

voronota-lt \
  --pdb-or-mmcif-join-models \
  --pdb-or-mmcif-heteroatoms \
  --pdb-or-mmcif-radii-config-file "./input/custom_radii.txt" \
  --write-input-balls-to-file "./output/contacts_1ctf_pdb_as_assembly_with_heteroatoms_using_custom_radii_input_balls.txt" \
< "./input/assembly_1ctf.pdb1" \
2> "./output/contacts_1ctf_pdb_as_assembly_with_heteroatoms_using_custom_radii_summary.txt"

####################################################################

voronota-lt \
< "./input/assembly_1ctf.cif" \
2> "./output/contacts_1ctf_mmcif_assembly_summary.txt"

voronota-lt \
  --pdb-or-mmcif-heteroatoms \
  --write-input-balls-to-file "./output/contacts_1ctf_mmcif_assembly_with_heteroatoms_input_balls.txt" \
< "./input/assembly_1ctf.cif" \
2> "./output/contacts_1ctf_mmcif_assembly_with_heteroatoms_summary.txt"

####################################################################

mkdir -p "./output/written_files"

voronota-lt \
  -processors 40 \
  -probe 1.4 \
  -write-input-balls-to-file ./output/written_files/output_2zsk_input_balls.txt \
  -write-contacts-to-file ./output/written_files/output_2zsk_contacts.txt \
  -write-contacts-residue-level-to-file ./output/written_files/output_2zsk_contacts_residue_level.txt \
  -write-contacts-chain-level-to-file ./output/written_files/output_2zsk_contacts_chain_level.txt \
  -write-cells-to-file ./output/written_files/output_2zsk_cells.txt \
  -write-cells-residue-level-to-file ./output/written_files/output_2zsk_cells_residue_level.txt \
  -write-cells-chain-level-to-file ./output/written_files/output_2zsk_cells_chain_level.txt \
  -write-log-to-file ./output/written_files/output_2zsk_log.txt \
< "./input/balls_2zsk.xyzr" \
2> /dev/null

####################################################################

voronota-lt \
  -processors 40 \
  -probe 2 \
  -write-contacts-to-file ./output/written_files/output_cs_1x1_contacts.txt \
  -write-cells-to-file ./output/written_files/output_cs_1x1_cells.txt \
< "./input/balls_cs_1x1.xyzr" \
2> "./output/contacts_cs_1x1_summary.txt"

voronota-lt \
  -processors 40 \
  -probe 2 \
  -periodic-box-corners 0 0 0 200 250 300 \
  -write-contacts-to-file ./output/written_files/output_cs_1x1_periodic_contacts.txt \
  -write-cells-to-file ./output/written_files/output_cs_1x1_periodic_cells.txt \
< "./input/balls_cs_1x1.xyzr" \
2> "./output/contacts_cs_1x1_periodic_summary.txt"

voronota-lt \
  -processors 40 \
  -probe 2 \
  -periodic-box-directions  200 0 0  0 250 0  0 0 300 \
< "./input/balls_cs_1x1.xyzr" \
2> "./output/contacts_cs_1x1_periodic_by_directions_summary.txt"

voronota-lt -processors 40 -probe 2  < "./input/balls_cs_3x3.xyzr" -write-cells-chain-level-to-file ./output/written_files/output_cs_3x3_cells_chain_level.txt 2> "./output/contacts_cs_3x3_summary.txt"

####################################################################

voronota-lt \
  -i "./input/assembly_1ctf.cif" \
  --compute-only-inter-chain-contacts \
  --probe 1.4 \
  --mesh-output-obj-file "./output/written_files/output_1ctf_inter_chain_mesh.obj" \
  --mesh-print-topology-summary \
> "./output/contacts_1ctf_mmcif_assembly_inter_chain_mesh_table.txt" \
2> "./output/contacts_1ctf_mmcif_assembly_inter_chain_mesh_summary.txt"

####################################################################

voronota-lt \
  -i "./input/assembly_1ctf.cif" \
  --probe 1.4 \
  --write-tessellation-edges-to-file "./output/written_files/output_1ctf_full_tessellation_edges.txt" \
  --write-tessellation-vertices-to-file "./output/written_files/output_1ctf_full_tessellation_vertices.txt" \
&> /dev/null

voronota-lt \
  -i "./input/assembly_1ctf.cif" \
  --compute-only-inter-chain-contacts \
  --probe 1.4 \
  --write-tessellation-edges-to-file "./output/written_files/output_1ctf_inter_chain_tessellation_edges.txt" \
  --write-tessellation-vertices-to-file "./output/written_files/output_1ctf_inter_chain_tessellation_vertices.txt" \
&> /dev/null

####################################################################

voronota-lt \
  -processors 40 \
  -probe 2 \
  -write-tessellation-edges-to-file ./output/written_files/output_cs_1x1_full_tessellation_edges.txt \
  -write-tessellation-vertices-to-file ./output/written_files/output_cs_1x1_full_tessellation_vertices.txt \
< "./input/balls_cs_1x1.xyzr" \
&> /dev/null

voronota-lt \
  -processors 40 \
  -probe 2 \
  -periodic-box-corners 0 0 0 200 250 300 \
  -write-tessellation-edges-to-file ./output/written_files/output_cs_1x1_periodic_full_tessellation_edges.txt \
  -write-tessellation-vertices-to-file ./output/written_files/output_cs_1x1_periodic_full_tessellation_vertices.txt \
< "./input/balls_cs_1x1.xyzr" \
&> /dev/null

####################################################################

voronota-lt \
  -i "./input/assembly_1ctf.cif" \
  -probe 1.4 \
  -restrict-input-balls '[-chain A -rname PHE]' \
  -print-everything \
2> "./output/contacts_1ctf_mmcif_assembly_sliced_input_summary.txt" \
> "./output/contacts_1ctf_mmcif_assembly_sliced_input_tables.txt"

voronota-lt \
  -i "./input/assembly_1ctf.cif" \
  -probe 1.4 \
  -restrict-contacts '[-a1 [-rname PHE] -a2 [-chain A]]' \
  -write-raw-collisions-to-file "./output/contacts_1ctf_mmcif_assembly_sliced_candidate_contacts_raw_collisions.txt" \
  -print-everything \
2> "./output/contacts_1ctf_mmcif_assembly_sliced_candidate_contacts_summary.txt" \
> "./output/contacts_1ctf_mmcif_assembly_sliced_candidate_contacts_tables.txt"

voronota-lt \
  -i "./input/assembly_1ctf.cif" \
  -probe 1.4 \
  -restrict-contacts-for-output '[-a1 [-rname PHE] -a2 [-chain A]]' \
  -print-everything \
2> "./output/contacts_1ctf_mmcif_assembly_sliced_output_contacts_summary.txt" \
> "./output/contacts_1ctf_mmcif_assembly_sliced_output_contacts_tables.txt"

voronota-lt \
  -i "./input/assembly_1ctf.cif" \
  -probe 1.4 \
  -restrict-single-index-data-for-output '[-chain A -rname PHE]' \
  -print-everything \
2> "./output/contacts_1ctf_mmcif_assembly_sliced_cells_output_summary.txt" \
> "./output/contacts_1ctf_mmcif_assembly_sliced_cells_output_tables.txt"

####################################################################

mkdir -p "./output/api_usage_examples"

g++ -O3 -I../src/voronotalt -o "./output/api_usage_examples/example_basic_and_periodic" "./input/api_usage_example_basic_and_periodic.cpp"

./output/api_usage_examples/example_basic_and_periodic &> "./output/api_usage_examples/example_basic_and_periodic_output.txt"

rm "./output/api_usage_examples/example_basic_and_periodic"

####################################################################

mkdir -p "./output/api_usage_examples"

g++ -O3 -I../src/voronotalt -o "./output/api_usage_examples/example_for_cell_vertices_basic_and_periodic" "./input/api_usage_example_for_cell_vertices_basic_and_periodic.cpp"

./output/api_usage_examples/example_for_cell_vertices_basic_and_periodic &> "./output/api_usage_examples/example_for_cell_vertices_basic_and_periodic_output.txt"

rm "./output/api_usage_examples/example_for_cell_vertices_basic_and_periodic"

####################################################################

mkdir -p "./output/api_usage_examples"

g++ -O3 -I../src/voronotalt -o "./output/api_usage_examples/example_updateable_periodic" "./input/api_usage_example_updateable_periodic.cpp"

./output/api_usage_examples/example_updateable_periodic &> "./output/api_usage_examples/example_updateable_periodic_output.txt"

rm "./output/api_usage_examples/example_updateable_periodic"

####################################################################

mkdir -p "./output/tricky_cases_graphics"

########################################

{
cat << 'EOF'
0.0 0.0 0.0 1.0
0.5 0.0 0.0 1.0
1.0 0.0 0.0 1.0
EOF
} \
| voronota-lt -probe 1.0 -graphics-output-file-for-pymol "./output/tricky_cases_graphics/draw_case_1.py" -graphics-title "case_1" -graphics-restrict-representations balls faces wireframe sas sasmesh 2> /dev/null

########################################

{
cat << 'EOF'
0 0 0 1
0 0 1 1
0 1 0 1
0 1 1 1
EOF
} \
| voronota-lt -probe 2.0 -graphics-output-file-for-pymol "./output/tricky_cases_graphics/draw_case_2.py" -graphics-title "case_2" -graphics-restrict-representations balls faces wireframe sas sasmesh 2> /dev/null

########################################

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
| voronota-lt -probe 2.0 -graphics-output-file-for-pymol "./output/tricky_cases_graphics/draw_case_3.py" \
  -graphics-title "case_3" -graphics-restrict-representations balls faces wireframe sas sasmesh 2> /dev/null

########################################

{
cat << 'EOF'
0 0 0 1
0 0 0 0.5
1 0 0 1
EOF
} \
| voronota-lt -probe 0.5 -graphics-output-file-for-pymol "./output/tricky_cases_graphics/draw_case_4.py" \
  -graphics-title "case_4" -graphics-restrict-representations balls faces wireframe sas sasmesh 2> /dev/null

########################################

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
| voronota-lt -probe 1.0 -graphics-output-file-for-pymol "./output/tricky_cases_graphics/draw_case_5.py" \
  -graphics-title "case_5" -graphics-restrict-representations balls faces wireframe sas sasmesh 2> /dev/null

########################################

{
cat << 'EOF'
0 0 0 0.5
0 0 1 0.5
0 1 0 0.5
0 1 1 0.5
1 0 0 0.5
1 0 1 0.5
1 1 0 0.5
1 1 1 0.5
EOF
} \
| voronota-lt -probe 0.75 -periodic-box-corners -1 -1 -1 2 2 2 -graphics-output-file-for-pymol "./output/tricky_cases_graphics/draw_case_6.py" \
  -graphics-title "case_6" -graphics-restrict-representations balls faces wireframe sas sasmesh lattice 2> /dev/null

########################################

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
| voronota-lt -probe 1.0 -periodic-box-corners -1.6 -1.6 -0.6 1.6 1.6 3.1 -graphics-output-file-for-pymol "./output/tricky_cases_graphics/draw_case_7.py" \
  -graphics-title "case_7" -graphics-restrict-representations balls faces wireframe sas sasmesh lattice 2> /dev/null

########################################

{
cat << 'EOF'
big 0 0 2 1
small 0 1 0 0.5
small 0.38268343236509 0.923879532511287 0 0.5
small 0.707106781186547 0.707106781186548 0 0.5
small 0.923879532511287 0.38268343236509 0 0.5
small 1 6.12323399573677e-17 0 0.5
small 0.923879532511287 -0.38268343236509 0 0.5
small 0.707106781186548 -0.707106781186547 0 0.5
small 0.38268343236509 -0.923879532511287 0 0.5
small 1.22464679914735e-16 -1 0 0.5
small -0.38268343236509 -0.923879532511287 0 0.5
small -0.707106781186547 -0.707106781186548 0 0.5
small -0.923879532511287 -0.38268343236509 0 0.5
small -1 -1.83697019872103e-16 0 0.5
small -0.923879532511287 0.38268343236509 0 0.5
small -0.707106781186548 0.707106781186547 0 0.5
small -0.38268343236509 0.923879532511287 0 0.5
EOF
} \
| voronota-lt \
  -probe 1.0 \
  -graphics-output-file-for-pymol "./output/tricky_cases_graphics/draw_case_8.py" \
  -graphics-title "case_8" \
  -graphics-restrict-representations balls faces wireframe \
  -graphics-coloring-config "ball balls C0C0C0; contact faces 000000" \
2> /dev/null

########################################

{
cat << 'EOF'
big 0 0 2 1
small 0 1 0 0.5
small 0.38268343236509 0.923879532511287 0 0.5
small 0.707106781186547 0.707106781186548 0 0.5
small 0.923879532511287 0.38268343236509 0 0.5
small 1 6.12323399573677e-17 0 0.5
small 0.923879532511287 -0.38268343236509 0 0.5
small 0.707106781186548 -0.707106781186547 0 0.5
small 0.38268343236509 -0.923879532511287 0 0.5
small 1.22464679914735e-16 -1 0 0.5
small -0.38268343236509 -0.923879532511287 0 0.5
small -0.707106781186547 -0.707106781186548 0 0.5
small -0.923879532511287 -0.38268343236509 0 0.5
small -1 -1.83697019872103e-16 0 0.5
small -0.923879532511287 0.38268343236509 0 0.5
small -0.707106781186548 0.707106781186547 0 0.5
small -0.38268343236509 0.923879532511287 0 0.5
EOF
} \
| voronota-lt \
  -probe 1.0 \
  -periodic-box-corners -1.6 -1.6 -0.6 1.6 1.6 3.1 \
  -graphics-output-file-for-pymol "./output/tricky_cases_graphics/draw_case_9.py" \
  -graphics-title "case_9" \
  -graphics-restrict-representations balls faces wireframe lattice \
  -graphics-coloring-config-file <(echo -e "ball balls C0C0C0\ncontact faces 000000\nother lattice FF0000") \
2> /dev/null

################################################################################

git status -s ./output/

################################################################################




