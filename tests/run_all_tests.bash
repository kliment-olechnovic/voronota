#!/bin/bash

cd $(dirname "$0")

echo "External tools missing:             " $(./list_missing_external_tools.bash 2>&1 | wc -l)

echo "Static code analysis report lines:  " $(./check_cpp_code_statically.bash 2>&1 | wc -l)

echo "Tested options:                     " $(./list_untested_options_in_jobs_scripts.bash | egrep '^tested' | wc -l)

echo "Untested options:                   " $(./list_untested_options_in_jobs_scripts.bash | grep untested | wc -l)

./run_all_jobs_scripts.bash
echo "Altered jobs tests output files:    " $(hg status ./jobs_output/ 2>&1 | wc -l)
