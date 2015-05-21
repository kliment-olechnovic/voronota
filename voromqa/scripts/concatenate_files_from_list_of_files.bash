#!/bin/bash

INPUT_FILE_LIST=$1
OUTPUT_FILE=$2

(cat $INPUT_FILE_LIST | xargs -L 100 -P 1 cat) > $OUTPUT_FILE
