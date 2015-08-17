#!/bin/bash

readonly CONTACTS_INPUT_DIR="./output/contacts"
readonly CLUSTERS_OUTPUT_DIR="./output/clusters"
CLUSTERING_THRESHOLD="0.6"

if [ -n "$1" ]
then
	CLUSTERING_THRESHOLD=$1
fi

mkdir -p $CLUSTERS_OUTPUT_DIR

find $CONTACTS_INPUT_DIR -type f \
| voronota vectorize-contacts \
  --clustering-output $CLUSTERS_OUTPUT_DIR/list_of_clusters.txt \
  --clustering-threshold $CLUSTERING_THRESHOLD \
  --consensus-list $CLUSTERS_OUTPUT_DIR/list_of_consensus_scores.txt \
> /dev/null

{
CLUSTER_ID=1
IFS=''
cat $CLUSTERS_OUTPUT_DIR/list_of_clusters.txt | sed 's/.pdb//g' | while read LINE
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
} > $CLUSTERS_OUTPUT_DIR/pymol_commands.pml

if command -v voronota &> /dev/null
then
R --vanilla --args $CLUSTERS_OUTPUT_DIR/list_of_consensus_scores.txt $CLUSTERS_OUTPUT_DIR/distribution_of_consensus_scores.png &> /dev/null << 'EOF'
args=commandArgs(TRUE);
t=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
png(args[2]);
plot(density(t$V2), xlab="Consensus score", main="Distribution of consensus scores");
dev.off();
EOF
fi
