#!/bin/bash

cat ./output/complexes/contacts_list \
| xargs -L 500 -P 1 \
sbatch /scratch/lustre/kliment/voromqa/bin/run_args_loop.bash \
"/scratch/lustre/kliment/voromqa/bin/count_residues_and_atoms.bash -d"
