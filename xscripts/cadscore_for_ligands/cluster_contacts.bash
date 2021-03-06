#!/bin/bash

readonly ZEROARG=$0
INPUT_DIR=""
OUTPUT_DIR=""
CLUSTERING_THRESHOLD=""
HELP_MODE=false

while getopts "i:o:t:h" OPTION
do
	case $OPTION in
	i)
		INPUT_DIR=$OPTARG
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

if [ -z "$INPUT_DIR" ] || [ -z "$OUTPUT_DIR" ] || [ -z "$CLUSTERING_THRESHOLD" ] || $HELP_MODE
then
cat >&2 << EOF
Script parameters:
    -i input_directory
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

mkdir -p $OUTPUT_DIR

find $INPUT_DIR -type f \
| voronota x-vectorize-contacts \
  --clustering-output $OUTPUT_DIR/list_of_clusters.txt \
  --clustering-threshold $CLUSTERING_THRESHOLD \
  --consensus-list $OUTPUT_DIR/list_of_consensus_scores.txt \
> /dev/null
