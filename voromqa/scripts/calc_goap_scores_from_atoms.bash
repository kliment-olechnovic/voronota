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
  --drop-adjuncts \
| $BINDIR/voronota query-balls \
  --set-adjuncts "oc=1;tf=0" \
  --pdb-output $WORKDIR/atoms.pdb \
> /dev/null

cd $WORKDIR
echo -e "$BINDIR/goap\natoms.pdb" \
| $BINDIR/goap/goap \
> $WORKDIR/goap_scores
