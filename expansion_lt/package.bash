#!/bin/bash

cd $(dirname "$0")

FLAG="$1"

################################################################################

PACKAGE_NAME="voronota-lt_$(cat ./VERSION.txt | sed 's/^/ /' | sed 's/^.*\s\(\S\+\)$/\1/')_from_voronota_$(cat ../latest_release_version.txt)"

if [ "$FLAG" == "print-name-and-exit" ]
then
	echo "${PACKAGE_NAME}"
	exit 0
fi

################################################################################

rm -f "${PACKAGE_NAME}.tar.gz"
rm -r -f "$PACKAGE_NAME"

################################################################################

mkdir -p "${PACKAGE_NAME}"

cp -r "./src" "${PACKAGE_NAME}/src"

cp \
  "./LICENSE.txt" \
  "./README.md" \
  "./VERSION.txt" \
  "./index.html" \
  "${PACKAGE_NAME}/"

cat "./CMakeLists.txt" \
| sed 's|/\.\./expansion_lt/|/|' \
> "${PACKAGE_NAME}/CMakeLists.txt"

################################################################################

tar -czf "${PACKAGE_NAME}.tar.gz" "$PACKAGE_NAME"
rm -r -f "$PACKAGE_NAME"

echo "${PACKAGE_NAME}"
exit 0
