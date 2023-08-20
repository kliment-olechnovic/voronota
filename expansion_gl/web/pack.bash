#!/bin/bash

cd "$(dirname "$0")"

if [ -z "$EMSDK" ]
then
	source ~/software/emsdk/emsdk_env.sh
fi

if [ -z "$EMSDK" ] || [ ! -d "$EMSDK" ]
then
	echo "Error: Emscripten building environment not initialized."
	exit 1
fi

INFILE="$1"

if [ ! -s "$INFILE" ]
then
	echo "Error: no input file '$INFILE'."
	exit 1
fi

BASENAME="$(basename "$INFILE" | cut -f 1 -d '.')"

rm -f "${BASENAME}.data" "${BASENAME}.js"
python3 "${EMSDK}/upstream/emscripten/tools/file_packager.py" \
  ./${BASENAME}.data \
  --preload ${INFILE}@/ \
  --lz4 \
  --js-output=./${BASENAME}.js

