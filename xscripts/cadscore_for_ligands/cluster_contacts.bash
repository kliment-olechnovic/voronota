#!/bin/bash

set +e

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
| voronota vectorize-contacts \
  --clustering-output $OUTPUT_DIR/list_of_clusters.txt \
  --clustering-threshold $CLUSTERING_THRESHOLD \
  --consensus-list $OUTPUT_DIR/list_of_consensus_scores.txt \
> /dev/null

{
CLUSTER_ID=1
IFS=''
cat $OUTPUT_DIR/list_of_clusters.txt | sed 's/.pdb//g' | while read LINE
do
	TOKEN_ID=1
	echo $LINE | tr ' ' '\n' | while read TOKEN
	do
		echo "create cluster_$CLUSTER_ID, $TOKEN, 1, $TOKEN_ID"
		echo "delete $TOKEN"
		TOKEN_ID=$(( TOKEN_ID+1 ))
	done
	COLOR_NUMBER=$(( (CLUSTER_ID*111222333)%16777215 ))
	COLOR_NUMBER_HEX=$(echo "obase=16; $COLOR_NUMBER" | bc)
	echo "color 0x$COLOR_NUMBER_HEX, cluster_$CLUSTER_ID"
	CLUSTER_ID=$(( CLUSTER_ID+1 ))
done
echo "set all_states, on"
echo "zoom"
} > $OUTPUT_DIR/pymol_commands.pml

if command -v voronota &> /dev/null
then
R --vanilla --args $OUTPUT_DIR/list_of_consensus_scores.txt $OUTPUT_DIR/distribution_of_consensus_scores.png &> /dev/null << 'EOF'
args=commandArgs(TRUE);
t=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
png(args[2]);
plot(density(t$V2), xlab="Consensus score", main="Distribution of consensus scores");
dev.off();
EOF
fi
