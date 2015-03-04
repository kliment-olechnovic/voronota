#!/bin/bash

cat ./input/complexes_url_list \
| xargs -L 50 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/calc_contacts.bash -b /scratch/lustre/kliment/voromqa/bin -u -z -m -c -n -p -o /scratch/lustre/kliment/voromqa/output/complexes/contacts -i"
