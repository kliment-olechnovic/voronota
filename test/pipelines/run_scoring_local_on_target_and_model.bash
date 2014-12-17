#!/bin/bash

BIN_DIR=$1
TARGET_FILE=$2
MODEL_FILE=$3

TMP_DIR=$(mktemp -d)

cat $TARGET_FILE \
| $BIN_DIR/voronota get-balls-from-atoms-file --radii-file $BIN_DIR/radii --annotated \
| $BIN_DIR/voronota query-balls --drop-altloc-indicators --drop-atom-serials --seq-output $TMP_DIR/target_sequence \
> $TMP_DIR/target_raw_balls

cat $TMP_DIR/target_raw_balls \
| $BIN_DIR/voronota query-balls --set-ref-seq-num-adjunct $TMP_DIR/target_sequence \
| $BIN_DIR/voronota query-balls --renumber-from-adjunct refseq \
| tee $TMP_DIR/target_balls \
| $BIN_DIR/voronota calculate-contacts --annotated \
| tee $TMP_DIR/target_contacts \
| $BIN_DIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent \
> $TMP_DIR/target_contacts_reduced

cat $MODEL_FILE \
| $BIN_DIR/voronota get-balls-from-atoms-file --radii-file $BIN_DIR/radii --annotated \
| $BIN_DIR/voronota query-balls --drop-altloc-indicators --drop-atom-serials \
| $BIN_DIR/voronota query-balls --set-ref-seq-num-adjunct $TMP_DIR/target_sequence \
| $BIN_DIR/voronota query-balls --renumber-from-adjunct refseq \
| $BIN_DIR/voronota calculate-contacts --annotated \
| tee $TMP_DIR/model_contacts \
| $BIN_DIR/voronota query-contacts --match-min-seq-sep 1 --no-solvent \
> $TMP_DIR/model_contacts_reduced

cat $TMP_DIR/target_contacts \
| $BIN_DIR/voronota score-contacts --potential-file $BIN_DIR/potential --residue-atomic-scores-file $TMP_DIR/target_quality_scores \
> /dev/null

cat $TMP_DIR/model_contacts\
| $BIN_DIR/voronota score-contacts --potential-file $BIN_DIR/potential --residue-atomic-scores-file $TMP_DIR/model_quality_scores \
> /dev/null

cat $TMP_DIR/model_contacts_reduced \
| $BIN_DIR/voronota compare-contacts --residue-scores-file $TMP_DIR/model_cad_scores --target-contacts-file $TMP_DIR/target_contacts_reduced \
> /dev/null

cat $TMP_DIR/target_balls \
| $BIN_DIR/voronota query-balls --drop-adjuncts \
| $BIN_DIR/voronota query-balls --set-external-adjuncts $TMP_DIR/model_cad_scores --set-external-adjuncts-name a \
| $BIN_DIR/voronota query-balls --set-external-adjuncts $TMP_DIR/target_quality_scores --set-external-adjuncts-name b \
| $BIN_DIR/voronota query-balls --set-external-adjuncts $TMP_DIR/model_quality_scores --set-external-adjuncts-name c \
| awk '{print $1 " " $7}' \
| egrep 'a=.*b=.*c=.*' \
| tr ';' ' ' \
| sed 's/.=//g'
