#!/bin/bash

PACKAGE_NAME=$1

cd $(dirname "$0")

VERSION_MAJOR_STRING=$(./Release/voronota --version 2>&1 | awk '{print $3}')
VERSION_MINOR_STRING=$(hg branches | egrep '^default' | tr ':' ' ' | awk '{print $2}')
VERSION_STRING="voronota_$VERSION_MAJOR_STRING.$VERSION_MINOR_STRING"

if [ -z "$PACKAGE_NAME" ]
then
  PACKAGE_NAME=$VERSION_STRING
fi

rm -f $PACKAGE_NAME.tar.gz
rm -r -f $PACKAGE_NAME

mkdir -p $PACKAGE_NAME
cp -r src $PACKAGE_NAME/src
cp README $PACKAGE_NAME/README
cp LICENSE $PACKAGE_NAME/LICENSE
cp CMakeLists.txt $PACKAGE_NAME/CMakeLists.txt
cp resources/radii $PACKAGE_NAME/radii
cp Release/voronota $PACKAGE_NAME/voronota

tar -czf "$PACKAGE_NAME.tar.gz" $PACKAGE_NAME
rm -r $PACKAGE_NAME
