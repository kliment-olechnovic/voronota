#!/bin/bash

# 
# If backspace key does not work for HTML input, then the JS file
#   ~/software/emsdk/upstream/emscripten/src/library_glfw.js
# should be modified.
# This block:
#   if (event.keyCode === 8 || event.keyCode === 9) { event.preventDefault(); }
# should be replaced with this block:
#   if (event.keyCode === 9) { event.preventDefault(); }
#

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

emcc --std=c++14 \
-s "EXPORTED_FUNCTIONS=['_main','_voronota_viewer_enqueue_script','_voronota_viewer_execute_native_script','_voronota_viewer_get_last_script_output','_voronota_viewer_upload_file']" \
-s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']" \
../src/voronota_viewer.cpp ../src/dependencies/imgui/*.cpp ../src/dependencies/duktape/duktape.cpp ../src/dependencies/tmalign/TMalign.cpp \
-DFOR_WEB \
-I "../src/dependencies/" \
-I "/usr/include/glm/" \
-I "${EMSDK}/upstream/emscripten/system/include/emscripten/" \
-s USE_GLFW=3 \
-s ALLOW_MEMORY_GROWTH=1 \
-s DISABLE_EXCEPTION_CATCHING=0 \
-s LZ4=1 \
-s WASM=1 \
-s FORCE_FILESYSTEM=1 \
-O3 \
-o voronota_viewer.js

if [ ! -s "voronota_viewer.js" ]
then
	echo "Error: build failed."
	exit 1
fi

./pack.bash
