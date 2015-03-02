#!/bin/bash

find /scratch/lustre/kliment/voromqa/output/complexes/contacts/ -type f -name hbplus_output \
| sed 's/hbplus_output//' \
> /scratch/lustre/kliment/voromqa/output/complexes/contacts_list

cat ./output/complexes/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_summary.bash -b /scratch/lustre/kliment/voromqa/bin -x 6 -w -d"
