#!/bin/bash

WORKING_DIR=$1

find $WORKING_DIR -type f -name "*.summary.*" |  while read DFILE; do rm $DFILE ; done
find $WORKING_DIR -type f -name "*.test_subject.*" |  while read DFILE; do rm $DFILE ; done
find $WORKING_DIR -type f -name "*.comparison.*" |  while read DFILE; do rm $DFILE ; done
