#!/bin/bash

cd $(dirname "$0")

MODENAME="$1"

VERSIONID=$(./version.bash)

################################################################################

if [ "$MODENAME" == "make-new-release" ]
then
	VERSIONID=$(./version.bash next)
	echo "$VERSIONID" > "./latest_release_version.txt"
fi

PACKAGE_NAME="voronota-lt_v$(cat ./latest_release_version.txt)"

if [ "$MODENAME" == "print-name-and-exit" ]
then
	echo "${PACKAGE_NAME}"
	exit 0
fi

################################################################################

rm -f "${PACKAGE_NAME}.tar.gz"
rm -r -f "$PACKAGE_NAME"

################################################################################

./document.bash

mkdir -p "${PACKAGE_NAME}"

cp -r "./src" "${PACKAGE_NAME}/src"

cat ./src/voronotalt/version.h \
| sed "s|version_str=\".*\";|version_str=\"${VERSIONID}\";|" \
> "${PACKAGE_NAME}/src/voronotalt/version.h"

cp \
  "./LICENSE.txt" \
  "./README.md" \
  "./index.html" \
  "${PACKAGE_NAME}/"

cat "./latest_release_version.txt" \
| sed 's/^/Voronota-LT version /' \
> "${PACKAGE_NAME}/VERSION.txt"

cat "./CMakeLists.txt" \
| sed 's|/\.\./expansion_lt/|/|' \
> "${PACKAGE_NAME}/CMakeLists.txt"

################################################################################

tar -czf "${PACKAGE_NAME}.tar.gz" "$PACKAGE_NAME"
rm -r -f "$PACKAGE_NAME"

mkdir -p "./packages_for_release"

mv "${PACKAGE_NAME}.tar.gz" "./packages_for_release/"

echo "${PACKAGE_NAME}"
exit 0
