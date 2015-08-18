#!/bin/bash

mkdir -p ./output

find ./input/pdb_files/ \
  -type f \
  -name "manualfix_*.pdb" \
> ./output/list_of_ligands_files.txt

./calc_receptor_and_multiple_ligands_contacts.bash \
  -r ./input/pdb_files/receptor.pdb \
  -l ./output/list_of_ligands_files.txt \
  -s ./input/symmetry_map.txt \
  -o ./output/contacts

./filter_multiple_contacts.bash \
  -i ./output/contacts \
  -o ./output/contacts_nosolvent

./cluster_contacts.bash \
  -i ./output/contacts_nosolvent \
  -o ./output/clusters \
  -t 0.6

./show_clusters_in_pymol.bash \
  -c ./output/clusters \
  -l ./output/list_of_ligands_files.txt
