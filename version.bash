#!/bin/bash

cd $(dirname "$0")

VERSION_MAJOR=$(cat ./src/voronota.cpp | grep '"Voronota version ' | sed 's/.*"Voronota version \(\S\+\)";/\1/')
VERSION_MINOR=$(hg branches | egrep '^default' | tr ':' ' ' | awk '{print $2}')

echo "${VERSION_MAJOR}.${VERSION_MINOR}"
