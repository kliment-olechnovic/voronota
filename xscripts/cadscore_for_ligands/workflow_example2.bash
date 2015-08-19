#!/bin/bash

INDIR=./input
OUTDIR=./output_example2
mkdir -p $OUTDIR

# Creating list of ligands

find $INDIR/pdb_files/ \
  -type f \
  -name "manualfix_*.pdb" \
> $OUTDIR/list_of_ligands_files.txt

# Clustering coordinates

./cluster_coordinates.bash \
  -i $OUTDIR/list_of_ligands_files.txt \
  -o $OUTDIR/clusters_by_coordinates \
  -t 1.5

./generate_clusters_visualizations.bash \
  -c $OUTDIR/clusters_by_coordinates

# Viewing clusters in PyMol

./show_clusters_in_pymol.bash \
  -c $OUTDIR/clusters_by_coordinates \
  -l $OUTDIR/list_of_ligands_files.txt
