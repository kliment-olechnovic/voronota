#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

WORKDIR=$1

cp -r $BINDIR/DOOP $TMPDIR/DOOP
cp $WORKDIR/atoms.pdb $TMPDIR/DOOP/atoms.pdb

cd $TMPDIR/DOOP
echo "atoms.pdb" > list.txt
perl ./calcenergy.pl > $TMPDIR/doop_score

cd - &> /dev/null
mv $TMPDIR/doop_score $WORKDIR/doop_score
