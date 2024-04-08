#!/bin/bash

cd $(dirname "$0")

if [ "$1" == "clean" ]
then
	rm -f ./voronota
	rm -f ./expansion_js/voronota-js
	rm -f ./expansion_lt/voronota-lt
fi

export PATH="/usr/lib64/openmpi/bin:${PATH}"
export LD_LIBRARY_PATH="/usr/lib64/openmpi/lib:${LD_LIBRARY_PATH}"

echo "Started core tests on                       $(date +'%D %T')"
./tests/list_missing_external_tools.bash 2>&1
./tests/run_all_jobs_scripts.bash
echo "Altered core tests output files:            $(git status -s ./tests/jobs_output/ 2>&1 | wc -l)"

echo "Started expansion_js tests on               $(date +'%D %T')"
./expansion_js/tests/run_all_job_scripts.bash
echo "Altered expansion_js tests output files:    $(git status -s ./expansion_js/tests/jobs_output/ 2>&1 | wc -l)"

echo "Started expansion_lt tests on               $(date +'%D %T')"
./expansion_lt/tests/run_all_tests.bash
echo "Altered expansion_lt tests output files:    $(git status -s ./expansion_lt/tests/output/ ./expansion_lt/swig/tests/output/ 2>&1 | wc -l)"

echo "Finished tests on                           $(date +'%D %T')"

