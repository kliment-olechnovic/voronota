#!/bin/bash

cd $(dirname "$0")

./stock_plugins.bash

cp ./voronota-scripting-* ~/bin/

cp ../voronota ~/bin/
cp ../voronota-resources ~/bin/
cp ../viewer/voronota-viewer ~/bin/
