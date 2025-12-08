#!/bin/bash

cd $(dirname "$0")

./tests/run_all_tests.bash

./swig/tests/run_all_tests.bash

