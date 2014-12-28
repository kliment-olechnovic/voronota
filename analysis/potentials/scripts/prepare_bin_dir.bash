#!/bin/bash

BIN_DIR=$1

mkdir -p $BIN_DIR
cp ~/workspace/voronota/Release/voronota $BIN_DIR/voronota
cp ~/workspace/voronota/resources/radii $BIN_DIR/radii
cp ~/workspace/voronota/analysis/potentials/scripts/* $BIN_DIR
