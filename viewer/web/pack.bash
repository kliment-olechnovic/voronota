#!/bin/bash

cd "$(dirname "$0")"

if [ -z "$EMSDK" ] || [ ! -d "$EMSDK" ]
then
	echo "Error: Emscripten building environment not initialized."
	exit 1
fi

function package_data_file
{
	local INFILE=$1
	local BASENAME="$(basename "$INFILE" | cut -f 1 -d '.')"
	if ! cmp -s "$INFILE" "${BASENAME}.data"
	then
		rm -f "${BASENAME}.data" "${BASENAME}.js"
		python "${EMSDK}/upstream/emscripten/tools/file_packager.py" \
		  ./${BASENAME}.data \
		  --preload ${INFILE}@/ \
		  --js-output=./${BASENAME}.js
	fi
}

package_data_file ./startup_script.vvs
package_data_file ../../resources/radii
package_data_file ../../resources/voromqa_v1_energy_means_and_sds
package_data_file ../../resources/voromqa_v1_energy_potential
package_data_file ./2zsk.pdb

