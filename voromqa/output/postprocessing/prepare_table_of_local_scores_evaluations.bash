#!/bin/bash

WORKDIR=$1

{
	head -1 $WORKDIR/concatenated_local_scores_evaluations
	
	cat $WORKDIR/concatenated_local_scores_evaluations \
	| egrep -v '^target' \
	| egrep ' ' \
	| sort
} | column -t > $WORKDIR/table_of_local_scores_evaluations
