#!/bin/bash

cd $(dirname "$0")

rm -rf "./output"
mkdir -p "./output"

../voronota-lt < "./input/balls_2zsk.xyzr" > "./output/contacts_2zsk_summary.txt"

../voronota-lt < "./input/balls_3dlb.xyzr" > "./output/contacts_3dlb_summary.txt"

../voronota-lt < "./input/balls_7br8.xyzr" > "./output/contacts_7br8_summary.txt"

git status -s ./output/
