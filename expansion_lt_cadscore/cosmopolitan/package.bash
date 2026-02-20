#!/bin/bash

cd $(dirname "$0")

MODENAME="$1"

FILE_NAME="cosmopolitan_cadscore-lt_v$(cat ../latest_release_version.txt | tr -d '\n').exe"

if [ "$MODENAME" == "print-name-and-exit" ]
then
	echo "${FILE_NAME}"
	exit 0
fi

SOURCE_PACKAGE_NAME="$(../package.bash print-name-and-exit)"

if [ ! -s "../packages_for_release/${SOURCE_PACKAGE_NAME}.tar.gz" ]
then
	SOURCE_PACKAGE_NAME="$(../package.bash)"
fi

cd "../packages_for_release/"
rm -rf "./${SOURCE_PACKAGE_NAME}"
tar -xf "./${SOURCE_PACKAGE_NAME}.tar.gz"
cd - &> /dev/null

./build.bash "../packages_for_release/${SOURCE_PACKAGE_NAME}/src"

cd "../packages_for_release/"
rm -rf "./${SOURCE_PACKAGE_NAME}"
cd - &> /dev/null

cp "./cadscore-lt" "../packages_for_release/${FILE_NAME}"

echo "$FILE_NAME"

