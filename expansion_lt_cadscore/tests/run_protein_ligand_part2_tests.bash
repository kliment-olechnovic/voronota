#!/bin/bash

cd $(dirname "$0")

mkdir -p ./output/protein_ligand2/

####################################################################

exec > ./output/protein_ligand2/full_log 2>&1
set -v

####################################################################

cadscore-lt \
  --include-heteroatoms \
  --models "./input/protein_ligand2/models" \
  --scoring-levels atom residue \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-protein] -a2! [-protein]]" \
  --subselect-atoms "[-protein]" \
  --output-with-f1 \
  --output-with-identities \
| column -t

####################################################################

cadscore-lt \
  --include-heteroatoms \
  --models "./input/protein_ligand2/models" \
  --scoring-levels atom residue \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-protein] -a2! [-protein]]" \
  --subselect-atoms "[-protein]" \
  --output-with-f1 \
  --output-with-identities \
  --reference-sequences-file "./input/protein_ligand2/sequences.fasta" \
  --output-dir ./output/protein_ligand2/output_dir_01 \
  --save-sequence-alignments \
| column -t

####################################################################

cadscore-lt \
  --include-heteroatoms \
  --models "./input/protein_ligand2/models_as" \
  --scoring-levels atom residue \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-protein] -a2! [-protein]]" \
  --subselect-atoms "[-protein]" \
  --output-with-f1 \
  --output-with-identities \
| column -t

####################################################################

cadscore-lt \
  --include-heteroatoms \
  --models "./input/protein_ligand2/models_as" \
  --scoring-levels atom residue \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-protein] -a2! [-protein]]" \
  --subselect-atoms "[-protein]" \
  --output-with-f1 \
  --output-with-identities \
  --reference-sequences-file "./input/protein_ligand2/sequences_as.fasta" \
  --output-dir ./output/protein_ligand2/output_dir_02 \
  --save-sequence-alignments \
| column -t

####################################################################
