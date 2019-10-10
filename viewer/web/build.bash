#!/bin/bash

cd "$(dirname "$0")"

if [ -z "$EMSCRIPTEN" ] || [ ! -d "$EMSCRIPTEN" ]
then
	echo "Error: emscripten building environment not initialized."
	exit 1
fi

rm -f ./*.mem ./*.data ./*.wasm ./*.js

emcc \
-s "EXPORTED_FUNCTIONS=['_main','_application_add_command','_application_upload_file']" \
-s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']" \
../src/voronota_viewer.cpp ../src/imgui/*.cpp \
-DFOR_WEB \
-I "/usr/include/glm/" \
-I "${EMSCRIPTEN}/system/include/emscripten/" \
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

python "${EMSCRIPTEN}/tools/file_packager.py" \
  ./startup_script.data \
  --preload ./startup_script.vvs@/ \
  --js-output=./startup_script.js

python "${EMSCRIPTEN}/tools/file_packager.py" \
  ./radii.data \
  --preload ../../resources/radii@/ \
  --js-output=./radii.js

python "${EMSCRIPTEN}/tools/file_packager.py" \
  ./voromqa_v1_energy_means_and_sds.data \
  --preload ../../resources/voromqa_v1_energy_means_and_sds@/ \
  --js-output=./voromqa_v1_energy_means_and_sds.js

python "${EMSCRIPTEN}/tools/file_packager.py" \
  ./voromqa_v1_energy_potential.data \
  --preload ../../resources/voromqa_v1_energy_potential@/ \
  --lz4 \
  --js-output=./voromqa_v1_energy_potential.js

python "${EMSCRIPTEN}/tools/file_packager.py" \
  ./2zsk.data \
  --preload ./2zsk.pdb@/ \
  --lz4 \
  --js-output=./2zsk.js

