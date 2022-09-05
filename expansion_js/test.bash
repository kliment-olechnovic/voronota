#!/bin/bash

cd $(dirname "$0")

date

time -p ./tests/run_all.bash

date
