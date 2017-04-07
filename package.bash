#!/bin/bash

PACKAGE_NAME=$1

cd $(dirname "$0")

VERSIONID=$(./version.bash)

if [ -z "$PACKAGE_NAME" ]
then
  PACKAGE_NAME="voronota_$VERSIONID"
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
cp voronota-volumes $PACKAGE_NAME/voronota-volumes
cp voronota $PACKAGE_NAME/voronota

./stock.bash
cp voronota-resources $PACKAGE_NAME/voronota-resources
./voronota-resources radii > $PACKAGE_NAME/radii

./document.bash
cp ./README.markdown $PACKAGE_NAME/README.txt
cp ./index.html $PACKAGE_NAME/README.html

cp -r support $PACKAGE_NAME/support

tar -czf "$PACKAGE_NAME.tar.gz" $PACKAGE_NAME
rm -r $PACKAGE_NAME
