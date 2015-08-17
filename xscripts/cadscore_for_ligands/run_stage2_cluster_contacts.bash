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
  --clustering-output $CLUSTERS_OUTPUT_DIR/list.txt \
  --clustering-threshold $CLUSTERING_THRESHOLD \
> /dev/null

{
CLUSTER_ID=1
IFS=''
cat $CLUSTERS_OUTPUT_DIR/list.txt | sed 's/.pdb//g' | while read LINE
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
