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

rm -f "./voronota_lt_web.js" "./voronota_lt_web.wasm"

emcc \
  ./voronota_lt_web.cpp \
-s MODULARIZE \
-s EXPORT_NAME="createVoronotaLTWebModule" \
-s ALLOW_MEMORY_GROWTH=1 \
-s DISABLE_EXCEPTION_CATCHING=1 \
-s LZ4=0 \
-s WASM=1 \
-s FORCE_FILESYSTEM=0 \
-lembind \
-O3 \
-o voronota_lt_web.js

if [ ! -s "./voronota_lt_web.js" ] || [ ! -s "./voronota_lt_web.wasm" ]
then
	echo "Error: build failed."
	exit 1
fi

VERSION="$(cat ../latest_release_version.txt | tr -d '\n')"

cat "./index.html" \
| sed "s|(version\s\S\+)|(version ${VERSION})|" \
| sponge "./index.html"
