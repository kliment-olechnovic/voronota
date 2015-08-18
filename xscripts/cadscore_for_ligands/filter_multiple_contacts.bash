#!/bin/bash

set +e

readonly ZEROARG=$0
INPUT_DIR=""
OUTPUT_DIR=""
HELP_MODE=false

while getopts "i:o:h" OPTION
do
	case $OPTION in
	i)
		INPUT_DIR=$OPTARG
		;;
	o)
		OUTPUT_DIR=$OPTARG
		;;
	h)
		HELP_MODE=true
		;;
	esac
done

if [ -z "$INPUT_DIR" ] || [ -z "$OUTPUT_DIR" ] || $HELP_MODE
then
cat >&2 << EOF
Script parameters:
    -i input_directory
    -o output_directory
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

find $INPUT_DIR -type f | while read INPUT_FILE
do
	cat $INPUT_FILE | voronota query-contacts --no-solvent > $OUTPUT_DIR/$(basename $INPUT_FILE)
done
