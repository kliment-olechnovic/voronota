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

rm -f "./voronota_viewer_app/voronota_viewer.js" "./voronota_viewer_app/voronota_viewer.wasm"
mkdir -p "./voronota_viewer_app"

emcc --std=c++17 \
-s "EXPORTED_FUNCTIONS=['_main','_malloc','_free','_voronota_viewer_enqueue_script','_voronota_viewer_execute_native_script','_voronota_viewer_get_last_script_output','_voronota_viewer_upload_file','_voronota_viewer_upload_session','_voronota_viewer_setup_js_bindings_to_all_api_functions']" \
-s "EXPORTED_RUNTIME_METHODS=['ccall']" \
  ../src/voronota_gl.cpp \
  ../src/dependencies/imgui/*.cpp \
  ../../expansion_gl/src/dependencies/imgui/addons/*.cpp \
  ../../expansion_js/src/dependencies/lodepng/*.cpp \
  ../../expansion_js/src/dependencies/duktape/duktape.cpp \
  ../../expansion_js/src/dependencies/tmalign/TMalign.cpp \
  ../../expansion_js/src/dependencies/faspr/*.cpp \
  ../../expansion_js/src/dependencies/qcprot/*.cpp \
  ../../expansion_js/src/dependencies/tinf/*.cpp \
  ../../expansion_js/src/dependencies/pulchra_bb/*.cpp \
  ../../expansion_js/src/dependencies/utilities/*.cpp \
  ../../expansion_gl/src/dependencies/ImGuiColorTextEdit/*.cpp \
-DFOR_WEB \
-DUSE_LINENOISE=0 \
-I "../../expansion_js/src/dependencies/" \
-I "../src/dependencies/" \
-I "/usr/include/glm/" \
--use-port=contrib.glfw3 \
-s ALLOW_MEMORY_GROWTH=1 \
-s DISABLE_EXCEPTION_CATCHING=0 \
-s LZ4=1 \
-s WASM=1 \
-s FORCE_FILESYSTEM=1 \
-O3 \
-o ./voronota_viewer_app/voronota_viewer.js

if [ ! -s "./voronota_viewer_app/voronota_viewer.js" ] || [ ! -s "./voronota_viewer_app/voronota_viewer.wasm" ]
then
	echo "Error: build failed."
	exit 1
fi

