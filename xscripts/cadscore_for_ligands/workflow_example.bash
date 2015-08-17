#!/bin/bash

find ./input/pdb_files/ -type f -name "manualfix_*.pdb" > ./list_of_ligands_files.txt

./run_stage1_calc_receptor_and_multiple_ligands_contacts.bash -r ./input/pdb_files/receptor.pdb -l ./list_of_ligands_files.txt -s ./input/symmetry_map.txt -w ./workdir

./run_stage2_cluster_contacts.bash -w ./workdir -t 0.6

./run_stage3_show_clusters_in_pymol.bash -w ./workdir -l ./list_of_ligands_files.txt
