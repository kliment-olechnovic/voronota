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

cat $WORKDIR/balls \
| $BINDIR/voronota query-balls \
  --reset-serials \
  --drop-adjuncts \
| $BINDIR/voronota query-balls \
  --set-adjuncts "oc=1;tf=0" \
  --pdb-output $WORKDIR/atoms.pdb \
> /dev/null
