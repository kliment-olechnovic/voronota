#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

cat "./input/balls.xyzr" | ../voronota-lt > "./output/contacts_summary.txt"

