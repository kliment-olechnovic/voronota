#!/bin/bash

cd $(dirname "$0")

####################################################################

mkdir -p ./output/protein_heterodimer1/input

cp ./input/protein_heterodimer1/T0921-T0922.pdb ./output/protein_heterodimer1/input/target.pdb
cp ./input/protein_heterodimer1/TS188_1 ./output/protein_heterodimer1/input/model1.pdb
cp ./input/protein_heterodimer1/TS208_1 ./output/protein_heterodimer1/input/model2.pdb

cd ./output/protein_heterodimer1

####################################################################

exec > ./full_log 2>&1
set -v

####################################################################

cadscore-lt -h

####################################################################

cadscore-lt -t "./input/target.pdb" -m "./input/model1.pdb" | column -t

####################################################################

cadscore-lt -t "./input/target.pdb" -m "./input/model1.pdb" "./input/model2.pdb" | column -t

####################################################################

find ./input/ -name 'model*.pdb' | cadscore-lt -t "./input/target.pdb" | column -t

####################################################################

cadscore-lt -m "./input/target.pdb" "./input/model1.pdb" "./input/model2.pdb" | column -t

####################################################################

find ./input/ -name '*.pdb' | cadscore-lt | column -t

####################################################################

find ./input/ -name '*.pdb' | cadscore-lt | column -t

####################################################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-with-f1 \
| column -t

##################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-all-details \
| column -t

####################################################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-all-details \
  --restrict-raw-input "[-chain A]" \
| column -t

##################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-all-details \
  --restrict-raw-input "[-chain B]" \
| column -t

####################################################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-all-details \
  --subselect-contacts "[-min-sep 1 -atom1 [-protein-sidechain] -atom2 [-protein-sidechain]]" \
| column -t

####################################################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-all-details \
  --subselect-contacts "[-a1 [-chain A] -a2 [-chain B]]" \
| column -t

##################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-all-details \
  --subselect-contacts "[-inter-chain]" \
| column -t

####################################################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-all-details \
  --subselect-contacts "[-a1 [-chain B -rnum 39:51] -a2 [-chain B -rnum 39:66,75:87]]" \
| column -t

##################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-all-details \
  --subselect-contacts "(([-a1 [-chain B -rnum 39:51] -a2 [-chain B -rnum 39:66]]) or ([-a1 [-chain B -rnum 39:51] -a2 [-chain B -rnum 75:87]]))" \
| column -t

####################################################################

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --output-dir ./results_basic \
  --local-output-formats table pdb \
| column -t

cadscore-lt \
  -t "./input/target.pdb" \
  -m "./input/model1.pdb" "./input/model2.pdb" \
  --subselect-contacts "[-inter-chain]" \
  --output-dir ./results_inter_chain \
  --local-output-formats table mmcif contactmap graphics-pymol \
| column -t

####################################################################

