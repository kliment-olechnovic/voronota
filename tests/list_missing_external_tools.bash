#!/bin/bash

for TOOLCOMMAND in g++ cppcheck mpic++ mpirun cmake dssp hbplus
do
	command -v $TOOLCOMMAND &> /dev/null || { echo "'$TOOLCOMMAND' executable not in binaries path"; }
done
