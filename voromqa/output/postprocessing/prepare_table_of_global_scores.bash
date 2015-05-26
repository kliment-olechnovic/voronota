#!/bin/bash

WORKDIR=$1

{
	echo "casp_num target model qscore_atom qscore_residue cadscore_atom cadscore_residue cadscore_atom_avg cadscore_residue_avg goap dfire goap_ag tmscore atomscount qarea qenergy qsas"
	cat $WORKDIR/concatenated_scores_lists \
	| sed 's|^.*/entries/\(.*\)|\1|' \
	| sed 's|models/||' \
	| sed 's|/| |g' \
	| sed 's|CADSCORE._NA|1|g' \
	| sed 's|TMSCORE_NA|1|g' \
	| sort
} | grep -v "T0759" | column -t > $WORKDIR/table_of_global_scores
