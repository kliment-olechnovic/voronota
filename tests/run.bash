#!/bin/bash

set +e

cd $(dirname "$0")

../package.bash voronota_package
mv ../voronota_package.tar.gz ./voronota_package.tar.gz
tar -xf ./voronota_package.tar.gz
rm ./voronota_package.tar.gz

trap "rm -r ./voronota_package" EXIT

export VORONOTA=./voronota_package/voronota
export VORONOTA_RADII_FILE=./voronota_package/radii
export INPUTDIR=./input
export OUTPUTDIR=./output

rm -r -f $OUTPUTDIR
mkdir $OUTPUTDIR

./scripts/do_help_printing.bash
./scripts/do_basic_triangulation_and_contacts.bash
./scripts/do_balls_querying.bash
./scripts/do_contacts_querying.bash
./scripts/do_interface_contacts_comparison_and_drawing.bash
