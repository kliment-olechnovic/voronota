#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

cat "./input/balls_2zsk.xyzr" | ../voronota-lt > "./output/contacts_2zsk_summary.txt"

cat "./input/balls_3dlb.xyzr" | ../voronota-lt > "./output/contacts_3dlb_summary.txt"

