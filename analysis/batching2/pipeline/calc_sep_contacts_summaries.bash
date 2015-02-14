#!/bin/bash

readonly CONTACTSNAME=$1

cat ./output_derivatives/sizes | awk '{print $1}' \
| xargs -L 100 -P 1 sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash "/scratch/lustre/kliment/voromqa/bin/calc_sep_contacts_summaries.bash /scratch/lustre/kliment/voromqa/bin /scratch/lustre/kliment/voromqa/output $CONTACTSNAME"
