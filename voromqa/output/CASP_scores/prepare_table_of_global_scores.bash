#!/bin/bash

{
	echo "casp_num target model qscore_atom qscore_residue cadscore_atom cadscore_residue goap dfire goap_ag"
	
	cat ./concatenated_scores_lists \
	| sed 's|^.*/entries/\(.*\)|\1|' \
	| sed 's|models/||' \
	| sed 's|/| |g' \
	| sed 's|CADSCORE1_NA|1|g' \
	| sed 's|CADSCORE2_NA|1|g' \
	| sort
} | column -t
