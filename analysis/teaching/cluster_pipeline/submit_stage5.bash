#!/bin/bash

yes 6500 \
| head -1000 \
| xargs -L 5 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_potential.bash -b /scratch/lustre/kliment/voromqa/bin -i /scratch/lustre/kliment/voromqa/output/complexes/contacts_summaries_list -o /scratch/lustre/kliment/voromqa/output/complexes/potential/half -t -r -s"
