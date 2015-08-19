#!/bin/bash

INDIR=./input
OUTDIR=./output_example1
mkdir -p $OUTDIR

# Creating list of ligands

find $INDIR/pdb_files/ \
  -type f \
  -name "manualfix_*.pdb" \
> $OUTDIR/list_of_ligands_files.txt

# Calculating contacts

./calc_receptor_and_multiple_ligands_contacts.bash \
  -r $INDIR/pdb_files/receptor.pdb \
  -l $OUTDIR/list_of_ligands_files.txt \
  -s $INDIR/symmetry_map.txt \
  -o $OUTDIR/contacts

# Clustering contacts

./cluster_contacts.bash \
  -i $OUTDIR/contacts \
  -o $OUTDIR/clusters_by_contacts \
  -t 0.6

./generate_clusters_visualizations.bash \
  -c $OUTDIR/clusters_by_contacts

# Clustering contacts without solvent contacts

./filter_multiple_contacts.bash \
  -i $OUTDIR/contacts \
  -o $OUTDIR/contacts_nosolvent

./cluster_contacts.bash \
  -i $OUTDIR/contacts_nosolvent \
  -o $OUTDIR/clusters_by_contacts_nosolvent \
  -t 0.6

./generate_clusters_visualizations.bash \
  -c $OUTDIR/clusters_by_contacts_nosolvent

# Viewing clusters in PyMol

./show_clusters_in_pymol.bash \
  -c $OUTDIR/clusters_by_contacts_nosolvent \
  -l $OUTDIR/list_of_ligands_files.txt
