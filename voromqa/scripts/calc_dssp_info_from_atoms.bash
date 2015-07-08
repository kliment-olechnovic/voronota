#!/bin/bash

WORKDIR=$1

$BINDIR/dssp $WORKDIR/atoms.pdb > $WORKDIR/dssp_info
