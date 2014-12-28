#!/bin/bash

BIN_DIR=$1
OUTPUT_DIR=$2
INPUT_FILE=$3

mkdir -p $OUTPUT_DIR

TMP_DIR=$(mktemp -d)

dssp $INPUT_FILE > $TMP_DIR/dssp_output 2> /dev/null
xhbplus $INPUT_FILE > $TMP_DIR/hbplus_output 2> /dev/null

cat $INPUT_FILE \
| $BIN_DIR/voronota get-balls-from-atoms-file --radii-file $BIN_DIR/radii --annotated \
| $BIN_DIR/voronota query-balls --drop-altloc-indicators --drop-atom-serials --set-dssp-tags $TMP_DIR/dssp_output \
| sed 's/A<OXT>/A<O>/g' \
| tee $TMP_DIR/balls \
| $BIN_DIR/voronota calculate-contacts --annotated \
| $BIN_DIR/voronota query-contacts --set-hbplus-tags $TMP_DIR/hbplus_output \
| tee $TMP_DIR/ia_contacts \
| $BIN_DIR/voronota query-contacts --inter-residue \
> $TMP_DIR/ir_contacts

cat $TMP_DIR/balls | $BIN_DIR/voronota query-balls --match-tags 'dssp=H|dssp=G|dssp=I' | awk '{print $1}' > $TMP_DIR/helix_atom_ids
cat $TMP_DIR/balls | $BIN_DIR/voronota query-balls --match-tags 'dssp=B|dssp=E' | awk '{print $1}' > $TMP_DIR/sheet_atom_ids
cat $TMP_DIR/ir_contacts | $BIN_DIR/voronota query-contacts --match-tags 'hb'  | awk '{print $1 " " $2}'> $TMP_DIR/hbond_contact_ids

cat $TMP_DIR/ia_contacts \
| $BIN_DIR/voronota query-contacts --match-external-first $TMP_DIR/helix_atom_ids --set-tags 'H' \
| $BIN_DIR/voronota query-contacts --match-external-first $TMP_DIR/sheet_atom_ids --set-tags 'S' \
| $BIN_DIR/voronota query-contacts --match-external-first $TMP_DIR/helix_atom_ids --match-external-second $TMP_DIR/helix_atom_ids --set-tags 'HH' \
| $BIN_DIR/voronota query-contacts --match-external-first $TMP_DIR/sheet_atom_ids --match-external-second $TMP_DIR/sheet_atom_ids --set-tags 'SS' \
| $BIN_DIR/voronota query-contacts --match-external-pairs $TMP_DIR/hbond_contact_ids --set-tags 'rhb' \
> $OUTPUT_DIR/$(basename $INPUT_FILE)

rm -r $TMP_DIR
