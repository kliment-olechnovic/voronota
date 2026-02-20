#!/bin/bash

cd $(dirname "$0")

MODENAME="$1"

VERSIONID="$(cat ./latest_release_version.txt)"

################################################################################

if [ "$MODENAME" == "make-new-release" ]
then
	VERSIONID=$(./version.bash next)
	echo "$VERSIONID" > "./latest_release_version.txt"
fi

PACKAGE_NAME="cadscore-lt_v$(cat ./latest_release_version.txt)"

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
cp -r "../expansion_lt/src/voronotalt" "${PACKAGE_NAME}/src/voronotalt"
cp -r "../expansion_lt/src/voronotalt_cli" "${PACKAGE_NAME}/src/voronotalt_cli"

cat ./src/cadscorelt/version.h \
| sed "s|version_str=\".*\";|version_str=\"${VERSIONID}\";|" \
> "${PACKAGE_NAME}/src/cadscorelt/version.h"

cp \
  "./LICENSE.txt" \
  "./README.md" \
  "./index.html" \
  "${PACKAGE_NAME}/"

cat "./latest_release_version.txt" \
| sed 's/^/CAD-score-LT version /' \
> "${PACKAGE_NAME}/VERSION.txt"

cat "./CMakeLists.txt" \
| sed 's|/\.\./expansion_lt/|/|' \
| sed 's|/\.\./expansion_lt_cadscore/|/|' \
> "${PACKAGE_NAME}/CMakeLists.txt"

################################################################################

tar -czf "${PACKAGE_NAME}.tar.gz" "$PACKAGE_NAME"
rm -r -f "$PACKAGE_NAME"

mkdir -p "./packages_for_release"

mv "${PACKAGE_NAME}.tar.gz" "./packages_for_release/"

echo "${PACKAGE_NAME}"
exit 0
