#!/bin/bash

cd $(dirname "$0")

MODENAME="$1"

FILE_NAME="cosmopolitan_voronota-lt_v$(cat ../latest_release_version.txt | tr -d '\n').exe"

if [ "$MODENAME" == "print-name-and-exit" ]
then
	echo "${FILE_NAME}"
	exit 0
fi

./build.bash

mkdir -p "../packages_for_release"

FILE_PATH="../packages_for_release/${FILE_NAME}"

cp "./voronota-lt" "$FILE_PATH"

echo "$FILE_NAME"

