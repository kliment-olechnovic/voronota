#!/bin/bash

PACKAGE_NAME=$1

cd $(dirname "$0")

VERSION_MAJOR_STRING=$(./voronota | head -1 | awk '{print $3}')
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
cp voronota-cadscore $PACKAGE_NAME/voronota-cadscore
cp voronota-voromqa $PACKAGE_NAME/voronota-voromqa
cp voronota-contacts $PACKAGE_NAME/voronota-contacts
cp voronota $PACKAGE_NAME/voronota

./stock.bash
cp voronota-resources $PACKAGE_NAME/voronota-resources
./voronota-resources radii > $PACKAGE_NAME/radii

./document.bash
cp ./index.html $PACKAGE_NAME/README.html

tar -czf "$PACKAGE_NAME.tar.gz" $PACKAGE_NAME
rm -r $PACKAGE_NAME
