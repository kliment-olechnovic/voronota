#!/bin/bash

WORKDIR=""

while getopts "d:n:o:" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	?)
		echo "Unrecognized option." 1>&2
		exit 1
		;;
	esac
done

echo $(basename $WORKDIR) $(cat $WORKDIR/balls | grep 'A<CA>' | wc -l) $(cat $WORKDIR/balls | wc -l) > $WORKDIR/count_of_residues_and_atoms
