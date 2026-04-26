#!/bin/bash

cd $(dirname "$0")

####################################################################

mkdir -p ./output/protein_homotrimer1/input

cp ./input/protein_homotrimer1/T0860o.pdb ./output/protein_homotrimer1/input/T0860o.pdb
cp ./input/protein_homotrimer1/T0860TS203_1o ./output/protein_homotrimer1/input/T0860TS203_1o

cd ./output/protein_homotrimer1

####################################################################

exec > ./full_log 2>&1
set -v

####################################################################

cadscore-lt \
  -t "./input/T0860o.pdb" \
  -m "./input/T0860TS203_1o" \
  --subselect-contacts "[-inter-chain]" \
  --output-dir ./results_basic \
  --local-output-formats pdb contactmap graphics-pymol \
| column -t

####################################################################

cadscore-lt \
  -t "./input/T0860o.pdb" \
  -m "./input/T0860TS203_1o" \
  --subselect-contacts "[-inter-chain]" \
  --remap-chains \
  --output-dir ./results_after_remapping \
  --local-output-formats pdb contactmap graphics-pymol \
| column -t

####################################################################

cadscore-lt \
  -t "./input/T0860o.pdb" \
  -m "./input/T0860TS203_1o" \
  --split-input-into-dimers \
  --subselect-contacts "[-inter-chain]" \
  --remap-chains \
  --output-dir ./results_for_subdimers \
| column -t

####################################################################

cadscore-lt \
  -t "./input/T0860o.pdb" \
  -m "./input/T0860TS203_1o" \
  --fake-split-input-into-dimers \
  --subselect-contacts "[-inter-chain]" \
  --remap-chains \
  --output-dir ./results_for_fake_subdimers \
| column -t

####################################################################

cadscore-lt \
  -m "./input/T0860TS203_1o" \
  --split-input-into-dimers \
  --subselect-contacts "[-inter-chain]" \
  --remap-chains \
  --clustering-thresholds 0.8 \
  --output-dir ./results_for_subdimers_self_clustering \
| column -t

####################################################################

