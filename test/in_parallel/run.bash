#!/bin/bash

cd $(dirname "$0")

OUTPUT_DIR=./output/

rm -r -f $OUTPUT_DIR
mkdir $OUTPUT_DIR

MPI_BIN_DIR="/usr/lib64/openmpi/bin/"

TMP_DIR=$(mktemp -d)

zcat input.balls.gz | head -10000 > $TMP_DIR/input.balls

g++ -O3 -fopenmp -o $TMP_DIR/voronota_openmp ../../src/*.cpp

$MPI_BIN_DIR/mpic++ -O3 -DENABLE_MPI -o $TMP_DIR/voronota_mpi ../../src/*.cpp

time -p $TMP_DIR/voronota_openmp calculate-vertices --print-log < $TMP_DIR/input.balls > /dev/null 2> $OUTPUT_DIR/sequential_log

time -p $TMP_DIR/voronota_openmp calculate-vertices-in-parallel --method openmp --parts 4 --print-log < $TMP_DIR/input.balls > /dev/null 2> $OUTPUT_DIR/openmp_4p_log
time -p $TMP_DIR/voronota_openmp calculate-vertices-in-parallel --method openmp --parts 8 --print-log < $TMP_DIR/input.balls > /dev/null 2> $OUTPUT_DIR/openmp_8p_log
time -p $TMP_DIR/voronota_openmp calculate-vertices-in-parallel --method openmp --parts 16 --print-log < $TMP_DIR/input.balls > /dev/null 2> $OUTPUT_DIR/openmp_16p_log
time -p $TMP_DIR/voronota_openmp calculate-vertices-in-parallel --method openmp --parts 32 --print-log < $TMP_DIR/input.balls > /dev/null 2> $OUTPUT_DIR/openmp_32p_log

$MPI_BIN_DIR/mpirun -n 5 $TMP_DIR/voronota_mpi calculate-vertices-in-parallel --method mpi --parts 4 --print-log < $TMP_DIR/input.balls > /dev/null 2> $OUTPUT_DIR/mpi_4p_log
$MPI_BIN_DIR/mpirun -n 5 $TMP_DIR/voronota_mpi calculate-vertices-in-parallel --method mpi --parts 8 --print-log < $TMP_DIR/input.balls > /dev/null 2> $OUTPUT_DIR/mpi_8p_log
$MPI_BIN_DIR/mpirun -n 5 $TMP_DIR/voronota_mpi calculate-vertices-in-parallel --method mpi --parts 16 --print-log < $TMP_DIR/input.balls > /dev/null 2> $OUTPUT_DIR/mpi_16p_log
$MPI_BIN_DIR/mpirun -n 5 $TMP_DIR/voronota_mpi calculate-vertices-in-parallel --method mpi --parts 32 --print-log < $TMP_DIR/input.balls > /dev/null 2> $OUTPUT_DIR/mpi_32p_log

find $OUTPUT_DIR/ -type f -name "mpi_*p_log" | while read MPI_LOG_FILE
do
	echo $(basename $MPI_LOG_FILE)
	cat $MPI_LOG_FILE | grep seconds | sort
	cat $MPI_LOG_FILE | grep -v seconds | sponge $MPI_LOG_FILE
done

cat ./output/*_log | egrep '^tangent_spheres'

rm -r "$TMP_DIR"
