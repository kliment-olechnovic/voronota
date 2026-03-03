#!/bin/bash

cd $(dirname "$0")

mkdir -p ./output/protein_amyloids1/

####################################################################

exec > ./output/protein_amyloids1/full_log 2>&1
set -v

####################################################################

cadscore-lt \
  -t "./input/protein_amyloids1/7LNA-assembly1.cif" \
  -m "./input/protein_amyloids1/7QIG-assembly1.cif" "./input/protein_amyloids1/8EFU-assembly1.cif" \
  --reference-sequences-file "./input/protein_amyloids1/7LNA.fasta" \
  --save-sequence-alignments \
  --subselect-contacts "[-inter-chain -a1 [-chain A] -a2 [-chain B]]" \
  --output-dir ./output/protein_amyloids1/results_AB_t_7LNA \
  --local-output-formats pdb \
| column -t


cadscore-lt \
  -t "./input/protein_amyloids1/7LNA-assembly1.cif" \
  -m "./input/protein_amyloids1/7QIG-assembly1.cif" "./input/protein_amyloids1/8EFU-assembly1.cif" \
  --reference-sequences-file "./input/protein_amyloids1/7LNA.fasta" \
  --save-sequence-alignments \
  --subselect-contacts "[-a1 [-chain A] -a2 [-chain A]]" \
  --output-dir ./output/protein_amyloids1/results_AA_t_7LNA \
  --local-output-formats pdb \
| column -t
  
####################################################################

cadscore-lt \
  -t "./input/protein_amyloids1/7QIG-assembly1.cif" \
  -m "./input/protein_amyloids1/7LNA-assembly1.cif" "./input/protein_amyloids1/8EFU-assembly1.cif" \
  --reference-sequences-file "./input/protein_amyloids1/7QIG.fasta" \
  --save-sequence-alignments \
  --subselect-contacts "[-inter-chain -a1 [-chain A] -a2 [-chain B]]" \
  --output-dir ./output/protein_amyloids1/results_AB_t_7QIG \
  --local-output-formats pdb \
| column -t

cadscore-lt \
  -t "./input/protein_amyloids1/7QIG-assembly1.cif" \
  -m "./input/protein_amyloids1/7LNA-assembly1.cif" "./input/protein_amyloids1/8EFU-assembly1.cif" \
  --reference-sequences-file "./input/protein_amyloids1/7QIG.fasta" \
  --save-sequence-alignments \
  --subselect-contacts "[-a1 [-chain A] -a2 [-chain A]]" \
  --output-dir ./output/protein_amyloids1/results_AA_t_7QIG \
  --local-output-formats pdb \
| column -t

####################################################################

cadscore-lt \
  -t "./input/protein_amyloids1/8EFU-assembly1.cif" \
  -m "./input/protein_amyloids1/7LNA-assembly1.cif" "./input/protein_amyloids1/7QIG-assembly1.cif" \
  --reference-sequences-file "./input/protein_amyloids1/8EFU.fasta" \
  --save-sequence-alignments \
  --subselect-contacts "[-inter-chain -a1 [-chain A] -a2 [-chain B]]" \
  --output-dir ./output/protein_amyloids1/results_AB_t_8EFU \
  --local-output-formats pdb graphics-pymol \
| column -t

cadscore-lt \
  -t "./input/protein_amyloids1/8EFU-assembly1.cif" \
  -m "./input/protein_amyloids1/7LNA-assembly1.cif" "./input/protein_amyloids1/7QIG-assembly1.cif" \
  --reference-sequences-file "./input/protein_amyloids1/8EFU.fasta" \
  --save-sequence-alignments \
  --subselect-contacts "[-a1 [-chain A] -a2 [-chain A]]" \
  --output-dir ./output/protein_amyloids1/results_AA_t_8EFU \
  --local-output-formats pdb graphics-pymol \
| column -t

####################################################################

