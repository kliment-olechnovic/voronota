#!/bin/bash

readonly RECEPTOR_FILE="./input/pdb_files/receptor.pdb"
readonly LIGANDS_FILES_SEARCH_DIR="./input/pdb_files"
readonly LIGAND_FILE_NAME_PATTERN="manualfix_*.pdb"
readonly SYMMETRY_INPUT_FILE="./input/symmetry_map.txt"
readonly CONTACTS_OUTPUT_DIR="./output/contacts"

find $LIGANDS_FILES_SEARCH_DIR -type f -name "$LIGAND_FILE_NAME_PATTERN" \
| xargs -L 1 -P 4 \
  ./calc_receptor_and_ligand_contacts.bash \
  -r $RECEPTOR_FILE \
  -s $SYMMETRY_INPUT_FILE \
  -o $CONTACTS_OUTPUT_DIR \
  -l
