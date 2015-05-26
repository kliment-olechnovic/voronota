#!/bin/bash

{
	head -1 ./concatenated_local_scores_evaluations
	
	cat ./concatenated_local_scores_evaluations \
	| egrep -v '^target' \
	| egrep ' ' \
	| sort
} | column -t
