#!/bin/bash

PACKAGE_NAME=$1

cd $(dirname "$0")

VERSIONID=$(./version.bash)

################################################################################

if [ -z "$PACKAGE_NAME" ]
then
  PACKAGE_NAME="voronota_$VERSIONID"
fi

rm -f "${PACKAGE_NAME}.tar.gz"
rm -r -f "$PACKAGE_NAME"

################################################################################

./stock.bash

./document.bash

mkdir -p "$PACKAGE_NAME"

cp -r src "${PACKAGE_NAME}/src"
cp -r support "${PACKAGE_NAME}/support"

cp \
  "voronota" \
  "voronota-resources" \
  "voronota-cadscore" \
  "voronota-voromqa" \
  "voronota-contacts" \
  "voronota-volumes" \
  "voronota-pocket" \
  "voronota-membrane" \
  "CMakeLists.txt" \
  "LICENSE.txt" \
  "README.md" \
  "index.html" \
  "voronota.man" \
  "${PACKAGE_NAME}/"

./voronota-resources radii > "${PACKAGE_NAME}/radii"

################################################################################

./expansion_js/document.bash

mkdir -p "${PACKAGE_NAME}/expansion_js"

cp -r "./expansion_js/src" "${PACKAGE_NAME}/expansion_js/src"

cp -r "./expansion_js/voroif" "${PACKAGE_NAME}/expansion_js/voroif"

cp \
  "./expansion_js/voronota-js" \
  "./expansion_js/voronota-js-voromqa" \
  "./expansion_js/voronota-js-membrane-voromqa" \
  "./expansion_js/voronota-js-ifeatures-voromqa" \
  "./expansion_js/voronota-js-fast-iface-voromqa" \
  "./expansion_js/voronota-js-fast-iface-cadscore" \
  "./expansion_js/voronota-js-fast-iface-cadscore-matrix" \
  "./expansion_js/voronota-js-fast-iface-data-graph" \
  "./expansion_js/voronota-js-voroif-gnn" \
  "./expansion_js/voronota-js-ligand-cadscore" \
  "./expansion_js/CMakeLists.txt" \
  "./LICENSE.txt" \
  "./expansion_js/README.md" \
  "./expansion_js/index.html" \
  "${PACKAGE_NAME}/expansion_js/"

################################################################################

./expansion_gl/document.bash

mkdir -p "${PACKAGE_NAME}/expansion_gl"

cp -r "./expansion_gl/src" "${PACKAGE_NAME}/expansion_gl/src"

cp \
  "./expansion_gl/CMakeLists.txt" \
  "./LICENSE.txt" \
  "./expansion_gl/README.md" \
  "./expansion_gl/index.html" \
  "${PACKAGE_NAME}/expansion_gl/"

################################################################################

tar -czf "${PACKAGE_NAME}.tar.gz" "$PACKAGE_NAME"
rm -r -f "$PACKAGE_NAME"

echo "${PACKAGE_NAME}.tar.gz"

