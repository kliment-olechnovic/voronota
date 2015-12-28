#!/bin/bash

readonly ZEROARG=$0
RECEPTOR_FILE=""
LIGANDS_FILES_LIST=""
OUTPUT_DIR=""
INCLUDE_HYDROGENS_OPTION=""
SYMMETRY_INPUT_FILE_OPTION=""
PROCESSORS_COUNT=4
HELP_MODE=false

while getopts "r:l:o:gs:p:h" OPTION
do
	case $OPTION in
	r)
		RECEPTOR_FILE=$OPTARG
		;;
	l)
		LIGANDS_FILES_LIST=$OPTARG
		;;
	o)
		OUTPUT_DIR=$OPTARG
		;;
	g)
		INCLUDE_HYDROGENS_OPTION="-g"
		;;
	s)
		SYMMETRY_INPUT_FILE_OPTION=" -s $OPTARG "
		;;
	p)
		PROCESSORS_COUNT=$OPTARG
		;;
	h)
		HELP_MODE=true
		;;
	esac
done

if [ -z "$RECEPTOR_FILE" ] || [ -z "$LIGANDS_FILES_LIST" ] || [ -z "$OUTPUT_DIR" ] || $HELP_MODE
then
cat >&2 << EOF
Script parameters:
    -r receptor_file.pdb
    -l list_of_ligands_files.txt
    -o output_directory
    [-g] (flag to include hydrogen atoms)
    [-s symmetry_map.txt]
    [-p usable_processors_counts]
EOF
exit 1
fi

if [[ $ZEROARG == *"/"* ]]
then
	cd $(dirname $ZEROARG)
	export PATH=$(pwd):$PATH
	cd - &> /dev/null
fi

command -v calc_receptor_and_ligand_contacts.bash &> /dev/null || { echo >&2 "Error: 'calc_receptor_and_ligand_contacts.bash' executable not in binaries path"; exit 1; }

cat $LIGANDS_FILES_LIST \
| xargs -L 1 -P $PROCESSORS_COUNT \
  calc_receptor_and_ligand_contacts.bash \
  -r $RECEPTOR_FILE $SYMMETRY_INPUT_FILE_OPTION $INCLUDE_HYDROGENS_OPTION \
  -o $OUTPUT_DIR \
  -l
