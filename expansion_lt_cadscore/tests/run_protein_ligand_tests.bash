#!/bin/bash

cd $(dirname "$0")

mkdir -p ./output/protein_ligand1/

####################################################################

exec > ./output/protein_ligand1/full_log 2>&1
set -v

####################################################################

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_ligand1/1a30_target.cif" \
  -m ./input/protein_ligand1/*.cif \
  --scoring-levels atom residue \
  --scoring-types contacts sites \
  --subselect-contacts "[-a1 [-chain A,B] -a2 [-chain X]]" \
  --subselect-atoms "[-chain A,B]" \
| column -t

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_ligand1/1a30_target.cif" \
  -m ./input/protein_ligand1/*.cif \
  --scoring-levels atom residue \
  --scoring-types contacts sites \
  --subselect-contacts "[-a1 [-protein] -a2! [-protein]]" \
  --subselect-atoms "[-protein]" \
| column -t

####################################################################

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_ligand1/1a30_target.cif" \
  -m ./input/protein_ligand1/*.cif \
  --scoring-levels atom residue \
  --scoring-types contacts sites \
  --subselect-contacts "[-a1 [-chain A,B] -a2 [-chain X]]" \
  --subselect-atoms "[-chain A,B]" \
  --conflate-atom-types \
  --conflation-config-file "./input/protein_ligand1/1a30_target_groups_of_equivalent_atoms.tsv" \
| column -t

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_ligand1/1a30_target.cif" \
  -m ./input/protein_ligand1/*.cif \
  --scoring-levels atom residue \
  --scoring-types contacts sites \
  --subselect-contacts "[-a1 [-protein] -a2! [-protein]]" \
  --subselect-atoms "[-protein]" \
  --conflate-atom-types \
  --conflation-config-file "./input/protein_ligand1/1a30_target_groups_of_equivalent_atoms.tsv" \
| column -t

####################################################################

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_ligand1/1a30_target.cif" \
  -m ./input/protein_ligand1/*.cif \
  --scoring-levels atom \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-chain A,B] -a2 [-chain X]]" \
  --subselect-atoms "[-chain A,B]" \
  --conflate-atom-types \
  --conflation-config-file "./input/protein_ligand1/1a30_target_groups_of_equivalent_atoms.tsv" \
  --output-with-f1 \
  --output-with-identities \
| column -t

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_ligand1/1a30_target.cif" \
  -m ./input/protein_ligand1/*.cif \
  --scoring-levels atom \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-protein] -a2! [-protein]]" \
  --subselect-atoms "[-protein]" \
  --conflate-atom-types \
  --conflation-config-file "./input/protein_ligand1/1a30_target_groups_of_equivalent_atoms.tsv" \
  --output-with-f1 \
  --output-with-identities \
| column -t

####################################################################

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_ligand1/1a30_target.cif" \
  -m ./input/protein_ligand1/*.cif \
  --scoring-levels atom \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-chain A,B] -a2 [-chain X]]" \
  --subselect-atoms "[-chain A,B]" \
  --conflate-atom-types \
  --conflation-config-file "./input/protein_ligand1/1a30_target_groups_of_equivalent_atoms.tsv" \
  --output-with-areas \
| column -t

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_ligand1/1a30_target.cif" \
  -m ./input/protein_ligand1/*.cif \
  --scoring-levels atom \
  --scoring-types contacts \
  --subselect-contacts "[-a1 [-protein] -a2! [-protein]]" \
  --subselect-atoms "[-protein]" \
  --conflate-atom-types \
  --conflation-config-file "./input/protein_ligand1/1a30_target_groups_of_equivalent_atoms.tsv" \
  --output-with-areas \
| column -t

####################################################################

cadscore-lt \
  --include-heteroatoms \
  -t "./input/protein_ligand1/1a30_target.cif" \
  -m ./input/protein_ligand1/*.cif \
  --scoring-levels atom \
  --subselect-contacts "[-a1 [-chain A,B] -a2 [-chain X]]" \
  --subselect-atoms "[-chain A,B]" \
  --conflate-atom-types \
  --conflation-config-file "./input/protein_ligand1/1a30_target_groups_of_equivalent_atoms.tsv" \
  --output-with-f1 \
  --output-with-areas \
  --output-with-identities \
  --local-output-formats pdb contactmap graphics-pymol \
  --local-output-levels atom \
  --output-dir ./output/protein_ligand1/output_dit_01 \
| column -t

####################################################################


