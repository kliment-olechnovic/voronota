#!/bin/bash

OUTPUT_FILE=$1
INPUT_FILE_LIST=$2

(cat $INPUT_FILE_LIST | xargs -L 100 -P 1 cat) > $OUTPUT_FILE
