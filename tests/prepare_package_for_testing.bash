#!/bin/bash

cd $(dirname "$0")

rm -rf ./voronota_package

../package.bash voronota_package
mv ../voronota_package.tar.gz ./voronota_package.tar.gz
tar -xf ./voronota_package.tar.gz
rm ./voronota_package.tar.gz

