#!/bin/bash

readonly COMMANDBUNDLE=$1

for ARGVALUE in ${@:2}
do
	$COMMANDBUNDLE $ARGVALUE
done
