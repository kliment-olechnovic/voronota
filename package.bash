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
cp LICENSE.txt CMakeLists.txt $PACKAGE_NAME
cp voronota-cadscore voronota-voromqa voronota-contacts voronota-volumes voronota-pocket voronota-membrane $PACKAGE_NAME

./stock.bash
cp voronota-resources $PACKAGE_NAME/voronota-resources
./voronota-resources radii > $PACKAGE_NAME/radii

./document.bash
cp ./README.markdown $PACKAGE_NAME/README.txt
cp ./index.html $PACKAGE_NAME/README.html

cp -r support $PACKAGE_NAME/support

AUTODIST_NAME="voronota-$VERSIONID"
rm -r -f $AUTODIST_NAME
./autodist.bash &> /dev/null
tar -xf $AUTODIST_NAME.tar.gz
rm -f $AUTODIST_NAME.tar.gz
if [ -d "$AUTODIST_NAME" ]
then
	cp $(find $AUTODIST_NAME -maxdepth 1 -type f -not -name 'voronota') $PACKAGE_NAME
fi
rm -r -f $AUTODIST_NAME

cp voronota $PACKAGE_NAME/voronota

tar -czf "$PACKAGE_NAME.tar.gz" $PACKAGE_NAME
rm -r -f $PACKAGE_NAME
