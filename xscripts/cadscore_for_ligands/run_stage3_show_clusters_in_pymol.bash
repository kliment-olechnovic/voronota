#!/bin/bash

readonly RECEPTOR_FILE="./input/pdb_files/receptor.pdb"
readonly LIGANDS_FILES_SEARCH_DIR="./input/pdb_files"
readonly LIGAND_FILE_NAME_PATTERN="manualfix_*.pdb"
readonly PYMOL_SCRIPT_FILE="./output/clusters/pymol_commands.pml"

pymol $RECEPTOR_FILE $(find $LIGANDS_FILES_SEARCH_DIR -type f -name "$LIGAND_FILE_NAME_PATTERN") $PYMOL_SCRIPT_FILE
