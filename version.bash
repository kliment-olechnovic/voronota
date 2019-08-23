#!/bin/bash

cd $(dirname "$0")

VERSION_MAJOR=$(cat ./src/voronota.cpp | grep '"Voronota version ' | sed 's/.*"Voronota version \(\S\+\)";/\1/')
VERSION_MINOR=$(git rev-list --all --count)

echo "${VERSION_MAJOR}.${VERSION_MINOR}"
