#!/bin/bash

readonly ZEROARG=$0
RECEPTOR_FILE=""
LIGAND_FILE=""
INCLUDE_HYDROGENS_OPTION=""
SYMMETRY_FILE_OPTION=""
OUTPUT_DIR=""
HELP_MODE=false

while getopts "r:l:gs:o:h" OPTION
do
	case $OPTION in
	r)
		RECEPTOR_FILE=$OPTARG
		;;
	l)
		LIGAND_FILE=$OPTARG
		;;
	g)
		INCLUDE_HYDROGENS_OPTION="--include-hydrogens"
		;;
	s)
		SYMMETRY_FILE_OPTION=" --renaming-map $OPTARG "
		;;
	o)
		OUTPUT_DIR=$OPTARG
		;;
	h)
		HELP_MODE=true
		;;
	esac
done

if [ -z "$RECEPTOR_FILE" ] || [ -z "$LIGAND_FILE" ] || $HELP_MODE
then
cat >&2 << EOF
Script parameters:
    -r receptor_file.pdb
    -l ligand_file.pdb
    [-g] (flag to include hydrogen atoms)
    [-s symmetry_map.txt]
    [-o output_directory]
EOF
exit 1
fi

if [[ $ZEROARG == *"/"* ]]
then
	cd $(dirname $ZEROARG)
	export PATH=$(pwd):$PATH
	cd - &> /dev/null
fi

command -v voronota &> /dev/null || { echo >&2 "Error: 'voronota' executable not in binaries path"; exit 1; }
command -v voronota-resources &> /dev/null || { echo >&2 "Error: 'voronota-resources' executable not in binaries path"; exit 1; }

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

cat $RECEPTOR_FILE \
| voronota get-balls-from-atoms-file \
  --include-heteroatoms $INCLUDE_HYDROGENS_OPTION \
  --annotated \
  --radii-file <(voronota-resources radii) \
| voronota query-balls \
  --drop-atom-serials \
> $TMPDIR/receptor_balls

cat $LIGAND_FILE \
| voronota get-balls-from-atoms-file \
  --include-heteroatoms $INCLUDE_HYDROGENS_OPTION \
  --annotated \
  --radii-file <(voronota-resources radii) \
| voronota query-balls \
  --drop-atom-serials \
> $TMPDIR/ligand_balls

cat $TMPDIR/receptor_balls $TMPDIR/ligand_balls \
| voronota calculate-contacts \
  --annotated \
| voronota query-contacts \
  --match-external-first $TMPDIR/ligand_balls $SYMMETRY_FILE_OPTION \
> $TMPDIR/all_contacts

{
cat $TMPDIR/all_contacts \
| voronota query-contacts \
  --match-external-first $TMPDIR/ligand_balls \
  --match-external-second $TMPDIR/receptor_balls

cat $TMPDIR/all_contacts \
| voronota query-contacts \
  --match-external-first $TMPDIR/ligand_balls \
  --match-second 'c<solvent>'
} > $TMPDIR/contacts

if [ -n "$OUTPUT_DIR" ]
then
	mkdir -p $OUTPUT_DIR
	mv $TMPDIR/contacts $OUTPUT_DIR/$(basename $LIGAND_FILE)
else
	cat $TMPDIR/contacts
fi
