#!/bin/bash

PACKAGE_NAME=$1

cd $(dirname "$0")

VERSIONID=$(../version.bash)

if [ -z "$PACKAGE_NAME" ]
then
  PACKAGE_NAME="voronota-js_$VERSIONID"
fi

rm -f $PACKAGE_NAME.tar.gz
rm -r -f $PACKAGE_NAME

mkdir -p "$PACKAGE_NAME/src/expansion_js"

cp -r "../src" "$PACKAGE_NAME/src/src"
cp -r "../expansion_js/src" "$PACKAGE_NAME/src/expansion_js/src"

rm -f "$PACKAGE_NAME/src/src/voronota.cpp"
rm -rf "$PACKAGE_NAME/src/src/modes"

cp "./README.markdown" "$PACKAGE_NAME/README.txt"
cp "./CMakeLists.txt" "$PACKAGE_NAME/CMakeLists.txt"
cp "./voronota-js" "$PACKAGE_NAME/voronota-js"
cp "./voronota-js-voromqa" "$PACKAGE_NAME/voronota-js-voromqa"
cp "../LICENSE.txt" "$PACKAGE_NAME/LICENSE.txt"

tar -czf "$PACKAGE_NAME.tar.gz" $PACKAGE_NAME
rm -r -f $PACKAGE_NAME

