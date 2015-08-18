#!/bin/bash

set +e

readonly ZEROARG=$0
CLUSTERS_DIR=""
LIGANDS_FILES_LIST=""
RECEPTOR_FILE=""
HELP_MODE=false

while getopts "c:l:r:h" OPTION
do
	case $OPTION in
	c)
		CLUSTERS_DIR=$OPTARG
		;;
	l)
		LIGANDS_FILES_LIST=$OPTARG
		;;
	r)
		RECEPTOR_FILE=$OPTARG
		;;
	h)
		HELP_MODE=true
		;;
	esac
done

if [ -z "$CLUSTERS_DIR" ] || [ -z "$LIGANDS_FILES_LIST" ] || $HELP_MODE
then
cat >&2 << EOF
Script parameters:
    -c clusters_directory
    -l list_of_ligands_files.txt
    [-r receptor_file.pdb]
EOF
exit 1
fi

PYMOL_SCRIPT_FILE="$CLUSTERS_DIR/pymol_commands.pml"

pymol $RECEPTOR_FILE $(cat $LIGANDS_FILES_LIST) $PYMOL_SCRIPT_FILE
