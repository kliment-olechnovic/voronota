#!/bin/bash

WORKDIR=$1

cd $BINDIR/rwplus

./calRWplus $WORKDIR/atoms.pdb \
| egrep '^RW potential' \
> $WORKDIR/rwplus_score
