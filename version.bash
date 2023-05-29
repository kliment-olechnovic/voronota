#!/bin/bash

cd $(dirname "$0")

VERSION_MAJOR=$(cat ./src/voronota_version.h | grep 'version_str="' | sed 's/.*"\(\S\+\)".*/\1/')
VERSION_MINOR=$(git rev-list --all --count)

echo "${VERSION_MAJOR}.${VERSION_MINOR}"
