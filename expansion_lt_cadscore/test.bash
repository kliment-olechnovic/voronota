#!/bin/bash

cd $(dirname "$0")

MODE="$1"

./tests/run_all_tests.bash "$MODE"

