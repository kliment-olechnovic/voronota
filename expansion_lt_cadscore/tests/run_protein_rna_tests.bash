#!/bin/bash

cd $(dirname "$0")

mkdir -p ./output/protein_rna1/

####################################################################

exec > ./output/protein_rna1/full_log 2>&1
set -v

####################################################################

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_rna1/6FPQ_target.pdb" \
  -m "./input/protein_rna1" \
  --scoring-types contacts sites \
  --subselect-contacts "[-a1 [-chain A] -a2 [-chain B]]" \
  --subselect-atoms "[-chain A]" \
  --output-with-identities \
| column -t

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_rna1/6FPQ_target.pdb" \
  -m "./input/protein_rna1" \
  --scoring-types contacts sites \
  --subselect-contacts "[-a1 [-protein] -a2 [-nucleic]]" \
  --subselect-atoms "[-protein]" \
  --output-with-identities \
| column -t

####################################################################

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_rna1/6FPQ_target.pdb" \
  -m "./input/protein_rna1" \
  --scoring-levels atom residue \
  --scoring-types contacts sites \
  --subselect-contacts "[-a1 [-protein] -a2 [-nucleic]]" \
  --subselect-atoms "[-protein]" \
| column -t

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_rna1/6FPQ_target.pdb" \
  -m "./input/protein_rna1" \
  --scoring-levels atom residue \
  --scoring-types contacts sites \
  --subselect-contacts "[-a1 [-protein] -a2 [-nucleic]]" \
  --subselect-atoms "[-protein]" \
  --conflate-atom-types \
| column -t

####################################################################

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_rna1/6FPQ_target.pdb" \
  -m "./input/protein_rna1/6FPQ_model1a.pdb" \
  --scoring-levels residue \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-protein] -a2 [-nucleic]]" \
  --conflate-atom-types \
  --local-output-formats pdb contactmap graphics-pymol \
  --local-output-levels residue atom \
  --output-dir ./output/protein_rna1/output_dir_01 \
| column -t

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_rna1/6FPQ_target.pdb" \
  -m "./input/protein_rna1/6FPQ_model1a.pdb" \
  --scoring-levels atom \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-protein] -a2 [-nucleic]]" \
  --conflate-atom-types \
  --local-output-formats contactmap \
  --local-output-levels atom residue chain \
  --output-dir ./output/protein_rna1/output_dir_02 \
| column -t

####################################################################


