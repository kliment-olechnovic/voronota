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

time -p $TMP_DIR/voronota_openmp --mode calculate-vertices --print-log --clog-file $OUTPUT_DIR/sequential_log < $TMP_DIR/input.balls > /dev/null

time -p $TMP_DIR/voronota_openmp --mode calculate-vertices-in-parallel --method openmp --parts 4 --print-log --clog-file $OUTPUT_DIR/openmp_4p_log < $TMP_DIR/input.balls > /dev/null
time -p $TMP_DIR/voronota_openmp --mode calculate-vertices-in-parallel --method openmp --parts 8 --print-log --clog-file $OUTPUT_DIR/openmp_8p_log < $TMP_DIR/input.balls > /dev/null
time -p $TMP_DIR/voronota_openmp --mode calculate-vertices-in-parallel --method openmp --parts 16 --print-log --clog-file $OUTPUT_DIR/openmp_16p_log < $TMP_DIR/input.balls > /dev/null
time -p $TMP_DIR/voronota_openmp --mode calculate-vertices-in-parallel --method openmp --parts 32 --print-log --clog-file $OUTPUT_DIR/openmp_32p_log < $TMP_DIR/input.balls > /dev/null

$MPI_BIN_DIR/mpirun -n 5 $TMP_DIR/voronota_mpi --mode calculate-vertices-in-parallel --method mpi --parts 4 --print-log --clog-file $OUTPUT_DIR/mpi_4p_log < $TMP_DIR/input.balls > /dev/null
$MPI_BIN_DIR/mpirun -n 5 $TMP_DIR/voronota_mpi --mode calculate-vertices-in-parallel --method mpi --parts 8 --print-log --clog-file $OUTPUT_DIR/mpi_8p_log < $TMP_DIR/input.balls > /dev/null
$MPI_BIN_DIR/mpirun -n 5 $TMP_DIR/voronota_mpi --mode calculate-vertices-in-parallel --method mpi --parts 16 --print-log --clog-file $OUTPUT_DIR/mpi_16p_log < $TMP_DIR/input.balls > /dev/null
$MPI_BIN_DIR/mpirun -n 5 $TMP_DIR/voronota_mpi --mode calculate-vertices-in-parallel --method mpi --parts 32 --print-log --clog-file $OUTPUT_DIR/mpi_32p_log < $TMP_DIR/input.balls > /dev/null

rm -r "$TMP_DIR"

cat ./output/*_log | egrep '^tangent_spheres'
