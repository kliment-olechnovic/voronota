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
cp LICENSE.txt $PACKAGE_NAME/LICENSE.txt
cp CMakeLists.txt $PACKAGE_NAME/CMakeLists.txt
cp resources/radii $PACKAGE_NAME/radii
cp resources/energy_potential $PACKAGE_NAME/energy_potential
cp resources/energy_means_and_sds $PACKAGE_NAME/energy_means_and_sds
cp scripts/cadscore $PACKAGE_NAME/cadscore
cp scripts/voromqa $PACKAGE_NAME/voromqa
cp Release/voronota $PACKAGE_NAME/voronota

./document.bash
mv ./README.html $PACKAGE_NAME/README.html

tar -czf "$PACKAGE_NAME.tar.gz" $PACKAGE_NAME
rm -r $PACKAGE_NAME
