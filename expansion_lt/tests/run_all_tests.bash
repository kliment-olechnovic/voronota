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

voronota-lt -processors 40 -probe 2  < "./input/balls_cs_1x1.xyzr" \
  -write-contacts-to-file ./output/written_files/output_cs_1x1_contacts.txt \
  -write-cells-to-file ./output/written_files/output_cs_1x1_cells.txt \
2> "./output/contacts_cs_1x1_summary.txt"

voronota-lt -processors 40 -probe 2  < "./input/balls_cs_1x1.xyzr" -periodic-box-corners 0 0 0 200 250 300 \
  -write-contacts-to-file ./output/written_files/output_cs_1x1_periodic_contacts.txt \
  -write-cells-to-file ./output/written_files/output_cs_1x1_periodic_cells.txt \
2> "./output/contacts_cs_1x1_periodic_summary.txt"

voronota-lt -processors 40 -probe 2  < "./input/balls_cs_3x3.xyzr" -write-cells-chain-level-to-file ./output/written_files/output_cs_3x3_cells_chain_level.txt 2> "./output/contacts_cs_3x3_summary.txt"

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
| voronota-lt -probe 1.0 -write-contacts-graphics-to-file "./output/tricky_cases_graphics/draw_case_1.py" -graphics-title "case_1" 2> /dev/null

########################################

{
cat << 'EOF'
0 0 0 1
0 0 1 1
0 1 0 1
0 1 1 1
EOF
} \
| voronota-lt -probe 2.0 -write-contacts-graphics-to-file "./output/tricky_cases_graphics/draw_case_2.py" -graphics-title "case_2" 2> /dev/null

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
| voronota-lt -probe 2.0 -write-contacts-graphics-to-file "./output/tricky_cases_graphics/draw_case_3.py" -graphics-title "case_3" 2> /dev/null

########################################

{
cat << 'EOF'
0 0 0 1
0 0 0 0.5
1 0 0 1
EOF
} \
| voronota-lt -probe 0.5 -write-contacts-graphics-to-file "./output/tricky_cases_graphics/draw_case_4.py" -graphics-title "case_4" 2> /dev/null

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
| voronota-lt -probe 1.0 -write-contacts-graphics-to-file "./output/tricky_cases_graphics/draw_case_5.py" -graphics-title "case_5" 2> /dev/null

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
| voronota-lt -probe 0.75 -periodic-box-corners -1 -1 -1 2 2 2 -write-contacts-graphics-to-file "./output/tricky_cases_graphics/draw_case_6.py" -graphics-title "case_6" 2> /dev/null

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
| voronota-lt -probe 1.0 -periodic-box-corners -1.6 -1.6 -0.6 1.6 1.6 3.1 -write-contacts-graphics-to-file "./output/tricky_cases_graphics/draw_case_7.py" -graphics-title "case_7" 2> /dev/null

################################################################################

git status -s ./output/

################################################################################

../swig/tests/run_all_tests.bash


