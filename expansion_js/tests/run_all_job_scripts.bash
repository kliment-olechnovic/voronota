#!/bin/bash

cd $(dirname "$0")

export CORETESTSDIR="../../tests"

${CORETESTSDIR}/prepare_package_for_testing.bash

export VORONOTAJSDIR="${CORETESTSDIR}/voronota_package/expansion_js"
export INPUTDIR=./input
export OUTPUTDIR=./jobs_output

rm -r -f $OUTPUTDIR
mkdir $OUTPUTDIR

find ../ -maxdepth 1 -mindepth 1 -type f -name 'voronota-js-*' \
| while read -r VJSSCRIPTFILE
do
	VJSSCRIPTNAME="$(basename ${VJSSCRIPTFILE})"
	if [ ! -f "${VORONOTAJSDIR}/${VJSSCRIPTNAME}" ]
	then
		cp "${VJSSCRIPTFILE}" "${VORONOTAJSDIR}/"
	fi
done

find ./jobs_scripts/ -type f -name "*.bash" | xargs -L 1 -P 10 bash

