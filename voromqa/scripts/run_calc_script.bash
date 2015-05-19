#!/bin/bash

export BINDIR=$1
COMMANDBUNDLE=$2

for ARGVALUE in ${@:3}
do
	$COMMANDBUNDLE $ARGVALUE
done
