#!/bin/bash

cd $(dirname "$0")

export PATH="/usr/lib64/openmpi/bin:${PATH}"
export LD_LIBRARY_PATH="/usr/lib64/openmpi/lib:${LD_LIBRARY_PATH}"

echo "Started core tests on                       $(date +'%D %T')"
./tests/list_missing_external_tools.bash 2>&1
./tests/run_all_jobs_scripts.bash
echo "Altered core tests output files:            $(git status -s ./tests/jobs_output/ 2>&1 | wc -l)"

echo "Started expansion_js tests on               $(date +'%D %T')"
./expansion_js/tests/run_all_job_scripts.bash
echo "Altered expansion_js tests output files:    $(git status -s ./expansion_js/tests/jobs_output/ 2>&1 | wc -l)"

echo "Finished tests on                           $(date +'%D %T')"

