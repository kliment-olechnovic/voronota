#!/bin/bash

cd $(dirname "$0")

####################################################################

cadscore-lt --help &> "./output/help_message.txt"

####################################################################

find ./input/protein_homodimer1 -type f -name '*.pdb' | sort \
| cadscore-lt \
  --output-global-scores ./output/protein_homodimer1/global_scores_v01.txt

cadscore-lt \
  -m './input/protein_homodimer1' \
  --output-global-scores ./output/protein_homodimer1/global_scores_v02.txt

cadscore-lt \
  -m './input/protein_homodimer1' \
> ./output/protein_homodimer1/global_scores_v03.txt

diff ./output/protein_homodimer1/global_scores_v01.txt ./output/protein_homodimer1/global_scores_v02.txt \
> ./output/protein_homodimer1/global_scores_diff_v01_v02.txt

diff ./output/protein_homodimer1/global_scores_v01.txt ./output/protein_homodimer1/global_scores_v03.txt \
> ./output/protein_homodimer1/global_scores_diff_v01_v03.txt

####################################################################

cadscore-lt \
  -t './input/protein_homodimer1/target.pdb' \
  -m './input/protein_homodimer1' \
  --subselect-contacts '[-inter-chain]' \
  --output-all-details \
| column -t \
> ./output/protein_homodimer1/global_scores_v04.txt

cadscore-lt \
  -m './input/protein_homodimer1' \
  --scoring-levels atom residue chain \
  --subselect-contacts '[-inter-chain]' \
  --output-with-f1 \
| column -t \
> ./output/protein_homodimer1/global_scores_v05.txt

cadscore-lt \
  -m './input/protein_homodimer1' \
  --scoring-levels atom residue chain \
  --subselect-contacts '[-inter-chain]' \
  --output-with-f1 \
  --remap-chains \
| column -t \
> ./output/protein_homodimer1/global_scores_v06.txt

cadscore-lt \
  -m './input/protein_homodimer1' \
  --scoring-levels atom residue chain \
  --subselect-contacts '[-inter-chain]' \
  --output-with-f1 \
  --remap-chains \
  --conflate-atom-types \
  --output-dir ./output/protein_homodimer1/output_dir_v07 \
| column -t \
> ./output/protein_homodimer1/global_scores_v07.txt

cadscore-lt \
  -m './input/protein_homodimer1' \
  --scoring-levels atom residue chain \
  --subselect-contacts '[-inter-chain]' \
  --output-with-f1 \
  --remap-chains \
  --conflate-atom-types \
  --output-dir ./output/protein_homodimer1/output_dir_v08 \
  --compact-output \
| column -t \
> ./output/protein_homodimer1/global_scores_v08.txt

####################################################################

cadscore-lt \
  -t './input/protein_homodimer1/target.pdb' \
  -m './input/protein_homodimer1/model1.pdb' \
  --subselect-contacts '[-inter-chain]' \
  --output-all-details \
  --output-global-scores _none \
  --local-output-formats table pdb mmcif contactmap graphics-pymol graphics-chimera \
  --output-dir ./output/protein_homodimer1/output_dir_v09

####################################################################

cadscore-lt \
  -t './input/protein_homodimer1/target.pdb' \
  -m './input/protein_homodimer1/model1.pdb' \
  --subselect-contacts '[-inter-chain]' \
  --output-all-details \
  --output-global-scores _none \
  --scoring-levels atom \
  --local-output-formats table pdb mmcif contactmap graphics-pymol graphics-chimera \
  --local-output-levels atom \
  --output-dir ./output/protein_homodimer1/output_dir_v10

####################################################################

find ./input/rna_monomer1 -type f -name '*.pdb' | sort \
| cadscore-lt \
  --output-global-scores ./output/rna_monomer1/global_scores_v01.txt

####################################################################

cadscore-lt \
  -t './input/rna_monomer1/target.pdb' \
  -m './input/rna_monomer1/model2.pdb' \
  --output-all-details \
  --local-output-formats table pdb mmcif contactmap \
  --output-dir ./output/rna_monomer1/output_dir_v02 \
> ./output/rna_monomer1/global_scores_v02.txt

####################################################################

cadscore-lt \
  -t './input/rna_monomer1/target.pdb' \
  -m './input/rna_monomer1/model2.pdb' \
  --output-all-details \
  --output-global-scores ./output/rna_monomer1/global_scores_v03.txt \
  --scoring-levels atom \
  --local-output-levels atom \
  --local-output-formats table pdb mmcif contactmap \
  --output-dir ./output/rna_monomer1/output_dir_v03

####################################################################

