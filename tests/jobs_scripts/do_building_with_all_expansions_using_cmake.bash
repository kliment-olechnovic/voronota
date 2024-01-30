#!/bin/bash

TMPLDIR=$(mktemp -d)
trap "rm -r $TMPLDIR" EXIT

SUBDIR=$OUTPUTDIR/building_with_all_expansions_using_cmake
mkdir -p $SUBDIR

cp -r $VORONOTADIR $TMPLDIR/package
cd $TMPLDIR/package
rm -f ./voronota
rm -f ./expansion_js/voronota-js
rm -f ./expansion_lt/voronota-lt
cmake . -DEXPANSION_JS=ON -DEXPANSION_LT=ON -DEXPANSION_GL=ON &> /dev/null
make &> /dev/null
cd - &> /dev/null

$TMPLDIR/package/voronota --help &> $SUBDIR/voronota_help
$TMPLDIR/package/expansion_js/voronota-js --version &> $SUBDIR/voronota_js_version_output
$TMPLDIR/package/expansion_lt/voronota-lt --help &> $SUBDIR/voronota_lt_help

