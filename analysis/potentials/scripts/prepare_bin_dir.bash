#!/bin/bash

BIN_DIR=$1

mkdir -p $BIN_DIR
cp ~/workspace/voronota/Release/voronota $BIN_DIR/voronota
cp ~/workspace/voronota/resources/radii $BIN_DIR/radii
cp ~/workspace/voronota/analysis/potentials/scripts/prepare_enhanced_raw_contacts.bash $BIN_DIR/prepare_enhanced_raw_contacts.bash
cp ~/workspace/voronota/analysis/potentials/scripts/prepare_contacts_summaries.bash $BIN_DIR/prepare_contacts_summaries.bash
