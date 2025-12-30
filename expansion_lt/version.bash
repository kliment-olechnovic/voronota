#!/bin/bash

cd $(dirname "$0")

VERSION_MODE="$1"

VERSION_MAJOR=$(cat ./src/voronotalt/version.h | grep 'version_str="' | sed 's/.*"\(\S\+\)".*/\1/')

if [ "$VERSION_MODE" == "major" ]
then
	echo "$VERSION_MAJOR"
	exit 0
fi

VERSION_MINOR=$(git rev-list --all --count -- ./)

if [ "$VERSION_MODE" == "next" ]
then
	VERSION_MINOR="$(echo ${VERSION_MINOR} | awk '{print ($1+1)}')"
fi

echo "${VERSION_MAJOR}.${VERSION_MINOR}"
