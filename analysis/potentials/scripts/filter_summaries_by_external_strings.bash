#!/bin/bash

LIST=$1
DIR=$2

find $DIR -type f | while read FFILE
do
	ANTILIST=$(mktemp)
	( cat $FFILE | awk '{print $1}' ; cat $FFILE | awk '{print $2}' ) | grep -v -f $LIST | sort | uniq > $ANTILIST
	cat $FFILE | grep -v -f $ANTILIST | sponge $FFILE
	rm $ANTILIST
done
