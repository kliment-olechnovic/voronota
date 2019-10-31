#!/bin/bash

for TOOLCOMMAND in g++ mpic++ mpirun cmake dssp hbplus Scwrl4 valgrind
do
	command -v $TOOLCOMMAND &> /dev/null || { echo "'$TOOLCOMMAND' executable not in binaries path"; }
done
