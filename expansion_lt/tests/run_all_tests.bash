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

voronota-lt --help-full &> "./output/help_message_full.txt"

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

voronota-lt -processors 40 -probe 1.4 -test-updateable-tessellation < "./input/balls_2zsk.xyzr" 2> "./output/contacts_2zsk_summary_updateable_test.txt"

voronota-lt -processors 40 -probe 1.4 -test-updateable-tessellation < "./input/balls_3dlb.xyzr" 2> "./output/contacts_3dlb_summary_updateable_test.txt"

voronota-lt -processors 40 -probe 1.4 -test-updateable-tessellation < "./input/balls_7br8.xyzr" 2> "./output/contacts_7br8_summary_updateable_test.txt"

voronota-lt \
  -processors 40 \
  -probe 2 \
  -periodic-box-corners 0 0 0 200 250 300 \
  -test-updateable-tessellation \
< "./input/balls_cs_1x1.xyzr" \
2> "./output/contacts_cs_1x1_periodic_summary_updateable_test.txt"

####################################################################

voronota-lt -processors 40 -probe 1.4 -test-updateable-tessellation-with-backup < "./input/balls_2zsk.xyzr" 2> "./output/contacts_2zsk_summary_updateable_test_with_backup.txt"

voronota-lt -processors 40 -probe 1.4 -test-updateable-tessellation-with-backup < "./input/balls_3dlb.xyzr" 2> "./output/contacts_3dlb_summary_updateable_test_with_backup.txt"

voronota-lt -processors 40 -probe 1.4 -test-updateable-tessellation-with-backup < "./input/balls_7br8.xyzr" 2> "./output/contacts_7br8_summary_updateable_test_with_backup.txt"

voronota-lt \
  -processors 40 \
  -probe 2 \
  -periodic-box-corners 0 0 0 200 250 300 \
  -test-updateable-tessellation-with-backup \
< "./input/balls_cs_1x1.xyzr" \
2> "./output/contacts_cs_1x1_periodic_summary_updateable_test_with_backup.txt"

####################################################################

voronota-lt -processors 40 -probe 1.4 -test-maskable-tessellation < "./input/balls_2zsk.xyzr" 2> "./output/contacts_2zsk_summary_maskable_test.txt"

voronota-lt -processors 40 -probe 1.4 -test-maskable-tessellation < "./input/balls_3dlb.xyzr" 2> "./output/contacts_3dlb_summary_maskable_test.txt"

voronota-lt -processors 40 -probe 1.4 -test-maskable-tessellation < "./input/balls_7br8.xyzr" 2> "./output/contacts_7br8_summary_maskable_test.txt"

voronota-lt \
  -processors 40 \
  -probe 2 \
  -periodic-box-corners 0 0 0 200 250 300 \
  -test-maskable-tessellation \
< "./input/balls_cs_1x1.xyzr" \
2> "./output/contacts_cs_1x1_periodic_summary_maskable_test.txt"

####################################################################

voronota-lt -processors 1 -probe 1.4 -test-second-order-cell-volumes-calculation < "./input/balls_2zsk.xyzr" > "./output/contacts_2zsk_second_order_cell_volumes_test.txt"

####################################################################

mkdir -p "./output/api_usage_examples"

g++ -O3 -I../src/voronotalt -o "./output/api_usage_examples/example_basic_and_periodic" "./input/api_usage_example_basic_and_periodic.cpp"

./output/api_usage_examples/example_basic_and_periodic &> "./output/api_usage_examples/example_basic_and_periodic_output.txt"

rm "./output/api_usage_examples/example_basic_and_periodic"

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
| voronota-lt -probe 1.0 -graphics-output-file "./output/tricky_cases_graphics/draw_case_1.py" -graphics-title "case_1" -graphics-restrict-representations balls faces wireframe xspheres 2> /dev/null

########################################

{
cat << 'EOF'
0 0 0 1
0 0 1 1
0 1 0 1
0 1 1 1
EOF
} \
| voronota-lt -probe 2.0 -graphics-output-file "./output/tricky_cases_graphics/draw_case_2.py" -graphics-title "case_2" -graphics-restrict-representations balls faces wireframe xspheres 2> /dev/null

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
| voronota-lt -probe 2.0 -graphics-output-file "./output/tricky_cases_graphics/draw_case_3.py" \
  -graphics-title "case_3" -graphics-restrict-representations balls faces wireframe xspheres 2> /dev/null

########################################

{
cat << 'EOF'
0 0 0 1
0 0 0 0.5
1 0 0 1
EOF
} \
| voronota-lt -probe 0.5 -graphics-output-file "./output/tricky_cases_graphics/draw_case_4.py" \
  -graphics-title "case_4" -graphics-restrict-representations balls faces wireframe xspheres 2> /dev/null

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
| voronota-lt -probe 1.0 -graphics-output-file "./output/tricky_cases_graphics/draw_case_5.py" \
  -graphics-title "case_5" -graphics-restrict-representations balls faces wireframe xspheres 2> /dev/null

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
| voronota-lt -probe 0.75 -periodic-box-corners -1 -1 -1 2 2 2 -graphics-output-file "./output/tricky_cases_graphics/draw_case_6.py" \
  -graphics-title "case_6" -graphics-restrict-representations balls faces wireframe xspheres 2> /dev/null

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
| voronota-lt -probe 1.0 -periodic-box-corners -1.6 -1.6 -0.6 1.6 1.6 3.1 -graphics-output-file "./output/tricky_cases_graphics/draw_case_7.py" \
  -graphics-title "case_7" -graphics-restrict-representations balls faces wireframe xspheres 2> /dev/null

################################################################################

git status -s ./output/

################################################################################

../swig/tests/run_all_tests.bash


