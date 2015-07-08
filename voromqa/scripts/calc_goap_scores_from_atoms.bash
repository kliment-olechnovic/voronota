#!/bin/bash

WORKDIR=""

while getopts "d:" OPTION
do
	case $OPTION in
	d)
		WORKDIR=$OPTARG
		;;
	esac
done

cd $WORKDIR
echo -e "$BINDIR/goap\natoms.pdb" \
| $BINDIR/goap/goap \
> $WORKDIR/goap_scores
