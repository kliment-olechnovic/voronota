#!/bin/bash

readonly BINDIR=$1
readonly INPUTFILE=$2

readonly TMPDIR=$(mktemp -d)

cp $BINDIR/hbplus $TMPDIR/hbplus
cp $INPUTFILE $TMPDIR/struct.pdb
cd $TMPDIR
./hbplus ./struct.pdb > /dev/null 2> /dev/null
cat ./struct.hb2

rm -r $TMPDIR
