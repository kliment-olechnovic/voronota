#!/bin/bash

cd $(dirname "$0")

cp ./voronota-scripting-env ~/bin/voronota-scripting-env
cp ./voronota-scripting-interactive-env ~/bin/voronota-scripting-interactive-env
cp ./v-tmalign ~/bin/v-tmalign
cp ./v-scwrl ~/bin/v-scwrl
cp ./v-fetch ~/bin/v-fetch
cp ./v-save-image ~/bin/v-save-image

cp ../voronota ~/bin/voronota
cp ../voronota-resources ~/bin/voronota-resources
cp ../viewer/voronota-viewer ~/bin/voronota-viewer
