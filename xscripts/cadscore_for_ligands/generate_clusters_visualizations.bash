#!/bin/bash

set +e

readonly ZEROARG=$0
CLUSTERS_DIR=""
HELP_MODE=false

while getopts "c:h" OPTION
do
	case $OPTION in
	c)
		CLUSTERS_DIR=$OPTARG
		;;
	h)
		HELP_MODE=true
		;;
	esac
done

if [ -z "$CLUSTERS_DIR" ] || $HELP_MODE
then
cat >&2 << EOF
Script parameters:
    -c clusters_directory
EOF
exit 1
fi

{
CLUSTER_ID=1
IFS=''
cat $CLUSTERS_DIR/list_of_clusters.txt | sed 's/.pdb//g' | while read LINE
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
} > $CLUSTERS_DIR/pymol_commands.pml

if command -v R &> /dev/null
then
R --vanilla --args $CLUSTERS_DIR/list_of_consensus_scores.txt $CLUSTERS_DIR/distribution_of_consensus_scores.png &> /dev/null << 'EOF'
args=commandArgs(TRUE);
t=read.table(args[1], header=FALSE, stringsAsFactors=FALSE);
png(args[2]);
plot(density(t$V2), xlab="Consensus score", main="Distribution of consensus scores");
dev.off();
EOF
fi
