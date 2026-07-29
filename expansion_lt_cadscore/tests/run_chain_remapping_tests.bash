#!/bin/bash

cd $(dirname "$0")

mkdir -p ./output/protein_chain_remapping/

####################################################################

exec > ./output/protein_chain_remapping/full_log 2>&1
set -v

####################################################################

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_homooligomer1/T2235TS* \
  --remap-chains false \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_homooligomer1/T2235TS* \
  --remap-chains true \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_homooligomer1/T2235TS* \
  --reference-sequences-file "./input/protein_homooligomer1/T2235.fasta" \
  --reference-stoichiometry 6 \
  --remap-chains false \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_homooligomer1/T2235TS* \
  --reference-sequences-file "./input/protein_homooligomer1/T2235.fasta" \
  --reference-stoichiometry 6 \
  --remap-chains true \
| column -t
  
####################################################################

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer1/H2227TS* \
  --remap-chains false \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer1/H2227TS* \
  --remap-chains true \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer1/H2227TS* \
  --reference-sequences-file "./input/protein_heterooligomer1/H2227.fasta" \
  --reference-stoichiometry 1 6 \
  --remap-chains false \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer1/H2227TS* \
  --reference-sequences-file "./input/protein_heterooligomer1/H2227.fasta" \
  --reference-stoichiometry 1 6 \
  --remap-chains true \
| column -t
  
####################################################################

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer2/H2217TS* \
  --remap-chains false \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer2/H2217TS* \
  --remap-chains true \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer2/H2217TS* \
  --reference-sequences-file "./input/protein_heterooligomer2/H2217.fasta" \
  --reference-stoichiometry 2 2 2 2 2 2 \
  --remap-chains false \
| column -t

#cadscore-lt \
#  --subselect-contacts '[-inter-chain]' \
#  -m ./input/protein_heterooligomer2/H2217TS* \
#  --reference-sequences-file "./input/protein_heterooligomer2/H2217.fasta" \
#  --reference-stoichiometry 2 2 2 2 2 2 \
#  --remap-chains true \
#| column -t
  
####################################################################

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer3/H2236TS* \
  --remap-chains false \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer3/H2236TS* \
  --remap-chains true \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer3/H2236TS* \
  --reference-sequences-file "./input/protein_heterooligomer3/H2236.fasta" \
  --reference-stoichiometry 3 6 \
  --remap-chains false \
| column -t

cadscore-lt \
  --subselect-contacts '[-inter-chain]' \
  -m ./input/protein_heterooligomer3/H2236TS* \
  --reference-sequences-file "./input/protein_heterooligomer3/H2236.fasta" \
  --reference-stoichiometry 3 6 \
  --remap-chains true \
| column -t
  
####################################################################

