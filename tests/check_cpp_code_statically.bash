#!/bin/bash

cd $(dirname "$0")

../package.bash voronota_package
mv ../voronota_package.tar.gz ./voronota_package.tar.gz
tar -xf ./voronota_package.tar.gz
rm ./voronota_package.tar.gz

trap "rm -r ./voronota_package" EXIT

cppcheck --enable=all --force --quiet ./voronota_package/src/
