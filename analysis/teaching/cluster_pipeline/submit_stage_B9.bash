#!/bin/bash

cat /scratch/lustre/kliment/voromqa/output/casp/contacts_list \
| xargs -L 200 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts_summary.bash -b /scratch/lustre/kliment/voromqa/bin -x 1 -d"
