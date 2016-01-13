#!/bin/bash

readonly ZEROARG=$0
LIGANDS_FILES_LIST=""=""
OUTPUT_DIR=""
CLUSTERING_THRESHOLD=""
HELP_MODE=false

while getopts "i:o:t:h" OPTION
do
	case $OPTION in
	i)
		LIGANDS_FILES_LIST=$OPTARG
		;;
	o)
		OUTPUT_DIR=$OPTARG
		;;
	t)
		CLUSTERING_THRESHOLD=$OPTARG
		;;
	h)
		HELP_MODE=true
		;;
	esac
done

if [ -z "$LIGANDS_FILES_LIST" ] || [ -z "$OUTPUT_DIR" ] || [ -z "$CLUSTERING_THRESHOLD" ] || $HELP_MODE
then
cat >&2 << EOF
Script parameters:
    -i list_of_ligands_files.txt
    -o output_directory
    -t clustering_threshold
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

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

cat $LIGANDS_FILES_LIST | while read LIGAND_FILE
do
	cat $LIGAND_FILE | voronota get-balls-from-atoms-file --annotated > $TMPDIR/$(basename $LIGAND_FILE)
done

mkdir -p $OUTPUT_DIR

find $TMPDIR -type f \
| voronota x-vectorize-points \
  --clustering-output $OUTPUT_DIR/list_of_clusters.txt \
  --clustering-threshold $CLUSTERING_THRESHOLD \
  --consensus-list $OUTPUT_DIR/list_of_consensus_scores.txt \
> /dev/null
