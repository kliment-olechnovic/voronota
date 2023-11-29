#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

cd ../

export PATH="$(pwd):${PATH}"

cd - &> /dev/null

####################################################################

voronota-lt -processors 40 -probe 1.4 -print-everything < "./input/balls_2zsk.xyzr" 2> "./output/contacts_2zsk_summary.txt" > "./output/contacts_2zsk_tables.txt"

voronota-lt -processors 40 -probe 1.4  < "./input/balls_3dlb.xyzr" 2> "./output/contacts_3dlb_summary.txt"

voronota-lt -processors 40 -probe 1.4  < "./input/balls_7br8.xyzr" 2> "./output/contacts_7br8_summary.txt"

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

################################################################################

git status -s ./output/

