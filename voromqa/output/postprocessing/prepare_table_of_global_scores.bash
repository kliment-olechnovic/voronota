#!/bin/bash

WORKDIR=$1

{
	echo "set_name target model qscore_atom cadscore_atom cadscore_residue cadscore_atom_avg cadscore_residue_avg goap dfire goap_ag tmscore atomscount qarea qenergy qsas rwplus doop"
	cat $WORKDIR/concatenated_scores_lists \
	| sed 's|^.*/entries/\(.*\)|\1|' \
	| sed 's|models/||' \
	| sed 's|/| |g' \
	| sed 's|CADSCORE._NA|1|g' \
	| sed 's|TMSCORE_NA|1|g' \
	| egrep -v 'GOAPSCORE._NA' \
	| egrep -v 'DOOP_NA' \
	| sort
} | column -t > $WORKDIR/table_of_global_scores
