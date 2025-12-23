#!/bin/bash

cd $(dirname "$0")

FLAG="$1"

if [[ "$FLAG" != "clean" && "$FLAG" != "noclean" ]] || [[ -n "$2" ]]
then
	echo "Please provide a single argument - either 'clean' or 'noclean'"
	exit 1
fi

if [ "$FLAG" == "clean" ] || [ ! -s "./voronota-lt" ]
then
	./build.bash
fi

FILE_NAME="cosmopolitan_$(../package.bash print-name-and-exit).exe"

FILE_PATH="../../packages_for_release/${FILE_NAME}"

cp "./voronota-lt" "$FILE_PATH"

