#!/bin/bash

set +e

TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

SUBDIR=$OUTPUTDIR/parallel_vertices_calculations_using_mpi
mkdir -p $SUBDIR

MPI_BIN_DIR="/usr/lib64/openmpi/bin/"

$MPI_BIN_DIR/mpic++ -O3 -DENABLE_MPI -o $TMPDIR/voronota_mpi $VORONOTADIR/src/*.cpp

cat $INPUTDIR/single/structure.pdb \
| $VORONOTA get-balls-from-atoms-file --radii-file $VORONOTADIR/resources/radii --include-heteroatoms \
> $TMPDIR/balls

$MPI_BIN_DIR/mpirun -n 5 $TMPDIR/voronota_mpi calculate-vertices-in-parallel --method mpi --parts 4 --print-log < $TMPDIR/balls > /dev/null 2> $SUBDIR/mpi_4p_log
$MPI_BIN_DIR/mpirun -n 5 $TMPDIR/voronota_mpi calculate-vertices-in-parallel --method mpi --parts 8 --print-log < $TMPDIR/balls > /dev/null 2> $SUBDIR/mpi_8p_log
$MPI_BIN_DIR/mpirun -n 5 $TMPDIR/voronota_mpi calculate-vertices-in-parallel --method mpi --parts 16 --print-log < $TMPDIR/balls > /dev/null 2> $SUBDIR/mpi_16p_log
$MPI_BIN_DIR/mpirun -n 5 $TMPDIR/voronota_mpi calculate-vertices-in-parallel --method mpi --parts 32 --print-log < $TMPDIR/balls > /dev/null 2> $SUBDIR/mpi_32p_log

find $SUBDIR/ -type f -name "mpi_*p_log" | while read MPI_LOG_FILE
do
	cat $MPI_LOG_FILE | grep -v seconds | sponge $MPI_LOG_FILE
done
