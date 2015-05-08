#!/bin/bash

cd $(dirname "$0")

cppcheck --enable=all --force --verbose --quiet ../src/
